
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
  lastRtcSync = 0;
  lat = lng = 0;
  gps_sats = 0;
  battery_state = 0;
  battery_refresh = false;
  battery_last_data = 0;
}

void MultiInfoScreen::init() {  
  uint8_t i = 0;
  while( configs[i].type > SCREEN_TYPE_NULL ) {
    screen_by_type[configs[i].type] = configs[i].disp_num;
    const ScreenConfig * conf = &configs[i];
    drawTitle(conf);
    i++;
  }
  
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

void MultiInfoScreen::drawTitle(const ScreenConfig * conf) {
    Adafruit_SH1106 * disp = muxdis->get(conf->disp_num);
    int16_t x,y;
    uint16_t w,h;
    disp->setFont();
    disp->setTextSize(1);
    disp->setTextColor(WHITE);
    disp->getTextBounds((char*)conf->title,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,0);
    disp->print(conf->title);
}

void MultiInfoScreen::tick() {
  static boolean invertState = false;
  lastTick = millis();
  muxdis->select(3);
  muxdis->current()->invertDisplay(invertState);
  invertState = !invertState;

  drawClock();
  drawLocation();
  drawBattery();
}

void MultiInfoScreen::drawGPSNoSignal() {
    const char * text = "wait for GPS";
    char buf[30];
    int16_t x,y;
    uint16_t w,h;
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_POSITION);
    disp->fillRect(0,10,128,54,BLACK);
    disp->setFont(&FreeSans9pt7b);
    disp->setTextSize(1);
    disp->setTextColor(WHITE);    
    disp->getTextBounds((char*)text,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,40);
    disp->print(text);

    sprintf(buf,"Satellites %d", gps_sats);
    disp->setFont();
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    setx = (disp->width()-w)/2;
    disp->setCursor(setx,56);
    disp->print(buf);
    displayByType(SCREEN_TYPE_POSITION);
}

void MultiInfoScreen::drawClock() {
    char buf[20];
    int16_t x,y;
    uint16_t w,h;
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_DATETIME);
    disp->fillRect(0,10,128,54,BLACK);
    disp->setFont(&FreeSans12pt7b);
    disp->setTextSize(1);
    disp->setTextColor(WHITE);
    sprintf(buf, "%02d:%02d:%02d", rtc->getHours(), rtc->getMinutes(), rtc->getSeconds());     
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,40);
    disp->print(buf);

    sprintf(buf,"%02d-%02d-%d", rtc->getDay(), rtc->getMonth(), rtc->getYear());
    disp->setFont();
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    setx = (disp->width()-w)/2;
    disp->setCursor(setx,56);
    disp->print(buf);
    displayByType(SCREEN_TYPE_DATETIME);
}

void MultiInfoScreen::drawGPSLocation() {
    char buf[20];
    int16_t x,y;
    uint16_t w,h;
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_POSITION);
    disp->fillRect(0,0,128,64,BLACK);
    sprintf(buf,"LOCATION (%d)", gps_sats);
    disp->setFont();
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,0);
    disp->print(buf);
    
    disp->setFont(&FreeSans9pt7b);
    disp->setTextSize(1);
    disp->setTextColor(WHITE);
    disp->setCursor(7,28);
    sprintf(buf,"%c %f", (lat<0)?'S':'N', lat);
    disp->print(buf);
    disp->setCursor(7,52);
    sprintf(buf,"%c %f", (lng<0)?'W':'E', lng);
    disp->print(buf);
    displayByType(SCREEN_TYPE_POSITION);
}

void MultiInfoScreen::drawBattery() {
    char buf[20];
    int16_t x,y;
    uint16_t w,h;
    if( battery_state == 0 ) {
       return;
    }
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_BATTERY);
    disp->fillRect(0,10,128,54,BLACK);
    disp->setFont(&FreeSans18pt7b);
    disp->setTextSize(1);
    disp->setTextColor(WHITE);
    sprintf(buf, "%d%%", battery_soc);
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,44);
    disp->print(buf);

    disp->setFont();
    if( millis() - battery_last_data < 1000*20 ) {
      sprintf(buf,"%s", batteryStateStr(battery_state) );
    } else {
      sprintf(buf,"<%s>", batteryStateStr(battery_state) );
    }
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    setx = (disp->width()-w)/2;
    disp->setCursor(setx,56);
    disp->print(buf);
    displayByType(SCREEN_TYPE_BATTERY);
   
    battery_refresh = false;
}

void MultiInfoScreen::drawLocation() {
  if( lat == 0 and lng == 0 ) {
    drawGPSNoSignal();
  } else {
    drawGPSLocation();
  }
}

void MultiInfoScreen::setGPSTimeAndDate(struct TinyGPSTime gpstime, struct TinyGPSDate gpsdate) {
  if( !gpstime.isValid() || !gpsdate.isValid() || millis() - lastRtcSync < 1000*60) {
    return;
  }
  if( rtc->getHours() !=  gpstime.hour() ){
    rtc->setTime(gpstime.hour(), gpstime.minute(), gpstime.second());
    rtc->setDate(gpsdate.day(), gpsdate.month(), gpsdate.year());
  } else {
    if( rtc->getMinutes() !=  gpstime.minute() ){
       rtc->setMinutes(gpstime.minute());
    }
    if( rtc->getSeconds() !=  gpstime.second() ){
       rtc->setSeconds(gpstime.second());
    }
  }
  lastRtcSync = millis();
}

void MultiInfoScreen::setGPSLocation(struct TinyGPSLocation gpslocation, struct TinyGPSInteger sats) {
  if( sats.isValid()) {
     gps_sats = sats.value();
  } else {
     gps_sats = 0;
  }
  
  if( !gpslocation.isValid() ){
    lat = lng = 0;
  } else {
    lat = gpslocation.lat();
    lng = gpslocation.lng();
  }
}

void MultiInfoScreen::setGPSSpeed(struct TinyGPSSpeed gpsspeed) {
  
}

void MultiInfoScreen::setBatteryData(uint8_t level) {
  
}

void MultiInfoScreen::setEngineBattery( uint8_t state, uint8_t soc) {
  if( battery_state != state || battery_soc != soc ) {
    battery_state = state;
    battery_soc = soc;
    battery_refresh = true; 
  }
  battery_last_data = millis();
}


const char *  MultiInfoScreen::batteryStateStr( uint8_t state) {
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

