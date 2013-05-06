/*
  GameOfLight.cpp - GameOfLight library
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

#include "GameOfLight.h"

GameOfLight::GameOfLight() {
  for (uint8_t i = 0; i < 8; i++) {
    red[i] = &buff[i][64];
    green[i] = &buff[i][0];
  }
}

/* Pick location in the buffer for subsequent writes */
void GameOfLight::gotoXY(const uint8_t index, const uint8_t line) {
  _curr_line = line & 0x07;   //range [0,  7]
  _curr_index = index & 0x3F; //range [0, 63]
}

/*Returns the current line*/
uint8_t GameOfLight::getLine() {
  return _curr_line;
}

/*Returns the current index*/
uint8_t GameOfLight::getIndex() {
  return _curr_index;
}

/*Returns the current colour*/
uint8_t GameOfLight::getColour() {
  return _colour;
}

/* Enters the given data at the current cursor position with the current colour*/
void GameOfLight::write(const uint8_t data) {
  //Clears any off-colour data present in positions occupied by 'data'. If further
  //clearing is required use clear(amount) first to empty the area first.

  if (_colour & GREEN) {
    //add green in position indicated by 'data'
    buff[_curr_line][_curr_index] |= data;
  } else {
    //clear green if it shouldn't be set
    buff[_curr_line][_curr_index] &= ~data;
  }
  if (_colour & RED) {
    //add red in position indicated by 'data'
    buff[_curr_line][_curr_index + 64] |= data;
  } else {
    //Clear red if it shouldn't be set
    buff[_curr_line][_curr_index + 64] &= ~data;
  }

  // Update buffer position by wrapping to new line if overflowing.
  _curr_index++;
  if (_curr_index > 63) {
    _curr_index = 0;
    _curr_line++;
    if (_curr_line > 7) {
      _curr_line = 0;
    }	
  }
}

/*
 * Copies an 8x8 tile from program memory to the screen buffer.
 * Only the parts that are actually on the screen gets copied.
 */
void GameOfLight::blit(const uint8_t *sprite, int x, int y) {
  int8_t offset = y & 0x07; // Combined ABS and modulo
  int8_t maxCol = min(64 - x, 8); // Used to avoid drawing outside the right edge
  int8_t start = max(-x, 0); // Used to avoid drawing outside the left edge.
  int8_t i;
  y >>= 3; // Divide by 8 and round towards negative infinity
  
  if (y < -1) {
    return;

  } else if (y > -1) { // Draw the top half
    for (i = start; i < maxCol; i++) {
      // Green column
      green[y][i + x] |= pgm_read_byte(sprite + (i << 1)) << offset;
      // Red column
      red[y][i + x] |= pgm_read_byte(sprite + (i << 1) + 1) << offset;
    }
  }

  // Check if we are done, or if the bottom is off the bottom edge.
  if (!offset || y >= 7) {
    return;
  }

  y++;
  offset = 8 - offset;
  for (i = start; i < maxCol; i++) { // Draw the bottom half
    // Green column
    green[y][i + x] |= pgm_read_byte(sprite + (i << 1)) >> offset;
    // Red column
    red[y][i + x] |= pgm_read_byte(sprite + (i << 1) + 1) >> offset;
  }
}

void GameOfLight::print(const char ch) {
  uint8_t i;
  for(i=0; i<5; i++) {
    /* Fetch and print the pieces that makes up the letter given */	
    write(pgm_read_byte(font+(ch-0x20)*5+i));
  }
  write(0x00); //Space between letters
}

void GameOfLight::print(char *string) {
  while(*string) {
    print(*string);
    string++;
  }
}

void GameOfLight::print(const char ch, const uint8_t colour) {
  setColour(colour);
  print(ch);
}

void GameOfLight::print(const char *string, const uint8_t colour) {
  setColour(colour);
  print((char*)string);
}

void GameOfLight::setColour(const uint8_t colour) {
  _colour = colour & 0x03; // Only GREEN, RED and ORANGE allowed
}

void GameOfLight::clear() {
  // Clears the internal buffer
  memset(buff, 0, sizeof buff);
}

void GameOfLight::clear(int count) {
  //Clears count spaces ahead of cursor. Stops once count has been cleared or
  // screen overflows back to position 0,0

  //A bit slow, change to memset implementation later? (But make sure to check whetever all of
  // count ends up within the actual array before you do!)
  for (uint8_t line = _curr_line; line < 8 && count; line++) {
    for (uint8_t x = _curr_index; x < 64 && count; x++) {
      buff[line][x] = 0;		//Clear green
      buff[line][x + 64] = 0; //Clear red
      count--;
    }
  }
  /*if (count) {
  //Overran the last line. Clear rest:
  clear(count);
  }*/
}

// Returns the colour of the given pixel
uint8_t GameOfLight::getPixel(const uint8_t x, const uint8_t y) {
  uint8_t colour = 0;
  uint8_t line = y >> 3;
  uint8_t y_bm = (1 << (y % 8));

  if (buff[line][x] & y_bm) {
    //contains green
    colour |= 0x01;
  }
  if (buff[line][x + 64] & y_bm) {
    //contains red
    colour |= 0x02;
  }
  return colour;
}

void GameOfLight::setPixel(uint8_t x, uint8_t y, uint8_t colour) {
  uint8_t line, y_bm;

  //sanityfix in case of too large a value. Max 63 to prevent write accidents
  y &= 0x3F;
  x &= 0x3F; // Should these protections be removed for more speedy execution?

  line = y >> 3; // y/8
  y_bm = (1 << (y % 8));
	
  //clear current content
  buff[line][x] &= ~(y_bm);
  buff[line][x + 64] &= ~(y_bm);

  //Note: Orange is generated by triggering both tests
  if (colour & GREEN) {
    buff[line][x] |= y_bm;
  }
  if (colour & RED) {
    buff[line][x + 64] |= y_bm;
  }
}

uint8_t GameOfLight::getA(uint8_t player){
  if(!A[player]){
    A[player] = 1;
    return 1;
  }  
  return 0;
}

uint8_t GameOfLight::getB(uint8_t player){
   if(!B[player]){
    B[player] = 1;
    return 1;
  }  
  return 0;

}

uint8_t GameOfLight::getX(uint8_t player){
  if(!X[player]){
    X[player] = 1;
    return 1;
  }  
  return 0;
}

uint8_t GameOfLight::getY(uint8_t player){
   if(!Y[player]){
    Y[player] = 1;
    return 1;
  }  
  return 0;
}

uint8_t GameOfLight::getStart(uint8_t player){
   if(!Start[player]){
    Start[player] = 1;
    return 1;
  }  
  return 0;
}

uint8_t GameOfLight::getSelect(uint8_t player){
   if(!Select[player]){
    Select[player] = 1;
    return 1;
  }  
  return 0;
}

uint8_t GameOfLight::getL(uint8_t player){
   if(!L[player]){
    L[player] = 1;
    return 1;
  }  
  return 0;
}

uint8_t GameOfLight::getR(uint8_t player){
   if(!R[player]){
    R[player] = 1;
    return 1;
  }  
  return 0;
}

direction GameOfLight::getDir(uint8_t player){
  if (!N[player]) { 
    N[player] = 1;
    return NORTH;
  }
  if (!S[player]) { 
    S[player] = 1;
    return SOUTH;
  }
  if (!W[player]) { 
    W[player] = 1;
    return WEST;}
  if (!E[player]) { 
    E[player] = 1;
    return EAST;
  }
  return NONE;
}
void GameOfLight::resetButtons(){
  for(uint8_t i = 0; i<4; i++){
    A[i] = 1;
    B[i] = 1;
    X[i] = 1;
    Y[i] = 1;
    Start[i] = 1;
    Select[i] = 1;
    L[i] = 1;
    R[i] = 1;
    N[i] = 1; 
    S[i] = 1;
    W[i] = 1;
    E[i] = 1;
  }
  
  
}
