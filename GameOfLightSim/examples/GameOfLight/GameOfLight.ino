/*
  GameOfLight - the main menu for the GameOfLight project
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

#include <GameOfLightSim.h>
GameOfLightSim frame;

#define PROGRAMCOUNT 2
#define IDLE_START_COUNT 0


uint8_t player[4]; //Set to 0 if player is not playing, otherwise non-zero.
uint8_t playerCnt;

uint8_t arrowL[] PROGMEM = {0x18, 0x0, 0x3c, 0x0, 0x7e, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
uint8_t arrowR[] PROGMEM = {0xff, 0x0, 0xff, 0x0, 0x7e, 0x0, 0x3c, 0x0, 0x18, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

uint8_t idle_counter = IDLE_START_COUNT;
uint8_t curr = 0;

/*void begin2() {
  frame.clear();
}*/

void (*menu_option[PROGRAMCOUNT])(); //Draw splashscreen of current item
void (*menu_idle[PROGRAMCOUNT])(uint8_t); //If inactive on a menu item; run this every frame
void (*menu_run[PROGRAMCOUNT])(); //If 'start' is pressed, run the current menu item

void setup() {
    menu_option[0] = snake_splash;
    menu_idle[0] = snake_idle;
    menu_run[0] = snake_run;

    menu_option[1] = langton_splash;
    menu_idle[1] = 0;
    menu_run[1] = langton_run;

    frame.begin();
    (*menu_option[0])();
    frame.resetButtons();
}

/*
void menu_test() {
  menu_playerStart(2);
  frame.gotoXY(0, 7);
  frame.print("Hei");
  frame.update();
  delay(1000);
}*/


uint8_t menu_flipByte(uint8_t in) {
  uint8_t out;
  //This could be made in a more elegant fashion
  out = in & 0x80 ? 0x01 : 0;
  out |= in & 0x40 ? 0x02 : 0;
  out |= in & 0x20 ? 0x04 : 0;
  out |= in & 0x10 ? 0x08 : 0;
  out |= in & 0x08 ? 0x10 : 0;
  out |= in & 0x04 ? 0x20 : 0;
  out |= in & 0x02 ? 0x40 : 0;
  out |= in & 0x01 ? 0x80 : 0;
  return out;
}


//BUG: slight miss on width parameter
void menu_flipArea(uint8_t line, uint8_t x0, uint8_t width, uint8_t flip) {
  //Flips a part of the given line both horizontally and vertically
  //Flip sets the behaviour.
  // b00  - Does nothing
  // b01 - flips horizontally
  // b10 - flips vertically
  // b11 - flips both (rotate 180)
  uint8_t i, tmp, x1;

  if (flip & 1) {
    //Flip horizontally
    x1 = x0 + width;
    for (i = x0; i <= x1; i++) {
      tmp = frame.buff[line][i];
      frame.buff[line][i] = frame.buff[line][x1];
      frame.buff[line][x1] = tmp;

      tmp = frame.buff[line][i+64];
      frame.buff[line][i+64] = frame.buff[line][x1+64];
      frame.buff[line][x1+64] = tmp;
      x1--;
    }
  }
  if (flip & 2) {
    //Flip vertically
    x1 = x0 + width;
    for (i = x0; i <= x1; i++) {
      frame.buff[line][i] = menu_flipByte(frame.buff[line][i]);
      frame.buff[line][i+64] = menu_flipByte(frame.buff[line][i+64]);
    }
  }
}

//Have this accept a functionpointer which should pick what to display once a player joins?
//Could be useful for showing a player's colours/name?
void menu_playerStart(uint8_t maxPlayers) {
  //Asks active players to press start, then sets the player[] accordingly.
  uint8_t countDown, i;
  playerCnt = 0; countDown = 80;
  memset(player, 0, 4);
  frame.clear();
  frame.resetButtons();

  //Wait for players to join.
  while(countDown && playerCnt < maxPlayers) {
    frame.clear();
    if (playerCnt > 0) {
      //Player joined, start timeout counter
      countDown--;
      frame.gotoXY(30, 3);
      frame.print('0'+(countDown>>4), RED);
      menu_flipArea(3, 30, 4, 3);
      frame.gotoXY(30, 4);
      frame.print('0'+(countDown>>4), GREEN);
    }

    delay(50);

    for (i = 0; i < 4; i++) {
      if (!player[i] && frame.getStart(i)) {
        //This player pressed start, make player active
        player[i] = 1;
        playerCnt++;
        if (playerCnt == maxPlayers) break; //max players reached
      }

      if (i > 1) {
        frame.gotoXY(7, 2+(2-i));
      } else {
        frame.gotoXY(6, 5+i);
      }
      frame.print('P', GREEN);
      frame.print('1'+i);
      frame.print(": ");
      
      if (i < 2) {
        frame.gotoXY(29, 5+i);
      } else {
        frame.gotoXY(29, 2+(2-i));
      }
      if (!player[i]) {
        if (idle_counter & 16) {
          frame.print("Press", ORANGE);
        } else {
          frame.print("Start", ORANGE);
        }
      } else {
        frame.print("+++++", i < 2 ? GREEN : RED);
      }
      if (i > 1) {
        menu_flipArea(2+(2-i), 7, 50, 3);
      }
    }
    idle_counter++;
    frame.update();
  }
}


void menu_Btns() {
  frame.gotoXY(1, 7);
  if (*menu_run[curr] && (idle_counter & 16)) {
    frame.print("Press", GREEN);
    frame.gotoXY(34, 7);
    frame.print("start");
  } else {
    frame.clear(62);
  }
  if (curr != 0) {
    //Display leftarrow if there's a program there
    frame.blit(arrowL, 1, 28);
  }
  if (curr != PROGRAMCOUNT-1) {
    //Display rightarrow if there's a program there
    frame.blit(arrowR, 58, 28);
  }
}


void menu_showOption() {
  (*menu_option[curr])();
}


void loop() {
  uint8_t dir = frame.getDir(PLAYER1);
  if (dir == WEST && (curr != 0)) {
    //Display previous menu item
    curr--;
    idle_counter = IDLE_START_COUNT;
    frame.resetButtons();
    menu_showOption();

  } else if (dir == EAST && (curr != PROGRAMCOUNT-1)) {
    //Display next menu item
    curr++;
    idle_counter = IDLE_START_COUNT;
    frame.resetButtons();
    menu_showOption();

  } else if (frame.getStart(PLAYER1)) {
    if (*menu_run[curr]) {
      Serial.println("start");
      (*menu_run[curr])(); //Run program
      frame.clear();
      menu_showOption();
      frame.resetButtons();
      idle_counter = IDLE_START_COUNT;
    }
  } else {
    //No valid user action this timestep. Give control to current program's
    //idle action for a bit.
    idle_counter++;
    if (*menu_idle[curr]) {
      (*menu_idle[curr])(idle_counter);
    }
  }
  menu_Btns();
  frame.update();
  delay(50);
}
