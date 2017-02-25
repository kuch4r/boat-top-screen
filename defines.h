#ifndef __DEFINES_H_
#define __DEFINES_H_

const uint8_t ButtonPin = 2;     // the number of the pushbutton pin
const uint8_t DisplayResetPin = 28;     // the number of the pushbutton pin
const uint8_t Supply9VenablePin = 11;     // the number of the pushbutton pin
const uint8_t SustainEnablePin = 15;     // the number of the pushbutton pin
const uint8_t LedPin = 4;
const uint8_t PowerButtonPin = 6;

// number of displays
#define MUXDISPLAY_MAX 9

const uint8_t displayConfigs[MUXDISPLAY_MAX][3] = {
  {0, 1, 3},
  {2, 1, 2},
  {2, 0, 2},
  {3, 1, 2},
  {3, 0, 2},
  {5, 1, 0},
  {5, 0, 0},
  {4, 1, 0},
  {4, 0, 0}
};


#endif
