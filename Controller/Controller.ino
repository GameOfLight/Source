//#include <controller.h>

int clock = 4;
int latch = 6;
int data[] = {8,9,10,11};

enum direction {NORTH, SOUTH, EAST, WEST, NONE};


void setup(){
	Serial.begin(9600);
	pinMode(data[0], INPUT);
	pinMode(data[1], INPUT);
	pinMode(data[2], INPUT);
	pinMode(data[3], INPUT);

	pinMode(clock, OUTPUT);
	pinMode(latch, OUTPUT);
}

long time = -100;
boolean B[4];
boolean Y[4];
boolean Select[4];
boolean Start[4];
boolean N[4];
boolean S[4];
boolean W[4];
boolean E[4];
boolean A[4];
boolean X[4];
boolean L[4];
boolean R[4];


void loop(){
	#ifndef MANUAL_UPDATE
	if(time - millis()>50){
		getButtons();
	}
	#endif

	Serial.print(N[0]);
	Serial.print(S[0]);
	Serial.print(E[0]);
	Serial.println(W[0]);

	delay(250);
}

boolean getA(int player){
	return A[player];
}
boolean getB(int player){
	return B[player];
}
boolean getX(int player){
	return X[player];
}
boolean getY(int player){
	return Y[player];
}
boolean getStart(int player){
	return Start[player];
}
boolean getSelect(int player){
	return Select[player];
}
direction getDir(int player){
	if (N[player]) return NORTH;
	if (S[player]) return SOUTH;
	if (W[player]) return WEST;
	if (E[player]) return EAST;
	return NONE;
}







void getButtons(){
	digitalWrite(latch, HIGH);
	digitalWrite(latch, LOW);

	for(int i = 0; i<4; i++){
		B[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		Y[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		Select[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		Start[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		N[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		S[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		W[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		E[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		A[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		X[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		L[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		R[i]=(digitalRead(data[i])==HIGH);
	}

}

void tick(){
	digitalWrite(clock,HIGH);
	digitalWrite(clock,LOW);
}




