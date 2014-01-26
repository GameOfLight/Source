/*
  Brian's Brain - a programexample for the GameOfLight libraries
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
* A cellular automaton with 3 states: dead, dying and alive
* Rules used:
*  A cell becomes alive if it's dead with exactly 2 alive neighbours
*  An alive cell turns into a dying one
*  Dying cells die (Quite a surprise, right?)
*
* Tends to lead to directional movement.
* See http://en.wikipedia.org/wiki/Brian's_Brain for more information
*
* Controls:
*  PLAYER1 start - return to menu
*  PLAYER1 select - add som random data to the screen to seed the algorithm
*/

#include <GameOfLightSim.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
extern GameOfLightSim frame;
uint8_t rand_8(); //from Snake.ino

#define BRAIN_DEAD BLACK
#define BRAIN_DYING RED
#define BRAIN_ALIVE ORANGE

void brain_splash() {
	frame.clear();
	
	//Write title:
	frame.gotoXY(11, 16);
  	frame.print("Brian's", GREEN);
  	brain_idle(0);
}

uint8_t brain_step(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end) {
	//One step of the Brian's Brain algorithm (within the defined window). Returns 1 if screen is blank
	uint8_t px;
	uint8_t gameEnd = 1;

	for (int x = x_start; x < x_end; x++) {
		//Number of alive pixels in the window function
		uint8_t top = 0;
		uint8_t mid = 0;
		uint8_t btm = 0;

		//Check Moore neighbourhood for alive neighbours.
		//Note that since we don't have the previous frame any tests directed at a pixel we've
		// already calculated (this frame) needs to check for dying instead of alive.
		//Basicly we're using the following window around the current (X)
		// DDL
		// DXL
		// DLL
		// D = dying (actually alive, but we've processed it so it is marked as dying now), L = alive		

		//Precalculate (what will become the) mid row of the window function
		if (x > 0 && frame.getPixel(x-1, y_start) == BRAIN_DYING) btm++;
		if (frame.getPixel(x, y_start) == BRAIN_ALIVE) btm++;
		if (x < 63 && frame.getPixel(x+1, y_start) == BRAIN_ALIVE) btm++;

		for (int y = y_start; y < y_end; y++) {
			px = frame.getPixel(x, y);

			top = mid;
			mid = btm;
			btm = 0;

			if (x > 0 && y < 63 && frame.getPixel(x-1, y+1) == BRAIN_DYING) btm++;
			if (y < 63 && frame.getPixel(x, y+1) == BRAIN_ALIVE) btm++;
			if (x < 63 && y < 63 && frame.getPixel(x+1, y+1) == BRAIN_ALIVE) btm++;

			if (px == BRAIN_DEAD) {
				//Dead pixel, check to see if it should be alive this frame:

				//Set to alive if it had exactly 2 live neighbours
				if (top + mid + btm == 2) frame.setPixel(x, y, BRAIN_ALIVE);
				

			} else if (px == BRAIN_ALIVE) {
				//Pixels are only alive for one frame
				frame.setPixel(x, y, BRAIN_DYING);
				gameEnd = 0; //Still something happening, don't stop yet!
			} else {
				//Dying pixels turn into dead pixels after one frame
				frame.setPixel(x, y, BRAIN_DEAD);
			}
		}
	}
	return gameEnd;
}

void brain_randomStart() {
	//Add some random data as a start point
	frame.gotoXY(22,16);
	frame.setColour(BRAIN_ALIVE);
	for (int i = 0; i < 20; i++) {
		frame.write(rand_8());
	}
	frame.gotoXY(22,40);
	for (int i = 0; i < 20; i++) {
		frame.write(rand_8());
	}
}

void brain_idle(uint8_t counter) {
	//Runs the cellular automaton in a window around the text "Brain" in the Game of Light menu
	frame.gotoXY(17, 32);
  	frame.print("Brain", ORANGE);

	brain_step(12, 52, 23, 48);
	
	if (counter & 4) {
		//brain_step() will turn the text RED, this periodically reverts that
		// causing the text to blink red->orange
		frame.gotoXY(17, 32);
		frame.print("Brain", ORANGE);
	}
}

void brain_run() {
	char buff[6];
	//Starts the cellular automaton using whatever state we had in the menu as seed
	while(1) {
		if (brain_step(0, 64, 0, 64)) {
			//Board empty, time to reseed it!
			frame.update();
			delay(500);
			brain_randomStart();
		}

		frame.update();
		delay(9);
		if (frame.getStart(PLAYER1)) break;

		if (frame.getSelect(PLAYER1)) brain_randomStart();
	}
}

