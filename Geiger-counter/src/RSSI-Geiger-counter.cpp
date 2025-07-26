 #include "pitches.h"

#include <Arduino.h>

#include <PCF8574.h>

#include <RadioLib.h>
#include <Wire.h>

// CC1101 instance: CS, GDO0(D2), GDO2(D4), SPI pins
CC1101 cc1101 = new Module(10, 2, 4);

PCF8574 PCF_20(0x20); // LEDs 1–8
PCF8574 PCF_24(0x24); // LEDs 9–16

// Pins
#define MODE_A_PIN 5
#define MODE_B_PIN 6
#define SETTING_AUDIO_PIN 7
#define SETTING_VIS_PIN 8
#define BUTTON_PIN 14
#define SPEAKER_PIN 3
#define VOLTMETER_PIN 9

// Globals
int rawRSSI[16];
int superArray[10]; // 0–7 RSSI, 8 max, 9 exclude              

// Frequencies
float frequencies[16] = {462.5625, 462.5875, 462.6125, 462.6375,
                         462.6625, 462.6875, 462.7125, 433.0000,
                         462.5500, 462.5750, 462.6000, 462.6250,
                         462.6500, 462.6750, 462.7000, 462.7250};

// Threshold
const int limRSSI = -50;

int pos = 7;

int list[8];    // list of the last few RSSI values
int pass;
int passHold;
int max = -999;
bool testLast = false;
int Exclude = 8;
bool ModeA;
bool ModeB;
bool ModeBoth;
bool Audio;
bool VIS;
bool AV; 
bool test;

void getOneRSSI(int);
void setExclude(void);
void maxFill(void);
void getMode(void);
void getSetting(void);
void getTest(void);

void ExcludePos(void);
void VOLT(void);

void MODE_A(void);
void MODE_B(void);
void MODE_BOTH(void);

void visPlay(void);
void audioPlay(void);
int mapRSSItoRad(int);

//-------------------Functions

void MODE_A() {
  
  if(test){
    pass = -1;
  }
  else{
    getOneRSSI(pos);
    testLast = false;
  }
}

void MODE_B(){
  if(test){
    if(VIS) {
      setExclude();
    }
    else {
      pass = -1;
    }
    testLast = true;
  }
  else{
    getOneRSSI(pos + 8 );
    testLast = false;
  }
}

void MODE_BOTH(){
    if(test){
    pass = -1;
  }
  else{
    getOneRSSI(pos);
    passHold = pass;
    getOneRSSI(pos + 8);

    if(passHold > pass) {
      pass = passHold;
    }

    testLast = false;
  }
}

void VOLT() {
  // constrain max between -100 and 0
  int voltOut = map(constrain(max, -100, 0), -100, 0, 0, 255);
  analogWrite(VOLTMETER_PIN, voltOut);

  Serial.print(F("Voltmeter output: "));
  Serial.println(voltOut);
}


void getMode(void){
  if(digitalRead(MODE_A_PIN)) {
    ModeA = true;
    ModeB = false;
    ModeBoth = false;
  }
  else if(digitalRead(MODE_B_PIN)) {
    ModeA = false;
    ModeB = true;
    ModeBoth = false;
  }
  else{
    ModeA = false;
    ModeB = false;
    ModeBoth = true;
  }
}

void getSetting(void){
  if(digitalRead(SETTING_AUDIO_PIN)) {
    Audio = true;
    VIS = false;
    AV = false;
  }
  else if(digitalRead(SETTING_VIS_PIN)) {
    Audio = false;
    VIS = true;
    AV = false;
  }
  else{
    Audio = false;
    VIS = false;
    AV = true;
  }
}

void getTest(void){
  
  if(digitalRead(BUTTON_PIN)) {
    test = true;
  }
  else{
    test = false;
  }
}

void setExclude(void){
  if (!testLast){
    Exclude++;
    if (Exclude > 8){
      Exclude = 0;
    }
  }
testLast = true;
}

void getOneRSSI(int spot) {
  
    cc1101.setFrequency(frequencies[spot]);
    cc1101.receiveDirect();
    delay(5);
    pass = cc1101.getRSSI();

}

void maxFill(void) {

  list[pos] = pass;
  max = pass;
  // Find max

  for (int i = 0; i < 8; i++) {
    if (list[i] > max) {
      max = list[i];
    }
  }
}

void ExcludePos(){
  if (pos == Exclude) {
    pass = -333;
  }

}



void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("Setup starting..."));

  pinMode(MODE_A_PIN, INPUT);
  pinMode(MODE_B_PIN, INPUT);
  pinMode(SETTING_AUDIO_PIN, INPUT);
  pinMode(SETTING_VIS_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  Wire.begin();
  PCF_20.begin();
  PCF_24.begin();

  if (cc1101.begin() != RADIOLIB_ERR_NONE) {
    Serial.println(F("CC1101 init failed!"));
    while (true)
      ;
  }
  cc1101.receiveDirect();

  Serial.println(F("Setup complete."));
}



void loop() {//------------------------------------------------------------------------------------------------------------- I found the LOOP!
//part 1
  pos++;
  if(pos > 7){
    pos = 0;
  }

  getMode();
  getSetting();
  getTest();

//part 2
  if(ModeA){
    MODE_A();
  }
  if(ModeB){
    MODE_B();
  }
  if (ModeBoth){
    MODE_BOTH();
  }

  ExcludePos();
  maxFill();

//part 3
  if (Audio || AV){
    audioPlay();
  }

  if (VIS || AV){
    visPlay();
    VOLT();
  }
  else{
    analogWrite(VOLTMETER_PIN, 0);
  }

}

// -------------------------------------------
// VISUAL OUTPUT
void visPlay() {

  if (pass > limRSSI){
    PCF_20.write(pos, 0);
    
  }
  else{
    PCF_20.write(pos, 1);
  }

  if (pass == -333){
    PCF_24.write(pos, 0);
  }
  else {
    PCF_24.write(pos, 1);
  }
    
}

// -------------------------------------------
// AUDIO OUTPUT
void audioPlay() {
  
  int radReal = mapRSSItoRad(max);

  int radLim = random(1, 101);

  if (radLim < radReal) {
    int noteDuration = 4;
    tone(SPEAKER_PIN, NOTE_E3, noteDuration);
    Serial.println(F("Playing tone."));
  }

  int voltOut = map(constrain(max, -100, 0), -100, 0, 0, 255);
  analogWrite(VOLTMETER_PIN, voltOut);
  Serial.print(F("Voltmeter output: "));
  Serial.println(voltOut);
}

// Helper: map RSSI to radReal
int mapRSSItoRad(int rssi) {
  if (rssi > -10)
    return 80;
  else if (rssi > -20)
    return 70;
  else if (rssi > -30)
    return 60;
  else if (rssi > -40)
    return 50;
  else if (rssi > -50)
    return 40;
  else if (rssi > -60)
    return 30;
  else if (rssi > -70)
    return 20;
  else if (rssi > -80)
    return 10;
  else if (rssi > -90)
    return 8;
  else if (rssi > -100)
    return 5;
  else
    return 1;
}
