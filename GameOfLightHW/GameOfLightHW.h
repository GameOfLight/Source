/*
  GameOfLightHW.h - GameOfLight library
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

#ifndef GameOfLightHW_h
#define GameOfLightHW_h
#include "Arduino.h"
#include <GameOfLightBase/GameOfLight.h> 

GameOfLightHW frame;

class GameOfLightHW : public GameOfLight {
 public:
	GameOfLightHW();
     void begin();

     /* methods for talking to the simulatorscreen */
     void update(); 
     void update(const uint8_t line);
     void clearDisplay();
     void screen_cmd(uint8_t type, uint8_t value);
     void screen_data(uint8_t data);
     void screen_goto(uint8_t index, uint8_t line);

 private:
     //Keep track of current screen position
     uint8_t _screen_line;
     uint8_t _screen_index;
};

#endif
