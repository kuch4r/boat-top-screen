#ifndef __MULITINFOSCREEN__
#define __MULTIINFOSCREEN__

#include "TinyGPSplus.h"
#include "MuxDisplay.h"
#include "Adafruit_SH1106.h"
#include "RTCDue.h"

#define SCREEN_TYPE_NULL 0
#define SCREEN_TYPE_DATETIME 1
#define SCREEN_TYPE_POSITION 2
#define SCREEN_TYPE_SPEED 3
#define SCREEN_TYPE_BATTERY 4
#define SCREEN_TYPE_TEMP 5

struct ScreenConfig {
  uint8_t type;
  uint8_t disp_num;
  const char * title;
};

class MultiInfoScreen {
  public:
    MultiInfoScreen(MuxDisplay *muxdis, RTCDue *rtc);

    void init();
    void tick();
    
    /* GPS functions */
    void setGPSTimeAndDate(struct TinyGPSTime gpstime, struct TinyGPSDate gpsdate);
    void setGPSLocation(struct TinyGPSLocation gpslocation, struct TinyGPSInteger sats);
    void setGPSSpeed(struct TinyGPSSpeed gpsspeed);

    void setEngineBattery( uint8_t state, uint8_t soc);

    /* Battery Data (from CAN) 
     * TODO: define values and resolution
     */
    void setBatteryData(uint8_t level);

    private:
      uint8_t screen_by_type[10];
          
      void drawTitle(const ScreenConfig * conf);
      void display(uint8_t screen);
      void displayByType(uint8_t type);
      Adafruit_SH1106 * getScreenByType(uint8_t type);

      void drawGPSNoSignal();
      void drawGPSLocation();
      
      void drawClock();
      void drawLocation();
      void drawBattery();

      const char * batteryStateStr(uint8_t state);
      
      const struct ScreenConfig configs[6] = {
        { SCREEN_TYPE_DATETIME, 1, "CLOCK" },
        { SCREEN_TYPE_POSITION, 2, "POSITION" },
        { SCREEN_TYPE_SPEED, 3, "SPEED" },
        { SCREEN_TYPE_BATTERY, 4, "BATTERY" },
        { SCREEN_TYPE_TEMP, 5, "TEMPERATURES" },
        { SCREEN_TYPE_NULL, 0, "" } // last row
      };
      RTCDue * rtc;
      MuxDisplay * muxdis;
      uint32_t lastTick;
      uint32_t lastRtcSync;

      double lat,lng;
      uint32_t gps_sats;

      boolean battery_refresh;
      uint32_t battery_last_data;
      uint8_t battery_state;
      uint8_t battery_soc;
};




#endif
