
#include <Adafruit_GFX.h>
#include "Adafruit_SH1106.h"
#include <RTCDue.h>
#include "TinyGPSplus.h"
#include "defines.h"
#include "MuxDisplay.h"
#include "MultiInfoScreen.h"



MultiInfoScreen::MultiInfoScreen(MuxDisplay _muxdis, RTCDue _rtc) {
  muxdis = &_muxdis;
  rtc = & _rtc;
}

void MultiInfoScreen::init() {
  uint8_t i = 0;
  while( configs[i].type > SCREEN_TYPE_NULL ) {
    drawTitle(i);
    display(i);
  }
}

void MultiInfoScreen::display(uint8_t screen) {
  muxdis->select(screen);
  muxdis->current()->display();
}

void MultiInfoScreen::drawTitle(uint8_t screen) {
    const struct ScreenConfig * config = &configs[screen];
    Adafruit_SH1106 * display = muxdis->get(config->disp_num);
    int16_t x,y;
    uint16_t w,h;
    display->getTextBounds(config->title,0,0,&x,&y,&w,&h);
    int16_t setx = (display->width()-w)/2;
    display->setCursor(setx,0);
    display->print(config->title);
}

void MultiInfoScreen::tick() {
  lastTick = millis();
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

