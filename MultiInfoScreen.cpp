
#include <Adafruit_GFX.h>
#include "Adafruit_SH1106.h"
#include <RTCDue.h>
#include "TinyGPSplus.h"
#include "defines.h"
#include "MuxDisplay.h"
#include "MultiInfoScreen.h"



MultiInfoScreen::MultiInfoScreen(MuxDisplay * _muxdis, RTCDue *_rtc) {
  muxdis = _muxdis;
  rtc = _rtc;
}

void MultiInfoScreen::init() {  
  uint8_t i = 0;
  while( configs[i].type > SCREEN_TYPE_NULL ) {
    const ScreenConfig * conf = &configs[i];
    muxdis->get(conf->disp_num)->setFont();
    muxdis->get(conf->disp_num)->setTextSize(1);
    muxdis->get(conf->disp_num)->setTextColor(WHITE);
    drawTitle(conf);
    i++;
  }
  
  i = 0;
  while( configs[i].type > SCREEN_TYPE_NULL ) {
    display(configs[i++].disp_num);
  }
}

void MultiInfoScreen::display(uint8_t screen) {
  muxdis->select(screen);
  muxdis->current()->display();
}

void MultiInfoScreen::drawTitle(const ScreenConfig * conf) {
    Adafruit_SH1106 * disp = muxdis->get(conf->disp_num);
    int16_t x,y;
    uint16_t w,h;
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
}

void MultiInfoScreen::setGPSTimeAndDate(struct TinyGPSTime gpstime, struct TinyGPSDate gpsdate) {
  rtc->setHours(gpstime.hour());
  rtc->setMinutes(gpstime.minute());
  rtc->setSeconds(gpstime.second());
  rtc->setDay(gpsdate.day());
  rtc->setMonth(gpsdate.month());
  rtc->setYear(gpsdate.year());
}

void MultiInfoScreen::setGPSLocation(struct TinyGPSLocation gpslocation) {
  
}

void MultiInfoScreen::setGPSSpeed(struct TinyGPSSpeed gpsspeed) {
  
}

void MultiInfoScreen::setBatteryData(uint8_t level) {
  
}

