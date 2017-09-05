#ifndef __DEFINES_H_
#define __DEFINES_H_

const uint8_t ButtonPin = 2;     // the number of the pushbutton pin
const uint8_t DisplayResetPin = 28;     // the number of the pushbutton pin
const uint8_t Supply9VenablePin = 11;     // the number of the pushbutton pin
const uint8_t SustainEnablePin = 15;     // the number of the pushbutton pin
const uint8_t LedPin = 4;
const uint8_t PowerButtonPin = 6;


#define DISPLAY_9V_ENABLE digitalWrite(Supply9VenablePin, LOW)
#define DISPLAY_9V_DISABLE digitalWrite(Supply9VenablePin, HIGH)

#define DISPLAY_RESET_ON digitalWrite(DisplayResetPin, LOW)
#define DISPLAY_RESET_OFF digitalWrite(DisplayResetPin, HIGH)

#define POWER_SUSTAIN_ENABLE digitalWrite(SustainEnablePin, HIGH)
#define POWER_SUSTAIN_DISABLE digitalWrite(SustainEnablePin, LOW)

#define LED_ON digitalWrite(LedPin, HIGH);
#define LED_OFF digitalWrite(LedPin, LOW);

// number of displays
#define MUXDISPLAY_MAX 9


const uint8_t displayConfigs[MUXDISPLAY_MAX][3] = {
/* multiplexer port, i2c addr (0-0x3c, 1-0x3d), rotation (0 - normal, 1- 90, 2 - 180, 3 - 270) */
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
