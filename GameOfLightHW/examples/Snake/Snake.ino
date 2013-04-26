/*
  Snake, a game for the GameOfLight libraries
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

#include <MsTimer2.h>
#include <SPI.h>
#include <Controller.h>
#include <GameOfLightHW.h>
GameOfLightHW frame;
Controller SNES;

#define MAXSEGMENTS 127 //maximum number of segments in a snake
#define SPEEUP_FACTOR 6 //The nr of ms framedelay to remove per food eaten
#define PLAYER1 0
#define PLAYER2 1

uint8_t snakeX[2][MAXSEGMENTS+1]; //all zero
uint8_t snakeY[2][MAXSEGMENTS+1];
uint8_t snake_head[2]; //position of head in the buffer
#define snake_tail MAXSEGMENTS

uint8_t snake_count[2]; //nr of segments
uint8_t snake_headPosX[2]; //position of head on the board
uint8_t snake_headPosY[2];

volatile int8_t snake_dirX[2];
volatile int8_t snake_dirY[2];
	
uint8_t foodX;
uint8_t foodY;

uint8_t delayT; // time between each frame of the game
uint8_t rand_8();

//Xorshift psuedorandomizer variables. Values from http://en.wikipedia.org/wiki/Xorshift truncated to 8-bits.
uint8_t randX = 21;
uint8_t randY = 229;
uint8_t randZ = 181;
uint8_t randW = 51;


uint8_t rand_8() {
	uint8_t tmp = randX ^ (randX << 3);
	randX = randY;
	randY = randZ;
	randZ = randW;
	randW ^= (randW >> 2) ^ (tmp ^ (tmp >> 3));

	return randW;
}


void snake_drawBorder() {
	frame.gotoXY(0, 0);
	//Line 0
	frame.setColour(ORANGE);
	frame.write(0xFF);
	for (uint8_t i = 1; i < 31; i++) {
		frame.write(0x01);
	}
	frame.write(0xFF);

	//Line 1
	frame.gotoXY(0, 1);
	frame.write(0xFF);
	for (uint8_t i = 1; i < 31; i++) {
		frame.write(0x80);
	}
	frame.write(0xFF);
}



void snake_addSeg(const uint8_t player) {
	if (snake_head[player] == 0) return; //Buffer full!
	snake_head[player]--;
	
	snakeX[player][snake_head[player]] = snake_headPosX[player];
	snakeY[player][snake_head[player]] = snake_headPosY[player];
	frame.setPixel(snake_headPosX[player], snake_headPosY[player], player+1);
	snake_count[player]++;
}


void snake_buttonHandler(uint8_t player) {
	uint8_t dir;
	dir = SNES.getDir(player);
	
	if (dir != 4) {
		snake_dirX[player] = dir < 2 ? 0 : (dir == 2 ? 1 : -1);
		snake_dirY[player] = dir > 1 ? 0 : (dir == 0 ? -1 : 1);
	}	
}


void snake_buttonUpdate() {
	//This function automatically tracks the users last directionchange since the last frame
	//Called by timer2 every 1 ms to sample the buttons.
	SNES.getButtons();
	snake_buttonHandler(PLAYER1);
	snake_buttonHandler(PLAYER2);
}


uint8_t snake_checkCollision(uint8_t player) {
	if (frame.getPixel(snake_headPosX[player], snake_headPosY[player])) {
		//Checks new head location for collision
		return 1;
	}
	return 0;
}


void snake_newFood() {
	do {
		foodX = rand_8()>>3; //max 32
		foodY = rand_8()>>4; //max 16
		
	} while (frame.getPixel(foodX, foodY));
	delayT -= SPEEUP_FACTOR; //speed up game
	frame.setPixel(foodX, foodY, ORANGE);
}


uint8_t snake_move(uint8_t player) {
	//snake_buttonHandler(player); //grab currently pressed buttons and set desired direction
	
	//Update with the new direction (if changed)
	snake_headPosX[player] += snake_dirX[player];//dirX[snake_dir[player]];
	snake_headPosY[player] += snake_dirY[player];//dirY[snake_dir[player]];
	
	if (snake_headPosX[player] == foodX && snake_headPosY[player] == foodY) {
		snake_newFood();
		//has eaten, update head, keep rest of snake stationary
		snake_addSeg(player);
	} else if (snake_checkCollision(player)) {
		//Dead snake
		return 0;
	} else {
		//Blank the pixel containing the tailsegment
		frame.setPixel(snakeX[player][snake_tail],snakeY[player][snake_tail],0);
		
		//Move entire snake one step forward
		for (uint8_t i = 0; i < snake_count[player]-1; i++) {
			snakeX[player][snake_tail-i] = snakeX[player][snake_tail-i-1];
			snakeY[player][snake_tail-i] = snakeY[player][snake_tail-i-1];			
		}
		snakeX[player][snake_head[player]] = snake_headPosX[player];
		snakeY[player][snake_head[player]] = snake_headPosY[player];
		frame.setPixel(snake_headPosX[player], snake_headPosY[player], player+1);
	}
	//remember to redraw!
	return 1;	
}


void snake_reDraw(uint8_t player) {
	//redraws snake body from memory
	uint8_t x, y;
	for (uint8_t i = 0; i < snake_count[player]; i++) {
		x = snakeX[player][MAXSEGMENTS-i];
		y = snakeY[player][MAXSEGMENTS-i];
		frame.setPixel(x, y, player+1);
	}
}


uint8_t isStartPressed() {
	SNES.getButtons();

	if (SNES.getStart(PLAYER1)) {
		return 1;
	}
	return 0;
}


void snake_newGame() {
	//reset snake body
	
	//Player 1 body init:
	snake_headPosX[PLAYER1] = 7;
	snake_headPosY[PLAYER1] = 7;
	snakeX[PLAYER1][MAXSEGMENTS-2] = snake_headPosX[PLAYER1];
	snakeX[PLAYER1][MAXSEGMENTS-1] = snake_headPosX[PLAYER1]-1;
	snakeX[PLAYER1][MAXSEGMENTS] = snake_headPosX[PLAYER1]-2;
	snakeY[PLAYER1][MAXSEGMENTS-2] = snake_headPosY[PLAYER1];
	snakeY[PLAYER1][MAXSEGMENTS-1] = snake_headPosY[PLAYER1];
	snakeY[PLAYER1][MAXSEGMENTS] = snake_headPosY[PLAYER1];
	snake_head[PLAYER1] = MAXSEGMENTS - 2;
	snake_count[PLAYER1] = 3;
	snake_dirX[PLAYER1] = 1;
	snake_dirY[PLAYER1] = 0;
	
	//Player 2 body init:
	snake_headPosX[PLAYER2] = 24;
	snake_headPosY[PLAYER2] = 8;
	snakeX[PLAYER2][MAXSEGMENTS-2] = snake_headPosX[PLAYER2];
	snakeX[PLAYER2][MAXSEGMENTS-1] = snake_headPosX[PLAYER2]+1;
	snakeX[PLAYER2][MAXSEGMENTS] = snake_headPosX[PLAYER2]+2;
	snakeY[PLAYER2][MAXSEGMENTS-2] = snake_headPosY[PLAYER2];
	snakeY[PLAYER2][MAXSEGMENTS-1] = snake_headPosY[PLAYER2];
	snakeY[PLAYER2][MAXSEGMENTS] = snake_headPosY[PLAYER2];
	snake_head[PLAYER2] = MAXSEGMENTS - 2;
	snake_count[PLAYER2] = 3;
	snake_dirX[PLAYER2] = -1;
	snake_dirY[PLAYER2] = 0;

		
	frame.clearDisplay();
	frame.gotoXY(1, 0);
	frame.print("Snake", GREEN);
	frame.update();
	delay(500);
	while(!isStartPressed()); //wait until players are ready
	frame.clear();
	
	snake_drawBorder(); //also clears the screen. (CURRENTLY. Change if more matrices are used!)
	snake_reDraw(PLAYER1);
	snake_reDraw(PLAYER2);
	snake_newFood();
	
	frame.update();
	delay(2500);
}


void setup() {
	frame.begin();

	frame.gotoXY(1, 0);
	frame.print("Game", RED);
	frame.gotoXY(26, 0);
	frame.print("o", ORANGE);
	frame.gotoXY(1, 1);
	frame.print("Light", GREEN);

	frame.update();
	delay(2000);
	while(!isStartPressed()); //HOLD until startbutton is pressed!
	MsTimer2::set(1, snake_buttonUpdate);
	MsTimer2::start();
}


void loop() {
	uint8_t p1_alive, p2_alive, gameOn;
	char buff[5]; //Number to ascii buffer
	snake_newGame();
	gameOn = 1;
	delayT = 150;
	
	while(gameOn) {
		//Main game loop
		MsTimer2::stop();
		p1_alive = snake_move(PLAYER1);
//		p2_alive = snake_move(PLAYER2); //Player 2 stationary
		p2_alive = 1;
		//snake_dir[PLAYER1] = 4;
		//snake_dir[PLAYER2] = 4;
		MsTimer2::start();
		
		//p1 can reserve a square first, must check if we had a head-on collision
		if (!p2_alive && snake_headPosX[PLAYER1] == snake_headPosX[PLAYER2] && snake_headPosY[PLAYER1] == snake_headPosY[PLAYER2]) {
			p1_alive = 0;
		}
		
		if (!p1_alive || !p2_alive) {
			gameOn = 0;
			break;
		}
		frame.update();
		delay(delayT); //The timer2 interrrupt will ensure we still sample the player's D-pad while waiting
	}
	delay(3000);

	MsTimer2::stop();
	while(!isStartPressed()) {
		//Display result until both players press both buttons
		frame.clearDisplay();
		frame.gotoXY(7, 0);
		if (p1_alive == p2_alive) {
			//Both are dead		
			if (snake_count[PLAYER1] > snake_count[PLAYER2]) {
				frame.print("WIN", GREEN);	
			} else if (snake_count[PLAYER1] < snake_count[PLAYER2]) {
				frame.print("WIN", RED);
			} else {
				frame.gotoXY(5, 0);
				frame.print("DRAW", ORANGE);
			}
		} else if (p1_alive) {
			frame.print("WIN", GREEN);
		} else {
			frame.print("WIN", RED);
		}
		frame.update();
		if (isStartPressed()) {
			//Exit the scoreboard if the startbutton is held early
			break;
		}
		delay(1000);
		
		frame.clearDisplay();
		//display scores:
		itoa(snake_count[PLAYER1]-3, buff, 10);
		frame.gotoXY(1, 0);
		frame.print("P:", GREEN);
		frame.print(buff, GREEN);
		
		itoa(snake_count[PLAYER2]-3, buff, 10);
		frame.gotoXY(1, 1);
		frame.print("P:", RED);
		frame.print(buff, RED);
		
		frame.update();
		if (isStartPressed()) {
			//Exit the scoreboard if the startbutton is held early
			break;
		}
		delay(1000);

	}
	MsTimer2::start();
}
