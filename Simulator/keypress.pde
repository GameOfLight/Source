/*          
          dir  action     sta    sel
Player 1: WASD YGHJ      space  ctrl
Player 2: ULDR 8426(num) +(num) -(num)

0=pressed 1= not pressed
*/

// Keykodes p1:
static final int p1_UP = 87;    //W
static final int p1_LEFT = 65;  //A
static final int p1_DOWN = 83;  //S
static final int p1_RIGHT = 68; //D
static final int p1_X = 89;     //Y
static final int p1_Y = 71;     //G
static final int p1_B = 72;     //H
static final int p1_A = 74;     //J
static final int p1_START = 32; //Space
static final int p1_SELECT = 17;//Ctrl
//Keykodes p2:
static final int p2_UP = 38;    //up-arrow
static final int p2_LEFT = 37;  //left-arrow
static final int p2_DOWN = 40;  //right-arrow
static final int p2_RIGHT = 39; //down-arrow
static final int p2_X = 104; // num8
static final int p2_Y = 100; // num4
static final int p2_B = 98;  // num2
static final int p2_A = 102; // num6
static final int p2_START = 107; //num+
static final int p2_SELECT = 109;//num-


int startselect, p1, p2; //storage of key-bits

Serial port;
void setup() {
    port = new Serial(this, Serial.list()[0], 9600);
    port.bufferUntil('\n');            // ^- number may vary

    movement = Integer.MAX_VALUE;
    action = Integer.MAX_VALUE;
    startselect = Integer.MAX_VALUE;
}

void draw() {
    for(int i = 0; i < 8; i++) {
        if((p1 & (1 << 8-i)) != 0) {
            print(1);
        } else {
            print(0);
        }

    }
}


void keyReleased() {
    int key_press = keyCode;
    switch(key_press) {
        case p1_START : startselect |= (1 << 8); break;
        case p1_SELECT: startselect |= (1 << 7); break;
        case p2_START : startselect |= (1 << 6); break;
        case p2_SELECT: startselect |= (1 << 5); break;
        case p1_UP    : p1 |= (1 << 8);          break;
        case p1_LEFT  : p1 |= (1 << 7);          break;
        case p1_DOWN  : p1 |= (1 << 6);          break;
        case p1_RIGHT : p1 |= (1 << 5);          break;
        case p1_X     : p1 |= (1 << 4);          break;
        case p1_Y     : p1 |= (1 << 3);          break;
        case p1_B     : p1 |= (1 << 2);          break;
        case p1_A     : p1 |= (1 << 1);          break;
        case p2_UP    : p2 |= (1 << 8);          break;
        case p2_LEFT  : p2 |= (1 << 7);          break;
        case p2_DOWN  : p2 |= (1 << 6);          break;
        case p2_RIGHT : p2 |= (1 << 5);          break;
        case p2_X     : p2 |= (1 << 4);          break;
        case p2_Y     : p2 |= (1 << 3);          break;
        case p2_B     : p2 |= (1 << 2);          break;
        case p2_A     : p2 |= (1 << 1);          break;
    }
}

void keyPressed() {
    int key_press = keyCode;
    switch(key_press) {
        case p1_START : startselect &= ~(1 << 8); break;
        case p1_SELECT: startselect &= ~(1 << 7); break;
        case p2_START : startselect &= ~(1 << 6); break;
        case p2_SELECT: startselect &= ~(1 << 5); break;
        case p1_UP    : p1 &= ~(1 << 8);          break;
        case p1_LEFT  : p1 &= ~(1 << 7);          break;
        case p1_DOWN  : p1 &= ~(1 << 6);          break;
        case p1_RIGHT : p1 &= ~(1 << 5);          break;
        case p1_X     : p1 &= ~(1 << 4);          break;
        case p1_Y     : p1 &= ~(1 << 3);          break;
        case p1_B     : p1 &= ~(1 << 2);          break;
        case p1_A     : p1 &= ~(1 << 1);          break;
        case p2_UP    : p2 &= ~(1 << 8);          break;
        case p2_LEFT  : p2 &= ~(1 << 7);          break;
        case p2_DOWN  : p2 &= ~(1 << 6);          break;
        case p2_RIGHT : p2 &= ~(1 << 5);          break;
        case p2_X     : p2 &= ~(1 << 4);          break;
        case p2_Y     : p2 &= ~(1 << 3);          break;
        case p2_B     : p2 &= ~(1 << 2);          break;
        case p2_A     : p2 &= ~(1 << 1);          break;
    }
}

void poll_keys() {
    port.write(movement);
    port.write(action);
    port.write(startselect);
    port.write('\n');
}


