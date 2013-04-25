/*
  Controller.ino - GameOfLight library
  Copyright (c) 2013 Martin Hol√?s.  All right reserved.

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
#ifndef Controller_h
#define Controller_h

#define SNES_CLK_PIN 6
#define SNES_LATCH_PIN 7
#define PLAYER1 0
#define PLAYER2 1
#define PLAYER3 2
#define PLAYER4 3
#define TICK {digitalWrite(SNES_CLK_PIN,HIGH); digitalWrite(SNES_CLK_PIN,LOW);}

enum direction {NORTH, SOUTH, EAST, WEST, NONE};

class Controller{
public:
	Controller();
	void getButtons();
	uint8_t getA(int Player);
	uint8_t getB(int Player);
	uint8_t getX(int Player);
	uint8_t getY(int Player);
	uint8_t getL(int Player);
	uint8_t getR(int Player);
	uint8_t getStart(int Player);
	uint8_t getSelect(int Player);
	direction getDir(int Player); //returns enum NORTH SOUTH EAST WEST NONE TODO sideskift

private:
	//uint8_t data[4]; //Pins to the 4 controllers
	uint8_t B[4];
	uint8_t Y[4];
	uint8_t Select[4];
	uint8_t Start[4];
	uint8_t N[4];
	uint8_t S[4];
	uint8_t W[4];
	uint8_t E[4];
	uint8_t A[4];
	uint8_t X[4];
	uint8_t L[4];
	uint8_t R[4];
};
#endif