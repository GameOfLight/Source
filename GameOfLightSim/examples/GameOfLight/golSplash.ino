/*
  Conway's Game Of Life - a programexample for the GameOfLight libraries
  Copyright (c) 2013 André Kramer Orten.  All right reserved.

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


#include <GameOfLightSim.h>
#include <String.h>
extern GameOfLightSim frame;

#define BOARD 64


uint8_t gameOfLife_splashscrn[] PROGMEM =  {
  0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xf0, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0xfc, 0x0, 0xfc,
  0x0, 0xfc, 0x0, 0xf8, 0x0, 0xf0, 0x0, 0xe0, 0x0, 0xe0, 0x0, 0xf0, 0x0, 0xf8, 0x0, 0xfc,
  0x0, 0xfc, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0xf0, 0x0, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 0x0, 0x3f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0xff, 0x0, 0xff,
  0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,
  0x0, 0xff, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x3f, 0x0, 0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
  0x0, 0x3, 0x0, 0x7, 0x0, 0xf, 0x0, 0x1f, 0x0, 0xf, 0x0, 0x7, 0x0, 0x3, 0x0, 0x1,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xf0, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0xfc, 0x0, 0xfc,
  0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xf0, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0xfc, 0x0, 0xfc,
  0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xf0, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0xfc, 0x0, 0xfc,
  0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xf0, 0x0, 0xf8, 0x0, 0xf8, 0x0, 0xfc, 0x0, 0xfc
};


/*
  Inspired by Conway's game of life
  http://en.wikipedia.org/wiki/Conway's_Game_of_Life
*/

void gameOfLife_splash() {
  frame.clear();
  for(uint8_t y = 0; y < 3; y++) {
    for (uint8_t x = 0; x < 3; x++) {
      frame.blit(gameOfLife_splashscrn+x*16+y*48, 20+x*8, 5+y*8);
    }
  }
  frame.gotoXY(20, 4);
  frame.print("Game ", RED);
  frame.gotoXY(24, 5);
  frame.print("Of", GREEN);
  frame.gotoXY(20, 6);
  frame.print("Life", ORANGE);
}

void gameSetup() 
{
  frame.clear();

  pop(); //random start
  lifeCycle(); //start the game!
}

void pop() {
  for (int i = 0; i < BOARD*BOARD*0.4; i++) {
    int x = int(random(BOARD));
    int y = int(random(BOARD));
    //setter pixel til � leve
    frame.setPixel(x, y, ORANGE);
  }
}

void lifeCycle() {
  while(!frame.getStart(PLAYER1)) {

    //Empty the green area
    for (uint8_t i = 0; i < 8; i++) {
      memset(frame.green[i], 0, 64);
    }

    //Calculate the next generation
    for (uint8_t x = 0; x < 64; x++) {
      //Number of alive pixels in the window function
      uint8_t top = 0;
      uint8_t mid = 0;
      uint8_t btm = 0;

      //Precalculate (what will become the) mid row of the window function
      if (x > 0 && (frame.getPixel(x-1, 0) & RED)) btm++; //Checking ORANGE or RED
      if (frame.getPixel(x, 0) == RED) btm++;
      if (x < 63 && frame.getPixel(x+1, 0) == RED) btm++;

      for (uint8_t y = 0; y < 64; y++) {
        //The current pixel
        uint8_t alive = frame.getPixel(x, y);

        top = mid;
        mid = btm;
        btm = 0;

        if (y < 63) {
          if (x > 0 && (frame.getPixel(x-1, y+1) & RED)) btm++;
          if (frame.getPixel(x, y+1) == RED) btm++;
          if (x < 63 && frame.getPixel(x+1, y+1) == RED) btm++;
        }

        //Number of neighbours (including this square)
        uint8_t count = top + mid + btm;

        if (alive == RED) {
          if ((count == 3) || (count == 4))  { //NOTE: count includes this square hence testing on 3 and 4
            frame.setPixel(x, y, ORANGE);
          }
        } else if (count == 3) { //dead, to be resurrected
          frame.setPixel(x, y, GREEN);
        }
        //Otherwise the LED becomes/stays off.
      }
    }

    //Clear previous (now red). This will leave both the orange and green pixels as green pixels.
    for (uint8_t i = 0; i < 8; i++) {
      memset(frame.red[i], 0, 64);
    }

    frame.update();

    //Copy current frame to old in preparation for the next frame
    for (uint8_t i = 0; i < 8; i++) {
      memcpy(frame.red[i], frame.green[i], 64);
    }
  }
}
