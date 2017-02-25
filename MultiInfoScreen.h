#ifndef __MULITINFOSCREEN__
#define __MULTIINFOSCREEN__

#include "TinyGPSplus.h"
#include "MuxDisplay.h"
#include "RTCDue.h"

#define SCREEN_TYPE_NULL 0
#define SCREEN_TYPE_DATETIME 1
#define SCREEN_TYPE_SPEED 2
#define SCREEN_TYPE_BATTERY 3

struct ScreenConfig {
  uint8_t type;
  uint8_t disp_num;
  char * title;
};

class MultiInfoScreen {
  public:
    MultiInfoScreen(MuxDisplay muxdis, RTCDue rtc);

    void init();
    void tick();
    
    /* GPS functions */
    void setGPSTimeAndDate(struct TinyGPSTime gpstime, struct TinyGPSDate gpsdate);
    void setGPSLocation(struct TinyGPSLocation gpslocation);
    void setGPSSpeed(struct TinyGPSSpeed gpsspeed);

    /* Battery Data (from CAN) 
     * TODO: define values and resolution
     */
    void setBatteryData(uint8_t level);

    private:
      void drawTitle(uint8_t screen);
      void display(uint8_t screen);
      
      struct ScreenConfig configs[3] = {
        { SCREEN_TYPE_DATETIME, 1, "Clock" },
        { SCREEN_TYPE_SPEED, 1, "Speed" },
        { SCREEN_TYPE_NULL, 0, "" } // last row
      };
      RTCDue * rtc;
      MuxDisplay * muxdis;
      uint32_t lastTick;
};




#endif
