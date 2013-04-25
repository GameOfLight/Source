
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
#include <Arduino.h>
#ifndef Controller_h
#define Controller_h

class Controller{
public:
	void getButtons();
	boolean getA(int Player);
	boolean getB(int Player);
	boolean getX(int Player);
	boolean getY(int Player);
	boolean getStart(int Player);
	boolean getSelect(int Player);
	direction getDir(int Player); //retruns enum NORTH SOUTH EAST WEST NONE TODO sideskift

private:
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

};
#endif
