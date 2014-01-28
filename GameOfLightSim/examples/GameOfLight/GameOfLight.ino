/*
  GameOfLight - the main menu for the GameOfLight project
  Copyright (c) 2013-2014 Stian Selbek.  All right reserved.

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
GameOfLightSim frame;

#define GOL_CONTROLS_NOINVERT; //Turn off control inversion on player 3 and 4
#define PROGRAMCOUNT 9
#define IDLE_START_COUNT 0

extern uint8_t rand_8();
extern int __bss_end;
extern int *__brkval;

int8_t player[4]; // -1 if not playing, otherwise filled up to playerCnt with playernumbers
uint8_t playerCnt;

uint8_t PROGMEM arrowL[] = {0x18, 0x0, 0x3c, 0x0, 0x7e, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
uint8_t PROGMEM arrowR[] = {0xff, 0x0, 0xff, 0x0, 0x7e, 0x0, 0x3c, 0x0, 0x18, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

uint8_t idle_counter = IDLE_START_COUNT;
uint8_t curr = 0;

void (*menu_option[PROGRAMCOUNT])(); //Draw splashscreen of current item
void (*menu_idle[PROGRAMCOUNT])(uint8_t); //If inactive on a menu item; run this every frame
void (*menu_run[PROGRAMCOUNT])(); //If 'start' is pressed, run the current menu item

void setup() {
    menu_option[0] = snake_splash;
    menu_idle[0] = snake_idle;
    menu_run[0] = snake_run;

    menu_option[1] = FR_splash;
    menu_idle[1] = FR_idle;
    menu_run[1] = FR_run;

    menu_option[2] = tetris_splash;
    menu_idle[2] = tetris_idle;
    menu_run[2] = tetris_run;

    menu_option[3] = langton_splash;
    menu_idle[3] = 0;
    menu_run[3] = langton_run;

    menu_option[4] = gameOfLife_splash;
    menu_idle[4] = 0;
    menu_run[4] = gameSetup;

    menu_option[5] = plasma_splash;
    menu_idle[5] = plasma_idle;
    menu_run[5] = plasma_run;

    menu_option[6] = brain_splash;
    menu_idle[6] = brain_idle;
    menu_run[6] = brain_run;

    menu_option[7] = countdown_splash;
    menu_idle[7] = countdown_idle;
    menu_run[7] = countdown_run;

    menu_option[8] = about_splash;
    menu_idle[8] = 0;
    menu_run[8] = about_run;

    frame.begin();
    Serial.println(freeRam());
    (*menu_option[0])();
    frame.resetButtons();
}


int freeRam() {
  //Fetches the amount of available ram. Useful for debugging
  int free_memory;

  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}


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



//Assigns the players to the slots of the player[] in the order they press the start button.
// Attempts to do so without doing undue damage to the currently displayed image.
//  Will only clear around the actual prompts.
//Displays a player join prompt at the coordinates given:
// player_x[] - x coordinates of the prompts for the (up to) 4 players
// player_y[] - y coordinates of the prompts for the (up to) 4 players
//Once minPlayers has been reached the unjoined player's prompts change to a countdown
//The max number of players is specified by leaving the last few slots of player_x as -1.
//A "press start"-prompt is displayed centered on the y-coordinates specified in 'press_start_y'
// This prompt can be disabled by setting this parameter to -1.
void menu_playerStart(uint8_t minPlayers, int8_t press_start_y, const int8_t player_x[4], const int8_t player_y[4]) {
  uint8_t maxPlayers, i;
  uint8_t countDown = 80;
  uint8_t player_joined = 0;
  memset(player, -1, 4);
  playerCnt = 0;
  idle_counter = 0;

  //Find the number of coordinates specified and hence the max number of players
  for (i = 0; i < 4; i++) {
    if (player_x[i] == -1) {
      //maxPlayers = i;
      break;
    }
  }
  maxPlayers = i;

  frame.resetButtons(); //Remove any unused startbutton presses from all controllers

  //Wait for players to join  
  while((playerCnt < minPlayers || countDown) && playerCnt < maxPlayers) {
    for (i = 0; i < 4; i++) {
      if (frame.getStart(i) && !(player_joined & (1 << i))) {
        //Player i reserved spot playerCnt
        player[playerCnt] = i;
        frame.gotoXY(player_x[playerCnt], player_y[playerCnt]);
        frame.clear(11);
        frame.print("P", GREEN);
        frame.print('1'+i, GREEN);

        playerCnt++;
        player_joined |= (1 << i);
        if (playerCnt == maxPlayers) {
          break;
        }
      }
    }

    //Add countdown timers if applicable
    if (playerCnt >= minPlayers) {
      countDown--;
      for (int i = playerCnt; i < maxPlayers; i++) {
        //Enough players reached, display countdown for the remaining spots
        frame.gotoXY(player_x[i], player_y[i]);
        frame.clear(11);
        frame.print(' ');
        frame.print('0'+(countDown>>4), RED);
      }
    } else if ((idle_counter & 0x07) == 0) {
      //Epilleptic-fit display function ... (adds ?? to each unused slot on the screen in various colours)
      for (i = playerCnt; i < maxPlayers; i++) {
        frame.gotoXY(player_x[i], player_y[i]);
        frame.clear(11);
        frame.print("??", (rand_8() % 3) + 1);
      }
    }

    //Blinking press start prompt
    if (press_start_y != -1 && (idle_counter & 0x0F) == 0) {
      frame.gotoXY(17, press_start_y);
      frame.clear(30);
      if (idle_counter & 0x10) {
        frame.print("Start", (rand_8() % 3) + 1);
      } else {
        frame.print("Press", (rand_8() % 3) + 1);
      }
    }
    idle_counter++;
    delay(50);
    frame.update();
  }

  //Clear the start prompt line and the unused playerslot locations (if any)
  if (press_start_y != -1) {
    frame.gotoXY(17, press_start_y);
    frame.clear(30);
  }
  for (i = playerCnt; i < maxPlayers; i++) { //Unused player slots
    frame.gotoXY(player_x[i], player_y[i]);
    frame.clear(11);
  }
  //Display the final positions of each joined player for 1 second:
  frame.update();
  delay(1000);

  //Remove the player prompts as well:
  for (i = 0; i < playerCnt; i++) {
    frame.gotoXY(player_x[i], player_y[i]);
    frame.clear(11);
  }
  frame.update();
}


void menu_Btns() {
  frame.gotoXY(1, 56);
  if (*menu_run[curr] && (idle_counter & 16)) {
    frame.print("Press", GREEN);
    frame.gotoXY(34, 56);
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
    delay(250);
    frame.resetButtons();
    menu_showOption();

  } else if (dir == EAST && (curr != PROGRAMCOUNT-1)) {
    //Display next menu item
    curr++;
    idle_counter = IDLE_START_COUNT;
    delay(250);
    frame.resetButtons();
    menu_showOption();

  } else if (frame.getStart(PLAYER1)) {
    if (*menu_run[curr]) {
      Serial.println("start");
      frame.resetButtons(); //Remove any lingering unused button presses
      (*menu_run[curr])(); //Run program
      frame.clear();
      frame.setFont(FONT_5x7);
      menu_showOption();
      frame.update();
      delay(1000);
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

  //DEBUG - Displays available ram.
  /*char buff[8];
  int free;
  free = freeRam();
  itoa(free, buff, 10);
  frame.gotoXY(0,0);
  frame.clear(40);
  frame.print(buff, ORANGE);*/

  menu_Btns();
  frame.update();
  delay(50);
}

