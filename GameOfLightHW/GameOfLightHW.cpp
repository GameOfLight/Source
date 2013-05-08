/*
  GameOfLightHW.cpp - GameOfLight library
  Copyright (c) 2013 Stian Selbek.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include "GameOfLightHW.h"

GameOfLightHW::GameOfLightHW() {
	uint8_t _screen_line = 0;
	uint8_t _screen_index = 0;
	controller[0] = 2;
	controller[1] = 3;
	controller[2] = 4;
	controller[3] = 5; //pins on Arduino where the SNES-controller data pins are connected
}


void GameOfLightHW::begin() {
	// Initialize the communication with the display controller
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV8);
	pinMode(SCREEN_SS_PIN, OUTPUT);
	pinMode(controller[0], INPUT);
	pinMode(controller[1], INPUT);
	pinMode(controller[2], INPUT);
	pinMode(controller[3], INPUT);
	pinMode(SNES_CLK_PIN, OUTPUT);
	pinMode(SNES_LATCH_PIN, OUTPUT);
	digitalWrite(SCREEN_SS_PIN, HIGH);
	digitalWrite(SCREEN_SS_PIN, LOW);
	clearDisplay();
}


void GameOfLightHW::update() {
	// Sends the entire screen buffer to the screen
	int line;
	for (line = 0; line < 8; line++) {
		update(line);
		delayMicroseconds(20); //Ensures the screencontroller's buffer can keep up
	}
}


//Note: The screencontroller has a limited buffersize. Care must be taken to not call this function
// too often, lest the buffer overflows and lines are lost. 6-7 lines in rapid succesion is the aprox 
//limit.
//Also note that issuing updates of concecutive lines in numerical order is a little bit faster
// than random access as the requests are automatically combined for faster writes to the screen.
void GameOfLightHW::update(const uint8_t line) {
	// Sends a single line to the screen 
	int i;
	digitalWrite(SCREEN_SS_PIN, HIGH); //disable slave SPI
	digitalWrite(SCREEN_SS_PIN, LOW);  //enable slave SPI -> resync with slave
	screen_cmd(CMD_TYPE_LINE , line); //To indicate a line is coming and which line it is
	for (i = 0; i < 128; i++) {
		SPI.transfer(buff[line][i]);
	}
	
	//Keep track of where the screen is currently at:
	_screen_line++;
	if (_screen_line >= 8) _screen_line = 0; //Note: _screen_index unchanged due to wrap
}


void GameOfLightHW::clearDisplay() {
	// Clears both the internal buffer and the screen
	screen_cmd(CMD_TYPE_CLEAR, 0);
	_screen_line = 0;
	_screen_index = 0;

	clear();
}


void GameOfLightHW::screen_cmd(uint8_t type, uint8_t value) {
	// Write a command to the display
	SPI.transfer(type | value);
}


//WARNING: For the time being this is an unsupported operation, it will be ignored.
void GameOfLightHW::screen_data(uint8_t data) {
	// Write data directly to the display at current display-position.
	screen_cmd(CMD_TYPE_DATA, 0);
	SPI.transfer(data);
	//Update trackers of screen position
	_screen_index++;
	if (_screen_index > 127) {
		_screen_index = 0;
		_screen_line++;
		if (_screen_line > 7) {
			_screen_line = 0;
		}
	}
	//delayMicroseconds(6);
}


void GameOfLightHW::screen_goto(uint8_t index, uint8_t line) {
	line &= 0x07;  //Allows range [0,   7]
	index &= 0x7F; //Allows range [0, 127]
	if (_screen_index != index) {
		screen_cmd(CMD_TYPE_SETX, index);
		_screen_index = index;
	}
	if (_screen_line != line) {
		screen_cmd(CMD_TYPE_SETY, line);
		_screen_line = line;
	}
}

/* Reads input from snes-controllers and stores in buffers.
 * Input from controllers are inverted, so 0 means a button is active. */
void GameOfLightHW::getButtons(){
  digitalWrite(SNES_LATCH_PIN, HIGH);
  digitalWrite(SNES_LATCH_PIN, LOW);

	for(int i = 0; i<4; i++){
		B[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		Y[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		Select[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		Start[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		N[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		S[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		W[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		E[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		A[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		X[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		L[i]=digitalRead(controller[i]);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		R[i]=digitalRead(controller[i]);
	}
}