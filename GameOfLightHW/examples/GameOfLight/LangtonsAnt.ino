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

#include <GameOfLightHW.h>
extern GameOfLightHW frame;


//Todo:
//Allow player to select number of states and the rule to apply in each

uint8_t langton_splashscrn[] PROGMEM = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x30, 0x0, 0x80, 0x0, 0x0,
0x0, 0x0, 0x0, 0xf, 0x0, 0x70, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0x70, 0x0, 0xf, 0x0, 0x0,
0x0, 0x0, 0x0, 0x80, 0x0, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0xc0, 0x0, 0x20, 0x0, 0x91,
0x0, 0xd3, 0x0, 0x56, 0x0, 0x7c, 0x0, 0xff, 0x0, 0xff, 0x0, 0x7c, 0x0, 0x56, 0x0, 0xd3,
0x0, 0x91, 0x0, 0x20, 0x0, 0xc0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x3, 0x0, 0x81, 0x0, 0x60, 0x0, 0x1c, 0x0, 0x3,
0x0, 0x0, 0x0, 0xe, 0x0, 0x3f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x3f, 0x0, 0xe, 0x0, 0x0,
0x0, 0x3, 0x0, 0x1c, 0x0, 0x60, 0x0, 0x81, 0x0, 0x3, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0
};

//Inspired by
// http://en.wikipedia.org/wiki/Langton's_ant
// http://www.youtube.com/watch?v=1X-gtr4pEBU

//As per the wiki-page, a pattern will eventually converge to creating a sort of highway.
//Though as we're not using an unlimited board this may not always happen with our move-
//choices.

//PATTERNS TO TRY:
//Where the letters indicate the turnLeft/turnRight functions in the case structure below

//4 colour patterns:
//LLRR
//The current pattern of LLRR creates geometric patterns 
//within an everexpanding outline.

//RLLL
//Creates a chaotic pattern walled in by orange.

//3 colour patterns - Change the green case from BLACK to ORANGE for these patterns:
//CHANGE THESE TO ONLY CONTAIN 3 STEPS
//RRRL
//Expanding chaotic pattern

//LRRL
//Creates a chaotic pattern walled in by orange

//2 colour pattern - change the orange case from RED to BLACK

//LR
//Starts off with a random pattern then creates a highway.


//Start coordinates of ant
uint8_t antX;
uint8_t antY;

uint8_t dirX[] = {0, 1, 0, -1};
uint8_t dirY[] = {1, 0, -1, 0};
uint8_t antDir; //Starts going up

void (*langton_dir[4])();

void langton_splash() {
  frame.clear();

  for (uint8_t y = 0; y < 3; y++) {
    for (uint8_t x = 0; x < 3; x++) {
      frame.blit(langton_splashscrn+x*16+y*48, 20+x*8, 29+y*8);
    }
  }
  frame.gotoXY(5, 2);
  frame.print("Langton's", GREEN);
  frame.gotoXY(41, 3);
  frame.print("Ant", ORANGE);
}

void langton_run() {
  uint8_t langton_isRunning = 1;

  antX = 32;
  antY = 31;
  antDir = 0;

  frame.clear();
  frame.update();
  delay(500);
  while (langton_isRunning) {
    langton_loop();
    
    if (frame.getStart(PLAYER1)) langton_isRunning = 0;
  }
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

void langton_loop() {
  uint8_t colour;

  //Update ant position
  antX = (antX + dirX[antDir]) & 0x3f;//% 64;
  antY = (antY + dirY[antDir]) & 0x3f;//% 64;

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
  frame.update(antY>>3); //Red data
  //delay(3);
  delayMicroseconds(50);
}




