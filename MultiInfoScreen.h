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
#define SCREEN_TYPE_BOARD 6
#define SCREEN_TYPE_MPU 7
#define SCREEN_TYPE_ENGINE 8
#define SCREEN_TYPE_MAINBAT 9

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

typedef struct MPUData {
  double deg;
} MPUData;

typedef struct TemperaturesData {
  float outsidetemp;
  float watertemp;
} TemperaturesData;

typedef struct EngineData {
  int32_t velocity;
  int16_t torque;
  uint16_t status;
} EngineData;


class MultiInfoScreen {
  public:
    MultiInfoScreen(MuxDisplay *muxdis, RTCDue *rtc) ;

    void init();
    void tick();
    boolean toggleAlternativeMode();
    
    /* GPS functions */
    void setGPSData( TinyGPSPlus * data);

    /* MPU function */
    void setMPUData( MPUData data );
  
    void setEngineBatteryData( uint8_t state, uint8_t soc);
    void setInverterData( int32_t velocity, int16_t torque, uint16_t status );
    void setBoardData( uint8_t state);
    void setSetMainBattery( uint8_t voltage );
    void setTemperaturesData( float OutsideTemp, float WaterTemp);

    /* Battery Data (from CAN) 
     * TODO: define values and resolution
     */
    void setBatteryData(uint8_t level);

    private:
      static const char * batteryStateToStr(uint8_t state);
      
      uint8_t screen_by_type[10];
      const struct ScreenConfig configs[10] = {
        { SCREEN_TYPE_DATETIME, 1, "CLOCK" },
        { SCREEN_TYPE_POSITION, 2, "POSITION" },
        { SCREEN_TYPE_SPEED, 3, "SPEED" },
        { SCREEN_TYPE_BATTERY, 4, "BATTERY" },
        { SCREEN_TYPE_TEMP, 5, "TEMPERATURES" },
        { SCREEN_TYPE_BOARD, 0, "BOARD" },
        { SCREEN_TYPE_MPU, 6, "ROLL" },
        { SCREEN_TYPE_ENGINE, 7, "ENGINE" },
        { SCREEN_TYPE_MAINBAT, 8, "MAIN BATTERY" },
        { SCREEN_TYPE_NULL, 0, "" } // last row
      };
      RTCDue * rtc;
      MuxDisplay * muxdis;
      
      uint32_t lastTick;
      uint8_t countTick;
      uint32_t lastRtcSync;

      boolean alternativeState;

      GPSData gpsdata;
      MPUData mpudata;
      EngineData enginedata;
      TemperaturesData temperaturesdata;

      boolean battery_refresh;
      uint32_t battery_last_data;
      uint8_t battery_state;
      uint8_t battery_soc;
      uint8_t board;

      uint8_t main_bat_voltage;
      
      void display(uint8_t screen);
      void displayByType(uint8_t type);
      
      void drawCenter(Adafruit_SH1106 * disp, uint8_t pos_y, const char * value, uint8_t font_size);
      void drawValueCenter(Adafruit_SH1106 * disp, const char * value, uint8_t font_size);
      void drawTwoValueCenter(Adafruit_SH1106 * disp, const char * line1, const char * line2, uint8_t font_size);
      void drawFooterCenter(Adafruit_SH1106 * disp, const char * value);
      void drawHeaderCenter(Adafruit_SH1106 * disp, const char * value);
      
      
      Adafruit_SH1106 * getScreenByType(uint8_t type);

      void drawGPSNoSignal();
      void drawGPSLocation();
      void drawSpeed();
      
      void drawClock();
      void drawLocation();
      void drawBattery();
      void drawMainBattery();

      void drawTemepratures();

      void drawBoard();

      void drawEngine();

      void drawMPU();
};




#endif
