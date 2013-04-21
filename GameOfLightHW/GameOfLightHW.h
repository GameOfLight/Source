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
