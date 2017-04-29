
#include <Adafruit_GFX.h>
#include "Adafruit_SH1106.h"
#include <RTCDue.h>
#include "TinyGPSplus.h"
#include "defines.h"
#include "MuxDisplay.h"
#include "MultiInfoScreen.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>



MultiInfoScreen::MultiInfoScreen(MuxDisplay * _muxdis, RTCDue *_rtc) {
  muxdis = _muxdis;
  rtc = _rtc;
  alternativeState = false;
  countTick = 0;
  lastRtcSync = 0;
  battery_state = 0;
  battery_refresh = false;
  battery_last_data = 0;
  board = 111;
  main_bat_voltage = 0;
}

boolean MultiInfoScreen::toggleAlternativeMode() {
  alternativeState = !alternativeState;
  return alternativeState;
}

void MultiInfoScreen::init() {  
  uint8_t i = 0;
  while( configs[i].type > SCREEN_TYPE_NULL ) {
    screen_by_type[configs[i].type] = configs[i].disp_num;
    this->drawHeaderCenter(this->getScreenByType(configs[i].type), configs[i].title);
    i++;
  }
  lastRtcSync = 0;
  
  i = 0;
  while( configs[i].type > SCREEN_TYPE_NULL ) {
    display(configs[i++].disp_num);
  }

  drawGPSNoSignal();
  drawClock();
}

void MultiInfoScreen::display(uint8_t screen) {
  muxdis->select(screen);
  muxdis->current()->display();
}

void MultiInfoScreen::displayByType(uint8_t type) {
  display(screen_by_type[type]);
}

Adafruit_SH1106 * MultiInfoScreen::getScreenByType(uint8_t type) {
  return muxdis->get(screen_by_type[type]);
}

void MultiInfoScreen::drawCenter(Adafruit_SH1106 * disp, uint8_t pos_y, const char * value, uint8_t font_size) {
  int16_t x,y;
  uint16_t w,h;

  // select font
  if( font_size == 9 ) {
    disp->setFont(&FreeSans9pt7b);
  } else if( font_size == 12 ) {
    disp->setFont(&FreeSans12pt7b);
  } else if( font_size == 18 ) {
    disp->setFont(&FreeSans18pt7b);
  } else if( font_size == 24 ) {
    disp->setFont(&FreeSans24pt7b);
  } else {
    disp->setFont();
  }
  
  disp->setTextSize(1);
  disp->setTextColor(WHITE);
  disp->getTextBounds((char*)value,0,0,&x,&y,&w,&h);
  int16_t setx = (disp->width()-w)/2;
  disp->setCursor(setx,pos_y);
  disp->print(value);
}

void MultiInfoScreen::drawValueCenter(Adafruit_SH1106 * disp, const char * value, uint8_t font_size) {
  int8_t xdiff = 0;
  disp->fillRect(0,8,128,56,BLACK);
  if( font_size == 9 ) {
     xdiff = -4;
  } else if( font_size == 18 ) {
     xdiff = 4;
  } else if( font_size == 24 ) {
     xdiff = 6;
  }
  this->drawCenter(disp, 40 + xdiff, value, font_size);
}

void MultiInfoScreen::drawTwoValueCenter(Adafruit_SH1106 * disp, const char * line1, const char * line2, uint8_t font_size) {
  disp->fillRect(0,8,128,56,BLACK);
  this->drawCenter(disp, 28, line1, font_size);
  this->drawCenter(disp, 52, line2, font_size);
}

void MultiInfoScreen::drawFooterCenter(Adafruit_SH1106 * disp, const char * value) {
  disp->fillRect(0,56,128,64,BLACK);
  this->drawCenter(disp, 56, value, 0);
}

void MultiInfoScreen::drawHeaderCenter(Adafruit_SH1106 * disp, const char * value) {
  disp->fillRect(0,0,128,8,BLACK);
  this->drawCenter(disp, 0, value, 0);
}

void MultiInfoScreen::tick() {
  lastTick = millis();
  if( countTick++ == 100 ) {
    countTick = 0;
  }
  
  //static boolean invertState = false;
  //muxdis->select(3);
  //muxdis->current()->invertDisplay(invertState);
  //invertState = !invertState;

  if( countTick % 4 ) { 
    drawLocation();
    drawBattery();
    drawBoard();
    drawSpeed();
    drawMainBattery();
    
  }
  drawEngine();
  drawClock();
  drawMPU();
}

void MultiInfoScreen::drawGPSNoSignal() {
    char buf[30];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_POSITION);
    
    this->drawValueCenter(disp, "Wait for GPS", 9);
    
    sprintf(buf,"Satellites %ld", gpsdata.satelites);
    this->drawFooterCenter(disp, buf);
    
    displayByType(SCREEN_TYPE_POSITION);
}

void MultiInfoScreen::drawClock() {
    char buf[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_DATETIME);
    
    if( !alternativeState ) {
      sprintf(buf, "%02d:%02d:%02d", rtc->getHours(), rtc->getMinutes(), rtc->getSeconds());     
    } else {
      sprintf(buf,"%02d-%02d-%d", rtc->getDay(), rtc->getMonth(), rtc->getYear());
    }
    this->drawValueCenter(disp, buf, 12);

    if( !alternativeState ) {
      sprintf(buf,"%02d-%02d-%d", rtc->getDay(), rtc->getMonth(), rtc->getYear());
    } else {
      sprintf(buf, "%02d:%02d:%02d", rtc->getHours(), rtc->getMinutes(), rtc->getSeconds());
    }
    this->drawFooterCenter(disp, buf);
    
    displayByType(SCREEN_TYPE_DATETIME);
}

void MultiInfoScreen::drawGPSLocation() {
    char buf[20], buf2[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_POSITION);
    
    sprintf(buf,"LOCATION (%ld)", gpsdata.satelites);
    this->drawHeaderCenter(disp, buf);
    
    sprintf(buf,"%c %.4f", (gpsdata.lat<0)?'S':'N', gpsdata.lat);
    sprintf(buf2,"%c %.4f", (gpsdata.lng<0)?'W':'E', gpsdata.lng);
    this->drawTwoValueCenter(disp, buf, buf2, 12);
    displayByType(SCREEN_TYPE_POSITION);
}

void MultiInfoScreen::drawMPU() {
    char buf[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_MPU);
    
    sprintf(buf,"%u", abs(round(mpudata.deg)));
    this->drawValueCenter(disp, buf, 18);
        
    displayByType(SCREEN_TYPE_MPU);
}

void MultiInfoScreen::drawMainBattery() {
    char buf[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_MAINBAT);

    if( main_bat_voltage ) {
        sprintf(buf,"%.1f", main_bat_voltage/10.0);
        this->drawValueCenter(disp, buf, 18);
    } else {
        this->drawValueCenter(disp, "no signal", 9);
    }
    displayByType(SCREEN_TYPE_MAINBAT);
}

void MultiInfoScreen::drawSpeed() {
    char buf[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_SPEED);
    
    sprintf(buf,"%.1f", gpsdata.speed);
    this->drawValueCenter(disp, buf, 18);
    
    displayByType(SCREEN_TYPE_SPEED);
}


void MultiInfoScreen::drawEngine() {
    char buf[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_ENGINE);
    
    sprintf(buf,"%d", enginedata.velocity);
    this->drawValueCenter(disp, buf, 18);
    
    sprintf(buf,"Torque %.1f%% S:%d", abs(enginedata.torque)/10.0, enginedata.status);
    this->drawFooterCenter(disp, buf);
    
    displayByType(SCREEN_TYPE_ENGINE);
}


void MultiInfoScreen::drawBattery() {
    char buf[20];
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_BATTERY);
    
    if( battery_state == 0 ) {
       this->drawValueCenter(disp, "no signal", 9);
       displayByType(SCREEN_TYPE_BATTERY);
       return;
    }
    
    sprintf(buf, "%d%%", battery_soc);
    this->drawValueCenter(disp, buf, 18);
    
    if( millis() - battery_last_data < 1000*20 ) {
      sprintf(buf,"%s", batteryStateToStr(battery_state) );
    } else {
      sprintf(buf,"<no signal>");
    }
    this->drawFooterCenter(disp, buf);
    
    displayByType(SCREEN_TYPE_BATTERY);
    battery_refresh = false;
}

void MultiInfoScreen::drawLocation() {
  if( gpsdata.lat == 0 and gpsdata.lng == 0 ) {
    drawGPSNoSignal();
  } else {
    drawGPSLocation();
  }
}

void MultiInfoScreen::drawBoard() {
  Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_BOARD);
  disp->fillRect(0,10,64,118,BLACK);
  
  if( board == 111 ) {
    this->drawCenter(disp, 55, "no",9);
    this->drawCenter(disp, 75, "signal",9);
  } else {
    uint8_t h;
    char buf[10];
    if( board <= 0 ) {
      sprintf(buf, "UP");
    } else if ( board >= 100 ) {
      sprintf(buf, "DOWN");
    } else {
      sprintf(buf,"%d%%", board);
    }
    this->drawCenter(disp, 45, buf,9);
    if( board >= 100 ) {
      h = 0;
    } else {
      h = (uint16_t)((100-board)*1.08); 
    }

    disp->fillRect(0,10,64,118-h, INVERSE);
  }
  displayByType(SCREEN_TYPE_BOARD);
}


/*
 * Sets GPS values 
 */
void MultiInfoScreen::setGPSData(TinyGPSPlus * gps) {
  /* sync date and time data - every 10 minutes */
  if( gps->time.isValid() && gps->date.isValid() && (rtc->getYear() == 2007 || millis() - lastRtcSync > 1000*600)) {    
    /* to save time we do full sync only if hour is different */
    if( rtc->getHours() !=  gps->time.hour() || rtc->getYear() == gps->date.year() ){
      rtc->setTime(gps->time.hour(), gps->time.minute(), gps->time.second());
      rtc->setDate(gps->date.day(), gps->date.month(), gps->date.year());
      rtc->setClock(rtc->unixtime() + 7200);
    } else {
      /* if RTC has the same hour we fix minute and seconds if there is delay */
      if( rtc->getMinutes() !=  gps->time.minute() ){
         rtc->setMinutes(gps->time.minute());
      }
      if( rtc->getSeconds() !=  gps->time.second() ){
         rtc->setSeconds(gps->time.second());
      }
    }
    lastRtcSync = millis();
  }

  /* number of satelites in use */
  if( gps->satellites.isValid()) {
     gpsdata.satelites = gps->satellites.value();
  } else {
     gpsdata.satelites = 0;
  }

  /* gps location */
  if( gps->location.isValid() ){
    gpsdata.lat = gps->location.lat();
    gpsdata.lng = gps->location.lng();
  } else {
    gpsdata.lat = gpsdata.lng = 0;
  }

  /* speed */
  if( gps->speed.isValid() ){
    gpsdata.speed = gps->speed.kmph();
  } else {
    gpsdata.speed = 0;
  }

  /* course */  
  if( gps->course.isValid() ){
    gpsdata.course = gps->course.deg();
  } else {
    gpsdata.course = 0;
  }
}

void MultiInfoScreen::setMPUData( MPUData data ) {
  mpudata = data;
}

void MultiInfoScreen::setEngineBatteryData( uint8_t state, uint8_t soc) {
  if( battery_state != state || battery_soc != soc ) {
    battery_state = state;
    battery_soc = soc;
    battery_refresh = true; 
  }
  battery_last_data = millis();
}

void MultiInfoScreen::setInverterData( int32_t velocity, int16_t torque, uint16_t status ) {
    enginedata.velocity = velocity;  
    enginedata.torque = torque;
    enginedata.status = status;
}

void MultiInfoScreen::setBoardData( uint8_t state ) {
  board = state;
}


void MultiInfoScreen::setSetMainBattery( uint8_t voltage ) {
  main_bat_voltage = voltage;
}

/*
 * Static function that translate int BMS state to string
 */
const char *  MultiInfoScreen::batteryStateToStr( uint8_t state) {
  switch(state){
     case 1 :
        return "Init";
     case 2 :
        return "Discharge";
     case 3 :
        return "Low";
     case 4 :
        return "Empty";
     case 5 :
        return "Charging";
     case 6 :
        return "High";
     case 7 :
        return "Full";
     case 15 :
        return "Ready";
     case 16 :
        return "Error";
     case 32 :
        return "Sec Prot";
     case 48 :
        return "Over Current";
     case 64 :
        return "Overheated";
     case 80:
        return "LEM Alarm";
     case 96 :
        return "Inv Inp";
     case 224 :
        return "Power Save";
     case 240 :
        return "Param";
  }
  return "Unknown";
}

