/*
  GameOfLight.cpp - GameOfLight library
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

#include "GameOfLight.h"
#include "utility/MsTimer2.h"

//Pointer to self for use with the MsTimer2 library
static GameOfLight *thisclass = 0;

GameOfLight::GameOfLight() {
  for (uint8_t i = 0; i < 8; i++) {
    red[i] = &buff[i][64];
    green[i] = &buff[i][0];
  }
  thisclass = this;
}


void GameOfLight::begin() {
  #if defined(GameOfLightHW)
    Serial.begin(500000);  //DEBUG
    // Initialize the communication with the display controller
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV8);
    pinMode(SCREEN_SS_PIN, OUTPUT);
    pinMode(controller[0], INPUT);
    pinMode(controller[1], INPUT);
    pinMode(controller[2], INPUT);
    pinMode(controller[3], INPUT);
    pinMode(SNES_CLK_PIN, OUTPUT);
    pinMode(SNES_LATCH_PIN, OUTPUT);
    digitalWrite(SCREEN_SS_PIN, HIGH);
    digitalWrite(SCREEN_SS_PIN, LOW);
    clearDisplay();
    controller[0] = 4;
    controller[1] = 5;
    controller[2] = 2;
    controller[3] = 3; //pins on Arduino where the SNES-controller data pins are connected
    MsTimer2::set(1, btnCallBack); //Autotriggering of the getButton-routine every 1ms    
    Serial.println("HW");  //DEBUG
  #else
    Serial.begin(500000);
    MsTimer2::set(15, btnCallBack);
    btnRequestSent = 0;
    Serial.println("Sim"); //DEBUG
  #endif

  _screen_line = 0;
  _screen_index = 0;
  resetButtons();
  setFont(FONT_5x7);
  MsTimer2::start();
}

/* Pick location in the buffer for subsequent writes */
void GameOfLight::gotoXY(const uint8_t x, const uint8_t y) {
  _curr_line = y >> 3;   //range [0,  7]
  _curr_shift = y & 0x07; //y % 8
  _curr_index = x & 0x3F; //range [0, 63]
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
  //Only adds colourdata, doesn't remove what is allready there. 
  // If clearing is required use clear(amount) to empty the area first.

  if (_curr_index > 63) return; //Ignore out of bounds

  if (_colour & GREEN) {
    //add green in position indicated by 'data'
    buff[_curr_line][_curr_index] |= data << _curr_shift;
  }
  if (_colour & RED) {
    //add red in position indicated by 'data'
    buff[_curr_line][_curr_index + 64] |= data << _curr_shift;
  }

  if ((_curr_line != 7) && _curr_shift) {
    //Draw the remainder, if any, into the byte below this one
    if (_colour & GREEN) {
      //add green in position indicated by 'data'
      buff[_curr_line+1][_curr_index] |= data >> (8 - _curr_shift);
    }
    if (_colour & RED) {
      //add red in position indicated by 'data'
      buff[_curr_line+1][_curr_index + 64] |= data >> (8 - _curr_shift);
    }
  }

  _curr_index++; //Will eventually push it to 64 at which point no new write operations will take place
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
  
  if (y < -1 || y > 7) {
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
  for(i=0; i<_font_width; i++) {
    /* Fetch and print the pieces that makes up the letter given */	
    write(pgm_read_byte(_font+(ch-0x20)*_font_width+i));
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

void GameOfLight::setFont(uint8_t fonttype) {
  if (fonttype == FONT_3x5) {
    //Tiny 3x5 font
    _font_width = 3;
    _font = font_3x5;

  } else {
    //Default 5x7 font
    _font_width = 5;
    _font = font_5x7;
  }
}


void GameOfLight::setColour(const uint8_t colour) {
  _colour = colour & 0x03; // Only GREEN, RED and ORANGE allowed
}

void GameOfLight::clear() {
  // Clears the internal buffer
  memset(buff, 0, sizeof buff);
}

void GameOfLight::clear(uint8_t count) {
  //Clears all data count spaces ahead of cursor. Stops at end of line if encountered.
  clear(count, 0xff);
}

void GameOfLight::clear(uint8_t count, uint8_t pattern) {
  //Clears count spaces ahead of current cursor position with the specified pattern.
  // Stops at the end of the line.
  // Pattern is which bits to remove in a 8 bit vertical column dropping down from
  //  the current coordinates.
  uint8_t end = min(64, _curr_index + count);
  uint8_t pattern_low = ~pattern >> (8 - _curr_shift);
  pattern = ~pattern << _curr_shift;

  for (uint8_t x = _curr_index; x < end; x++) {
    buff[_curr_line][x] &= pattern; //Clear green
    buff[_curr_line][x + 64] &= pattern; //Clear red

    if (_curr_line != 7) {
      buff[_curr_line+1][x] &= pattern_low; //Clear green
      buff[_curr_line+1][x + 64] &= pattern_low; //Clear red
    }
  }
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

  //Ignore too large a value to prevent write accidents and allow more creative use
  if (x > 63 || y > 63) return;

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


void GameOfLight::drawLine(int8_t x1, int8_t y1) {
  //Draws a line from the end of the previous line segment to the given coordinates.
  //Also works with rectangle end coordinates
  drawLine(_x0, _y0, x1, y1);
}

void GameOfLight::drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
  //Draws a line from (x0, y0) to (x1, y1) using only integer arithmatic (hence fast on this platform)
  //Bresenham's line drawing algorithm, see http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
  //Uses the previously used colour
  uint8_t dx, dy;
  int8_t sx, sy, err, e2;

  dx = abs(x1 - x0);
  dy = abs(y1 - y0);

  sx = x0 < x1 ? 1 : -1;
  sy = y0 < y1 ? 1 : -1;

  err = dx - dy;

  while(1) {
    setPixel(x0, y0, _colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = err << 1;

    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (x0 == x1 && y0 == y1) {
      setPixel(x0, y0, _colour);
      break;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }

  _x0 = x1;
  _y0 = y1;
}


void GameOfLight::drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t colour) {
  //Draws a line from (x0, y0) to (x1, y1) using only integer arithmatic (hence fast on this platform)
  //Bresenham's line drawing algorithm, see http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
  setColour(colour);
  drawLine(x0, y0, x1, y1);
}


void GameOfLight::drawRect(int8_t x0, int8_t y0, uint8_t width, uint8_t height) {
  //Draws an unfilled rectangle with the upper left corner in the coordinates specified (x0, y0)
  //Uses the previously used colour
  width--;
  height--;
  drawLine(x0, y0, x0, y0+height, _colour);
  drawLine(x0, y0, x0+width, y0, _colour);
  drawLine(x0, y0+height, x0+width, y0+height, _colour);
  drawLine(x0+width, y0, x0+width, y0+height, _colour);

  _x0 = x0+width;
  _y0 = y0+height;
}


void GameOfLight::drawRect(int8_t x0, int8_t y0, uint8_t width, uint8_t height, uint8_t colour) {
  //Draws an unfilled rectangle with the upper left corner in the coordinates specified (x0, y0)
  setColour(colour);
  drawRect(x0, y0, width, height);
}


void GameOfLight::drawCircle(int8_t x0, int8_t y0, uint8_t radius) {
  //Draws an unfilled circle with center around the given coordinates
  //See http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  int8_t err, x, y;
  err = -radius;
  x = radius;
  y = 0;

  while(x >= y) {
    //The circle is XY symmetrical, reuse the calculation to plot in each quadrant
    setPixel(x0 + x, y0 + y, _colour);
    setPixel(x0 - x, y0 + y, _colour);
    setPixel(x0 + x, y0 - y, _colour);
    setPixel(x0 - x, y0 - y, _colour);

    setPixel(x0 + y, y0 + x, _colour);
    setPixel(x0 - y, y0 + x, _colour);
    setPixel(x0 + y, y0 - x, _colour);
    setPixel(x0 - y, y0 - x, _colour);

    err += y++;
    err += y;
    if (err >= 0) {
      err -= x--;
      err -= x;
    }
  }
}


void GameOfLight::drawCircle(int8_t x0, int8_t y0, uint8_t radius, uint8_t colour) {
  //Draws an unfilled circle with center around the given coordinates
  //See http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  //Uses the previously used colour
  setColour(colour);
  drawCircle(x0, y0, radius);
}


//Scrolls the entire board 1px left leaving a blank column along the right edge
void GameOfLight::scrollLeft() {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    scrollLeft(i, 0, 0);
  }
}

//Scrolls the given line 1px to the left. Pads the end of the line with the given bytes
void GameOfLight::scrollLeft(uint8_t line, uint8_t padGreen, uint8_t padRed) {
  uint8_t i;

  for (i = 0; i < 127; i++) {
    //Green & red. The wrong data at buff[line][63] is fixed in a bit...
    buff[line][i] = buff[line][i+1];
  }

  buff[line][63] = padGreen;
  buff[line][127] = padRed;
}


//Scrolls the entire board 1px right leaving a blank column along the left edge
void GameOfLight::scrollRight() {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    scrollRight(i, 0, 0);
  }
}

//Scrolls the given line 1px to the right. Pads the start of the line with the given bytes
void GameOfLight::scrollRight(uint8_t line, uint8_t padGreen, uint8_t padRed) {
  uint8_t i;
  for (i = 127; i < 128; i--) { //Note: uint, hence seemingly weird test
    //Green & red. The wrong data at buff[line][64] is fixed in a bit...
    buff[line][i] = buff[line][i-1];
  }

  buff[line][0] = padGreen;
  buff[line][64] = padRed;
}


//Scrolls the entire board 1px up leaving a blank line along the bottom edge
void GameOfLight::scrollUp() {
  uint8_t i;
  for (i = 0; i < 64; i++) {
    scrollUp(i, BLACK);
  }
}

//Scrolls the given column 1px up. Pads in a pixel of the given colour
void GameOfLight::scrollUp(uint8_t index, uint8_t padcolour) {
  uint8_t i, carry, carryOld, curr;

  //Could use some optimization. Using the carry flag is an option

  carry = (padcolour & GREEN) << 7; //Padding with green
  
  for (i = 7; i < 8; i--) {
    carryOld = carry;
    curr = buff[i][index];
    carry = curr << 7;
    curr >>= 1;
    curr |= carryOld;
    buff[i][index] = curr;
  }

  carry = (padcolour & RED) << 6; //Padding with red

  for (i = 7; i < 8; i--) {
    carryOld = carry;
    curr = buff[i][index + 64];
    carry = curr << 7;
    curr >>= 1;
    curr |= carryOld;
    buff[i][index + 64] = curr;
  }
}


//Scrolls the entire board 1px down leaving a blank line along the top edge
void GameOfLight::scrollDown() {
  uint8_t i;
  for (i = 0; i < 64; i++) {
    scrollDown(i, BLACK);
  }
}

//Scrolls the given column 1px down. Pads in a pixel of the given colour
void GameOfLight::scrollDown(uint8_t index, uint8_t padcolour) {
  uint8_t i, carry, carryOld, curr;

  //Could use some optimization. Using the carry flag is an option

  carry = padcolour & GREEN; //Padding with green

  for (i = 0; i < 8; i++) {
    carryOld = carry;
    curr = buff[i][index];
    carry = curr >> 7;
    curr <<= 1;
    curr |= carryOld;
    buff[i][index] = curr;
  }

  carry = (padcolour & RED) >> 1; //Padding with red

  for (i = 0; i < 8; i++) {
    carryOld = carry;
    curr = buff[i][index + 64];
    carry = curr >> 7;
    curr <<= 1;
    curr |= carryOld;
    buff[i][index + 64] = curr;
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

uint8_t GameOfLight::getDir(uint8_t player){
  #ifndef GOL_CONTROLS_NOINVERT
    //Invert direction for players on the opposite side of the board
    if (!N[player]) { 
      N[player] = 1;
      return (player > PLAYER2 ? SOUTH : NORTH);
    }
    if (!S[player]) { 
      S[player] = 1;
      return (player > PLAYER2 ? NORTH : SOUTH);
    }
    if (!W[player]) { 
      W[player] = 1;
      return (player > PLAYER2 ? EAST : WEST);
    }
    if (!E[player]) { 
      E[player] = 1;
      return (player > PLAYER2 ? WEST : EAST);
    }

  #else
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
  #endif
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


void GameOfLight::update() {
  // Sends the entire screen buffer to the screen
  uint8_t line;
  #ifndef GameOfLightHW
    _screen_line = 0xff; //Illegal value to force a resync on first line sent
    for (line = 0; line < 8; line++) {
      delay(1);
      update(line);
    }  
  #else
    for (line = 0; line < 8; line++) {
      update(line);
      delayMicroseconds(20); //Ensures the screencontroller's buffer can keep up
    }
  #endif
}


/* Sends a single line to the screen */
void GameOfLight::update(const uint8_t line) {
  // Sends a single line to the screen 
  uint8_t i;
  #ifndef GameOfLightHW
    screen_goto(0, line);
    //Turn off the SNES-interrupts as they may destroy the update by injecting data into the
    //serial stream midtransfer:
    MsTimer2::stop();
    //Switch to burst mode to reduce transmission overhead:
    Serial.write(SCREEN_DATA_BURST);
    Serial.write('\n');
    //Transfer entire line:
    for (i = 0; i < 128; i++) {
      Serial.write(buff[line][i]);
    }
    Serial.write('\n'); //End transmission and burst mode
    MsTimer2::start();

  #else
    //Note: The screencontroller has a limited buffersize. Care must be taken to not call this function
    // too often, lest the buffer overflows and lines are lost. 6-7 lines in rapid succesion is the aprox 
    //limit.
    //Also note that issuing updates of concecutive lines in numerical order is a little bit faster
    // than random access as the requests are automatically combined for faster writes to the screen.
    digitalWrite(SCREEN_SS_PIN, HIGH); //disable slave SPI
    digitalWrite(SCREEN_SS_PIN, LOW);  //enable slave SPI -> resync with slave
    screen_cmd(CMD_TYPE_LINE , line); //To indicate a line is coming and which line it is
    for (i = 0; i < 128; i++) {
      SPI.transfer(buff[line][i]);
    }
  #endif
  
  //Keep track of where the screen is currently at:
  _screen_line++;
  if (_screen_line >= 8) _screen_line = 0; //Note: _screen_index unchanged due to wrap
}


void GameOfLight::clearDisplay() {
  // Clears both the internal buffer and the screen
  screen_cmd(CMD_TYPE_CLEAR, 0);
  _screen_line = 0;
  _screen_index = 0;
  clear();
}


/* Write a command to the display */
void GameOfLight::screen_cmd(uint8_t type, uint8_t value) {
  #ifndef GameOfLightHW
    MsTimer2::stop();
    Serial.write(SCREEN_CMD);
    //Could technically insert more commands in this spot to reduce overhead. The screen understands this
    // extension of the protocol.
    Serial.write(type | value);
    Serial.write('\n');
    MsTimer2::start();
  #else
    SPI.transfer(type | value);
  #endif
}


/* Write data directly to the display at current display position. */
void GameOfLight::screen_data(uint8_t data) {
  #ifndef GameOfLightHW
    //WARNING: 2 bytes overhead per byte sent, use update(line) for large transfers. 
    MsTimer2::stop();
    Serial.write(SCREEN_DATA);
    Serial.write(data);
    Serial.write('\n');
    MsTimer2::start();
  #else
    //WARNING: For the time being this is an unsupported operation, it will be ignored by the screen.
    screen_cmd(CMD_TYPE_DATA, 0);
    SPI.transfer(data);
  #endif
  //Update trackers of screen position
  _screen_index++;
  if (_screen_index > 127) {
    _screen_index = 0;
    _screen_line++;
    if (_screen_line > 7) {
      _screen_line = 0;
    }
  }
}


void GameOfLight::screen_goto(uint8_t index, uint8_t line) {
  line &= 0x07;  //Allows range [0,   7]
  index &= 0x7F; //Allows range [0, 127]
  if (_screen_index != index) {
    screen_cmd(CMD_TYPE_SETX, index);
    _screen_index = index;
  }
  if (_screen_line != line) {
    screen_cmd(CMD_TYPE_SETY, line);
    _screen_line = line;
  }
}


/* Asks simulator/HW for updated controller values. */
void GameOfLight::getButtons() {
  #ifndef GameOfLightHW
    //Simulator implementation. Reads response from serial whenever it becomes available.
    //Data arrives as NUM_PLAYERS*2 bytes.
    if (Serial.available() >= 4) {
      // Reading from Serial
      Serial.readBytes(serial_data, NUM_PLAYERS*2);
      for(int i = 0; i < NUM_PLAYERS; i++) {
        //Will keep a read buttonpush saved until it's given get-function resets it
        Start[i] &= ((serial_data[i*2] & (1 << 7)) == 0)? 1 : 0;
        Select[i] &= ((serial_data[i*2] & (1 << 6)) == 0)? 1 : 0;
        L[i] &= ((serial_data[i*2] & (1 << 5)) == 0)? 1 : 0;
        R[i] &= ((serial_data[i*2] & (1 << 4)) == 0)? 1 : 0; 
        N[i] &= ((serial_data[i*2+1] & (1 << 7)) == 0)? 1 : 0;
        W[i] &= ((serial_data[i*2+1] & (1 << 6)) == 0)? 1 : 0;
        S[i] &= ((serial_data[i*2+1] & (1 << 5)) == 0)? 1 : 0;
        E[i] &= ((serial_data[i*2+1] & (1 << 4)) == 0)? 1 : 0;
        X[i] &= ((serial_data[i*2+1] & (1 << 3)) == 0)? 1 : 0;
        Y[i] &= ((serial_data[i*2+1] & (1 << 2)) == 0)? 1 : 0;
        B[i] &= ((serial_data[i*2+1] & (1 << 1)) == 0)? 1 : 0;
        A[i] &= ((serial_data[i*2+1] & (1 << 0)) == 0)? 1 : 0;
      }
      btnRequestSent = 0;
    } else if (!btnRequestSent) {
      // Sending request to simulator for controler status
      Serial.write(REQUEST_KEYS);
      Serial.write('\n');
      btnRequestSent = 1;
    }
  #else
    //HW implementation. Reads the 4 attached SNES controllers.
    digitalWrite(SNES_LATCH_PIN, HIGH);
    digitalWrite(SNES_LATCH_PIN, LOW);

    for(int i = 0; i<4; i++){
      B[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      Y[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      Select[i]=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      Start[i]=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      N[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      S[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      W[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      E[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      A[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      X[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      L[i]&=digitalRead(controller[i]);
    }
    TICK;
    
    for(int i = 0; i<4; i++){
      R[i]&=digitalRead(controller[i]);
    }

  #endif
}


//A bit of a hack to get around the calling conventions of the MsTimer2 library.
void GameOfLight::btnCallBack() {
  if (thisclass) {
    thisclass->getButtons();
  }
}
