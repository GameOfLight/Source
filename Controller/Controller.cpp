
  Controller.ino - GameOfLight library
  Copyright (c) 2013 Martin Hol√?s.  All right reserved.

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
#include "Controller.h"

#define TICK  digitalWrite(clock,HIGH); digitalWrite(clock,LOW);

boolean Controller::getA(int player){
return A[player];
}
boolean Controller::getB(int player){
return B[player];
}
boolean Controller::getX(int player){
return X[player];
}
boolean Controller::getY(int player){
return Y[player];
}
boolean Controller::getStart(int player){
return Start[player];
}
boolean Controller::getSelect(int player){
return Select[player];
}
direction Controller::getDir(int player){
if (N[player]) return NORTH;
if (S[player]) return SOUTH;
if (W[player]) return WEST;
if (E[player]) return EAST;
return NONE;
}
boolean Controller::getA(int player){
return A[player];
}
boolean Controller::getB(int player){
return B[player];
}
boolean Controller::getX(int player){
return X[player];
}
boolean Controller::getY(int player){
return Y[player];
}
boolean Controller::getStart(int player){
return Start[player];
}
boolean Controller::getSelect(int player){
return Select[player];
}
direction Controller::getDir(int player){
if (N[player]) return NORTH;
if (S[player]) return SOUTH;
if (W[player]) return WEST;
if (E[player]) return EAST;
return NONE;
}







void Controller::getButtons(){
  digitalWrite(latch, HIGH);
  digitalWrite(latch, LOW);

	for(int i = 0; i<4; i++){
		B[i]=(digitalRead(data[i])==HIGH);
	}
	tick();
	
	for(int i = 0; i<4; i++){
		Y[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		Select[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		Start[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		N[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		S[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		W[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		E[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		A[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		X[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		L[i]=(digitalRead(data[i])==HIGH);
	}
	TICK;
	
	for(int i = 0; i<4; i++){
		R[i]=(digitalRead(data[i])==HIGH);
	}

}

