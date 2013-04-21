#ifndef GameOfLightSim_h
#define GameOfLightSim_h
#include "Arduino.h"
#include <GameOfLightBase/GameOfLight.h> 

#define SCREEN_DATA 0x11
#define SCREEN_CMD 0x12
#define SCREEN_DATA_BURST 0x13

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

 private:
     //Keep track of current screen position
     uint8_t _screen_line;
     uint8_t _screen_index;
};

#endif
