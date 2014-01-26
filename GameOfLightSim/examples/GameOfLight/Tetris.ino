/*
  Tetris - a programexample for the GameOfLight libraries
  Copyright (c) 2014 Stian Selbek.  All right reserved.

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


/*
* Controls:
* LEFT/RIGHT       - move left/right
* UP               - drops the piece into the indicated position. (no lock delay)
* DOWN             - increase gravity (lock delay)
* Shoulder buttons - rotate the piece
*
* Max players 3, minimum 1.
* Play areas are assigned in the following order: left, right, middle. Based on who pressed their startbutton first
*
* Score:
* Clearing 1, 2, 3 or 4 rows gives 1, 3, 5 and 8 points respectivly. 
* No bonus based on level or difficult rotations.
* Points are tracked up to 65,535, but only the first 999 are displayed. +++ displayed for scores above this.
*
* The game speeds up automatically for every 10 rows cleared. Max level 20.
*/
#include <GameOfLightSim.h>
#include <avr/pgmspace.h>

extern GameOfLightSim frame;
extern int8_t player[4]; //must call menu_playerStart for valid content
extern uint8_t playerCnt; //must call menu_playerStart for valid count
extern void menu_playerStart(uint8_t minPlayers, int8_t press_start_line, const int8_t player_x[4], const int8_t player_line[4]);
extern uint8_t rand_8(); //from Snake.ino

#define TETRIS_NORMAL 0
#define TETRIS_DEAD 1
#define TETRIS_GRACEPERIOD 2
#define TETRIS_GRACEPERIOD_IMMUNE 3
#define TETRIS_LOCK_DELAY 4
#define TETRIS_LOCKED 5
#define TETRIS_CLEARLINE 6 //This has to be the last state as any state above it is counted as a linenumber for clearing purposes

#define TETRIS_SPAWNHEIGHT 18

//TODO:
//Fix bug occuring when the game board is full. A new piece is redrawn on top of old content


struct piece {
  uint8_t x, y;
  uint8_t ghost_x, ghost_y; //Where did we last draw the indicator of where this piece will land?
  uint8_t type;
  uint8_t rotation;
};

//The pieces I, J, L, O, S, T, Z represented as coordinates of each of their 4 rotation frames and 4 parts
int8_t PROGMEM tetris_pieces[][4][4][2] = {{{{-2, 0}, {0, 0}, {2, 0}, {4, 0}}, {{2, 4}, {2, 2}, {2, 0}, {2, -2}}, {{-2, 2}, {0, 2}, {2, 2}, {4, 2}}, {{0, 4}, {0, 2}, {0, 0}, {0, -2}}},
{{{-2, 0}, {0, 0}, {2, 0}, {-2, -2}}, {{0, 2}, {0, 0}, {0, -2}, {2, -2}}, {{2, 2}, {-2, 0}, {0, 0}, {2, 0}}, {{-2, 2}, {0, 2}, {0, 0}, {0, -2}}},
{{{-2, 0}, {0, 0}, {2, 0}, {2, -2}}, {{0, 2}, {2, 2}, {0, 0}, {0, -2}}, {{-2, 2}, {-2, 0}, {0, 0}, {2, 0}}, {{0, 2}, {0, 0}, {-2, -2}, {0, -2}}},
{{{0, 0}, {2, 0}, {0, -2}, {2, -2}}, {{0, 0}, {2, 0}, {0, -2}, {2, -2}}, {{0, 0}, {2, 0}, {0, -2}, {2, -2}}, {{0, 0}, {2, 0}, {0, -2}, {2, -2}}},
{{{-2, 0}, {0, 0}, {0, -2}, {2, -2}}, {{2, 2}, {0, 0}, {2, 0}, {0, -2}}, {{-2, 2}, {0, 2}, {0, 0}, {2, 0}}, {{0, 2}, {0, 0}, {-2, 0}, {-2, -2}}},
{{{-2, 0}, {0, 0}, {2, 0}, {0, -2}}, {{0, 2}, {0, 0}, {2, 0}, {0, -2}}, {{0, 2}, {0, 0}, {2, 0}, {-2, 0}}, {{0, 2}, {-2, 0}, {0, 0}, {0, -2}}},
{{{0, 0}, {2, 0}, {-2, -2}, {0, -2}}, {{0, 2}, {0, 0}, {2, 0}, {2, -2}}, {{0, 2}, {2, 2}, {-2, 0}, {0, 0}}, {{-2, 2}, {-2, 0}, {0, 0}, {0, -2}}}};

uint8_t PROGMEM tetris_colours[] = {RED, GREEN, ORANGE, ORANGE, GREEN, RED, RED};

const uint8_t tetris_bounds[3][2] = {{1, 21},{43, 63},{22, 42}};

//Contains 1 byte per player with bits 0-6 indicating which of the 7 pieces have recently been generated.
//Marked bits prevent that piece from generating. Once all 7 pieces have been set start anew with a zeroed byte.
uint8_t tetris_pieceBag[3];
uint8_t tetris_boundsCheckDisable; //Set to 1 to allow drawing of tetrispieces outside the play area


void tetris_splash() {
  frame.clear();

  frame.gotoXY(14, 24);
  frame.print("TETRIS", RED);

  menu_flipArea(3, 32, 4, 0b01); //Turn the R into a cyrillic Ya. A missuse of letters and tribute to the original game :)

  //The hard way of adding an orange outline?
  for (uint8_t x = 13; x < 50; x++) {
    for (uint8_t y = 23; y < 32; y++) {
      uint8_t count = 0;
      if (frame.getPixel(x, y) == BLACK) {
        if (frame.getPixel(x-1, y-1) == RED) count++;
        if (frame.getPixel(x-1, y) == RED) count++;
        if (frame.getPixel(x-1, y+1) == RED) count++;
        if (frame.getPixel(x, y-1) == RED) count++;
        if (frame.getPixel(x, y) == RED) count++;
        if (frame.getPixel(x, y+1) == RED) count++;
        if (frame.getPixel(x+1, y-1) == RED) count++;
        if (frame.getPixel(x+1, y) == RED) count++;
        if (frame.getPixel(x+1, y+1) == RED) count++;
        if (count) frame.setPixel(x, y, ORANGE);
      }
    }
  }
  //Add some tetrominoes
  tetris_boundsCheckDisable = 1;
  tetris_drawPiece(22, 45, 2, 1); //L piece
  tetris_drawPiece(24, 41, 4, 3); //S piece
  tetris_drawPiece(26, 45, 6, 2); //Z piece
  tetris_drawPiece(30, 41, 5, 2); //T piece
  tetris_drawPiece(34, 45, 1, 3); //J piece
  tetris_drawPiece(34, 43, 0, 1); //I piece
  tetris_drawPiece(38, 47, 3, 0); //O piece

  for (int i = 34; i < 50; i+=2) {
    frame.setPixel(21, i, ORANGE);
    frame.setPixel(42, i, ORANGE);
  }
  for (int i = 21; i < 42; i+=2) {
    frame.setPixel(i, 49, ORANGE); 
  }
}


void tetris_idle(uint8_t counter) {
  //Do occasional random rotations on the hanging T piece
  if ((counter & 0x7) == 0 && rand_8() > 150) {
    uint8_t rot, prev_rot;

    //Extract previous rotation:
    if (!frame.getPixel(30, 43)) prev_rot = 0;
    else if (!frame.getPixel(28, 41)) prev_rot = 1;
    else if (!frame.getPixel(30, 39)) prev_rot = 2;
    else prev_rot = 3;

    //Clear current piece.
    tetris_clearPiece(30, 41, 5, prev_rot);
    
    //Determine if we're going to rotate left or right
    rot = rand_8() >> 7;
    if (rot & 1) rot = prev_rot + 1;
    else rot = prev_rot - 1;
    
    rot &= 0x03; //range 0-3

    //Re-add the piece with its new rotation
    tetris_drawPiece(30, 41, 5, rot);
  }  
}


uint8_t tetris_checkLocation(uint8_t p, uint8_t piece, uint8_t new_X, uint8_t new_Y, uint8_t new_rot) {
  int8_t xmod, ymod;

  //Check availability
  for (int px = 0; px < 4; px++) {
    //Check each of the 4 pixels that make up a piece to make sure it's not taken
    //Since each pixel is 2x2 we simplify and check just one corner
    xmod = pgm_read_byte(&tetris_pieces[piece][new_rot][px][0]);
    ymod = pgm_read_byte(&tetris_pieces[piece][new_rot][px][1]);
    if (new_Y + ymod > 22 && frame.getPixel(new_X + xmod, new_Y + ymod)) {
      //Taken!
      return 1;
    } else if (new_Y + ymod > 60 || new_X + xmod < tetris_bounds[p][0] || new_X + xmod >= tetris_bounds[p][1]) {
      //Out of bounds under screen, left of this playarea or right of this playarea
      return 2;
    }
  }
  return 0;
}


void tetris_getNextPiece(uint8_t p, uint8_t pieces_next[3], struct piece* active) {
  const uint8_t posLookup[] = {9, 51, 30};
  active->type = pieces_next[p];
  active->rotation = 0;
  active->x = posLookup[p];
  active->y = TETRIS_SPAWNHEIGHT;
  active->ghost_x = 0; //Since this isn't equal to active->x, will force a recalculation of ghost
  active->ghost_y = 0; //Set <22 so that we don't clear something as soon as the next piece starts

  //Gets and displays the next piece this player will get as well as returning the new current piece
  pieces_next[p] = tetris_randomPiece(p);

  //Clear the old piece from the playarea's next-piece-box
  tetris_boundsCheckDisable = 1;
  tetris_clearPiece(posLookup[p], 18, active->type, 0);
  //Display the new piece
  tetris_drawPiece(posLookup[p], 18, pieces_next[p], 0);
  tetris_boundsCheckDisable = 0;
}


void tetris_clearPiece(uint8_t x, uint8_t y, uint8_t piece, uint8_t rotation) {
  int8_t xmod, ymod;

  for (int px = 0; px < 4; px++) {
    xmod = pgm_read_byte(&tetris_pieces[piece][rotation][px][0]);
    ymod = pgm_read_byte(&tetris_pieces[piece][rotation][px][1]);
    if (tetris_boundsCheckDisable || y+ymod >= 22) {
      //Clear only inside playarea unless the check is disabled
      frame.setPixel(x + xmod, y + ymod, BLACK);
      frame.setPixel(x + xmod + 1, y + ymod, BLACK);
      frame.setPixel(x + xmod, y + ymod + 1, BLACK);
      frame.setPixel(x + xmod + 1, y + ymod + 1, BLACK);
    }
  }
}



void tetris_drawPiece(uint8_t x, uint8_t y, uint8_t piece, uint8_t rotation) {
  int8_t xmod, ymod;
  uint8_t colour;
  colour = pgm_read_byte(&tetris_colours[piece]);

  //Draw the piece:
  for (int px = 0; px < 4; px++) {
    xmod = pgm_read_byte(&tetris_pieces[piece][rotation][px][0]);
    ymod = pgm_read_byte(&tetris_pieces[piece][rotation][px][1]);
    if (tetris_boundsCheckDisable || y+ymod >= 22) {
      //Draw only inside playarea unless the check is disabled
      frame.setPixel(x + xmod, y + ymod, colour);
      frame.setPixel(x + xmod + 1, y + ymod, colour);
      frame.setPixel(x + xmod, y + ymod + 1, colour);
      frame.setPixel(x + xmod + 1, y + ymod + 1, colour);
    }
  }
}


void tetris_drawGhostPiece(uint8_t p, struct piece* active, uint8_t hasRotated) {
  if ((active->x != active->ghost_x) || hasRotated) {
    //The ghost only moves if the player has shifted left or right or if a rotation has been executed
    uint8_t offset = 2;
    while (tetris_checkLocation(p, active->type, active->x, active->y + offset, active->rotation) == 0) {
      //Move one square down and repeat until we hit something
      offset += 2;
    }
    active->ghost_y = active->y + offset - 2;
    active->ghost_x = active->x;
  }

  if (active->ghost_y != active->y) {
    int8_t xmod, ymod;
    uint8_t colour = pgm_read_byte(&tetris_colours[active->type]);
    //Redraw the dotted outline of where this piece will land from it's current position
    for (int px = 0; px < 4; px++) {
      xmod = pgm_read_byte(&tetris_pieces[active->type][active->rotation][px][0]);
      ymod = pgm_read_byte(&tetris_pieces[active->type][active->rotation][px][1]);
      if (active->ghost_y+ymod >= 22) {
        //Only half the pixels are filled in allowing the user to discern the ghost from the real piece
        frame.setPixel(active->ghost_x + xmod, active->ghost_y + ymod, colour);
        frame.setPixel(active->ghost_x + xmod + 1, active->ghost_y + ymod + 1, colour);
      }
    }
  }
}


void tetris_clearGhostPiece(struct piece* active) {
  //Clear the ghost outline showing where this piece will end up if it fell straight down
  tetris_clearPiece(active->ghost_x, active->ghost_y, active->type, active->rotation);
}


uint8_t tetris_randomPiece(uint8_t player) {
  //Fetches a new random-ish piece for this player. To be placed in the next piece area.
  //Ensures fairness by having each piece present once per 7 piece window.
  uint8_t ran;
  if (tetris_pieceBag[player] == 0x7F) {
    //Sequence complete. Start a new sequence
    tetris_pieceBag[player] = 0;
  }

  //Pick a random piece of the ones remaining in the sequence. (Can take a few attempts)
  do {
    ran = rand_8() >> 5; //Get an equally distributed number 0-7
  } while (ran == 7 || (tetris_pieceBag[player] & (1 << ran))); //Keep if 0-6 and not used
  //Set used:
  tetris_pieceBag[player] |= (1 << ran);
  return ran;
}


uint8_t tetris_move(uint8_t p, struct piece* active, uint8_t dir) {
  if (dir == WEST) {
    if (tetris_checkLocation(p, active->type, active->x - 2, active->y, active->rotation) == 0) {
      active->x -= 2;
      return 1; //Success!
    }
  } else if (dir == EAST) {
    if (tetris_checkLocation(p, active->type, active->x + 2, active->y, active->rotation) == 0) {
      active->x += 2;
      return 1; //Success! 
    }
  } else if (dir == SOUTH) {
    //Gravity
    if (tetris_checkLocation(p, active->type, active->x, active->y + 2, active->rotation) == 0) {
      active->y += 2;
      return 1; //Success! 
    }
  }
  return 0; //No (legal) move executed
}


uint8_t tetris_kickPiece(uint8_t p, struct piece* active) {
  //If current location is illegal, will check to the right then left.
  // This allows rotation when near a wall

  if (tetris_checkLocation(p, active->type, active->x, active->y, active->rotation) == 0) {
    return 1;
  } else if (tetris_checkLocation(p, active->type, active->x + 2, active->y, active->rotation) == 0) {
    active->x += 2;
    return 1;
  } else if (tetris_checkLocation(p, active->type, active->x - 2, active->y, active->rotation) == 0) {
    active->x -= 2;
    return 1;
  } //The I-piece is 4 long with the centre offset. Add a special longer kick for it in rotation 0 and 2

  return 0; //Illegal rotation, can't easily kick into a legal one either
}


uint8_t tetris_rotatePiece(uint8_t p, struct piece* active) {
  uint8_t oldRotation = active->rotation;

  if (frame.getL(player[p])) {
    //Rotate piece left
    active->rotation = (active->rotation - 1) & 0x03;
    if (tetris_kickPiece(p, active)) {
     //New legal position found. Keep new rotation.
      return 1;
    }
  } else if (frame.getR(player[p])) {
    //Rotate piece right
    active->rotation = (active->rotation + 1) & 0x03;
    if (tetris_kickPiece(p, active)) {
     //New legal position found. Keep new rotation.
      return 1;
    }
  }

  active->rotation = oldRotation;
  return 0;
}


uint8_t tetris_isLineFull(uint8_t p, uint8_t y) {  
  for (int x = tetris_bounds[p][0]; x < tetris_bounds[p][1]; x += 2) {
    if (!frame.getPixel(x, y)) {
      //Found an empty square. This line can't be full
      return 0;
    }
  }
  return 1;
}


uint8_t tetris_lineClearedCheck(uint8_t p, struct piece* active) {
  int8_t ymod, prev = -1;
  
  for (int px = 0; px < 4; px++) {
    if (px != prev) {
      prev = px;
      ymod = pgm_read_byte(&tetris_pieces[active->type][active->rotation][px][1]);
      
      if (tetris_isLineFull(p, active->y + ymod)) {
        //Line full!
        return active->y + ymod; //Save line coordinates
      }
    }
  }
  return 0;
}


uint8_t tetris_isWithinSpawnBox(struct piece* active) {
  //Checks if any part of the locked piece is above the play area
  // returns 1 if that is the case.
  int8_t ymod;

  for (int px = 0; px < 4; px++) {
    //Check the location of each of the 4 pixels that make up the piece
    //Since each pixel is 2x2 we simplify and check just one corner
    ymod = pgm_read_byte(&tetris_pieces[active->type][active->rotation][px][1]);
    if (active->y + ymod < 22) {
      //Ended up within the spawn area. Game over!
      return 1;
    }
  }
  return 0;
}


void tetris_drawBoard() {
  frame.clear();

  frame.gotoXY(14, 0);
  frame.print("TETRIS", RED);
  menu_flipArea(0, 32, 4, 0b01);

  //Draw dotted top and bottom lines:
  for (int i = 1; i < 64; i+=2) {
      frame.setPixel(i, 21, ORANGE);
      frame.setPixel(i, 62, ORANGE);
  }

  //Draw vertical dotted dividing lines:
  for (int i = 8; i < 64; i+=2) {
    frame.setPixel(0, i, ORANGE);
    frame.setPixel(21, i, ORANGE);
    frame.setPixel(42, i, ORANGE);
    frame.setPixel(63, i, ORANGE);
  }
  frame.update();
}


void tetris_drawLevelLine(uint8_t p, uint8_t progress) {
  //Draw the little line that tells the player how far it is to the next level
  frame.drawLine(tetris_bounds[p][0]+1, 63, tetris_bounds[p][1], 63, BLACK);
  if (progress) {
    //Draw 0-18 px green line below the playing field
    frame.drawLine(tetris_bounds[p][0]+1, 63, tetris_bounds[p][0] + progress*2, 63, GREEN);
  }
}


void tetris_drawScore(uint8_t p, uint16_t score) {
  //Score has apparently changed. Update the display!
  frame.gotoXY(tetris_bounds[p][0]+2, 8);
  frame.clear(17);
  if (score <= 999) {
    char buff[4];
    for (int8_t i = 2; i >= 0; i--) {
      buff[i] = (score % 10) + '0';
      score /= 10;      
    }
    buff[3] = 0;
    frame.print(buff, RED);
  } else {
    frame.print("+++", RED);
  }  
}


void tetris_newGame() {
  const int8_t x_coord[] = {5, 47, 26, -1};
  const int8_t y_coord[] = {40, 40, 40, -1};
  //Reset used pieces
  tetris_pieceBag[0] = 0;
  tetris_pieceBag[1] = 0;
  tetris_pieceBag[2] = 0;

  tetris_drawBoard();
  delay(1000);

  //Remove the "Tetris" print so that we may display the "press/start" print on that line:
  frame.gotoXY(14, 0);
  frame.clear(35);
  menu_playerStart(1, 0, x_coord, y_coord); //Fetch players
  tetris_drawBoard(); //Lost our Tetris title when we fetched the players. Redraw it.
}


void tetris() {
  uint8_t level[3]; //level/10 is current level. Max level 20. level%10 is steps to next level
  uint8_t rowsAtOnce[3]; //How many rows did the last action clear. Used for calculating score multiplier
  uint16_t scores[3];
  uint8_t pieces_next[3]; //just the type of the next piece
  struct piece pieces[3]; //The current piece of each (potential) player
  long int last_time[3];  //When did this player last get calculated?
  uint8_t player_state[3];//What is the state of that player's gameboard?
  uint8_t anyAlive; //Any remaining players?

  for (int i = 0; i < playerCnt; i++) {
    //generate the first pieces and display them in the next piece area
    pieces_next[i] = 0; //Has to be set to something or else we might clear an unintended part of the screen first time
    tetris_getNextPiece(i, pieces_next, &pieces[i]);
    last_time[i] = 0;
    level[i] = 140; //For testing purposes. Set to 0 later
    scores[i] = 0;
    player_state[i] = TETRIS_NORMAL;
    rowsAtOnce[i] = 0;
    tetris_drawScore(i, 0);
  }
  frame.update();
  delay(1500); //Allow the players to see their first piece for a little while

  //Move the pieces into play
  for (int i = 0; i < playerCnt; i++) {
    tetris_getNextPiece(i, pieces_next, &pieces[i]);
  }
  anyAlive = 1;

  //Main game loop
  do {
    uint8_t updated = 0;
    long int currTime = millis();
    for (int i = 0; i < playerCnt; i++) {
      if ((player_state[i] != TETRIS_DEAD) && (abs(last_time[i] - currTime) > (768 - (level[i]/10)*38))) {
        //Time for this player to make a move
        //Roughly 768ms per move at level 0, 46ms at level 19
        last_time[i] = currTime;
        updated = 1;
        if (player_state[i] == TETRIS_LOCKED) {
          //PIECE PLACED. CHECK IF IT MADE ANY LINES FULL

          player_state[i] = tetris_lineClearedCheck(i, &pieces[i]);
          //player_state[i] now contains a line number or is 0 if no clearable lines were found

          if (player_state[i] == 0) {
            //No lines left to clear. Let's move on!
            player_state[i] = TETRIS_NORMAL;
            tetris_getNextPiece(i, pieces_next, &pieces[i]);

            if (rowsAtOnce[i]) {
              //Has cleared atleast one line. Update score based on how many.
              switch(rowsAtOnce[i]) {
                //(Fallthrough intentional)
                case 4:
                  scores[i] += 3;
                case 3:
                  scores[i] += 2;
                case 2:
                  scores[i] += 2;
                case 1:
                  scores[i] += 1;
                  break;
              }
              tetris_drawScore(i, scores[i]);
              rowsAtOnce[i] = 0;
            }
          } else {
            for (uint8_t x = tetris_bounds[i][0]; x < tetris_bounds[i][1]; x++) {
              //CHANGE TO BYTEBASED later on for speed
              frame.setPixel(x, player_state[i], BLACK);
              frame.setPixel(x, player_state[i]+1, BLACK);
            }
            if (level[i] < 200) {
              level[i]++;
            }
            rowsAtOnce[i]++;
            tetris_drawLevelLine(i, level[i]%10);
          }

        } else if (player_state[i] >= TETRIS_CLEARLINE) {
          //REMOVAL OF FULL LINES

          //Line cleared. Coordinate in player_state[i]. Move anything above that down         
          //THE SLOOOOOW WAY OF DOING IT. Swap with a bytebased method later!
          for (uint8_t y = player_state[i]; y >= 24; y-=2) { //Each line
            for (uint8_t x = tetris_bounds[i][0]; x < tetris_bounds[i][1]-1; x+=2) {
              uint8_t px1 = frame.getPixel(x, y-2);
              frame.setPixel(x, y, px1);
              frame.setPixel(x, y+1, px1);
              frame.setPixel(x+1, y, px1);
              frame.setPixel(x+1, y+1, px1);
            }
          }
          for (uint8_t x = tetris_bounds[i][0]; x < tetris_bounds[i][1]; x++) {
            frame.setPixel(x, 22, BLACK);
            frame.setPixel(x, 23, BLACK);
          }
          player_state[i] = TETRIS_LOCKED;

        } else {
          //PLAYER INTERACTION

          uint8_t rotateSuccess, dir;

          //Clear current position and it's drawn ghost outline:
          tetris_clearPiece(pieces[i].x, pieces[i].y, pieces[i].type, pieces[i].rotation);
          tetris_clearGhostPiece(&pieces[i]);

          //Translate piece left/right
          dir = frame.getDir(player[i]);//internal player number to controller number and get it's direction
          if (dir == NORTH) {
            //Hard drop!
            if (pieces[i].ghost_x != 0) {
              //Only do something if the ghost image has been calculated
              //This reduces accidental double-activation due to holding the key a smidge too long.

              //Hard drop => Warp piece to ghost location
              pieces[i].x = pieces[i].ghost_x;
              pieces[i].y = pieces[i].ghost_y;
              //Skip lock delay and jump straight to gravity locking the piece in place:
              player_state[i] = TETRIS_LOCK_DELAY;
            }

          } else {
            //Translate piece
            tetris_move(i, &pieces[i], dir);
            
            //Rotate piece if asked. Add left/right wall kick if needed to avoid the outer walls
            rotateSuccess = tetris_rotatePiece(i, &pieces[i]);

            if (player_state[i] == TETRIS_GRACEPERIOD_IMMUNE) {
              player_state[i] = TETRIS_NORMAL;
            } else if (rotateSuccess) {
              player_state[i] = TETRIS_GRACEPERIOD;
            }
          }
          
          if (player_state[i] == TETRIS_NORMAL || player_state[i] == TETRIS_LOCK_DELAY) {
            //Gravity
            if (tetris_move(i, &pieces[i], SOUTH)) {
              //Able to drop, reset lock delay
              player_state[i] = TETRIS_NORMAL;
            } else {
              //Piece blocked from falling
              if (player_state[i] == TETRIS_LOCK_DELAY) {
                //Unable to move for two frames, lock the piece!
                if (tetris_isWithinSpawnBox(&pieces[i])) {
                  //Piece locked within spawnbox. Game over!
                  player_state[i] = TETRIS_DEAD;
                  anyAlive = 0;
                  for (int p = 0; p < playerCnt; p++) {
                    //Check to see if there are any players left playing
                    if (player_state[p] != TETRIS_DEAD) {
                      anyAlive = 1;
                      break;
                    }
                  }

                } else {
                  //Piece locked. Check whether we cleared any lines next timestep
                  player_state[i] = TETRIS_LOCKED;
                }
              } else {
                player_state[i] = TETRIS_LOCK_DELAY;
              }
            }
          } else if (player_state[i] == TETRIS_GRACEPERIOD) {
            //Reset grace period
            player_state[i] = TETRIS_GRACEPERIOD_IMMUNE;
          }

          //Redraw ghost outline and the actual piece:
          tetris_drawGhostPiece(i, &pieces[i], rotateSuccess);
          tetris_drawPiece(pieces[i].x, pieces[i].y, pieces[i].type, pieces[i].rotation);

/*          Serial.print("Player ");
          Serial.print(player[i]);
          Serial.print(", in state: ");
          Serial.println(player_state[i]);*/
        }
      } //player step
    }
    if (updated) {
      frame.update();
    }
  } while(anyAlive);

  delay(2000);
}

void tetris_run() {

  //Seperated the fetching of players and the actual game into two functions
  // as otherwise we use too much ram of the little we have left on the UNO.
  tetris_newGame();
  tetris();
}