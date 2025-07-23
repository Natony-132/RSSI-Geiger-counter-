// #include "pitches.h"

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
#define BUTTON_PIN 9
#define SPEAKER_PIN 3
#define VOLTMETER_PIN A0

// Globals
int rawRSSI[16];
int superArray[10]; // 0–7 RSSI, 8 max, 9 exclude              

//-----new stuff-----------------------------------------------------------------------

int pos = 7;

int list[7];    // list of the last few RSSI values
int pass;
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



//-------------------Functions

void VOLT() {
  // constrain max between -100 and 0
  int voltOut = map(constrain(max, -100, 0), -100, 0, 0, 255);
  analogWrite(VOLTMETER_PIN, voltOut);

  Serial.print(F("Voltmeter output: "));
  Serial.println(voltOut);
}


void getMode(void){
  if(MODE_A_PIN){
    ModeA = true;
    ModeB = false;
    ModeBoth = false;
  }
  else if(MODE_B_PIN){
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

void getState(void){
  if(SETTING_AUDIO_PIN){
    Audio = true;
    VIS = false;
    AV = false;
  }
  else if(SETTING_VIS_PIN){
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
  if(BUTTON_PIN){
    test = true;
  }
  else{
    test = false;
  }
}

void setExclude(void){
  if (!testLast){
    Exclude++;
    if (Exclude > 9){
      Exclude = 0;
    }
  }
testLast = true;
}

void getOneRSSI(int pos) {
  
    cc1101.setFrequency(frequencies[pos]);
    cc1101.receiveDirect();
    delay(1);
    pass = cc1101.getRSSI();

    Serial.print(F("RSSI["));
    Serial.print(pos);
    Serial.print(F("] @ "));
    Serial.print(frequencies[pos], 4);
    Serial.print(F(" MHz = "));
    Serial.println(rawRSSI[pos]);
}

void maxFill(void) {
  list[0] = list[1];
  list[1] = list[2];
  list[2] = list[3];
  list[3] = list[4];
  list[4] = list[5];
  list[5] = list[6];
  list[6] = list[7];
  list[7] = list[pass];

  // Find max

  for (int i = 0; i < 8; i++) {
    if (list[i] > max) {
      max = list[i];
    }
  }
}

void ExcludePos(){
  if (pos = Exclude) {
    pass = -333;
  }

}

//------------------------------------------------------------------------------------------------------------


void getSwitchState(void);
void getRSSIState(void);
void outputState(void);
void visPlay(void);
void audioPlay(void);
void getTheRSSI(void);
int mapRSSItoRad(int);

enum SYSTEM_MODE { BOTH, A, B };
enum SYSTEM_MODE mode = BOTH;
bool audio_enabled = true;
bool video_enabled = true;
bool testLast = false;

// -------------------------------------------
// 1. SWITCH STATE

/*
class debounce {
private:
  const int _thePin;
  int debounce_count;
  bool value;

public:
  debounce(int pin) : _thePin(pin), debounce_count(3), value(false){};
  ~debounce(){};
  void tick(void) {
    if (digitalRead(_thePin)) {
      debounce_count = 3;
      value = false;
    } else if (debounce_count) {
      debounce_count--;
    } else {
      value = true;
    }
  }
  bool getValue(void) { return value; }
};

debounce modeA(MODE_A_PIN);
debounce modeB(MODE_B_PIN);
debounce setAudio(SETTING_AUDIO_PIN);
debounce setVis(SETTING_VIS_PIN);
debounce button(BUTTON_PIN);

void getSwitchState() {
  modeA.tick();
  modeB.tick();
  setAudio.tick();
  setVis.tick();
  button.tick();

  // MODE
  if (modeA.getValue())
    mode = A;
  else if (modeB.getValue())
    mode = B;
  else
    mode = BOTH;

  // SETTING
  audio_enabled = setAudio.getValue();
  video_enabled = setVis.getValue();
}

// I made this up
#define NOTE_E3 2500
*/


// Frequencies
float frequencies[16] = {462.5625, 462.5875, 462.6125, 462.6375,
                         462.6625, 462.6875, 462.7125, 433.0000,
                         462.5500, 462.5750, 462.6000, 462.6250,
                         462.6500, 462.6750, 462.7000, 462.7250};

// Threshold
const int limRSSI = -60;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
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


/*
bool test_mode_worker(bool test_mode) {
  bool testActive = false;
  if (test_mode) {
    testActive = true;
    if (button.getValue()) {
      if (!testLast && mode == B && video_enabled) {
        superArray[9]++;
        if (superArray[9] > 8)
          superArray[9] = 0;
        Serial.print(F("EXCLUDE updated to: "));
        Serial.println(superArray[9]);
        delay(20);
      }
    } else {
      // Test mode: fill superArray with high values & run outputs
      Serial.println(F("Test mode: Filling superArray with 0s."));
      for (int i = 0; i < 8; i++)
        superArray[i] = 0;
      visPlay();
      audioPlay();
    }
    testLast = button.getValue();
  }
  return testActive;
}
*/


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

  /*
  static int loop_timer = 0;

  int now = millis();
  if (now - loop_timer > 20) {
    loop_timer = now;
    getSwitchState();
    if (!test_mode_worker(false)) {
      getRSSIState();
      outputState();
    }
  }

  */

}

// -------------------------------------------
// 2. RSSI STATE

/*
void getRSSIState() {
  Serial.println(F("Getting RSSI..."));
  getTheRSSI();

  if (mode == A) {
    for (int i = 0; i < 8; i++)
      superArray[i] = rawRSSI[i];
  } else if (mode == B) {
    for (int i = 0; i < 8; i++)
      superArray[i] = rawRSSI[i + 8];
  } else { // BOTH
    for (int i = 0; i < 8; i++) {
      superArray[i] = max(rawRSSI[i], rawRSSI[i + 8]);
    }
  }

  // Apply EXCLUDE
  if (superArray[9] != 0) {
    int ex = superArray[9] - 1;
    if (ex >= 0 && ex < 8) {
      Serial.print(F("Excluding channel "));
      Serial.println(ex);
      superArray[ex] = -333;
    }
  }

  // Find max
  int maxVal = -999;
  for (int i = 0; i < 8; i++) {
    if (superArray[i] > maxVal)
      maxVal = superArray[i];
  }
  superArray[8] = maxVal;

  Serial.print(F("superArray: "));
  for (int i = 0; i < 10; i++) {
    Serial.print(superArray[i]);
    Serial.print(F(" "));
  }
  Serial.println();
}

void getTheRSSI() {
  for (int i = 0; i < 16; i++) {
    cc1101.setFrequency(frequencies[i]);
    cc1101.receiveDirect();
    delay(1);
    rawRSSI[i] = cc1101.getRSSI();
    Serial.print(F("RSSI["));
    Serial.print(i);
    Serial.print(F("] @ "));
    Serial.print(frequencies[i], 4);
    Serial.print(F(" MHz = "));
    Serial.println(rawRSSI[i]);
  }
}
*/


// -------------------------------------------
// 3. OUTPUT STATE
void outputState() {
  Serial.println(F("Outputting state..."));
  if (audio_enabled)
    audioPlay();
  if (video_enabled)
    visPlay();
}

// -------------------------------------------
// VISUAL OUTPUT
void visPlay() {

  if (pass > limRSSI){


  }

  /*
  Serial.println(F("VISUAL output:"));
  for (int i = 0; i < 8; i++) {
    if (superArray[i] > limRSSI) {
      PCF_20.write(i, 0);
      Serial.print(F("LED "));
      Serial.print(i + 1);
      Serial.println(F(": ON"));
    } else if (superArray[i] == -333) {
      PCF_24.write(i, 0);
      Serial.print(F("LED "));
      Serial.print(i + 9);
      Serial.println(F(": EXCLUDED ON"));
    } else {
      PCF_20.write(i, 1);
      PCF_24.write(i, 1);
      Serial.print(F("LED "));
      Serial.print(i + 1);
      Serial.println(F(": OFF"));
    }
  }
    */
}

// -------------------------------------------
// AUDIO OUTPUT
void audioPlay() {
  int rssiVal = superArray[8];
  int radReal = mapRSSItoRad(rssiVal);

  int radLim = random(1, 101);
  Serial.print(F("AUDIO: Max RSSI="));
  Serial.print(rssiVal);
  Serial.print(F(" RadReal="));
  Serial.print(radReal);
  Serial.print(F(" RadLim="));
  Serial.println(radLim);

  if (radLim < radReal) {
    int noteDuration = 4;
    tone(SPEAKER_PIN, NOTE_E3, noteDuration);
    Serial.println(F("Playing tone."));
  }

  int voltOut = map(constrain(rssiVal, -100, 0), -100, 0, 0, 255);
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
