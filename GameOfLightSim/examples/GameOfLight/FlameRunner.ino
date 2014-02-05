/*
  FlameRunner - a small platformer for the GameOfLight project
  Copyright (c) 2013 Sigmund Hansen, Chickensoft.  All right reserved.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#include <GameOfLightSim.h>
#include "chickensoft.h"
#include "flames.h"
#include "jumpman.h"
#include "fireball.h"

#define FR_START    0
#define FR_PLAYING  1
#define FR_GAMEOVER 2
#define FR_EXIT 3

#define FR_JUMPHEIGHT 6
#define FR_DELAY 50

extern GameOfLightSim frame;

//Use left and right to control character, B-button to jump.

static const uint8_t PROGMEM FR_manframes[] = {
  0, 1, 0, 2, 
  3, 4, 3, 5,
  6, 6, 6, 6,
  7, 7, 7, 7,
  8, 9, 10, 11,
  12, 13, 14, 15
};

uint8_t FR_odd, FR_x; //These could be moved to the FR_globals struct as well.

struct FR_globals {
  int8_t pX, pY;
  uint8_t pFrame;
  uint8_t state;
  uint8_t dir;
  uint8_t jumpHeight;
  uint8_t deathCountdown;
  uint8_t consecutives;
  uint8_t prevHeight;
  uint8_t screen;
};

struct FR_globals* FR;
uint8_t *FR_platforms; //Allocated on start. 64 bytes
char *FR_score; //Allocated as 5 bytes
int8_t *FR_ball; //Allocated as 4 bytes

void FR_run() {
  FR_score = (char *) malloc(5*sizeof(char));
  FR_platforms = (uint8_t *) malloc(64*sizeof(uint8_t));
  FR_ball = (int8_t *) malloc(4*sizeof(int8_t));
  FR = (struct FR_globals *) malloc(sizeof(FR_globals));

  FR_start();
  while (FR->screen != FR_EXIT) {
    FR_loop();
  }

  free(FR_score);
  free(FR_platforms);
  free(FR_ball);
  free(FR);
}

void FR_drawFlames(uint8_t y) {
  uint8_t FR_even = (FR_odd + 4) & 0x07;
  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_even * 16, i * 16 - (FR_x & 0x0F), y);
  }
  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_odd * 16, i * 16 + 8 - (FR_x & 0x0F), y);
  }
  FR_odd++;
  FR_odd &= 0x07;
}

void FR_splash() {
  frame.clear();
  
  FR_x = 0;
  FR_odd = 0;

  FR_drawStartScreen();
}

void FR_idle(uint8_t idle_count) {
  //Clear title and flame area so we may redraw with new state
  frame.gotoXY(14,16);
  frame.clear(36);
  frame.gotoXY(14,24);
  frame.clear(36);
  frame.gotoXY(14, 32);
  frame.clear(36);
  frame.gotoXY(0, 48);
  frame.clear(64);

  FR_drawStartScreen();
}

void FR_start() {
  randomSeed(5166);
  FR_x = 0;
  FR->pX = 32;
  FR->pY = 32;
  FR->pFrame = 0;
  FR->state = 0;
  FR->dir = 0;
  FR->jumpHeight = 0;
  FR_ball[2] = -8;
  FR->screen = FR_PLAYING;
  FR_odd = 0;
  FR->deathCountdown = 5;

  for (int i = 0; i < 64; i++) {
    FR_platforms[i] = 40;
  }

  FR->consecutives = 64;
  for (int i = 0; i < 4; i++) {
    FR_score[i] = '0';
  }
  FR_score[4] = 0;
  frame.resetButtons();
}

void FR_drawStartScreen() {
  FR_drawFlames(48);
  
  frame.blit(chickensoft, 24, 0);
  frame.blit(chickensoft + 16, 32, 0);
  frame.blit(chickensoft + 32, 24, 8);
  frame.blit(chickensoft + 48, 32, 8);
  
  frame.gotoXY(17, 24);
  frame.print("FLAME", RED);
  frame.gotoXY(14, 32);
  frame.print("RUNNER", RED);
  for (int i = 0; i < 16; i++) {
    for (int j = 14; j < 50; j++) { //reduced interval so we don't mess up the menu arrows
      if (frame.getPixel(j, i + 24) && random(2)) {
        frame.setPixel(j, i + 23, ORANGE);
      }
    }
  }
  frame.update();
}

void FR_drawGameOverScreen() {
  frame.gotoXY(6,24);
  frame.print("GAME OVER", RED);
  frame.gotoXY(20,32);
  frame.print(FR_score, RED);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 64; j++) {
      if (frame.getPixel(j, i + 24) && random(2)) {
        frame.setPixel(j, i + 23, ORANGE);
      }
    }
  }

  if (frame.getStart(0)) {
    FR->screen = FR_EXIT;
  }
  frame.update();
}

void FR_loop() {
  delay(FR_DELAY);
  frame.clear();

  FR_drawFlames(56);

  if (FR->state >= 4 && --FR->deathCountdown <= 0) {
    FR->screen = FR_GAMEOVER;
    FR->state = 0;
  }

//  frame.getButtons();
  switch(FR->screen) {
  case FR_START:
    FR_drawStartScreen();
    return;
  case FR_GAMEOVER:
    FR_drawGameOverScreen();
    return;
  }

  frame.gotoXY(20,0);
  frame.print(FR_score, RED);  

  if (FR_ball[3] > 62) {
    FR_ball[1] = 0;
  } else if (FR_ball[3] < 18) {
    FR_ball[1] = 4;
  }

  if (frame.getB(0) && FR->state < 2) {
    FR->jumpHeight = FR_JUMPHEIGHT;
  }

  if (FR->state < 4) {
    FR->state = 2;
  }
  if (FR->jumpHeight) {
    FR->pY--;
    FR->jumpHeight--;
  } 
  else {
    FR->pY++;
    for (int i = 1 + FR->dir; FR->state < 4 && i < 6 + FR->dir; i++) {
      if (FR_platforms[(FR_x + FR->pX + i) & 0x3F] - FR->pY == 7) {
        FR->pY--;
        FR->state = 0;
        break;
      }
    }
  }

  FR->pFrame = (FR->pFrame + 1) & 0x03;
  switch(frame.getDir(0)) {
  case EAST:
    FR->dir = 0;
    FR->pX++;
    break;
    
  case WEST:
    FR->dir = 1;
    FR->pX -= 2;
    break;
    
  default:
    if (FR->state < 2) {
      FR->pFrame = 0;
    }
    FR->pX--;
  }

  if (FR->pX < -1) {
    FR->pX = -1;
  } 
  else if (FR->pX > 57) {
    FR->pX = 57;
  }

  if (FR->dir) {
    FR->state |= 1;
  }

  frame.blit(jumpman + pgm_read_byte(FR_manframes+(FR->state)*4+FR->pFrame) * 16, FR->pX, FR->pY);

  if (FR->state < 4) {
    if (FR->pY == 52) {
      FR->state = 4 + (FR->state % 2 ? 1 : 0);
      FR->pFrame = 3;
    }
    
    for (int i = max(0, FR_ball[2] + 2); FR->state != 4 && i < min(FR_ball[2] + 6, 63); i++) {
      for (int j = FR_ball[3] + 1; FR->state != 4 && j < min(FR_ball[3] + 7, 52); j++) {
        if (frame.getPixel(i, j) == GREEN) {
          FR->state = 4 + (FR->state % 2 ? 1 : 0);
          FR->pFrame = 3;
        }
      }
    }
  }
  
  frame.blit(fireball + (FR_ball[0] + FR_ball[1]) * 16, FR_ball[2], FR_ball[3]);
  FR_ball[0] = (FR_ball[0] + 1) % 2;
  FR_ball[3] += -2 + FR_ball[1];
  FR_ball[2]--;

  for (int i = 0; i < 64; i++) {
    frame.setPixel(i, FR_platforms[(i + FR_x) & 0x3F], 2);
  }

  frame.update();

  int chance = random(3, 32);
  boolean hole = FR_platforms[(FR_x + 63) & 0x3F] == 63;
  if (chance < FR->consecutives || (hole && FR->consecutives > 10)) {
    if (!hole && !random(3)) {
      FR_platforms[FR_x] = 63;
      FR->consecutives = 0;
      if (FR_ball[2] <= -8) {
        FR_ball[2] = 64;
        FR_ball[3] = random(9, 31) * 2;
        FR_ball[1] = 4;
      }

    } 
    else {
      FR_platforms[FR_x] = random(16, 50);

      if (FR->prevHeight - FR_platforms[FR_x] > 5) {
        FR_platforms[FR_x] = FR->prevHeight - 5;
      }

      FR->consecutives = 0;
      FR->prevHeight = FR_platforms[FR_x];
    }

  } 
  else {
    FR_platforms[FR_x] = FR_platforms[(FR_x + 63) & 0x3F];
    FR->consecutives++;
  }

  FR_x++;

  if (FR_x == 64) {
    FR_x = 0;
  }

  if (FR_score[3] != '9') {
    FR_score[3]++;
  } 
  else if (FR_score[2] != '9') {
    FR_score[3] = '0';
    FR_score[2]++;
  } 
  else if (FR_score[1] != '9') {
    FR_score[3] = '0';
    FR_score[2] = '0';
    FR_score[1]++;
  } 
  else if (FR_score[0] != '9') {
    FR_score[3] = '0';
    FR_score[2] = '0';
    FR_score[1] = '0';
    FR_score[0]++;
  }
}



