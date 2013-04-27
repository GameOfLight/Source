/*		  dir  action	  sta	sel
Player 1: WASD YGHJ		 space  ctrl
Player 2: ULDR 8426(num) +(num) -(num)

0=pressed 1= not pressed

KEYKODES P1:
W,A,S,D 	: 87,65,83,68
Y,G,H,J 	: 89,71,72,74
space 		: 32	
ctrl 		: 17

KEYKODES P2:
UP,LEFT,DOWN,RIGHT: 38,37,40,39
(num) 8,4,2,6 	: 104,100,98,102
(num) +			: 107	
(num) -			: 109

*/
import processing.serial.*;

int movement, action, startselect; // storage of key-bits
Serial port;
void setup() {
	port = new Serial(this, Serial.list()[0], 9600);
 	port.bufferUntil('\n');            // ^- number may vary

	movement = Integer.MAX_VALUE;
	action = Integer.MAX_VALUE;
	startselect = Integer.MAX_VALUE;

	print("start:");
	printbits(movement);
}

void draw() {
	poll_keys();
}

void printbits(int unbyte) {
	for(int i = 0; i < 8; i++) {
		if((unbyte & (1 << 8-i)) != 0) {
			print(1);
		} else {
			print(0);
		}
	}
	println();
}


void keyReleased() {
	int key_press = keyCode;
	switch(key_press) {
		//start p1
		case 32 : startselect |= (1 << 8); break;	//space
		//select p1
		case 17: startselect |= (1 << 7); break;	//ctrl
		//start p2		
		case 107: startselect |= (1 << 6); break;	//num +
		//select p2
		case 109: startselect |= (1 << 5); break;	//num -
		//movement p1
		case 87 : movement |= (1 << 8); break;		//W
		case 65 : movement |= (1 << 7); break;		//A
		case 83 : movement |= (1 << 6); break; 		//S
		case 68 : movement |= (1 << 5); break;		//D
		//movement p2
		case 38 : movement |= (1 << 4); break; 		//UP
		case 37 : movement |= (1 << 3); break;		//LEFT
		case 40 : movement |= (1 << 2); break;		//DOWN
		case 39 : movement |= (1 << 1); break;		//RIGHT
		//action p1
		case 89 : action |= (1 << 8); break;		//Y
		case 71 : action |= (1 << 7); break;		//G
		case 72 : action |= (1 << 6); break;		//H
		case 74 : action |= (1 << 5); break;		//J
		//action p2	
		case 104: action |= (1 << 4); break;		//num8
		case 100: action |= (1 << 3); break;		//num4
		case 98 : action |= (1 << 2); break;		//num2
		case 102: action |= (1 << 1); break;		//num6
	}
}

void keyPressed() {
	int key_press = keyCode;
	// key in viable area:
	switch(key_press) {
		//start p1
		case 32 : startselect &= ~(1 << 8); break;	//space
		//select p1
		case 17 : startselect &= ~(1 << 7); break;	//ctrl
		//start p2
		case 107: startselect &= ~(1 << 6); break;	//num +
		//select p2
		case 109: startselect &= ~(1 << 5); break;	//num -
		//movement p1
		case 87 : movement &= ~(1 << 8); break;		//W
		case 65 : movement &= ~(1 << 7); break;		//A
		case 83 : movement &= ~(1 << 6); break;		//S
		case 68 : movement &= ~(1 << 5); break;		//D
		//movement p2
		case 38 : movement &= ~(1 << 4); break; 	//UP
		case 37 : movement &= ~(1 << 3); break;		//LEFT
		case 40 : movement &= ~(1 << 2); break;		//DOWN
		case 39 : movement &= ~(1 << 1); break;		//RIGHT
		//action p1
		case 89 : action &= ~(1 << 8); break;		//Y
		case 71 : action &= ~(1 << 7); break;		//G
		case 72 : action &= ~(1 << 6); break;		//H
		case 74 : action &= ~(1 << 5); break;		//J
		//action p2
		case 104: action &= ~(1 << 4); break;		//num8
		case 100: action &= ~(1 << 3); break;		//num4
		case 98 : action &= ~(1 << 2); break;		//num2
		case 102: action &= ~(1 << 1); break;		//num6
	}
}

void poll_keys() {
	port.write(movement);
	port.write(action);
	port.write(startselect);
	port.write('\n');
}

/*
void keyPressed() {
  println("pressed " + int(key) + " " + keyCode);
}*/