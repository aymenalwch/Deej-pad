#include <Arduino.h>
#include <SimpleRotary.h>
// #include <EEPROM.h>
#include <FastLED.h>
#include <Keypad.h>

#define RotaryCLK 4
#define RotaryDT 2
//#define RotarySW 6
#define DataPin 3
#define LedType WS2812
#define ColorOrder GRB
#define NUM_LEDS 13

// RGB Controles -------------------------
//CRGBArray<NUM_LEDS> leds;
CRGB rawLeds[NUM_LEDS];
CRGBSet leds(rawLeds, NUM_LEDS);
CRGBSet part1(leds(0,7));
CRGBSet part2(leds(8,12));
struct CRGB * ledArray[] ={part1, part2}; 

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

uint8_t gHue = 120;                                       //Keep track of the colour wanted
int NumOfModes = 5;
int CurrentLedBrightness = 40;
int currentLedMode = 0;
int Hue = 0;
int stripBrightness = 255;
int stickBgBrightness = stripBrightness*0.15;
int stripHue = 0;
int visLength;

// char Layer1[ROWS][COLS] = {
//   {"F1", "F2", "F3",  "F4",  "F5",  "F6"},
//   {"F7", "F8", "F9", "F10", "F11", "F12"}
// };
unsigned long start, finished, elapsed;
unsigned long startMillis, currentMillis, previousMillis;           //some global variables available anywhere in the program
const unsigned long displayTime = 2000;   //the value is a number of milliseconds

const int NUM_SLIDERS = 6;
const int analogInputs[NUM_SLIDERS] = {A0,A1,A3,A4,A6,A7};
const int NUM_BUTTONS = 12;
const int buttonInputs[NUM_BUTTONS] = {7,12,11,10,9,8,7,12,11,10, 9, 8};

int analogSliderValues[NUM_SLIDERS];
int buttonValues[NUM_BUTTONS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
SimpleRotary rotary(RotaryCLK,RotaryDT,40);    //Setup media rotary encoders
byte RDir, PrevRDir;                          //Keep track of Rotary Encoder Direction

// Constants for row and column sizes
const byte ROWS = 2;
const byte COLS = 6;
// Connections to Arduino
byte rowPins[ROWS] = {6, 5};
byte colPins[COLS] = {11,12,10,9,7,8};
// Array to represent keys on keypad
char Keymap1[ROWS][COLS] = {
  {'1', '2', '3', '4', '5', '6'},
  {'7', '8', '9', 'a', 'b', 'c'}
};


// Create keypad object
Keypad Pad1 = Keypad(makeKeymap(Keymap1), rowPins, colPins, ROWS, COLS);

// ------------------------------------------------------------------------------------------SETUP
void setup() {
  pinMode(RotaryCLK, INPUT_PULLUP);
  pinMode(RotaryDT, INPUT_PULLUP);
//  pinMode(RotarySW, INPUT_PULLUP);
  attachInterrupt (digitalPinToInterrupt (RotaryDT), nextLedMode, CHANGE); // pressed
  
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonInputs[i], INPUT_PULLUP);
  }
  
  // Read the initial state of CLK
  lastStateCLK = digitalRead(RotaryCLK);
  //  EEPROM.begin(4);                                         //Begin EEPROM, allow us to store
  //  Fast LED setup
  FastLED.addLeds<LedType,DataPin,ColorOrder>(leds, NUM_LEDS);
  Serial.begin(9600);
}

// ------------------------------------------------------------------------------------------LOOP
void loop() {
//  unsigned long currentMillis = millis();
  start=millis();

  updateMatrix();
  updateSerialValues();
  sendSerialData();           // Actually send data (all the time)
  // encoder();
  LedMode(currentLedMode);

    // Put in a slight delay to help debounce the reading
  delay(4);
}

// ------------------------------------------------------------------------------------------MORE
void F1() { }
void F2() { }
void F3() { stripHue = map(analogSliderValues[2], 0, 1023, 0, 255); }
void F4() { FastLED.setBrightness(stripBrightness = map(analogSliderValues[3], 0, 1023, 0, 255)); }
void F5() { if (stickBgBrightness != 0) {stickBgBrightness = 0;} else {stickBgBrightness = (stripBrightness*0.15);} }
void F6() { nextLedMode(); }
void F7() { }
void F9() { }
void F8() { }
void F10() { }
void F11() { }
void F12() { previousLedMode(); }
// ------------------------------------------------------------------------------------------
void sendSerialData() {
  String builtString = String("");
  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += "s";
    builtString += String((int)analogSliderValues[i]);
    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  builtString += String("|");
  for (int i = 0; i < NUM_BUTTONS; i++) {
    builtString += "b";
    builtString += String((int)buttonValues[i]);
    if (i < NUM_BUTTONS - 1) {
      builtString += String("|");
    }
  }
  Serial.println(builtString);
}
// ------------------------------------------------------------------------------------------
void updateSerialValues() {

  for (int i = 0; i < NUM_SLIDERS; i++) {
    int oldValue = analogSliderValues[i];
    analogSliderValues[i] = analogRead(analogInputs[i]);
    int newValue = analogSliderValues[i];
    int oldLength = map(oldValue, 0, 1000, 1, 8);
    int newLength = map(newValue, 0, 1000, 1, 8);
    if (oldLength != newLength) {
      visLength = newLength;
      startMillis = millis();
//      Serial.println("newLength "+ String(i) + String(newLength));
    }
  }
}

// void encoder() {
//   // Remember last CLK state
//   lastStateCLK = currentStateCLK;
//   // Read the button state
//   int btnState = digitalRead(RotarySW);
//  //If we detect LOW signal, button is pressed
//   if (btnState == LOW) {
//    //if 50ms have passed since last LOW pulse, it means that the
//    //button has been pressed, released and pressed again
//     if (millis() - lastButtonPress > 50) {
//     Serial.println("Button pressed!");
//   }
//    // Remember last button press event
//   lastButtonPress = millis();
//   }
// }

// --------------------------------------------------------------------------------------------
void updateMatrix() {
  // Get key value if pressed
  char customKey = Pad1.getKey();
    if (customKey) {
      // Serial.print(customKey);  // print the key hex
      switch (customKey) {
      case '1':
        buttonValues[0] = 0;
        F1();
        break;
      case '2':
        buttonValues[1] = 0;
        F2();
        break;
      case '3':
        buttonValues[2] = 0;
        F3();
        break;
      case '4':
        buttonValues[3] = 0;
        F4();
        break;
      case '5':
        buttonValues[4] = 0;
        F5();
        break;
      case '6':
        buttonValues[5] = 0;
        F6();
        break;
      case '7':
        buttonValues[6] = 0;
        F7();
        break;
      case '8':
        buttonValues[7] = 0;
        F8();
        break;
      case '9':
        buttonValues[8] = 0;
        F9();
        break;
      case 'a':
        buttonValues[9] = 0;
        F10();
        break;
      case 'b':
        buttonValues[10] = 0;
        F11();
        break;
      case 'c':
        buttonValues[11] = 0;
        F12();
        break;
      default:
        break;
      }
    } else if ( Pad1.getState() == RELEASED) {
        for (int i = 0; i < NUM_BUTTONS; i++) {
          buttonValues[i] = 1;
      }
    }
}
// ------------------------------------------------------------------------------------------
void encoder() {
    // Read the current state of CLK
  currentStateCLK = digitalRead(RotaryCLK);
    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
  // if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 0){
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(RotaryDT) == currentStateCLK) {
      counter += 1;
      currentDir ="CW";
      // Serial.println(counter);
    } else {
      // Encoder is rotating CW so increment
      counter -= 1;
      currentDir ="CCW";
    }
    Serial.print(String(currentDir));
    Serial.println(counter);
  }
}

void visualizer (int volValue) {
  fill_solid(ledArray[0], 8, CHSV( 0, 0, stickBgBrightness));
  fill_solid(ledArray[0], volValue, CHSV(stripHue, 255, (stripBrightness*0.25)));
  if (millis() - startMillis >= displayTime) {
    fill_solid(ledArray[0], 8, CHSV( 0, 0, stickBgBrightness));
  }
  FastLED.show();
}

// -------------------------------------------------------------------------RGB
void previousLedMode() {
  if (currentLedMode > 0) {
    currentLedMode--;
  } else {
    currentLedMode = NumOfModes;
  }
}
void nextLedMode() {
  if (currentLedMode < (NumOfModes)) {
    currentLedMode++;
  } else {
    currentLedMode = 0;
  }
}

void LedMode (int Select) {
  // int brightness = analogRead(analogInputs[0]);
  // brightness = map(brightness, 0, 1023, 0, 255);
  switch(Select)
  {
    case 0:
      fill_solid(rawLeds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      break;
    case 1:
      visualizer(visLength);
      fill_solid(ledArray[1], 5, CHSV( stripHue, 255, stripBrightness));   //4 is number of leds in the strip
      // fill_solid(ledArray[1], 5, CHSV( gHue++, 255, stripBrightness));   //vertical rainbow swipe
      for (int i = 0; i < NUM_BUTTONS; i++) {
      }
      break;

    case 2:
      visualizer(visLength);
      fill_solid(ledArray[1], 5, CHSV( gHue+=2, 255, stripBrightness));   //4 is number of leds in the strip
      // fill_solid(ledArray[1], 5, CHSV( gHue++, 255, stripBrightness));   //vertical rainbow swipe
      break;

    case 3:
      visualizer(visLength);
      fill_rainbow(ledArray[1], 5, gHue--, 16);   //vertical rainbow swipe
      // fill_rainbow(ledArray[0], 8, gHue--, 11);   //vertical rainbow swipe
      FastLED.show();
      //  fill_gradient(ledArray[1], 4, CHSV( 120, 255, 255), CHSV( 220, 255, 255),FORWARD_HUES );
      break;

    case 4:
      // SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
      SnowSparkle(20, random(200,1600));
      break;
    case 5:
      // Sparkle - first led, last led, speed delay
      // SparkleRandom(0, 7, 30, 0);
      SparkleRandom(8, 12, stripBrightness, 0);
      break;
  }
}

// =========================================================================================================================================================================
// *************************
// ** LEDEffect Functions **
// *************************

void FadeInPixel(int pixel, byte red, byte green, byte blue, int SpeedDelay){
  float r, g, b;
  for(int k = 0; k < 256; k=k+(SpeedDelay)) {
      r = (k/256.0)*red;
      g = (k/256.0)*green;
      b = (k/256.0)*blue;
      setPixel(pixel,r,g,b);
      FastLED.show();
  }
  FadeOutPixel(pixel,r,g,b,SpeedDelay);
}
void FadeOutPixel(int pixel, byte red, byte green, byte blue, int SpeedDelay){
  float r, g, b;
  for(int k = 255; k >= 0; k=k-(SpeedDelay)) {
      r = (k/256.0)*red;
      g = (k/256.0)*green;
      b = (k/256.0)*blue;
      setPixel(pixel,r,g,b);
      FastLED.show();
  }
}
void SnowSparkle(int SparkleDelay, int SpeedDelay) {
  setAll(1,1,1);
  fill_solid(ledArray[0], 8, CHSV(stripHue, 255, (255*0.2)));
  fill_solid(ledArray[1], 5, CHSV(stripHue, 255, 255));
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel,0xff,0xff,0xff);
  FastLED.show();
  delay(SparkleDelay);
  fill_solid(ledArray[0], 8, CHSV(stripHue, 255, (255*0.2)));
  fill_solid(ledArray[1], 5, CHSV(stripHue, 255, 255));
  FastLED.show();
  delay(SpeedDelay);
}

void SparkleRandom(int px1, int px2, int brightness, unsigned long SpeedDelay) {
  setAll(0,0,0);
  byte r = random(0,brightness);
  byte g = random(0,brightness);
  byte b = random(0,brightness);
  int Pixel = random(px1,px2);
  int fade = random(10,25) / 10;
  currentMillis=millis();
  if(currentMillis - previousMillis > SpeedDelay) {
    FadeInPixel(Pixel,r,g,b,fade);
    previousMillis=millis();
  }
}


// void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
//   int Position=0;

//   for(int i=0; i<NUM_LEDS*2; i++) {
//     Position++; // = 0; //Position + Rate;
//     for(int i=0; i<NUM_LEDS; i++) {
//       // sine wave, 3 offset waves make a rainbow!
//       //float level = sin(i+Position) * 127 + 128;
//       //setPixel(i,level,0,0);
//       //float level = sin(i+Position) * 127 + 128;
//       setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
//                  ((sin(i+Position) * 127 + 128)/255)*green,
//                  ((sin(i+Position) * 127 + 128)/255)*blue);
//     }

//     FastLED.show();
//     delay(WaveDelay);
//   }
// }

// ***************************************
// ** FastLed/NeoPixel Common Functions **
// ***************************************

// Set a LED color (not yet visible)
void setPixel(int Pixel, byte red, byte green, byte blue) {
  #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
    leds[Pixel].r = red;
    leds[Pixel].g = green;
    leds[Pixel].b = blue;
  #endif
}

// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}