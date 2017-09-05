#include <MPU6050.h>

#include <MPU6050.h>

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

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

RTCDue rtc(RC);
MuxDisplay muxdis(displayConfigs);
TinyGPSPlus gps;
MPU6050 mpu(0x68);

MultiInfoScreen screens(&muxdis, &rtc);

void loop_gps();
void can_callback_bms(CAN_FRAME *frame);
void can_callback_board(CAN_FRAME *frame);
void can_callback_inverter(CAN_FRAME * frame);

boolean StartUpFlag = false;

void setup()   {
  //power sustian
  pinMode(SustainEnablePin, OUTPUT);
  StartUpFlag = true;
  POWER_SUSTAIN_ENABLE;
  
  
  pinMode(LedPin, OUTPUT);
  LED_OFF;
    
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
 
  Can0.begin(CAN_BPS_125K);

  // CAN BMS
  Can0.setRXFilter(1, 0x186, 0x186, false);
  Can0.setCallback(1, can_callback_bms);
  // CAN maszt i miecz
  Can0.setRXFilter(2, 0x190, 0x190, false);
  Can0.setCallback(2, can_callback_board);
  // CAN inverter
  Can0.setRXFilter(3, 0x383, 0x383, false);
  Can0.setCallback(3, can_callback_inverter);
  
  pinMode(ButtonPin, INPUT_PULLUP);
  pinMode(DisplayResetPin, OUTPUT);
  pinMode(Supply9VenablePin, OUTPUT);
  pinMode(PowerButtonPin, INPUT_PULLUP);

  DISPLAY_9V_ENABLE;
  delay(10);
  DISPLAY_RESET_ON;
  delay(100);
  DISPLAY_RESET_OFF;

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
  if(digitalRead(PowerButtonPin) == HIGH){
    StartUpFlag = false;
  }
  
  if(digitalRead(PowerButtonPin) == LOW && StartUpFlag == false){
    PowerPinIsPressed = true;
    DISPLAY_9V_DISABLE;//Display power off
    DISPLAY_RESET_ON;
  } else {
    if( PowerPinIsPressed ) {
      delay(10);
      LED_ON;
      POWER_SUSTAIN_DISABLE;
    }
  }

  // handels switching to alternative state
  static boolean ButtonPinIsPressed = false;
  if(digitalRead(ButtonPin) == LOW){
    if( !ButtonPinIsPressed ) {
       if( screens.toggleAlternativeMode() ) {
          LED_ON;
          Serial.println("LED_ON");
       } else {
          LED_OFF;
          Serial.println("LED_OFF");
       }
    }
    ButtonPinIsPressed = true;
  } else {
    ButtonPinIsPressed = false;
  }
   

  static const unsigned long REFRESH_INTERVAL = 250; // ms
  static unsigned long lastRefreshTime = 0;

  screens.setMPUData( get_MPU_data() );
  
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime = millis();
    screens.tick(); 
  }
  delay(10);   
}

void loop_gps() {
  
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (gps.encode(c)) {
      //Serial.println("GPS data");
      screens.setGPSData(&gps);
      yield();
    }
   }
  delay(500);
}

/*
 * CAN callback
 */

/* BMS can callback */
void can_callback_bms(CAN_FRAME *frame) {
  if( frame->id == 0x186 ) {
    Serial.println("Can MSG 186");
    screens.setEngineBatteryData(frame->data.bytes[7], frame->data.bytes[4]);
  }
}

/* Winches controler (Board and mast) */
void can_callback_board(CAN_FRAME *frame) {
  if( frame->id == 0x190 ) {
    screens.setBoardData(frame->data.bytes[1]);
    screens.setSetMainBattery(frame->data.bytes[2]);
  }
}

/* Eletric Engine Inverter */
void can_callback_inverter(CAN_FRAME *frame) {
  if( frame->id == 0x383 ) {
    screens.setInverterData(frame->data.high, frame->data.s1, frame->data.s0);
  }
}

/*
 * Gets data from MPU 
 */

#define MAX_MPU_READINGS 4

MPUData get_MPU_data() {
  MPUData data;
  static float readings[MAX_MPU_READINGS];
  static uint8_t readings_ptr = 0;
  static bool has_full_read = false;
  
  int16_t ax, ay, az, gx, gy, gz;
  
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float nax = ax * .000061f * 9.80665f;
  float nay = ay * .000061f * 9.80665f;
  float deg  = (atan(nay/nax)*180.0)/M_PI;

  readings[readings_ptr++] = deg;
  if( readings_ptr >= MAX_MPU_READINGS ) {
    readings_ptr = 0;
    has_full_read = true;
  }
  if( has_full_read ) {
    float result = 0;
    for( uint8_t i = 0; i < MAX_MPU_READINGS; i++) {
      result += readings[i];
    }
    data.deg = (int)(result/MAX_MPU_READINGS);
  } else {
    data.deg = 0;
  }
  return data;
}




