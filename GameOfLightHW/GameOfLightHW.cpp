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
}


void GameOfLightHW::begin() {
	// Initialize the communication with the display controller
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV8);
	pinMode(SCREEN_SS_PIN, OUTPUT);
	pinMode(SCREEN_DC_PIN, OUTPUT);
	digitalWrite(SCREEN_SS_PIN, HIGH);
	digitalWrite(SCREEN_SS_PIN, LOW);
	clearDisplay();
}


void GameOfLightHW::update() {
	// Sends the entire screen buffer to the screen
	int line;
	for (line = 0; line < 8; line++) {
		update(line);
	}
}


void GameOfLightHW::update(const uint8_t line) {
	// Sends a single line to the screen 
	int i;
	screen_goto(0, line);

	digitalWrite(SCREEN_SS_PIN, HIGH); //disable slave SPI
	digitalWrite(SCREEN_SS_PIN, LOW);  //enable slave SPI -> resync with slave
	digitalWrite(SCREEN_DC_PIN, HIGH); //To indicate data
	for (i = 0; i < 128; i++) {
		SPI.transfer(buff[line][i]);
		//As the screencontroller has a limited buffersize we can only send so much data before
		//it starts dropping frames. While we could've gotten away with sending several complete
		//lines without delay, this delay ensures that even if sending is all the Arduino does
		//it can't overfill the screencontroller.
		delayMicroseconds(4);
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
	digitalWrite(SCREEN_DC_PIN, LOW); //command
	SPI.transfer(type | value);
	delayMicroseconds(6);
}


void GameOfLightHW::screen_data(uint8_t data) {
	// Write data directly to the display at current display-position.
	digitalWrite(SCREEN_DC_PIN, HIGH); //data
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
	delayMicroseconds(6);
}


void GameOfLightHW::screen_goto(uint8_t index, uint8_t line) {
	line &= 0x07;  //Allows range [0,   7]
	index &= 0x7F; //Allows range [0, 127]

	// Note: when using update() this doesn't send any actual goto-commands as the lines
	// naturally overflow into the next transfer meaning we don't need to issue goto-commands

	if (_screen_index != index) {
		screen_cmd(CMD_TYPE_SETX, index);
		_screen_index = index;
	}
	if (_screen_line != line) {
		screen_cmd(CMD_TYPE_SETY, line);
		_screen_line = line;
	}
}
