#include <GameOfLightHW.h>
#include "flames.h"
#include "jumpman.h"
#include "fireball.h"

#define FR_START    0
#define FR_PLAYING  1
#define FR_GAMEOVER 2
#define FR_EXIT 3

#define FR_JUMPHEIGHT 6
#define FR_DELAY 50

extern GameOfLightHW frame;

//Use left and right to control character, B-button to jump.

uint8_t FR_odd, FR_even, FR_x;

uint8_t FR_manframes[][4] = {
  {0, 1, 0, 2}, {3, 4, 3, 5}, {6, 6, 6, 6}, {7, 7, 7, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}
};

int8_t FR_pX, FR_pY;
uint8_t FR_pFrame;
uint8_t FR_state;
uint8_t FR_dir;
uint8_t FR_jumpHeight;

char FR_score[5];
uint8_t FR_consecutives;

uint8_t FR_platforms[64];
uint8_t FR_prevHeight;

uint8_t FR_screen;

int8_t FR_ball[4];

void FR_run() {
  FR_screen = FR_PLAYING;
  FR_odd = 0;
  FR_even = 4;
  FR_start();
  while (FR_screen != FR_EXIT) {
    FR_loop();
  }
}

void FR_splash() {
  frame.clear();

  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_even * 16, i * 16 - (FR_x & 0x0F), 48);
  }
  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_odd * 16, i * 16 + 8 - (FR_x & 0x0F), 48);
  }
  FR_odd++;
  FR_even++;
  FR_odd &= 0x07;
  FR_even &= 0x07;

  FR_drawStartScreen();
}

void FR_idle(uint8_t idle_count) {
  //Clear title and flame area so we may redraw with new state
  frame.gotoXY(14,3);
  frame.clear(36);
  frame.gotoXY(14, 4);
  frame.clear(36);
  frame.gotoXY(0, 6);
  frame.clear(64);
  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_even * 16, i * 16 - (FR_x & 0x0F), 48);
  }
  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_odd * 16, i * 16 + 8 - (FR_x & 0x0F), 48);
  }

  FR_odd++;
  FR_even++;
  FR_odd &= 0x07;
  FR_even &= 0x07;

  FR_drawStartScreen();
}

void FR_start() {
  randomSeed(5166);
  FR_x = 0;
  FR_pX = 32;
  FR_pY = 32;
  FR_pFrame = 0;
  FR_state = 0;
  FR_dir = 0;
  FR_jumpHeight = 0;
  FR_ball[2] = -8;

  for (int i = 0; i < 64; i++) {
    FR_platforms[i] = 40;
  }

  FR_consecutives = 64;
  for (int i = 0; i < 4; i++) {
    FR_score[i] = '0';
  }
  FR_score[4] = 0;
  frame.resetButtons();
}

void FR_drawStartScreen() {
  frame.gotoXY(17, 3);
  frame.print("FLAME", RED);
  frame.gotoXY(14, 4);
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
  frame.gotoXY(6,3);
  frame.print("GAME OVER", RED);
  frame.gotoXY(20,4);
  frame.print(FR_score, RED);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 64; j++) {
      if (frame.getPixel(j, i + 24) && random(2)) {
        frame.setPixel(j, i + 23, ORANGE);
      }
    }
  }

  if (frame.getStart(0)) {
    FR_screen = FR_EXIT;
  }
  frame.update();
}

void FR_loop() {
  delay(FR_DELAY);
  frame.clear();

  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_even * 16, i * 16 - (FR_x & 0x0F), 56);
  }
  for (int i = 0; i < 5; i++) {
    frame.blit(flames + FR_odd * 16, i * 16 + 8 - (FR_x & 0x0F), 56);
  }

  FR_odd++;
  FR_even++;
  FR_odd &= 0x07;
  FR_even &= 0x07;

  if (FR_state >= 4 && FR_pFrame == 3) {
    FR_screen = FR_GAMEOVER;
    FR_state = 0;
  }

//  frame.getButtons();
  switch(FR_screen) {
  case FR_START:
    FR_drawStartScreen();
    return;
  case FR_GAMEOVER:
    FR_drawGameOverScreen();
    return;
  }

  frame.gotoXY(20,0);
  frame.print(FR_score, RED);  

  frame.blit(fireball + (FR_ball[0] + FR_ball[1]) * 16, FR_ball[2], FR_ball[3]);
  FR_ball[0] = (FR_ball[0] + 1) % 2;
  FR_ball[3] += -2 + FR_ball[1];
  FR_ball[2]--;

  if (FR_ball[3] > 62) {
    FR_ball[1] = 0;
  } else if (FR_ball[3] < 18) {
    FR_ball[1] = 4;
  }

  if (!frame.B[0] && FR_state < 2) {
    frame.B[0] = 1;
    FR_jumpHeight = FR_JUMPHEIGHT;
  }

  if (FR_state < 4) {
    FR_state = 2;
  }
  if (FR_jumpHeight) {
    FR_pY--;
    FR_jumpHeight--;
  } 
  else {
    FR_pY++;
    for (int i = 1 + FR_dir; FR_state < 4 && i < 6 + FR_dir; i++) {
      if (FR_platforms[(FR_x + FR_pX + i) & 0x3F] - FR_pY == 7) {
        FR_pY--;
        FR_state = 0;
        break;
      }
    }
  }

  if (FR_state < 4 && (FR_pY == 52 || (FR_pX < FR_ball[2] + 6 && FR_pX > FR_ball[2] - 6 && FR_pY < FR_ball[3] + 6 && FR_pY > FR_ball[3] - 6))) {
    FR_state = 4;
    FR_pFrame = 3;
  }

  FR_pFrame = (FR_pFrame + 1) & 0x03;
  if (!frame.E[0]) {
    frame.E[0] = 1;
    FR_dir = 0;
    FR_pX++;
  } 
  else if (!frame.W[0]) {
    frame.W[0] = 1;
    FR_dir = 1;
    FR_pX -= 2;
  } 
  else {
    if (FR_state < 2) {
      FR_pFrame = 0;
    }
    FR_pX--;
  }

  if (FR_pX < -1) {
    FR_pX = -1;
  } 
  else if (FR_pX > 57) {
    FR_pX = 57;
  }

  if (FR_dir) {
    FR_state |= 1;
  }

  frame.blit(jumpman + FR_manframes[FR_state][FR_pFrame] * 16, FR_pX, FR_pY);

  for (int i = 0; i < 64; i++) {
    frame.setPixel(i, FR_platforms[(i + FR_x) & 0x3F], 2);
  }

  frame.update();

  int chance = random(3, 32);
  boolean hole = FR_platforms[(FR_x + 63) & 0x3F] == 63;
  if (chance < FR_consecutives || (hole && FR_consecutives > 10)) {
    if (!hole && !random(3)) {
      FR_platforms[FR_x] = 63;
      FR_consecutives = 0;
      if (FR_ball[2] <= -8) {
        FR_ball[2] = 64;
        FR_ball[3] = random(9, 31) * 2;
        FR_ball[1] = 4;
      }

    } 
    else {
      FR_platforms[FR_x] = random(16, 50);

      if (FR_prevHeight - FR_platforms[FR_x] > 5) {
        FR_platforms[FR_x] = FR_prevHeight - 5;
      }

      FR_consecutives = 0;
      FR_prevHeight = FR_platforms[FR_x];
    }

  } 
  else {
    FR_platforms[FR_x] = FR_platforms[(FR_x + 63) & 0x3F];
    FR_consecutives++;
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



