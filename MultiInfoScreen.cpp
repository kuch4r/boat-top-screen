
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
}

boolean MultiInfoScreen::toggleAlternativeMode() {
  alternativeState = !alternativeState;
  return alternativeState;
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
  lastTick = millis();
  if( countTick++ == 100 ) {
    countTick = 0;
  }
  
  //static boolean invertState = false;
  //muxdis->select(3);
  //muxdis->current()->invertDisplay(invertState);
  //invertState = !invertState;

  if( countTick % 2 ) { 
    drawLocation();
    drawBattery();
    drawBoard();
  }
  drawClock();
  drawMPU();
}

void MultiInfoScreen::drawGPSNoSignal() {
    const char * text = "warming GPS";
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

    sprintf(buf,"Satellites %ld", gpsdata.satelites);
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
    if( !alternativeState ) {
      sprintf(buf, "%02d:%02d:%02d", rtc->getHours(), rtc->getMinutes(), rtc->getSeconds());     
    } else {
      sprintf(buf,"%02d-%02d-%d", rtc->getDay(), rtc->getMonth(), rtc->getYear());
    }
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,40);
    disp->print(buf);
    if( !alternativeState ) {
      sprintf(buf,"%02d-%02d-%d", rtc->getDay(), rtc->getMonth(), rtc->getYear());
    } else {
      sprintf(buf, "%02d:%02d:%02d", rtc->getHours(), rtc->getMinutes(), rtc->getSeconds());
    }
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
    sprintf(buf,"LOCATION (%ld)", gpsdata.satelites);
    disp->setFont();
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,0);
    disp->print(buf);
        
    disp->setFont(&FreeSans9pt7b);
    disp->setTextSize(1);
    disp->setTextColor(WHITE);
    disp->setCursor(7,28);
    sprintf(buf,"%c %f", (gpsdata.lat<0)?'S':'N', gpsdata.lat);
    disp->print(buf);
    disp->setCursor(7,52);
    sprintf(buf,"%c %f", (gpsdata.lng<0)?'W':'E', gpsdata.lng);
    disp->print(buf);
    displayByType(SCREEN_TYPE_POSITION);
}

void MultiInfoScreen::drawMPU() {
    char buf[20];
    int16_t x,y;
    uint16_t w,h;
    Adafruit_SH1106 * disp = getScreenByType(SCREEN_TYPE_MPU);
    disp->fillRect(0,10,128,64,BLACK);
    
    disp->setFont(&FreeSans18pt7b);
    disp->setTextSize(1);
    disp->setTextColor(WHITE);
    sprintf(buf,"%.1f", mpudata.AcY);
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,44);
    disp->print(buf);
    //disp->setCursor(7,52);
    //sprintf(buf,"%d", mpudata.AcX);
    //disp->print(buf);

    sprintf(buf,"Temp %.0f AcX %.1f", mpudata.temp, mpudata.AcX);
    disp->setFont();
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    setx = (disp->width()-w)/2;
    disp->setCursor(setx,56);
    disp->print(buf);
    
    displayByType(SCREEN_TYPE_MPU);
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
      sprintf(buf,"%s", batteryStateToStr(battery_state) );
    } else {
      sprintf(buf,"<no signal>");
    }
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    setx = (disp->width()-w)/2;
    disp->setCursor(setx,56);
    disp->print(buf);
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
  disp->setTextSize(1);
  disp->setTextColor(WHITE);
  disp->setFont(&FreeSans9pt7b);
  if( board == 111 ) {
    disp->setFont(&FreeSans9pt7b);  
    disp->setCursor(20,55);
    disp->print("no");
    disp->setCursor(5,70);
    disp->print("signal");
  } else {
    int16_t x,y;
    uint16_t w,h;
    char buf[10];
    sprintf(buf,"%d%%", board);
    disp->getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (disp->width()-w)/2;
    disp->setCursor(setx,44);
    disp->print(buf);
    if( board >= 100 ) {
      h = 0;
    } else {
      h = (uint16_t)((100-board)*1.18); 
    }
    disp->fillRect(0,10+h,64,118-h, INVERSE);
  }
  displayByType(SCREEN_TYPE_BOARD);
}


/*
 * Sets GPS values 
 */
void MultiInfoScreen::setGPSData(TinyGPSPlus * gps) {
  /* sync date and time data - every 10 minutes */
  if( gps->time.isValid() && gps->date.isValid() && millis() - lastRtcSync > 1000*600) {    
    /* to save time we do full sync only if hour is different */
    if( rtc->getHours() !=  gps->time.hour() ){
      rtc->setTime(gps->time.hour(), gps->time.minute(), gps->time.second());
      rtc->setDate(gps->date.day(), gps->date.month(), gps->date.year());
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
    gpsdata.speed = gps->speed.knots();
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

void MultiInfoScreen::setEngineData( uint8_t power ) {
  
}

void MultiInfoScreen::setBoardData( uint8_t state ) {
  board = state;
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

