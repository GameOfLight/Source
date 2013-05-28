/*
  Stephen  Wolfram - Rule 30 - a programexample for the GameOfLight libraries
  Copyright (c) 2013 André Kramer Orten.  All right reserved.

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


#include <GameOfLightHW.h>
extern GameOfLightHW frame;

#define BOARD 64

uint8_t rule30_splashscrn[] PROGMEM = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0x40, 0x0, 0xa0,
0x0, 0x10, 0x0, 0x8, 0x0, 0x4, 0x0, 0xfe, 0x0, 0x4, 0x0, 0x8, 0x0, 0x10, 0x0, 0xa0,
0x0, 0x40, 0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x10, 0x0, 0x28, 0x0, 0x44, 0x0, 0x82, 0x0, 0x5, 0x0, 0x8, 0x0, 0x10, 0x0, 0x20,
0x0, 0x41, 0x0, 0x82, 0x0, 0x4, 0x0, 0xff, 0x0, 0x4, 0x0, 0x82, 0x0, 0x41, 0x0, 0x20,
0x0, 0x10, 0x0, 0x8, 0x0, 0x5, 0x0, 0x82, 0x0, 0x44, 0x0, 0x28, 0x0, 0x10, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x0, 0x4, 0x0, 0x8,
0x0, 0x10, 0x0, 0x20, 0x0, 0x41, 0x0, 0x7f, 0x0, 0x41, 0x0, 0x20, 0x0, 0x10, 0x0, 0x8,
0x0, 0x4, 0x0, 0x2, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

/*
  Inspired by Stephen Wolfram's cellular automata Rule 30
  http://en.wikipedia.org/wiki/Rule_30
*/

void rule30_splash() {
  frame.clear();
  for(uint8_t y = 0; y < 3; y++) {
    for (uint8_t x = 0; x < 3; x++) {
      frame.blit(rule30_splashscrn+x*16+y*48, 20+x*8, 5+y*8);
    }
  }
  frame.gotoXY(10, 5);
  frame.print("Rule30", ORANGE);
  frame.update();
}


void ruleSetup() {
  frame.clear();
  frame.setPixel(BOARD/2, 0, RED);
  frame.update();
  delay(100);
  makeRule30();
}

void makeRule30() {
  for (int i = 1; i < BOARD; i++) {
    for (int j = -30; j < BOARD+30; j++) {
        frame.setPixel(j, i, state(j, i));
    }
    frame.update();
    delay(50);
  }
//  frame.clear();
//  frame.setPixel(BOARD/2, 0, ORANGE);
  frame.resetButtons();
  //avslutter når noen trykker på start
  while(!frame.getStart(PLAYER1));
}

/*
Pattern used for rule30:
current| pattern:             111  110  101  100  011  010  001  000
new state for center cell:     0    0    0    1    1    1    1    0
*/

/*
  returnerer fargen på ny pixel
  Denne funksjonen kan gjøres Mye penere
*/
uint8_t state(int i, int j) {
  uint8_t col1 = frame.getPixel((i-1)%BOARD, j-1);
  uint8_t col2 = frame.getPixel(i%BOARD, j-1);
  uint8_t col3 = frame.getPixel((i+1)%BOARD, j-1);
  if (col1 != BLACK) {
    if (col2 != BLACK) {
       if (col3 != BLACK) {
         //111 -> 0 (BLACK)
         return BLACK;
       } else {
         //110 -> 0 (kan sende en annen farge for svart)
         return BLACK;
       }
    //10     
    } else {
      if (col3 != BLACK) {
        //101
        return BLACK; 
      } else {
        //100
        return ORANGE;
      }
    }
  //0
  } else {
    if (col2 != BLACK) {
       if (col3 != BLACK) {
         //011
          return ORANGE;
       } else {
         //010
         return GREEN;
       }
     //00
    } else {
      if (col3 != BLACK) {
        //001
        return RED; 
      } else {
        //000
        return BLACK;
      }
    }
  }
  return BLACK;
}
