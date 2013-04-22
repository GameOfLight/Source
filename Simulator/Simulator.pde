/*
  Simulator.pde - GameOfLight library
  Copyright (c) 2013 Eivind Wikheim.  All right reserved.

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

import processing.serial.*;

/* Markers used to mark a message as for the simulator or as a message to be 
 * printed to the terminal in case such a character isn't present */
static final int SCREEN_DATA = 0x11;      //DC1 char - device control 1
static final int SCREEN_CMD = 0x12;       //DC2 char - device control 2
static final int SCREEN_DATA_BURST = 0x13;//DC3 char - device control 3

/* 19_04: added coords and lines*/
/* 21_04: change coord/line function to fix a bug*/

/*
 Original program by Eivind
 
 New features by Stian
  * Updated transfer-protocol
    * SCREEN_DATA = single data transfer
      - 1 transfer per send as datafield may contain contents equaling the controlcharacters. Use DC3 for multiple
      - Example usage:
        0x11 - DC1
        0x22 - write this at current position with current colour
        0x08 - '\n' ends transmission

    * SCREEN_CMD = one or more cmd transfer(s)
      - blink removed as blink off equals '\n' making a right mess of things
      - Example usage:
        0x12 - DC2
        0x20 - display clear
        0x47 - jump to line 7
        0xB4 - jump to index 52, GREEN colour
        0x08 - '\n' ends command mode
        
    * SCREEN_DATA_BUSRT = entire line of data at once. That is both the red and the GREEN info for a single line
      - Example usage:
        0x13 - DC3
        0x08 - '\n'
        128 databytes
        0x08 - '\n' ends transmission
        
        For transferring a complete frame this leads to an overhead of 24 bytes, 
        whereas using single transfer-mode uses 2048 bytes as overhead.
        
  * Allowed Arduino to print debug messages without interfering with the simulated screen
    - Drawback: data transfers have to be blocking otherwise the user may insert a debug message in the 
        middle of a screen update transfer. Interrupt based transfers hence not implemented.
        Still, the interface allows a good bit of time for calculations between frames.
    - Also allows usage of Serial.print('H') followed by a screen data/cmd request without making a mess
      -Ie this works fine:
      0x48 Char 'H', printed to terminal
      0x65 Char 'e', printed to terminal
      0x69 Char 'i', printed to terminal (note lack of '\n')
      0x12 Command header
      %more commands%
      0x08 - '\n' end of command mode

  * Implemented noLoop: Screen only updated when new content is present
    
 Todo:
  [DONE]* Update draw logic to match physical screen
  [DONE]  - When writing index 63 of GREEN the next write should be index 0 of RED on the same line
  [DONE]    - When going past index 63 of RED it should switch to the next line
  [DONE]  - Bytes are displayed the wrong way around on the screen
  [DONE]  - The goto function for picking a line should pick a line, not the index along the y-axis
  [DONE]    - Example: Writing in position 0,0 should make a 8-light vertical line starting in the upper left corner
  [DONE]    - Writing in position 1,0 should make a 8-light vertical line starting 8 pixels down from the upper left corner
  [DONE] *Should display_clear also reset the draw position to 0,0?
  [] *Programcrash when loading without an Arduino connected. (Also: What happens if it's not port[0]?)
  [DONE] *Show (x,y) coords of led on mouse-hover (added lines aswell)
  [] *Add support for keyboad-controllers 
 */

Serial port;
/* Color codes*/
final static color RED = #FF0000;        
final static color GREEN = #00FF00;      
final static color ORANGE = #fd971f;      
final static color BLACK = #000000;       
final static color WHITE = #FFFFFF;      
final static color GREY = #272822;        

final static int BOARD_SIZE = 640;       /* Total length/witdh of board on screen */
final static int HALF = 320;
final static int SIZE = 64;              /* Number of leds */
final static int RES = 10;  /* Space per square */
final static int LED_SIZE = 8;           /* Space use by color for each led */
final static int INFO_BAR = 30;
int line, index;            /* Coordinates of current led */
int x, y;
long writetime;
char cmd, data;             /* Used for receiving input trough serial*/

/* Contains information about what blocks are drawn as: filled[line][index]
 * index 0-63 contains info about RED, index 64-127 about GREEN (both active means ORANGE) */
int [][]filled = new int[8][128];

/* Setting burstmode to 1 allows 128 byte transfers of serial data directly to the LED-screen 
 * See SCREEN_DATA_BURST */
int burstmode = 0;

/* Used by mouseClick() for writing currenct coords + lines */
int ledx, ledy, linex, liney;

void setup() {
  size(BOARD_SIZE, BOARD_SIZE + INFO_BAR);
  println(Serial.list());            
  port = new Serial(this, Serial.list()[0], 500000);//666666);
  port.bufferUntil('\n');            // ^- number may vary
  //frameRate(30);
  noLoop();
  noStroke();
  reset();
  textSize(25);
  textAlign(CENTER);
  line = 0;
  index = 0;
  ledx = 0;
  ledy = 0;
  linex = 1;
  liney = 1;


}

/* DEBUGING: Uncommenting will spam the terminal as long as new data is coming in */
void draw() {

}

void mouseClicked() {
  if(mouseX < BOARD_SIZE + INFO_BAR) {
    for(int i = 0; i < 640; i++) {
      set(linex - 1, i, GREY);
      set(linex + 8, i, GREY);
      set(i, liney - 1, GREY);
      set(i, liney + 8, GREY);
    }

    if(mouseX < BOARD_SIZE && (mouseX != linex || mouseY != liney)) {
      ledx = (mouseX/10);
      ledy = (mouseY/10);
      linex = ledx*10;
      liney = ledy*10;
      fill(GREY);
      rect(0, BOARD_SIZE, BOARD_SIZE, INFO_BAR);
      fill(WHITE);
      text(("("+ledx+","+ledy+")"), HALF, BOARD_SIZE + INFO_BAR - 8);
      for(int i = 0; i < 640; i++) {
        set(linex - 1, i, WHITE);
        set(linex + 8, i, WHITE);
        set(i, liney - 1, WHITE);
        set(i, liney + 8, WHITE);
      }
    }
    redraw();    
  }  
}


/* To get unsigned int */
void paint(char input) {
  paint(input & 0xFF);  //int
}

/* Iterates trough the bits of input, fills in appropriate colors 
 * based on what bits are set. */
void paint(int input) {
  int green, red;
  color c;
  filled[line][index] = input;
  if(index < SIZE) {
    green = input;
    red = filled[line][index+SIZE]; 
  } else {
    green = filled[line][index-SIZE];
    red = input;
  }
  
  int mask;
  for(int bit = 0; bit < 8; bit++) {
    mask = (1 << bit);
    c = BLACK;
    if((green & mask) != 0) {
      c = GREEN;
    }
    if ((red & mask) != 0) {
      if(c != BLACK) {
        c = ORANGE;
      } else {
        c = RED;
      }
    }
    fill(c);
    draw_block(bit);
  }
  next_byte();
}

void paint_line() {
  int green, red, mask;
  color c;
  for(int i = 0; i < SIZE; i++) {
    green = filled[line][i];
    red = filled[line][i + SIZE];
    
    for(int bit = 0; bit < 8; bit++) {
      mask = (1 << bit);
      c = BLACK;
      if((green & mask) != 0) {
        c = GREEN;
      }
      if ((red & mask) != 0) {
        if(c != BLACK) {
          c = ORANGE;
        } else {
          c = RED;
        }
      }
      fill(c);  
      draw_block(bit);
     }
     next_byte();
  }
  index = 0;
  if(line < 7) {
    line++;
  } else {
    line = 0;
  }
} 

/* Converts [line][index] to [x][y] and paints currently chosen color to [x][y] */
void draw_block(int bit) {
  y = line*8 + bit;
  x = (index < SIZE) ? index : (index - SIZE);
  rect(x*RES, y*RES, LED_SIZE, LED_SIZE);
}

/* Increases [line][index] to next coordinate */
void next_byte() {
  if(index < 127) {
    index++;
  } else {
    index = 0;
    if(line < 7) {
      line++;
    }
    else {
      line = 0;
    }
  }
}

/* Sets background and fills all blocks with BLACK */
void reset() {
  line = 0;
  index = 0;
  println("reset");
  background(GREY);
  fill(BLACK);
  filled = new int[8][128];
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      rect(i*RES, j*RES, LED_SIZE, LED_SIZE);
    }
  }
  fill(WHITE);
  textSize(15);
  textAlign(LEFT);
  text("Click on a block to show coordinates and lines. Click on this border to remove lines.", 
    0, BOARD_SIZE + INFO_BAR - 8);
  textSize(25);
  redraw();
}



/* Parses cmd coming from serial (see protocol for details) */
void cmd_data_handler(byte[] buff) {
  cmd = (char) buff[0];
  
  if (cmd != SCREEN_DATA && cmd != SCREEN_CMD && cmd != SCREEN_DATA_BURST) {
    //DEBUG
     println("ERROR!!!");
     return; 
  }

  if (cmd == SCREEN_DATA) {
    /* single data received, paint the byte */
    data = (char)buff[1];
    paint(data);
    redraw();
    
  } else if (cmd == SCREEN_CMD) {
    /* One or more commands received, interpret all of them! */
    for (int i = 1; i < buff.length-1; i++) {
      data = (char)buff[i];
      /* Set index (x) and color */
      if ( (data & 0x80) != 0 ) {
        index = (data & 0x7F);
      } 
      /* Set line (y) */
      else if ( (data & 0x40) != 0 ) {
        line = (data & 0x3F);
      }
      /* Clear display */
      else if ( (data & 0x20) != 0 ) {
        reset();
      }
      /* Adjust global brightness*/
      else if ( (data & 0x10) != 0) {
        // TODO
      }
    }
  } else {
    /*Multi-byte data transfer initiated. Must first fetch the line in question
     * Next serialevent will be interpreted as a data burst */
    burstmode = 1;
    port.buffer(129);
  }
}


/* Parses cmd when using burstmode */
void serial_action(byte[] buff) {
  byte curr = buff[0];
    
  if (curr != SCREEN_DATA && curr != SCREEN_CMD && curr != SCREEN_DATA_BURST) {
     /* First char wasn't a control character, but if the user used Serial.print() rather than println() then a control
      * character may appear further into the sequence. */
      for (int i = 0; i < buff.length; i++) {
        /* Scanning string for control characters */
        curr = buff[i];
        if (curr == SCREEN_DATA || curr == SCREEN_CMD || curr == SCREEN_DATA_BURST) {
          /* FOUND! Time to split buffer into a string segment and a data/cmd segment.
           * Note that as cmd/data blocks allways end with a '\n' no further splits of the buff[] will be needed 
           * due to how serial data is buffered. */

          /* Cutting out the data/dmg segment */
          byte[] s1 = subset(buff, i, buff.length - i);
//          println(s1);
          serial_action(s1); //Execute data/cmd segment
          
          /* Get and print the text segment. Order swapped, but guess that doesn't make a difference... */
          buff = subset(buff, 0, i); //i elements starting from element 0
          break;
        }
      }
      /* Can now print the string without losing screen-related info */
      String toPrint = new String(buff);
      print(toPrint);
      
  } else {
    //Cmd or data
//    println("cmd/data handler");
    cmd_data_handler(buff);
  }
}



/* Listens for action on the serial port
 * Reads bytes untill '\n' and executes appropriate cmd to execute */
void serialEvent(Serial port) {
  byte[] inBuffer;

  if (burstmode == 0) {
    inBuffer = port.readBytesUntil('\n');
    /* inBuffer may contain cmd/data or debugprints */
    serial_action(inBuffer);
  } else {
    inBuffer = port.readBytes();
    /* inBuffer contains an entire line of data to be interpreted as LED-data */
    if (inBuffer.length != 129) {
       println("Protocol ERROR!");
    }

    for (int i = 0; i < inBuffer.length-1; i++) {
      filled[line][i] = (inBuffer[i] & 0xFF);
    }
    paint_line();

    redraw(); 
    burstmode = 0;
    port.bufferUntil('\n');
  }
}

/* DEBUGING */
void printbits(char c) {
 for(int i = 0; i < 8; i++) {
   if((c & (1 << (7-i))) != 0) {
     print("1");
   } else {
     print("0");
   }
 }
 println();
}
