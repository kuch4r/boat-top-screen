
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SH1106.h"
#include "defines.h"
#include "MuxDisplay.h"


/*
 * Constructor
 */
MuxDisplay::MuxDisplay( const uint8_t _configs[][3]) {
  // init internval vars
  selectedPort = 99;
  selectedDisplay = 99;

  // init displays based on provided configs
  for( uint8_t i = 0 ; i < MUXDISPLAY_MAX ; i++ ) {
     //store config
     configs[i][0] = _configs[i][0];
     configs[i][1] = _configs[i][1];
     configs[i][2] = _configs[i][2];
  }
}

void MuxDisplay::init() {
  Wire1.begin();
  
  for( uint8_t i = 0 ; i < MUXDISPLAY_MAX ; i++ ) {
     // select port on mux
     selectPort( configs[i][0]);

     // init display
     displays[i].initDisplay(SH1106_EXTERNALVCC , 0x3C + configs[i][1]);
     displays[i].setRotation(configs[i][2]);    
     displays[i].clearDisplay();
     displays[i].display();
  }
}

void MuxDisplay::select( uint8_t num ) {
  if( selectedDisplay == num ) {
    return;
  }
  // switch port on mux
  selectPort( configs[num][0] ); 
  selectedDisplay = num;
}

Adafruit_SH1106 * MuxDisplay::current() {
  return &displays[selectedDisplay];
}

Adafruit_SH1106 * MuxDisplay::get(uint8_t num) {
  return &displays[num];
}


void MuxDisplay::selectPort( uint8_t port ) {
  if( selectedPort != port ) {    
    Wire1.beginTransmission(0x70);
    Wire1.write(1 << port);
    Wire1.endTransmission();
    selectedPort = port;
  }
}

