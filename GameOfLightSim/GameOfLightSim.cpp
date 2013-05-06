/*
  GameOfLightSim.cpp - GameOfLight library
  Copyright (c) 2013 Stian Selbek.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301	 USA
*/
#include <Arduino.h>
#include "GameOfLightSim.h"

GameOfLightSim::GameOfLightSim() {
	uint8_t _screen_line = 0;
	uint8_t _screen_index = 0;
}


void GameOfLightSim::begin() {
	Serial.begin(500000);
}


/* Write a command to the display */
void GameOfLightSim::screen_cmd(uint8_t type, uint8_t value) {
	Serial.write(SCREEN_CMD);
	//Could technically insert more commands in this spot to reduce overhead. The screen understands this
	// extension of the protocol.
	Serial.write(type | value);
	Serial.write('\n');
}


/* Write data directly to the display at current display position.
 * WARNING: 2 bytes overhead per byte sent, use update(line) for large transfers. 
 * NOTE: This warning only applies to the simulator */
void GameOfLightSim::screen_data(uint8_t data) {
	Serial.write(SCREEN_DATA);
	Serial.write(data);
	Serial.write('\n');

	//Update trackers of screen position
	_screen_index++;
	if (_screen_index > 127) {
		_screen_index = 0;
		_screen_line++;
		if (_screen_line > 7) {
			_screen_line = 0;
		}
	}

}


void GameOfLightSim::clearDisplay() {
	// Clears both the internal buffer and the screen
	screen_cmd(CMD_TYPE_CLEAR, 0);
	clear();
}


void GameOfLightSim::update() {
//	GameOfLight::update(); //doesn't use the correct class when updating :/
	int line;
	for (line = 0; line < 8; line++) {
		update(line);
	}
}


/* Sends a single line to the screen */
void GameOfLightSim::update(const uint8_t line) {
	int i;
	screen_goto(0, line);
	//Switch to burst mode to reduce transmission overhead:
	Serial.write(SCREEN_DATA_BURST);
	Serial.write('\n');
	//Transfer entire line:
	for (i = 0; i < 128; i++) {
		Serial.write(buff[line][i]);
	}
	Serial.write('\n'); //End transmission and burst mode
	
	//Keep track of where the screen is currently at:
	_screen_line++;
	if (_screen_line >= 8) _screen_line = 0; //Note: _screen_index unchanged due to wrap
}



void GameOfLightSim::screen_goto(uint8_t index, uint8_t line) {
	line &= 0x07;  //Allows range [0,   7]
	index &= 0x7F; //Allows range [0, 127]

	/* Note: when using update() this doesn't send any actual goto-commands as the lines
	 * naturally overflow into the next transfer meaning we don't need to issue goto-commands */

	if (_screen_index != index) {
		screen_cmd(CMD_TYPE_SETX, index);
		_screen_index = index;
	}
	if (_screen_line != line) {
		screen_cmd(CMD_TYPE_SETY, line);
		_screen_line = line;
	}
}

/* Asks simulator for stored keyboard-values, then reads response from serial 
 * Data comes in NUM_PLAYERS*2 bytes.  */

void GameOfLightSim::getButtons() {
	// Sending request to arduino
	Serial.write(REQUEST_KEYS);
	Serial.write('\n');

	// Reading from Serial
	bytes_red = Serial.readBytes(serial_data, NUM_PLAYERS*2);

	if(bytes_red == NUM_PLAYERS*2) { 
		for(int i = 0; i < NUM_PLAYERS; i++) {
			Start[i] = ((serial_data[i] & (1 << 7)) == 0)? 1 : 0;
			Select[i] = ((serial_data[i] & (1 << 6)) == 0)? 1 : 0;
			L[i] = ((serial_data[i] & (1 << 5)) == 0)? 1 : 0;
			R[i] = ((serial_data[i] & (1 << 4)) == 0)? 1 : 0; 
			N[i] = ((serial_data[i+1] & (1 << 7)) == 0)? 1 : 0;
			W[i] = ((serial_data[i+1] & (1 << 6)) == 0)? 1 : 0;
			S[i] = ((serial_data[i+1] & (1 << 5)) == 0)? 1 : 0;
			E[i] = ((serial_data[i+1] & (1 << 4)) == 0)? 1 : 0;
			X[i] = ((serial_data[i+1] & (1 << 3)) == 0)? 1 : 0;
			Y[i] = ((serial_data[i+1] & (1 << 2)) == 0)? 1 : 0;
			B[i] = ((serial_data[i+1] & (1 << 1)) == 0)? 1 : 0;
			A[i] = ((serial_data[i+1] & (1 << 0)) == 0)? 1 : 0;
		}
	}
}
