#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SH1106.h"
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#include <Scheduler.h>

#include <due_can.h>
#include "TinyGPSplus.h"

#include "defines.h"
#include "MuxDisplay.h"
#include "MultiInfoScreen.h"

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

RTCDue rtc(RC);
MuxDisplay muxdis(displayConfigs);
TinyGPSPlus gps;

MultiInfoScreen screens(&muxdis, &rtc);

void setup()   {
  pinMode(SustainEnablePin, OUTPUT);
  digitalWrite(SustainEnablePin, HIGH);
  
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, LOW);
    
  //inicjalizacje                
  Serial.begin(9600);
  Serial.println("Power up");

  Serial1.begin(9600);

  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Can0.begin(CAN_BPS_250K);
  Can0.watchFor();
  
  pinMode(ButtonPin, INPUT_PULLUP);
  pinMode(PowerButtonPin, INPUT_PULLUP);
  pinMode(DisplayResetPin, OUTPUT);
  pinMode(Supply9VenablePin, OUTPUT);

  digitalWrite(Supply9VenablePin, LOW);
  delay(10);
  digitalWrite(DisplayResetPin, LOW);
  delay(100);
  digitalWrite(DisplayResetPin, HIGH);

  Serial.println("Init 1");
  rtc.begin();
  Serial.println("Init 2");
  muxdis.init();
  Serial.println("Init 3");
  
  screens.init();
  Serial.println("Init 4");

  Scheduler.startLoop(loop_com);
  Serial.println("Init 5");
}


void loop() {
  CAN_FRAME incoming;
  
  if(digitalRead(PowerButtonPin) == LOW){
    digitalWrite(LedPin, HIGH);
  }
  else{
    digitalWrite(LedPin, LOW);
  }
  
  if(digitalRead(ButtonPin) == LOW){
    digitalWrite(SustainEnablePin, LOW);
  }

   if (Can0.available() > 0) {
      Can0.read(incoming);
      //muxdis.select(1);
      //drawDisp( incoming.data.bytes[0], muxdis.current() );
   }
   
  static const unsigned long REFRESH_INTERVAL = 1000; // ms
  static unsigned long lastRefreshTime = 0;
  
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
  {
    Serial.println("Screen tick");
    lastRefreshTime = millis();
    screens.tick(); 
  }
  delay(10);   
   //MPUTest();
}

void loop_com() {
  Serial.println("GPS tick");
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (gps.encode(c)) {
      //screens.setGPSTimeAndDate(gps.time, gps.date);
      yield();
    }
   }
  delay(300);
}



/*
void setCursorForCenterText( Adafruit_SH1106 &display, const char*buf){
  int16_t x,y;
    uint16_t w,h;
    display.getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    display.setCursor((display.width()-w)/2,((display.height()-h)/2)+h);
}

void setCursorForTopText( Adafruit_SH1106 &display, const char*buf){
    int16_t x,y;
    uint16_t w,h;
    display.getTextBounds((char*)buf,0,0,&x,&y,&w,&h);
    int16_t setx = (display.width()-w)/2;
    display.setCursor(setx,0);
}

void drawDisp( uint8_t num, Adafruit_SH1106 display ) {
  char buf[3];
  buf[0] = '0' + num;
  buf[1] = '0' + num;
  buf[2] = 0;
  display.clearDisplay();
  
  
  display.setFont();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  setCursorForTopText(display,"Time");
  display.print("Time");
  
  display.setFont(&FreeSans24pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  setCursorForCenterText(display, buf);
  display.print(buf);
  
  display.display();
}

void MPUTest() {
  int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
  
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(0x68,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  Serial.print("AcX = "); Serial.print((int)AcX/182.05);
  Serial.print(" | AcY = "); Serial.print((int)AcY/182.05);
  Serial.print(" | AcZ = "); Serial.print((int)AcZ/182.05);
  Serial.print(" | Tmp = "); Serial.println(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
  //Serial.print(" | GyX = "); Serial.print(GyX);
  //Serial.print(" | GyY = "); Serial.print(GyY);
  //Serial.print(" | GyZ = "); Serial.println(GyZ);
  delay(333);
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  
  Serial.print( " SAT: ");
    if( gps.satellites.isValid() ) {
      Serial.print( gps.satellites.value() );
    } else {
      Serial.print("INVALID");
    }
  Serial.print( " LAT: ");
    if( gps.altitude.isValid() ) {
      Serial.print( gps.altitude.meters() );
    } else {
      Serial.print("INVALID");
    }
  Serial.println();

  Serial.print( " SPEED: ");
    if( gps.speed.isValid() ) {
      Serial.print( gps.speed.kmph() );
    } else {
      Serial.print("INVALID");
    }
  Serial.println();

  Serial.print( " COURS: ");
    if( gps.course.isValid() ) {
      Serial.print( gps.course.deg() );
    } else {
      Serial.print("INVALID");
    }
  Serial.println();
}
*/

