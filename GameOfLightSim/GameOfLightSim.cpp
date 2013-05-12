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
#include "utility/MsTimer2.h"

//Pointer to self for use with the MsTimer2 library
static GameOfLightSim *thisclass = 0;

GameOfLightSim::GameOfLightSim() {
	_screen_line = 0;
	_screen_index = 0;
	btnRequestSent = 0;
	thisclass = this;
	//pinMode(13, OUTPUT);
  	MsTimer2::set(5, GameOfLightSim::btnCallBack);
}


void GameOfLightSim::begin() {
	Serial.begin(500000);
	MsTimer2::start();
}


/* Write a command to the display */
void GameOfLightSim::screen_cmd(uint8_t type, uint8_t value) {
	MsTimer2::stop();
	Serial.write(SCREEN_CMD);
	//Could technically insert more commands in this spot to reduce overhead. The screen understands this
	// extension of the protocol.
	Serial.write(type | value);
	Serial.write('\n');
	MsTimer2::start();
}


/* Write data directly to the display at current display position.
 * WARNING: 2 bytes overhead per byte sent, use update(line) for large transfers. */
void GameOfLightSim::screen_data(uint8_t data) {
	MsTimer2::stop();
	Serial.write(SCREEN_DATA);
	Serial.write(data);
	Serial.write('\n');
	MsTimer2::start();
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
	uint8_t line;
	_screen_line = 0xff; //Illegal value to force a resync on first line sent
	for (line = 0; line < 8; line++) {
		update(line);
	}
}


/* Sends a single line to the screen */
void GameOfLightSim::update(const uint8_t line) {
	uint8_t i;
	screen_goto(0, line);
	//Turn off the SNES-interrupts as they may destroy the update by injecting data into the
	//serial stream midtransfer:
	MsTimer2::stop();
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
	MsTimer2::start();
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

/* Asks simulator for stored keyboard-values, 
 * then reads response from serial whenever it becomes available.
 * Data arrives as NUM_PLAYERS*2 bytes. */
void GameOfLightSim::getButtons() {
	if (Serial.available() >= 4) {
		// Reading from Serial
		Serial.readBytes(serial_data, NUM_PLAYERS*2);
		for(int i = 0; i < NUM_PLAYERS; i++) {
			//Will keep a read buttonpush saved until it's given get-function resets it
			Start[i] &= ((serial_data[i*2] & (1 << 7)) == 0)? 1 : 0;
			Select[i] &= ((serial_data[i*2] & (1 << 6)) == 0)? 1 : 0;
			L[i] &= ((serial_data[i*2] & (1 << 5)) == 0)? 1 : 0;
			R[i] &= ((serial_data[i*2] & (1 << 4)) == 0)? 1 : 0; 
			N[i] &= ((serial_data[i*2+1] & (1 << 7)) == 0)? 1 : 0;
			W[i] &= ((serial_data[i*2+1] & (1 << 6)) == 0)? 1 : 0;
			S[i] &= ((serial_data[i*2+1] & (1 << 5)) == 0)? 1 : 0;
			E[i] &= ((serial_data[i*2+1] & (1 << 4)) == 0)? 1 : 0;
			X[i] &= ((serial_data[i*2+1] & (1 << 3)) == 0)? 1 : 0;
			Y[i] &= ((serial_data[i*2+1] & (1 << 2)) == 0)? 1 : 0;
			B[i] &= ((serial_data[i*2+1] & (1 << 1)) == 0)? 1 : 0;
			A[i] &= ((serial_data[i*2+1] & (1 << 0)) == 0)? 1 : 0;
		}
		btnRequestSent = 0;
	} else if (!btnRequestSent) {
		// Sending request to arduino
		Serial.write(REQUEST_KEYS);
		Serial.write('\n');
		btnRequestSent = 1;
	}
}

//A bit of a hack to get around the calling conventions of the MsTimer2 library.
void GameOfLightSim::btnCallBack() {
	//digitalWrite(13, ledMode);
	if (thisclass) {
		thisclass->getButtons();
	}
	//ledMode ^= 1;
}