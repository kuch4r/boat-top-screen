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

#include <MPU6050.h>
#include "KalmanFilter.h"

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

KalmanFilter kalmanX(0.001, 0.003, 0.03);
KalmanFilter kalmanY(0.001, 0.003, 0.03);

RTCDue rtc(RC);
MuxDisplay muxdis(displayConfigs);
TinyGPSPlus gps;
MPU6050 mpu(0x68);

MultiInfoScreen screens(&muxdis, &rtc);

void loop_gps();
void can_callback_bms(CAN_FRAME *frame);
void can_callback_board(CAN_FRAME *frame);
void can_callback_inverter(CAN_FRAME * frame);
void setup()   {
  pinMode(SustainEnablePin, OUTPUT);
  digitalWrite(SustainEnablePin, HIGH);
  
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, LOW);
    
  //Init                
  Serial.begin(9600);
  Serial.println("Power up");

  // GPS serial
  Serial1.begin(9600);

  Wire.begin();

  mpu.initialize();

  mpu.setXAccelOffset(-461);
  mpu.setYAccelOffset(1623);
  mpu.setZAccelOffset(1455);
  mpu.setXGyroOffset(1);
  mpu.setXGyroOffset(39);
  mpu.setXGyroOffset(55);
 
  Can0.begin(CAN_BPS_250K);

  // CAN BMS
  Can0.setRXFilter(1, 0x186, 0x186, false);
  Can0.setCallback(1, can_callback_bms);
  // CAN maszt i miecz
  Can0.setRXFilter(2, 0x286, 0x286, false);
  Can0.setCallback(2, can_callback_board);
  // CAN inverter
  Can0.setRXFilter(3, 0x383, 0x383, false);
  Can0.setCallback(3, can_callback_inverter);
  
  pinMode(ButtonPin, INPUT_PULLUP);
  pinMode(PowerButtonPin, INPUT_PULLUP);
  pinMode(DisplayResetPin, OUTPUT);
  pinMode(Supply9VenablePin, OUTPUT);

  digitalWrite(Supply9VenablePin, LOW);
  delay(10);
  digitalWrite(DisplayResetPin, LOW);
  delay(100);
  digitalWrite(DisplayResetPin, HIGH);

  // start RTC clock
  rtc.begin();

  // init 1Wire displays behind multiplexer
  muxdis.init();
  // init screens class (draws)
  screens.init();
 
  // start gps loop
  Scheduler.startLoop(loop_gps);
}


void loop() {
  // handels powering off
  static boolean PowerPinIsPressed = false;
  if(digitalRead(PowerButtonPin) == LOW){
    PowerPinIsPressed = true;
  } else {
    if( PowerPinIsPressed ) {
      delay(10);
      digitalWrite(LedPin, HIGH);
      digitalWrite(SustainEnablePin, LOW);
    }
  }

  // handels switching to alternative state
  static boolean ButtonPinIsPressed = false;
  if(digitalRead(ButtonPin) == LOW){
    if( !ButtonPinIsPressed ) {
       if( screens.toggleAlternativeMode() ) {
          digitalWrite(LedPin, HIGH);
       } else {
          digitalWrite(LedPin, LOW);
       }
    }
    ButtonPinIsPressed = true;
  } else {
    ButtonPinIsPressed = false;
  }
   
  static const unsigned long REFRESH_INTERVAL = 500; // ms
  static unsigned long lastRefreshTime = 0;

  screens.setMPUData( get_MPU_data() );
  
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime = millis();
    screens.tick(); 
  }
  delay(10);   
  
}

void loop_gps() {
  Serial.println("GPS tick");
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (gps.encode(c)) {
      screens.setGPSData(&gps);
      yield();
    }
   }
  delay(300);
}

void can_callback_bms(CAN_FRAME *frame) {
  if( frame->id == 0x186 ) {
    screens.setEngineBatteryData(frame->data.bytes[7], frame->data.bytes[4]);
  }
}

void can_callback_board(CAN_FRAME *frame) {
  if( frame->id == 0x286 ) {
    screens.setBoardData(frame->data.bytes[0]);
  }
}

void can_callback_inverter(CAN_FRAME *frame) {
  if( frame->id == 0x383 ) {
    screens.setInverterData(frame->data.low, frame->data.s2, frame->data.s3);
  }
}



MPUData get_MPU_data() {
  MPUData data;

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float nax = ax * .000061f * 9.80665f;
  float nay = ay * .000061f * 9.80665f;
  float deg  = (atan(nay/nax)*180.0)/M_PI;

  // Kalman filter
  float degkal = kalmanY.update(deg, gz * .000061f);
  
  data.temp = (mpu.getTemperature()/340.)+36.53;
  data.AcX = (int)degkal;
  data.AcY = (int)deg;
  return data;
}

//MPUData get_MPU_data() {
//  int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
//  MPUData data;
//    
//  Wire.beginTransmission(0x68);
//  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
//  Wire.endTransmission(false);
//  Wire.requestFrom(0x68,14,true);  // request a total of 14 registers
//  
//  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
//  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
//  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
//  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
//  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
//  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
//  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
//  
//  //Serial.print("AcX = "); Serial.print((int)AcX/182.05);
//  //Serial.print(" | AcY = "); Serial.print((int)AcY/182.05);
//  //Serial.print(" | AcZ = "); Serial.print((int)AcZ/182.05);
//  //Serial.print(" | Tmp = "); Serial.println(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
//  //Serial.print(" | GyX = "); Serial.print(GyX);
//  //Serial.print(" | GyY = "); Serial.print(GyY);
//  //Serial.print(" | GyZ = "); Serial.println(GyZ);
//
//  data.temp = Tmp/340.00+36.53;
//  data.AcX = (int)AcX/182.05;
//  data.AcY = (int)AcY/182.05;
//  data.AcZ = (int)AcZ/182.05;
//  return data;
//}


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

