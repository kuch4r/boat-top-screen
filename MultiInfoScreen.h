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

typedef struct GPSData {
  int32_t satelites;
  double lat;
  double lng;
  double speed;
  double course;
  GPSData() { 
    satelites = lat = lng = speed = course = 0; 
  }
} GPSData;

class MultiInfoScreen {
  public:
    MultiInfoScreen(MuxDisplay *muxdis, RTCDue *rtc) ;

    void init();
    void tick();
    
    /* GPS functions */
    void setGPSData( TinyGPSPlus * data);
    void setEngineBatteryData( uint8_t state, uint8_t soc);

    /* Battery Data (from CAN) 
     * TODO: define values and resolution
     */
    void setBatteryData(uint8_t level);

    private:
      static const char * batteryStateToStr(uint8_t state);
      
      uint8_t screen_by_type[10];
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

      GPSData gpsdata;

      boolean battery_refresh;
      uint32_t battery_last_data;
      uint8_t battery_state;
      uint8_t battery_soc;

      
      void display(uint8_t screen);
      void displayByType(uint8_t type);
      void drawTitle(const ScreenConfig * conf);
      
      Adafruit_SH1106 * getScreenByType(uint8_t type);

      void drawGPSNoSignal();
      void drawGPSLocation();
      
      void drawClock();
      void drawLocation();
      void drawBattery();
};




#endif
