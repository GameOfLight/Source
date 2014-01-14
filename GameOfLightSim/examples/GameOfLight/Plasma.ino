  /*
  Plasma - a programexample for the GameOfLight libraries
   Implementation of http://www.maths.tcd.ie/~nryan/demos/plasma.html 
   
   Game of Light version by Stian Selbek
  */


/*Usage:
* Player 1 start-button - returns to menu
* Player 1 select-button - re-randomize initial condition and stepsizes
* Player 1 A-button - toggle colourdither
*/


#include <GameOfLightSim.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
extern GameOfLightSim frame;
uint8_t rand_8(); //from Snake.ino

uint8_t plasma_vptr1, plasma_vptr2;
uint8_t plasma_hptr1, plasma_hptr2;
uint8_t plasma_ptr[4];

//round(31.5*cos(x)+31.5) lookup table:
uint8_t plasma_lookup[256] PROGMEM = {63, 63, 63, 63, 63, 63, 63, 63, 62, 62, 62, 62, 62, 61, 61, 61, 61, 60, 60, 60, 59, 59, 58, 58, 58, 57, 57, 56, 56, 55, 55, 54, 54, 53, 53, 52, 51, 51, 50, 50, 49, 48, 48, 47, 46, 46, 45, 44, 43, 43, 42, 41, 40, 40, 39, 38, 37, 37, 36, 35, 34, 34, 33, 32, 31, 31, 30, 29, 28, 27, 27, 26, 25, 24, 24, 23, 22, 21, 21, 20, 19, 19, 18, 17, 16, 16, 15, 14, 14, 13, 13, 12, 11, 11, 10, 10, 9, 8, 8, 7, 7, 6, 6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 10, 10, 11, 11, 12, 13, 13, 14, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 21, 22, 23, 24, 24, 25, 26, 27, 27, 28, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 43, 43, 44, 45, 46, 46, 47, 48, 48, 49, 50, 50, 51, 51, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 58, 59, 59, 60, 60, 60, 61, 61, 61, 61, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63};
uint8_t plasma_colours[4] = {BLACK, GREEN, ORANGE, RED};

//Runtime user-configurable variables.
uint8_t plasma_spd_mod[5]; //random modifiers for the stepsizes
uint8_t plasma_dither;

void plasma_splash() {
	frame.clear();
	
	//Make the plasma smaller for the text on the splashscreen.
	plasma_spd_mod[0] = 7; //3
	plasma_spd_mod[1] = 9; //6
	plasma_spd_mod[2] = 10; //6
	plasma_spd_mod[3] = 12; //9
	plasma_dither = 1;

	plasma_idle(0);
}

void plasma_onText(uint8_t x, uint8_t y, uint8_t colour) {
	//Writes the selected pixel only if there's something there allready. Useful for decorating text.
	if (frame.getPixel(x, y)) {
		frame.setPixel(x, y, colour);
	}
}

void plasma_direct(uint8_t x, uint8_t y, uint8_t colour) {
	frame.setPixel(x, y, colour);
}

void plasma_step(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end, void func(uint8_t x, uint8_t y, uint8_t colour)) {
	uint8_t px;

	plasma_vptr1 = plasma_ptr[2];
	plasma_vptr2 = plasma_ptr[3];

	for (int y = y_start; y < y_end; y++) {
		plasma_vptr1 += 1 + plasma_spd_mod[0];//1;
		plasma_vptr2 += 2 + plasma_spd_mod[1];//2;

		plasma_hptr1 = plasma_ptr[0];
		plasma_hptr2 = plasma_ptr[1];

		for (int x = x_start; x < x_end; x++) { 
			plasma_hptr1 += 2 + plasma_spd_mod[2];
			plasma_hptr2 += 3 + plasma_spd_mod[3];

			px = pgm_read_byte(plasma_lookup+plasma_hptr1) + pgm_read_byte(plasma_lookup+plasma_hptr2) + pgm_read_byte(plasma_lookup+plasma_vptr1) + pgm_read_byte(plasma_lookup+plasma_vptr2);

			if (plasma_dither) {
				px /= 32;
				if (px & 1 && px != 7) { //between two colour values
					//dither it!
					px += (x+y & 1);
				}
				px = px >> 1;
			} else {
				//Dithering disabled
				px /= 64;
			}
			//Decide what to do with this new pixel
			func(x, y, plasma_colours[px]);
		}
	}
	//Prepare for next frame:
	plasma_ptr[0] -= 3 + plasma_spd_mod[4];
	plasma_ptr[1] += 2 + plasma_spd_mod[4];
	plasma_ptr[2] -= 3 + plasma_spd_mod[4];
	plasma_ptr[3] += 1 + plasma_spd_mod[4];
}

void plasma_idle(uint8_t counter) {
	//Write title:
	frame.gotoXY(14, 2);
  	frame.print("Plasma", GREEN);
  	frame.gotoXY(14, 3);
  	frame.print("Plasma");
  	frame.gotoXY(14, 4);
  	frame.print("Plasma");

  	//Then decorate the text with the plasma effect:
  	plasma_step(12, 52, 16, 48, plasma_onText);
}

void plasma_randomize() {
	//Change the pointers
	for (int i = 0; i < 4; i++) {
		plasma_ptr[i] = rand_8();
	}

	//Changes the step variables
	for (int i = 0; i < 5; i++) {
		plasma_spd_mod[i] = rand_8() >> 6;
	}
}


void plasma_run() {
	plasma_randomize();

	while (1) {
		plasma_step(0, 64, 0, 64, plasma_direct);
		frame.update();
		rand_8(); //Increase percieved randomness. New pattern if requested will now also rely on when you press.

		delay(16); //Fixed delay to ensure buttons are read due to reseting timer on frame.update()
		if (frame.getStart(PLAYER1)) break;

		if (frame.getSelect(PLAYER1)) plasma_randomize();

		if (frame.getA(PLAYER1)) plasma_dither ^= 1;
	}
}