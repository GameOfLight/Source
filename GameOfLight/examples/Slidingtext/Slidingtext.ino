/*
  Slidingtext - a programexample for the GameOfLight libraries
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

/* Moves the centre of the screen along an octagonal shape. */

#define GameOfLightHW //With this line both libaries are compiled for some reason or another. Without only the Sim-libary is compiled. If this line is movd to the GameOfLight.h file the HW-library is imported as expected.
#include <GameOfLight.h>
#include <SPI.h> //Only used for HW
GameOfLight frame;

uint8_t dir = 0; //current direction index of the following arrays:
int8_t dirX[] = {0, 1, 1, 1, 0, -1, -1, -1}; //left/right movement
int8_t dirY[] = {1, 1, 0, -1, -1, -1, 0, 1}; //up/down movement

uint8_t i;

void setup() {
  frame.begin();
  
  frame.setColour(GREEN);
  frame.gotoXY(1, 24);
  frame.print("Game");
  
  frame.setColour(ORANGE);
  frame.gotoXY(28, 24);
  frame.print("of");
  
  frame.setColour(RED);
  frame.gotoXY(5, 32);
  frame.print("Light");
  
  frame.update();
}

void loop() {
   dir = (dir + 1) % 8; //Switch to the next direction

   for (i = 0; i < 8; i++) {
     //Take 8 steps in the current direction
    
     if (dirX[dir] == 1) {
       frame.scrollRight();
     } else if (dirX[dir] == -1) {
       frame.scrollLeft();
     }
     
     if (dirY[dir] == 1) {
       frame.scrollUp();
     } else if (dirY[dir] == -1) {
       frame.scrollDown();
     } else {
       //Delay compensation due to skipping the slower
       //Up/down scrolling
       delay(1); 
     }
    frame.update();
    delay(30);
  }
}
