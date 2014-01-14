/*
  GameOfLightSim.h - GameOfLight library
  Copyright (c) 2013 Stian Selbek.  All right reserved.

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

#ifndef GameOfLightSim_h
#define GameOfLightSim_h
#include "utility/GameOfLight.h"

#define SCREEN_DATA 0x11
#define SCREEN_CMD 0x12
#define SCREEN_DATA_BURST 0x13
#define REQUEST_KEYS 0x14

#define NUM_PLAYERS 2 //change up if more players

class GameOfLightSim : public GameOfLight {
 public:
	GameOfLightSim();
  void begin();

  /* methods for talking to the simulatorscreen */
  void update(); 
  void update(const uint8_t line);
  void clearDisplay();
  void screen_cmd(uint8_t type, uint8_t value);
  void screen_data(uint8_t data);
  void screen_goto(uint8_t index, uint8_t line);
  void getButtons();

  static void btnCallBack();

 private:
    //Keep track of current screen position
    uint8_t _screen_line;
    uint8_t _screen_index;
    uint8_t controller[4];
    char serial_data[10];
    int bytes_red;
    uint8_t btnRequestSent;
};

#endif
