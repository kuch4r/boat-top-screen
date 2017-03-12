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
/* multiplexer port, i2c addr (0-0x3c, 1-0x3d), rotation (0 - normal, 1- 90, 2 - 180, 3 - 270) */
  {0, 1, 1},
  {2, 1, 0},
  {2, 0, 0},
  {3, 1, 0},
  {3, 0, 0},
  {5, 1, 2},
  {5, 0, 2},
  {4, 1, 2},
  {4, 0, 2}
};


#endif
