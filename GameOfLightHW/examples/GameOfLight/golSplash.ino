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


#include <GameOfLightHW.h>
extern GameOfLightHW frame;

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
  pop();
  frame.update();
  lifeCycle();
}

void pop() {
  for (int i = 0; i < BOARD*BOARD*0.4; i++) {
    int x = int(random(BOARD));
    int y = int(random(BOARD));
    //setter pixel til � leve
    frame.setPixel(x, y, GREEN);
  }
}

void lifeCycle() 
{
  while(!frame.getStart(PLAYER1)) {
    for (int i = 1; i < (BOARD-1); i++) {
      for (int j = 1; j < (BOARD-1); j++) {
        //fargen p� n�v�rende pixel
        int alive = frame.getPixel(i, j);
        //antall naboer
        int count = neighbors(i, j);
        //en d�d celle
        if (alive == GREEN || alive == ORANGE) {
          if (count == 2 || count == 3) {
            //lever videre
            frame.setPixel(i, j, GREEN);
          } else {
            //Skal d� ut
            frame.setPixel(i, j, ORANGE);
          }
          //her kommer vi inn hvor cellene er d�de BLACK || RED
        } else {
          if (count == 3) {
            //gir cellen nytt liv
            frame.setPixel(i, j, RED);
          }
        }
      }
    }
//    frame.update();
//    delay(50);
    for (int i = 0; i < BOARD; i++) {
      for (int j = 0; j< BOARD; j++) {
        uint8_t col = frame.getPixel(i, j);
        if (col == RED) {
          frame.setPixel(i, j, GREEN);
        } if (col == ORANGE) {
          frame.setPixel(i,j, BLACK);
        }
      }
    }
    frame.update();
    delay(3);
  }
}


int neighbors(int x, int y) {
  return alivePixel((x+1)%BOARD, y) +
    alivePixel(x, (y+1)%BOARD) +
    alivePixel((x+BOARD-1)%BOARD, y) +
    alivePixel(x, (y+BOARD-1)%BOARD) +
    alivePixel((x+1)%BOARD, (y+1)%BOARD) +
    alivePixel((x+BOARD-1)%BOARD, (y+1)%BOARD) +
    alivePixel((x+BOARD-1)%BOARD, (y+BOARD-1)%BOARD) +
    alivePixel((x+1)%BOARD, (y+BOARD-1)%BOARD);
}

int alivePixel(int x, int y) {
  uint8_t col = frame.getPixel(x, y);
  if (col == GREEN || col == ORANGE) {
    return 1;
  }
  return 0;
}
