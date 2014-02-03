/*
  Countdown - a programexample for the GameOfLight libraries
  Copyright (c) 2014 Stian Selbek.  All right reserved.

  This file is part of Game Of Light.

  Game Of Light is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Game Of Light is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Game Of Light.  If not, see <http://www.gnu.org/licenses/>.
  */

/*
* A countdown timer with setable start value.
* Note that some time drift is to be expected as only millis() is used for timekeeping.
*
* Controls:
*  In time set mode:
*   Up/down - increment/decrement current value, ie hours, minutes or seconds
*   Left/Right - pick which value to change
*   Start - start counting down from the currently selected start-value
*  In countdown mode:
*   Select - return to time set mode at the current timer value
*/
#include <GameOfLightHW.h>
#define COUNTDOWN_HOUR 0
#define COUNTDOWN_MINUTE 1
#define COUNTDOWN_SECOND 2

#define COUNTDOWN_COLOUR RED

extern uint8_t brain_step(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end);
extern uint8_t rand_8();
extern GameOfLightHW frame;
extern const unsigned char font_3x5[];


void countdown_splash() {
	frame.clear();
	frame.gotoXY(5, 16);
  	frame.print("Countdown", RED);
}

void countdown_idle(uint8_t counter) {
	//Draws a large number counting down
	frame.gotoXY(27, 28);
	frame.clear(9);
	frame.gotoXY(27, 35);
	frame.clear(9);
	countdown_displayHugeNr((9-((counter>>3) % 10))+'0', 27, 28, RED);
}

void countdown_displayHugeNr(uint8_t ch, uint8_t x, uint8_t y, uint8_t colour) {
	//Displays the given char (ch) using 3x3px per px of the 3x5 font
	uint8_t piece;
	for(uint8_t j = 0; j<3; j++) {
    	/* Fetch and print the pieces that makes up the letter given */	
    	piece = pgm_read_byte(font_3x5+(ch-0x20)*3+j);

    	for (uint8_t i = 0; i < 5; i++) {
    		uint8_t y_curr = y+3*i;
    		//Write the 5 px of the font with 3x3 sized pixels
    		if (piece & (1 << i)) {
    			//Time to write some colour in this spot
    			frame.setPixel(x, y_curr, colour);
    			frame.setPixel(x+1, y_curr, colour);
    			frame.setPixel(x+2, y_curr, colour);
    			y_curr++;
    			frame.setPixel(x, y_curr, colour);
    			frame.setPixel(x+1, y_curr, colour);
    			frame.setPixel(x+2, y_curr, colour);
				y_curr++;
    			frame.setPixel(x, y_curr, colour);
    			frame.setPixel(x+1, y_curr, colour);
    			frame.setPixel(x+2, y_curr, colour);
    		}
    	}
    	x += 3;
    }
}


void countdown_display(uint8_t timer[], uint8_t y, uint8_t colour) {
	//Writes the hours, minutes and seconds the to the screen
	// using a 3x magnified version of the small 3x5 font
	uint8_t x = 0;
	for (uint8_t i = 0; i < 3; i++) {
		uint8_t ch;
		uint8_t curr = timer[i];
		for (int8_t dec = 10; dec >= 1; dec-=9) {
			ch = (curr / dec) + '0';
			curr = curr % 10;
			countdown_displayHugeNr(ch, x, y, colour);
			x+=10;
    	}
    	if (i != 2) {
    		frame.setPixel(x, y+5, colour);
    		frame.setPixel(x, y+9, colour);
    	}
    	x+=2;
  	}
}

void countdown_setTime(uint8_t timer[]) {
	//Allows the user to set the desired start value for the timer
	uint8_t hand = COUNTDOWN_HOUR;
	//No clearing of current counters to allow user to tune the display without having to enter it all over again
	frame.resetButtons();
	while(!frame.getStart(PLAYER1)) {
		uint8_t dir = frame.getDir(PLAYER1);
		if (dir == NORTH) {
			//Increment current value (HOURs, MINUTES or SECONDS)
			timer[hand]++;
		} else if (dir == SOUTH) {
			//Decrement current value (HOURs, MINUTES or SECONDS)
			timer[hand]--;
		}
		if (hand == COUNTDOWN_HOUR && timer[hand]>99) timer[hand] = 0;
		else if (hand != COUNTDOWN_HOUR && timer[hand]>59) timer[hand] = 0;

		//Switch to the previous/next number if asked
		if (dir == WEST && hand > 0) hand--;
		else if (dir == EAST && hand < 2) hand++;

		frame.clear(); //lazyness
		countdown_display(timer, 24, COUNTDOWN_COLOUR); //display currently selected timer

		frame.gotoXY(20,3);
		frame.print("Time", GREEN);
		frame.gotoXY(23,13);
		frame.print("Set");


		//Display which number is currently being manipulated:
		frame.gotoXY(11, 48);
		switch(hand) {
			case COUNTDOWN_HOUR:
				frame.print(" HOURS");
				frame.drawLine(0, 40, 18, 40, ORANGE);
				break;
			case COUNTDOWN_MINUTE:
				frame.print("MINUTES");
				frame.drawLine(22, 40, 40, 40, ORANGE);
				break;
			case COUNTDOWN_SECOND:
				frame.print("SECONDS");
				frame.drawLine(44, 40, 62, 40, ORANGE);
				break;
		}
		frame.update();
		delay(150);
	}
	frame.resetButtons();
}



void countdown_run() {
	frame.clear();

	unsigned long curr_time, last_time;
	uint8_t y, counting, dice, steps, timer[4];
	for (int i = 0; i < 4; i++) {
		timer[i] = 0;
	}
	
	//Prompt user for the time to start counting at
	countdown_setTime(timer);

	counting = 1;
	y = 24;
	last_time = millis();

	while(counting) {
		curr_time = millis();
		if ((curr_time - last_time) >= 1000) {
			//One ~second has passed
			last_time += 1000; //Set next trigger in ~1 second
			
			if (timer[COUNTDOWN_SECOND] == 0) {
				if (timer[COUNTDOWN_MINUTE] == 0) {
					if (timer[COUNTDOWN_HOUR] == 0) {
						counting = 0;
						continue;
					}
					timer[COUNTDOWN_HOUR]--;
					timer[COUNTDOWN_MINUTE] = 60; //will set to 59 due to the next line
				}
				timer[COUNTDOWN_MINUTE]--;
				if (timer[COUNTDOWN_HOUR] == 0 && timer[COUNTDOWN_MINUTE] == 0) {
					//Ensure that the last minute is counted down center-screen
					y = 24;
				} else {
					//Pick a random y coordinate for the text each minute
					while ((y = (rand_8() >> 2)) > 49); //max 49 due to text dimensions
				}

				timer[COUNTDOWN_SECOND] = 60; //will set to 59 due to the next line
			}
			timer[COUNTDOWN_SECOND]--;

			//Check if user asked to set time again
			if (frame.getSelect(PLAYER1)) {
				countdown_setTime(timer);
				if (timer[COUNTDOWN_HOUR] == 0 && timer[COUNTDOWN_MINUTE] == 0) {
					//Ensure that the last minute is counted down center-screen
					y = 24;
				}
				last_time = millis(); //Needed a refresh as we don't know how long we waited in setTime
			}


			frame.clear(); //lazyness
			countdown_display(timer, y, COUNTDOWN_COLOUR); //display currently selected timer
			frame.update();
		}		
	}
	delay(100);
	countdown_display(timer, y, ORANGE); //need to make it orange for Brian's Brain to react to it initially

	//Countdown fireworks. Adds random expanding diamondshapes to the board. The shapes 
	// are calculated and react according to the rules of Brian's Brain.
	dice = 0xff; //Ensure we start with some extra orange pixels
	steps = 0;
	while((!brain_step(0, 64, 0, 64) && steps < 255) || steps < 32) {
		countdown_display(timer, y, RED); //redraw the timer (brian's brain has erased it)
		frame.update();
		if (dice > 220) {
			uint8_t x, y2;
			x = rand_8() >> 2;
			y2 = rand_8() >> 2;
			//2x2 pixels will create an expanding diamond-shape.
			//(Out of bounds writes (63+1 etc) are ignored by setPixel.)
			frame.setPixel(x, y2, ORANGE);
			frame.setPixel(x+1, y2, ORANGE);
			frame.setPixel(x, y2+1, ORANGE);
			frame.setPixel(x+1, y2+1, ORANGE);
		}
		dice = rand_8();
		delay(9);
		steps++;
	}
}