
#ifndef __MUXDISPLAY__
#define __MUXDISPLAY__

#include "Adafruit_SH1106.h"

#ifndef MUXDISPLAY_MAX
#error("Please define MUXDISPLAY_MAX");
#endif

class MuxDisplay {
  public:
    MuxDisplay( const uint8_t display[][3] );

    void init();
    void select( uint8_t );
    Adafruit_SH1106 * get(uint8_t);
    Adafruit_SH1106 * current();

  private:
    uint8_t selectedPort;
    uint8_t selectedDisplay;
    uint8_t configs[MUXDISPLAY_MAX][3]; // first number is for mux port, second is for I2C address
    Adafruit_SH1106 displays[MUXDISPLAY_MAX];

    void selectPort( uint8_t port);
};














#endif
