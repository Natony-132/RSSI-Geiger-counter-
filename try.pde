#include <Wire.h>
#include <PCF8574.h>
#include <RadioLib.h>
#include "pitches.h"

// -------------------------------
// Pins
#define MODE_A_PIN         5
#define MODE_B_PIN         6
#define SETTING_AUDIO_PIN  7
#define SETTING_VIS_PIN    8
#define BUTTON_PIN         9 //TEST button

// -------------------------------
// CC1101 instance: CS, GDO0(D2), GDO2(D4)
CC1101 cc1101 = new Module(10, 2, 4);

void setup() {
  Serial.begin(9600);

  pinMode(MODE_A_PIN, INPUT);
  pinMode(MODE_B_PIN, INPUT);
  pinMode(SETTING_AUDIO_PIN, INPUT);
  pinMode(SETTING_VIS_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  Serial.println(F("Initializing CC1101…"));
  if (cc1101.begin() != RADIOLIB_ERR_NONE) {
    Serial.println(F("CC1101 init failed!"));
    while (true);
  }
  Serial.println(F("CC1101 ready."));

  cc1101.receiveDirect();

  Serial.println(F("Setup complete."));
}

void loop() {
  if (MODE_A_PIN){
    modeA();
  }
  else if(MODE_B_PIN){
    modeB();
  }
  else{
    modeBoth();
  }
}


void modeA(){

}


void modeB(){

}


void modeBoth(){

}





