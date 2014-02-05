

#include "GameOfLightSim.h"
extern GameOfLightSim frame;
uint8_t expl_right[4][16] PROGMEM = {
  { 0x24, 0x7e, 0x24, 0x7e, 0x18, 0x3c, 0x0, 0x3c, 0xe0, 0xf0, 0xe0, 0xe0, 0x0, 0x0, 0x0, 0x0 },
  { 0x24, 0x7e, 0x24, 0x7e, 0x5a, 0x7e, 0x24, 0x3c, 0x18, 0x5a, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0 }, 
  { 0xbd, 0xff, 0x3c, 0x7e, 0x5a, 0x7e, 0x66, 0xff, 0x7e, 0x7e, 0x99, 0xbd, 0x0, 0x18, 0x0, 0x0 },
  { 0x81, 0xc3, 0x0, 0x42, 0x42, 0x66, 0x66, 0xff, 0x5a, 0x7e, 0xbd, 0xff, 0x99, 0xff, 0x66, 0x66 }
};

uint8_t expl_left[4][16] PROGMEM = {
  { 0x0, 0x0, 0xe0, 0xe0, 0xe0, 0xf0, 0x40, 0x40, 0x0, 0x3c, 0x58, 0x7c, 0x24, 0x7e, 0x24, 0x7e },
  { 0x0, 0x0, 0x0, 0x0, 0x0, 0x10, 0x18, 0x5a, 0x24, 0x3c, 0x5a, 0x7e, 0x24, 0x7e, 0xa5, 0xff}, 
  { 0x0, 0x0, 0x0, 0x18, 0x99, 0xbd, 0x7e, 0x7e, 0x66, 0xff, 0x5a, 0x7e, 0x3c, 0x7e, 0xbd, 0xff},
  { 0x66, 0x66, 0x99, 0xff, 0xbd, 0xff, 0x5a, 0x7e, 0x66, 0xff, 0x42, 0x66, 0x0, 0x42, 0x81, 0xc3 }
};

#define CAR_LEN 5
#define ALIVE 0
#define EXPLODING 1
#define DEAD 2
#define MAX_OB_CARS 10
#define MAX_OB_LORRIES 10
#define MIN_DELAY 50 //ms
#define PLAYER_OFFSET 35
#define FRAME_DELAY 40
#define OBSTACLE_DELAY 100

unsigned long start_time;
unsigned long cur_time;
unsigned long time_obstacles;
int num = 0;

struct Player {
  int8_t x;          //int because it shares draw-methods with obstacles
  uint8_t y;
  uint8_t color;
  uint8_t player;
  uint8_t life_status; //alive, exploding, dead
  uint8_t explosion_stage;
  unsigned int distance_travled;
  unsigned long last_move; //time of last move in millis
};
  
struct Obstacle_Car {
  int8_t x;           //int because it starts off-screen (negative value)
  uint8_t y;
  uint8_t cur_status1; //status for player 1
  uint8_t cur_status2; //status for player 2
};

struct Obstacle_Lorry {
  int8_t x;          //int because it starts off-screen (negative value)
  uint8_t y;
  uint8_t cur_status1; //status for player 1
  uint8_t cur_status2; //status for player 2
};

struct Player p1;
struct Player p2;

struct Obstacle_Car *ob_cars;
struct Obstacle_Lorry *ob_lorries;

/* Checks what keys player p is pushing */
void driver_check(struct Player *p) {
  if (p->life_status == EXPLODING) {
    driver_drawExplosion(p);
  }
  else if (p->life_status == ALIVE) {
    switch (frame.getDir(p->player)) {
      case WEST: //forward
        if (p->y < (p->player == PLAYER1 ? 23 : (23 + PLAYER_OFFSET))) {
          if (driver_playerCollision(p->x, p->y+4)) {
            driver_startExplosion(p);
          }
          p->y += 4; 
        } 
        break;
      case EAST: //backward
        if (p->y > (p->player == PLAYER1 ? 3 : (3 + PLAYER_OFFSET))) {
          if (driver_playerCollision(p->x, p->y-4)) {
            driver_startExplosion(p);
          } 
          p->y -= 4;
        } 
        break;
      case SOUTH: //down 
        if (p->x < 59) {
          if (driver_playerCollision(p->x+1, p->y)) {
            driver_startExplosion(p);
          } 
          p->x += 1;
        }
        break;
      case NORTH:  //up 
        if (p->x > 0) {
          if (driver_playerCollision(p->x-2, p->y)) {
            driver_startExplosion(p);
          } 
          p->x -= 2;
        }
        break;
      default: 
        break;  
    }
  } 
  p->last_move = cur_time;
}

void driver_run() {
  frame.clear();
  start_time = millis();
  time_obstacles = start_time;
  struct Player first = {48, 14, GREEN, PLAYER1, ALIVE, 0, 0, time_obstacles}; 
  struct Player second = {48, 49, RED, PLAYER2, ALIVE, 0, 0, time_obstacles};
  p1 = first;
  p2 = second;

  ob_cars = (struct Obstacle_Car*) malloc(MAX_OB_CARS * sizeof(struct Obstacle_Car));
  ob_lorries = (struct Obstacle_Lorry*) malloc(MAX_OB_LORRIES * sizeof(struct Obstacle_Lorry));


  for(int i = 0; i < MAX_OB_CARS; i++) {
    struct Obstacle_Car ob = {0, 0, DEAD, DEAD};
    ob_cars[i] = ob;
  }
  for(int i = 0; i < MAX_OB_LORRIES; i++) {
    struct Obstacle_Lorry ob = {0, 0, DEAD, DEAD};
    ob_lorries[i] = ob;
  }
  driver_loop();
}


void driver_loop() {
  do {
	  cur_time = millis();
	  frame.clear();
	  driver_drawBorder();
	  driver_drawLane();

	  //check if players should be moved:
	  if(cur_time > p1.last_move + MIN_DELAY) {
	    driver_check(&p1);
	  }
	  if(cur_time > p2.last_move + MIN_DELAY) {
	    driver_check(&p2);
	  }

	  if(p1.life_status == ALIVE) {
	    if(driver_playerCollision(&p1)) {
	      driver_startExplosion(&p1);
	    } else {
	      driver_drawCar(&p1);
	    }
	  } else {
	    driver_deathText(&p1);
	  }
	  if(p2.life_status == ALIVE) {
	    if(driver_playerCollision(&p2)) {
	      driver_startExplosion(&p2);
	    } else {
	      driver_drawCar(&p2);
	    }
	  } else {
	    driver_deathText(&p2);
	  }

	  //check if its time to move obstacles
	  if(cur_time > time_obstacles + OBSTACLE_DELAY) {
	    num = (num > 2)? 0 : num+1; //for the moving lane
	    if(p1.life_status == ALIVE) p1.distance_travled++;
	    if(p2.life_status == ALIVE) p2.distance_travled++;
	    driver_drawObstacles();
	  }

	  frame.update();
	  delay(30);
	} while( !(frame.getStart(PLAYER1) && frame.getStart(PLAYER2) && (cur_time > start_time + 1000)) );

  free(ob_cars);
  free(ob_lorries);
}


/* Creates new obstacles, advances existing ones and checks for collisions */
void driver_drawObstacles() {
  for(int i = 0; i < MAX_OB_CARS; i++) {
    struct Obstacle_Car *c = &ob_cars[i];

    if((c->cur_status1 == DEAD) && c->cur_status2 == DEAD) {
      //Random chance of creating new obstacle. When the speed increases more obstacles are genereated.
      if (random(0, (FRAME_DELAY)*2) == 1) {
        int j = (random(0, 7)*4)+2; //sets the obstacle car in an random lane for player 1. i*4+2 returns middle of the i'th lane.
        int k = 0;
        while (driver_playerCollision(0, j) && k < MAX_OB_CARS) { //loop until we have a free lane or we have tried all obstacle_cars
          j = (random(0, 7)*4)+2;
          k++;
        } 
        if (k < 10) {
          c->y = j;
          c->cur_status1 = ALIVE;
          c->cur_status2 = ALIVE;
          c->x = -4; //start outside screen
        }
      }
    } else {
      if(c->cur_status1 == ALIVE) {
        if(c->x > 64) {
          c->cur_status1 = DEAD;
        } else {
          if(p1.life_status != DEAD) {
            driver_drawCar(c->x, c->y, ORANGE); //Player1
            driver_ObstacleCarCollision(&p1, c);
          } 
        }
      }
      if(c->cur_status2 == ALIVE) {
        if(c->x > 64) {
          c->cur_status2 = DEAD;
        } else {
          if(p2.life_status != DEAD) {
            driver_drawCar(c->x, c->y + PLAYER_OFFSET, ORANGE); // Player2<
            driver_ObstacleCarCollision(&p2, c);
          }
        }
      }
      c->x++;
    } 
  }
  
  for(int i = 0; i < MAX_OB_LORRIES; i++) {
    struct Obstacle_Lorry *c = &ob_lorries[i];
    //Random chance of creating new obstacle. When the speed increases more obstacles are genereated.
    if((c->cur_status1 == DEAD) && (c->cur_status2 == DEAD)) {
      if (random(0, (FRAME_DELAY)*5) == 1) {
        int j = (random(0, 7)*4)+2; //sets the obstacle car in an random lane for player 1. i*4+2 returns middle of the i'th lane.
        int k = 0;
        while (driver_playerLorryCollision(0, j) && k < MAX_OB_LORRIES) { //loop until we have a free lane or we have tried all obstacle_cars
          j = (random(0, 7)*4)+2;
          k++;
        } 
        if (k < 10) {
          c->y = j;
          c->cur_status1 = ALIVE;
          c->cur_status2 = ALIVE;
          c->x = -6; //start outside screen
        }
      }
    }
    else {
      //draw car for player1
      if (c->cur_status1 == ALIVE) {
        if(c->x > 64) {
          c->cur_status1 = DEAD;
        } else {
          if(p1.life_status != DEAD) {
            driver_drawLorry(c->x, c->y, ORANGE); 
            driver_ObstacleLorryCollision(&p1, c); 
          }
        }    
      } 
      //draw car for player2
      if(c->cur_status2 == ALIVE) {
        if(c->x > 64) { 
          c->cur_status2 = DEAD;
        } else {
          if(p2.life_status != DEAD) {
            driver_drawLorry(c->x, c->y + PLAYER_OFFSET, ORANGE); 
            driver_ObstacleLorryCollision(&p2, c);
          }
        }
      }
      c->x++; //advance obstacle 1px
    }
  }
}

/* Checks if obstacle-car c will with a player on the next step. 
   If yes: kill player p and remove c. */
void driver_ObstacleCarCollision(struct Player *p, struct Obstacle_Car *c) {
  int cy = (p->player == PLAYER1)? c->y : c->y + PLAYER_OFFSET;
  if ((p->life_status == ALIVE) &&
      (p->y == cy) && 
      (((p->x > c->x) && p->x < (c->x + 5)) || 
        ((p->x < c->x) && (c->x < (p->x + 6))))) {
    driver_startExplosion(p);
    if(p->player == PLAYER1) {
      c->cur_status1 = DEAD;
    } else {
      c->cur_status2 = DEAD;      
    }
  }
}


/* Checks if obstacle-lorry c will with a player on the next step. 
   If yes: kill player p and remove c. */
void driver_ObstacleLorryCollision(struct Player *p, struct Obstacle_Lorry *c) {
  int cy = (p->player == PLAYER1)? c->y : c->y +PLAYER_OFFSET;
  if ((p->life_status== ALIVE) && 
      (p->y == cy) && 
      (((p->x > c->x) && p->x < (c->x + 7)) || 
        ((p->x < c->x) && (c->x < (p->x + 8))))) {
    driver_startExplosion(p);
    if(p->player == PLAYER1) {
      c->cur_status1 = DEAD;
    } else {
      c->cur_status2 = DEAD;      
    }
  }
}


/* inputs: coordinates of head. 
   returns: wether any of the pixels the car will occupy is already colored */
boolean driver_playerCollision(int8_t x, uint8_t y) {
  return (frame.getPixel(x, y) != BLACK ||
          frame.getPixel(x+1, y) != BLACK ||
          frame.getPixel(x+2, y) != BLACK ||
          frame.getPixel(x+3, y) != BLACK || 
          frame.getPixel(x+CAR_LEN -1, y) != BLACK);
}

boolean driver_playerCollision(Player *p) {
  return driver_playerCollision(p->x, p->y);
}


boolean driver_playerLorryCollision(int8_t x, uint8_t y) {
  return (frame.getPixel(x, y) != BLACK ||
          frame.getPixel(x+1, y) != BLACK ||
          frame.getPixel(x+2, y) != BLACK ||
          frame.getPixel(x+3, y) != BLACK || 
          frame.getPixel(x+4, y) != BLACK ||
          frame.getPixel(x+5, y) != BLACK);
}


void driver_drawBorder() {
   frame.drawLine(0, 0, 63, 0, GREEN);
   frame.drawLine(0, 28, 63, 28, GREEN);
   frame.drawLine(0, 29, 63, 29, GREEN);
   frame.drawLine(0,34, 63, 34, RED);
   frame.drawLine(0,35, 63, 35, RED);
   frame.drawLine(0, 63, 63, 63, RED);
}


void driver_drawLane() {
  for (int i = 1; i < 7; i++) {
    for (int j = 1; j < 64; j+=5) {
      if(p1.life_status!= DEAD) {
        frame.drawLine((j-1) + num, i*4, j + num, i*4, RED);
      }
      if(p2.life_status!= DEAD) {
        frame.drawLine((j-1) + num, i*4+PLAYER_OFFSET, j + num, i*4+PLAYER_OFFSET, GREEN);
      }
    }
  }
}


/* checks if x is inside [0,64] before drawing */
void driver_safeSetPixel(int8_t x, uint8_t y, uint8_t color) {
  if((x < 64) && (x > -1)) {
    frame.setPixel(x, y, color);
  }
}


/* inputs: coordinates of head */ 
void driver_drawCar(int8_t x, uint8_t y, uint8_t color) { 
  driver_safeSetPixel(x, y, color); 
  driver_safeSetPixel(x+1, y, color); 
  driver_safeSetPixel(x+1, y+1, color); 
  driver_safeSetPixel(x+1, y-1,color); 
  driver_safeSetPixel(x+2, y, color); 
  driver_safeSetPixel(x+3, y, color); 
  if(CAR_LEN == 4) { 
    driver_safeSetPixel(x+3, y+1, color); 
    driver_safeSetPixel(x+3, y-1,color); 
    } else if(CAR_LEN == 5) { 
      driver_safeSetPixel(x+4, y,color); 
      driver_safeSetPixel(x+4, y+1, color); 
      driver_safeSetPixel(x+4, y-1,color); 
    } 
}

void driver_drawCar(Player *p) {
  driver_drawCar(p->x, p->y, p->color);
}


/* inputs: coordinates of head */ 
void driver_drawLorry(int8_t x, uint8_t y, uint8_t color) {
  driver_safeSetPixel(x, y, color);
  driver_safeSetPixel(x+1, y, color);
  driver_safeSetPixel(x+1, y+1, color);
  driver_safeSetPixel(x+1, y-1,color);
  driver_safeSetPixel(x+2, y, color);
  driver_safeSetPixel(x+2, y+1, color);
  driver_safeSetPixel(x+2, y-1, color);
  driver_safeSetPixel(x+3, y, color);  
  driver_safeSetPixel(x+4, y, color);
  driver_safeSetPixel(x+4, y+1, color);
  driver_safeSetPixel(x+4, y-1, color);
  driver_safeSetPixel(x+5, y, color);
  driver_safeSetPixel(x+5, y+1, color);
  driver_safeSetPixel(x+5, y-1, color);
  driver_safeSetPixel(x+6, y, color);
  driver_safeSetPixel(x+6, y+1, color);
  driver_safeSetPixel(x+6, y-1, color);
}


void driver_startExplosion(struct Player *p) {
  p->life_status= EXPLODING;
}


/* Draws one stage of the explosion each turn while player has status EXPLODING */
void driver_drawExplosion(struct Player *p) {
  if(p->life_status == EXPLODING) {
    int8_t offset = (p->y < 32)? 4 : 3; //if upper half, move explosion 1px up
    if (p->explosion_stage < 4) {
        frame.blit(expl_left[p->explosion_stage], p->x-5, p->y-offset);
        frame.blit(expl_right[p->explosion_stage], p->x+3, p->y-offset);     
      p->explosion_stage++;
    } else {
      p->life_status = DEAD;
      p->explosion_stage = 0;
    }
  }
}


void driver_deathText(struct Player *p) {
  frame.setColour(ORANGE);
  if(p->player == PLAYER1) {
    frame.gotoXY(10, 8);
    frame.print("P1 DEAD");
    frame.gotoXY(10,16);
    char a[16];
    String dist = String(p->distance_travled, DEC);
    dist.toCharArray(a, 16);
    frame.print(a);

  } else {
    frame.gotoXY(10, 40);
    frame.print("P2 DEAD");
    frame.gotoXY(10, 48);
    char a[16];
    String dist = String(p->distance_travled, DEC);
    dist.toCharArray(a, 16);
    frame.print(a);
  }
}

void driver_splash() {
  frame.clear();
  frame.gotoXY(20, 10);
  frame.print("Driver", ORANGE);
  driver_drawCar(40, 40, ORANGE);
  driver_drawCar(40, 34, ORANGE);
  driver_drawCar(40, 40, ORANGE);
  driver_drawCar(40, 40, ORANGE);

  driver_drawCar(-4, 40, ORANGE);

  frame.update();
}

/*
void setup() {
  frame.begin();
  Serial.begin(9600);
  Serial.println(freeRam());
  //  frame.resetButtons(); 
  driver_splash();
  delay(1000);
}

void loop() {
 // driver_run();
}



extern uint8_t rand_8();
extern int __bss_end;
extern int *__brkval;
int freeRam() {
  //Fetches the amount of available ram. Useful for debugging
  int free_memory;

  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}*/