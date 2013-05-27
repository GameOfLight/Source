 /*
  HT1632 Screencontroller
  Copyright (c) 2013 Stian Selbek.  All right reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301	 USA
  */

/*
 * HT1632_Screencontroller.c
 *  Target: ATMega328p
 *  Author: Stian Selbek
 * 
 * A serially (SPI) controlled controller for 8 DE-DP14211 32x16 LED matrix modules from Sure Electronics.
 * The modules are mounted in a 2x4 configuration for a total of 64x64 RG pixels.
 * The display is divided into 8 lines each 8px tall. Each line is represented as 64 green followed by 64 red pixels.
 * By adding a line transfer prefix byte to the transfer a complete line can be sent as one large packet to
 * this controller.
 *
 * Physically the modules have their CS, data and clock lines connected directly to this microcontroller. The CS_CLK signal
 * however is connected to the modules through a multiplexer allowing us to control which 'row' of modules we're currently
 * talking to.
 *
 * The shiftregister controlling the CS signal within each module is shifted to chips in this order:
 *  line0 - 0 1 4 5
 *  line1 - 2 3 6 7
 *
 * For transmission protocol to the actual chips see the HT1632C datasheet.
*/

//TODO:
//Re-add support for single data. (temporarily removed during line-mode rewrite)
//
//Once a linecommand is received, use memcpy to copy any data stored so far. No need to check every step. Will help
//with catchup after a long command.
//
//Only reset the transmission complete flag on the last transfer to a chip when sending lines.
//
//Do fewer select_clks if the previous command was also a linecommand?
//  Add a linemode-variable? Do 0 shifts if the last chip was 7, 2 shifts if 5.
//  Shift in the leading 1 before the loop.
//   shift once per iteration of doubleLine. Saves having to call select_chip (which defaults to 8 bits in the previous case)
//
//Extend buffer? Will become slower, but can contain more data. Which is better?
//
//Won't fix:
//Optimize select_none so it only empties the register rather than flush it with 8 bits no matter what - Turned out slower due to branching vs loop unrolling, hence not implemented


#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000
#include <util/delay.h>

#define LED_PIN PB1 //Mostly unused at this point
#define SCREEN_DATA_PIN PD1  //TX pin
#define SCREEN_CLK_PIN PD4   //XCLK pin
#define SELECT_DATA_PIN PD6
#define SELECT_CLK_PIN PD7

#define MUX_SEL0_PIN PC0  //Selects the output of the multiplexer. This controls which row of modules have their CS register clocked.
#define MUX_SEL1_PIN PC1

#define SCREEN_WRITE 0x05 //101b
#define SCREEN_CMD 0x04   //100b

#define MASK_TYPE 0x04
#define MASK_ADDR 0x40
#define MASK_DATA 0x80

#define CMD_SYS_DIS 0x00
#define CMD_SYS_EN 0x01
#define CMD_LED_OFF 0x02
#define CMD_LED_ON 0x03
#define CMD_BLINK_OFF 0x08
#define CMD_BLINK_ON 0x09
#define CMD_MASTER_MODE 0x18
#define CMD_COM8N 0x20

uint8_t curr_index = 0;
uint8_t curr_line = 0;
uint8_t last_chip = 0xff;
uint8_t curr_MUX = 0;

uint8_t lineBuff[256]; //1 to 2 lines are copied here before sending to the screen
uint8_t *linePtrs[16]; //Pointers into the lineBuff translating it to the individual chips. 0 & 1 are green and red for chip 0 etc.
uint8_t chip_lookup[8] = {0, 1, 4, 5, 0 , 1, 4, 5};
uint8_t chip_addr = 0;

//Data/Cmd queue:
volatile uint8_t buff[256];
uint8_t head = 0; //Modified by main program only
volatile uint8_t tail = 0; //Modified by interrupt only
uint8_t error = 0;  //Becomes 1 if the buffer overflows

void pin_init() {
	DDRD |= (1<<SCREEN_DATA_PIN) | (1<<SCREEN_CLK_PIN);
	DDRD |= (1<<SELECT_CLK_PIN) | (1<<SELECT_DATA_PIN);
	DDRC |= (1<<MUX_SEL0_PIN) | (1<<MUX_SEL1_PIN);
	DDRB |= _BV(LED_PIN);
}

void spi_init() {
	//Enable SPI in slave mode
	SPCR |= _BV(SPE);
	//Enable interrupt on receive complete
	SPCR |= _BV(SPIE);
	sei();
}


void MSPIM_init() {
//Use the built in USART interface in SPI-mode to send data to the displays at a quick pace.
//Whenever we cross a chip boundary the interface needs to be turned off momentarily to allow
//The datamode and address to be written. Use MSPIM_enable/disable for this purpose.
	UBRR0 = 0;
	// Set MSPI mode of operation and SPI data mode 3. CLK idles at 1, samples on rising. LSB first
	UCSR0C |= (1<<UMSEL01) | (1<<UMSEL00) | (1<<UCPHA0) | (1<<UCPOL0) | _BV(UDORD0);
	// Enable transmitter only.
	UCSR0B |= (1<<TXEN0); //disable to control port manually. Note: won't disable until transfers complete.
	
	UBRR0 = 1; // 7 = 4 Mbps
}

void MSPIM_disable() {
	//The USART device needs to be idle before calling this function.
	//Disable MSPIM; allowing manual bitbashing of data and clk pins
	UCSR0B &= ~_BV(TXEN0); //To allow adjusting the datapin
	UCSR0C &= ~(_BV(UMSEL00) | _BV(UMSEL01)); //to free up the clk pin
}

void MSPIM_enable() {
	UCSR0C = (1<<UMSEL01) | (1<<UMSEL00) | (1<<UCPHA0) | (1<<UCPOL0) | _BV(UDORD0);
	UCSR0B = _BV(TXEN0);
	UBRR0 = 1;
}

// Routine called each time a packet arrives
ISR(SPI_STC_vect) {
	//Safety check removed to allow for faster interrupts. Will cause drop of two lines if 
	//host sends data at too high a pace.
	/*if (tail+1 == head) {
		error = 1;
		return; //drop packet due to full buffer!
	}*/		
	buff[tail++] = SPDR; 	//Add the incoming data to the queue
	//tail &= 0x03FF; //keep it within the interval 0-1023
}


//Bitbashing serial send routine. Set bm=0x80 for sending 8 bits etc...
void serial_send(const uint8_t data, uint8_t bm) {
	while(bm) {
		//Clear data, set clk low
		PORTD &= ~((1<<SCREEN_CLK_PIN) | (1<<SCREEN_DATA_PIN));
		if (data & bm) {
			PORTD |= (1<<SCREEN_DATA_PIN);
		}
		PORTD |= (1<<SCREEN_CLK_PIN);
		bm >>= 1;
	}
}


//Like serial_send(), just LSB first
void serial_sendLSB(const uint8_t data, uint8_t bm) {
	while(bm) {
		//Clear data, set clk low
		PORTD &= ~((1<<SCREEN_CLK_PIN) | (1<<SCREEN_DATA_PIN));
		if (data & bm) {
			PORTD |= (1<<SCREEN_DATA_PIN);
		}
		PORTD |= (1<<SCREEN_CLK_PIN);
		bm <<= 1;
	}	
}


void serial_sendLSBfast(uint8_t data) {
	//Wait for the transmit buffer to be able to accept more data
	while (!(UCSR0A & (1<<UDRE0)));
	UCSR0A |= _BV(TXC0); //clear transmit complete
	UDR0 = data;
}


void select_none() {
	//Disable the CS signal on all chips in the currently selected row
	PORTD |= (1<<SELECT_DATA_PIN); //CSn is active low. Set entire shiftregister high:
	for (uint8_t i = 0; i < 8; i++) {
		PORTD &= ~((1<<SELECT_CLK_PIN));
		PORTD |= (1<<SELECT_CLK_PIN);
	}
}


void select_chip(const uint8_t chip) {
	//Selects the given chip by pulling it's CS signal low
	if (last_chip == 0xff || last_chip >= chip) {
		select_none(); //OPTIMIZE THIS?
		//Clock in the select bit:
		PORTD &= ~(_BV(SELECT_DATA_PIN) | _BV(SELECT_CLK_PIN));
		PORTD |= _BV(SELECT_CLK_PIN);
		last_chip = 0;
	}
	//Move select bit to desired chip:
	PORTD |= (1<<SELECT_DATA_PIN);
	while (last_chip < chip) {
		PORTD &= ~_BV(SELECT_CLK_PIN);
		PORTD |= _BV(SELECT_CLK_PIN);
		last_chip++;
	}
}


void select_all() {
	//Enable the CS signal on all chips in the currently selected row
	last_chip = 0xff;
	PORTD &= ~(1<<SELECT_DATA_PIN); //CSn is active low. Set entire shiftregister low:
	for (uint8_t i = 0; i < 8; i++) {
		PORTD &= ~((1<<SELECT_CLK_PIN));
		PORTD |= (1<<SELECT_CLK_PIN);
	}
}


void chip_prepare() {
	//NOTE: ALL transmissions using the USART SPI must be complete prior to calling this function
	//Reason for requirement: checking transfer complete here makes an endless loop if a transfer has yet to take place.
	MSPIM_disable(); //disable MSPIM, allow manual bitbashing
	serial_send(SCREEN_WRITE, MASK_TYPE);
	serial_send(chip_addr, MASK_ADDR); //start memory address
	//Chip prepared, time to turn the hardware SPI interface back on
	MSPIM_enable();
}



void select_mux(uint8_t row) {
	//All transmissions should be stopped before calling
	row = row & 0x03;
	PORTC = (PORTC & 0xFC) | row;
	curr_MUX = row;
}

void chip_goto() {
	//Find the right chip to select. 
	//Each chip has addresses [0, 15] for green and [16, 31] for red pixels.
	//Hence index [0, 15] and [64, 79] end up at chip 0 and so on
	//Chips are arranged as follows:
	//line0 - 0 1 4 5
	//line1 - 2 3 6 7
	// ....
	//line6 - 0 1 4 5
	//line7 - 2 3 6 7
	uint8_t chip = chip_lookup[curr_index >> 4]; //Dividing by 16 is fast (swaps nibbles after O3)
	if (curr_line & 0x01) chip +=2;
	
	//Select chip and goto the current address:
	while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
	select_chip(chip);
	chip_prepare();
}


//StartLine must be either 0, 2, 4 or 6. The data for both lines must be in lineBuff[].
void screen_sendDoubleLine(uint8_t startLine) {
	uint8_t chip, i;
	uint8_t* data;
	
	chip_addr = 0;

	while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
	select_none(); //Release current selection lest it remain active while we talk to another line
	select_mux(startLine>>1);

	for (chip = 0; chip < 8; chip++) {
		data = linePtrs[chip*2];
		while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
		select_chip(chip);
		chip_prepare();
		//Send green
		for (i = 0; i < 16; i++) {
			serial_sendLSBfast(data[i]);
		}
		data = linePtrs[chip*2 + 1];
		//Send red
		for (i = 0; i < 16; i++) {
			serial_sendLSBfast(data[i]);
		}
	}
}


//Sends the line of data currently in lineBuff 0-127 to the screen
void screen_sendLine(uint8_t line) {
	uint8_t i, j, chip;
	uint8_t *data;
	
	chip_addr = 0;
	while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
	select_none(); //Release current selection lest it remain active while we talk to another line
	select_mux(line>>1);
	
	for (i = 0; i < 4; i++) {
		chip = chip_lookup[i];
		data = linePtrs[chip*2];
		while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
		if (line & 1) {
			select_chip(chip + 2);
		} else {
			select_chip(chip);
		}
		chip_prepare();
		for (j = 0; j < 16; j++) {
			serial_sendLSBfast(data[j]);
		}
		data = linePtrs[chip*2 + 1];
		//Send red
		for (j = 0; j < 16; j++) {
			serial_sendLSBfast(data[j]);
		}
	}
}


void screen_clear() {
	uint8_t i, row;
	chip_addr = 0;

	for (row = 0; row < 4; row++) {
		//Pull all the chips' CS lines low so we may talk to all of them simultaneously
		select_mux(row);
		//Clock in one 1 to be certain all chips are reset when select_all is run:
		PORTD |= (1<<SELECT_DATA_PIN);
		PORTD &= ~((1<<SELECT_CLK_PIN));
		PORTD |= (1<<SELECT_CLK_PIN);
		select_all();
	}
		
	chip_prepare();
	for (i = 0; i < 32; i++) {
		serial_sendLSBfast(0x00);//, MASK_DATA);
	}
	
	while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
	for (row = 0; row < 4; row++) {
		//Deselect all chips
		select_mux(row);
		select_none();
	}

	curr_line = 0;
	curr_index = 0;
	chip_goto();
}


void screen_init() {
	//Sends the initializationcommands necessary to start talking to the HT1632C chips.
	uint8_t row;
	pin_init();

	//Activate the CS of all the chips on the board
	for (row = 0; row < 4; row++) {
		select_mux(row);
		select_none();
		select_all();
	}
		
	serial_send(SCREEN_CMD, MASK_TYPE);
	serial_send(CMD_SYS_DIS, MASK_DATA);
	serial_send(0, 0x01);
	serial_send(CMD_COM8N, MASK_DATA);
	serial_send(0, 0x01);
	serial_send(CMD_MASTER_MODE, MASK_DATA);
	serial_send(0, 0x01);
	serial_send(CMD_SYS_EN, MASK_DATA);
	serial_send(0, 0x01);
	serial_send(CMD_LED_ON, MASK_DATA);
	serial_send(0, 0x01);
	
	//Now that we've initialized it's time to disable the CS on all the chips
	for (row = 0; row < 4; row++) {
		select_mux(row);
		select_none();
	}
		
	PORTC = 0;
}

int main(void) {
	uint8_t packet, i, line;
	
	linePtrs[0] = &lineBuff[0];
	linePtrs[1] = &lineBuff[64];
	linePtrs[2] = &lineBuff[16];
	linePtrs[3] = &lineBuff[80];
	linePtrs[4] = &lineBuff[128];
	linePtrs[5] = &lineBuff[192];
	linePtrs[6] = &lineBuff[144];
	linePtrs[7] = &lineBuff[208];
	linePtrs[8] = &lineBuff[32];
	linePtrs[9] = &lineBuff[96];
	linePtrs[10] = &lineBuff[48];
	linePtrs[11] = &lineBuff[112];
	linePtrs[12] = &lineBuff[160];
	linePtrs[13] = &lineBuff[224];
	linePtrs[14] = &lineBuff[176];
	linePtrs[15] = &lineBuff[240];

	screen_init();
	MSPIM_init();
	spi_init();
	
	screen_clear();

	 //Jump to the first chip and direct it to its first address in preparation for data.
	PORTC = 0;
	select_chip(0);
	chip_prepare();

    while(1) {
		while(head == tail); //await new data from SPI
		
		packet = buff[head]; //fetch packet. Could be anything, time to decode:
		if (packet & 0x80) {
			//Set X-address
			curr_index = packet & 0x7F;
			if ((((head + 1)) != tail) && (buff[(head + 1)] & 0x40)) {
				//Next command is change Y, skip changing chip for now
				continue;
			}			
			//Might as well switch chips at this point as we have time to spare until the next transmission
			chip_addr = curr_index % 16;
			if (curr_index > 63) chip_addr += 16; //Red colourspace
			chip_addr <<= 1; //each column has two addresses, we write to both at once with 8 bit transfers
			chip_goto();
			chip_addr = 0;
	
		} else if (packet & 0x40) {
			//Set Y-address
			curr_line = packet & 0x07;
			if ((((head + 1)) != tail) && (buff[(head + 1)] & 0x80)) {
				//Next command is change X, skip changing chip for now
				continue;
			}
			//Might as well switch chips at this point as we have time to spare until the next transmission
			chip_addr = curr_index % 16;
			if (curr_index > 63) chip_addr += 16; //Red colourspace
			chip_addr <<= 1; //each column has two addresses, we write to both at once with 8 bit transfers
			//ADD MATRIX ROW SWITCHING HERE WHEN THE MULTIPLEXER IS HOOKED UP
			chip_goto();
			chip_addr = 0;
	
		} else if (packet & 0x20) {
			//Display clear
			while ((UCSR0A & (1 << TXC0)) == 0); //Wait for all transmissions to complete
			screen_clear();
	
		} else if (packet & 0x10) {
			//Global brightness
	
		} else if (packet & 0x08) {
			line = packet & 0x07;
			//Entire line incoming
			for (i = 0; i < 128; i++) {
				head = (head + 1); //advance head
				while(head == tail); //await data
				//Fill buffer with complete line so we may write green and red simultaneously
				lineBuff[i] = buff[head];
			}
			//Check to see if we can combine two line operations
			//Operations can be combined if we are operating on line 0, 2, 4 or 6 and the next operation
			// is line 1, 3, 5 or 7 respectively.
			i = (head + 1); //next pos
			if (((line & 0x01) == 0) && (tail != i) && ((buff[i] & 0xF8) == 0x08) && ((buff[i] & 0x07) == (line + 1))) {
				//Load second line:
				head = i; //advance head to skip header
				for (i = 128; i != 0; i++) { //i will overflow to 0
					head = (head + 1); //advance head
					while(head == tail); //await data
					//Fill buffer with complete line so we may write green and red simultaneously
					lineBuff[i] = buff[head];
				}
				//Two lines acquired, send to screen using lineBuff[0-255]
				screen_sendDoubleLine(line);
				
			} else {
				//Single line acquired. Time to send it to the screen using lineBuff[0-127]
				screen_sendLine(line);
			}
	
		} else if (packet & 0x04) {
			//Single data
		}
		//Otherwise NO-OP. Useful for synchronizing.
		head++;
    }
}