  /*
  Langton's Ant - a programexample for the GameOfLight libraries
  Copyright (c) 2013 Stian Selbek.  All right reserved.

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


//#include <GameOfLightSim.h>
//GameOfLightSim frame;
#include <GameOfLightHW.h>
#include <SPI.h>
GameOfLightHW frame;

//Inspired by http://www.youtube.com/watch?v=1X-gtr4pEBU
//The current pattern of LLRR creates geometric patterns 
//within an everexpanding outline.


//Start coordinates of ant
uint8_t antX = 32;
uint8_t antY = 31;

uint8_t dirX[] = {0, 1, 0, -1};
uint8_t dirY[] = {1, 0, -1, 0};
uint8_t antDir = 0; //Starts going up

void setup() {
   frame.begin();
   frame.clearDisplay();
}

//Turns the ants' direction to the left
void turnLeft() {
  if (antDir == 0) antDir = 3;
  else antDir--;
}

//Turns the ants' direction to the right
void turnRight() {
  if (antDir == 3) antDir = 0;
  else antDir++;
}

void loop() {
  uint8_t colour;

  //Update ant position
  antX = (antX + dirX[antDir]) % 64;
  antY = (antY + dirY[antDir]) % 64;

  //Grab colour of new position and decide what to do:
  colour = frame.getPixel(antX, antY);
  switch (colour) {
    case BLACK:
      frame.setPixel(antX, antY, ORANGE);
      turnLeft();
      break;
    case ORANGE:
      frame.setPixel(antX, antY, RED); 
      turnLeft();
      break;
    case RED:
      frame.setPixel(antX, antY, GREEN);
      turnRight();
      break;
    case GREEN:
      frame.setPixel(antX, antY, BLACK);
      turnRight();
      break;
  }

  //Send only the two updated bytes:
  frame.update(antX, antY/8);
  //frame.screen_data(frame.green[antY/8][antX]); //Green data
  //frame.screen_goto(antX+64, antY/8);
  //frame.screen_data(frame.red[antY/8][antX]); //Red data
  delay(3);
}


