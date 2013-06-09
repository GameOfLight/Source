/*
  Slidingtext - a program-example for the GameOfLight libraries
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

/* Moves the centre of the screen along an octagonal shape. */

#include <GameOfLightSim.h>
GameOfLightSim frame;

uint8_t dir = 0; //current direction index of the following arrays:
int8_t dirX[] = {0, 1, 1, 1, 0, -1, -1, -1}; //left/right movement
int8_t dirY[] = {1, 1, 0, -1, -1, -1, 0, 1}; //up/down movement

uint8_t i;

void setup() {
  frame.begin();
  
  frame.setColour(GREEN);
  frame.gotoXY(1, 3);
  frame.print("Game");
  
  frame.setColour(ORANGE);
  frame.gotoXY(28, 3);
  frame.print("of");
  
  frame.setColour(RED);
  frame.gotoXY(5, 4);
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
