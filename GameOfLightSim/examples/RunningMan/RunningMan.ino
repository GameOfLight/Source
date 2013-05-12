#include <GameOfLightSim.h>
#include "man.h"
GameOfLightSim frame;

uint8_t frames[] = {0, 1, 0, 2};

int index = 0;
int x = 24;
int y = 32;
boolean moving = false;
boolean flip = false;
uint8_t dir;

void setup() {
  frame.begin();
}

void loop() {
  delay(35);
  frame.clear();
  if (!flip) {
    frame.blit(sheet + frames[index] * 16, x, y);
  } else {
    frame.blit(sheet + (frames[index] + 3) * 16, x, y);
  }
  frame.update();

  moving = false;

  dir = frame.getDir(PLAYER1);

  if (dir == NORTH) {
    y--;
    moving = true;
  } else if (dir == SOUTH) {
    y++;
    moving = true;
  }
  if (dir == EAST) {
    x++;
    moving = true;
    flip = false;
  } else if (dir == WEST) {
    x--;
    moving = true;
    flip = true;
  }
  if (x < -1) {
    x = -1;
  } else if (x > 57) {
    x = 57;
  }
  
  if (y < 0) {
    y = 0;
  } else if (y > 56) {
    y = 56;
  }
  index++;
  if (index > 3 || !moving) {
    index = 0;
  }
}
