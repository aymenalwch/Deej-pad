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
int CurrentLedMode = 1;
int CurrentLedBrightness = 40;
int NumOfModes = 6;
int Hue = 0;
int stickBgBrightness = 25;
int stripBrightness = 255;
int stripHue = 0;
int ledMode;
int visLength;

unsigned long startMillis;           //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long displayTime = 2000;   //the value is a number of milliseconds

const int NUM_SLIDERS = 6;
const int analogInputs[NUM_SLIDERS] = {A0,A1,A3,A4,A6,A7};
const int NUM_BUTTONS = 12;
const int buttonInputs[NUM_BUTTONS] = {7,12,11,10,9,8,7,12,11,10, 9, 8};

int analogSliderValues[NUM_SLIDERS];
int buttonValues[NUM_BUTTONS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
SimpleRotary rotary(RotaryCLK,RotaryDT,40);    //Setup media rotary encoder
byte RDir, PrevRDir;                        //Keep track of Rotary Encoder Direction

// Constants for row and column sizes
const byte ROWS = 2;
const byte COLS = 6;
// Connections to Arduino
byte rowPins[ROWS] = {6, 5};
byte colPins[COLS] = {7,12,11,10, 9, 8};
// Array to represent keys on keypad
char Layer1[ROWS][COLS] = {
  {'1', '2', '3', '4', '5', '6'},
  {'7', '8', '9', 'a', 'b', 'c'}
};
// Create keypad object
Keypad Pad1 = Keypad(makeKeymap(Layer1), rowPins, colPins, ROWS, COLS);

// ------------------------------------------------------------------------------------------SETUP
void setup() {
  pinMode(RotaryCLK, INPUT_PULLUP);
  pinMode(RotaryDT, INPUT_PULLUP);
//  pinMode(RotarySW, INPUT_PULLUP);
  
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
  updateMatrix();
  updateSerialValues();
  sendSerialData();           // Actually send data (all the time)
  LedMode(ledMode);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    // buttonValues[i] = digitalRead(buttonInputs[i]);
    if (buttonValues[3] == 0 ) {
      ledMode = 1;
    } else if (buttonValues[4] == 0) {
      ledMode = 2;
    } else if (buttonValues[5] == 0) {
      ledMode = 3;
    }
  }


    // Put in a slight delay to help debounce the reading
  delay(50);
}

// ------------------------------------------------------------------------------------------MORE
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
  // Serial.print(String(currentDir));
  // Serial.println(String(counter));
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

  // // Read the current state of CLK
  // currentStateCLK = digitalRead(RotaryCLK);
  //   // If last and current state of CLK are different, then pulse occurred
  //   // React to only 1 state change to avoid double count
  // if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
  //   // If the DT state is different than the CLK state then
  //   // the encoder is rotating CCW so decrement
  //   if (digitalRead(RotaryDT) != currentStateCLK) {
  //     counter --;
  //     currentDir ="CCW";
  //   } else {
  //     // Encoder is rotating CW so increment
  //     counter ++;
  //     currentDir ="CW";
  //   }
  // }
  
  // // Remember last CLK state
  // lastStateCLK = currentStateCLK;
  // // Read the button state
//  int btnState = digitalRead(RotarySW);
//  //If we detect LOW signal, button is pressed
//  if (btnState == LOW) {
//    //if 50ms have passed since last LOW pulse, it means that the
//    //button has been pressed, released and pressed again
//    if (millis() - lastButtonPress > 50) {
//      Serial.println("Button pressed!");
//    }
//    // Remember last button press event
//    lastButtonPress = millis();
//  }

  }

// --------------------------------------------------------------------------------------------
void updateMatrix() {
  // Get key value if pressed
  char customKey = Pad1.getKey();
    if (customKey) {
      // Serial.print(customKey);  // print the key hex
      switch (customKey) {
      case '1':
        buttonValues[0] = 0;
        break;
      case '2':
        buttonValues[1] = 0;
        break;
      case '3':
        buttonValues[2] = 0;
        break;
      case '4':
        buttonValues[3] = 0;
        break;
      case '5':
        buttonValues[4] = 0;
        break;
      case '6':
        buttonValues[5] = 0;
        break;
      case '7':
        buttonValues[6] = 0;
        break;
      case '8':
        buttonValues[7] = 0;
        break;
      case '9':
        buttonValues[8] = 0;
        break;
      case 'a':
        buttonValues[9] = 0;
        break;
      case 'b':
        buttonValues[10] = 0;
        break;
      case 'c':
        buttonValues[11] = 0;
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

// -------------------------------------------------------------------------RGB
void LedMode (int Select) {
  CurrentLedMode = Select;
  int brightness = analogRead(analogInputs[0]);
  brightness = map(brightness, 0, 1023, 0, 255);
  switch(Select)
  {
    case 0:
      fill_solid(rawLeds, NUM_LEDS, CRGB::Black);
      break;
    case 1:
      visualizer(visLength);
      fill_solid(ledArray[1], 5, CHSV( stripHue, 255, stripBrightness));   //4 is number of leds in the strip
      // fill_solid(ledArray[1], 5, CHSV( gHue++, 255, stripBrightness));   //vertical rainbow swipe
      for (int i = 0; i < NUM_BUTTONS; i++) {
      // buttonValues[i] = digitalRead(buttonInputs[i]);
        if (buttonValues[0] == 0 ) {
          stickBgBrightness = map(analogSliderValues[0], 0, 1023, 0, 255);
        }
        if (buttonValues[1] == 0 ) {
          stripBrightness = map(analogSliderValues[1], 0, 1023, 0, 255);
        }
        if (buttonValues[2] == 0 ) {
          stripHue = map(analogSliderValues[2], 0, 1023, 0, 255);
        }
      }
      break;
    case 2:
      fill_rainbow(ledArray[0], 8, gHue--, 11);   //vertical rainbow swipe
      fill_rainbow(ledArray[1], 5, gHue--, 16);   //vertical rainbow swipe
      //  fill_gradient(ledArray[1], 4, CHSV( 120, 255, 255), CHSV( 220, 255, 255),FORWARD_HUES );
      FastLED.setBrightness(brightness);
      FastLED.show();
      break;
    case 3:
      visualizer(visLength);
      fill_solid(ledArray[1], 5, CHSV( gHue+=2, 255, stripBrightness));   //4 is number of leds in the strip
      // fill_solid(ledArray[1], 5, CHSV( gHue++, 255, stripBrightness));   //vertical rainbow swipe
      for (int i = 0; i < NUM_BUTTONS; i++) {
      // buttonValues[i] = digitalRead(buttonInputs[i]);
        if (buttonValues[0] == 0 ) {
          stickBgBrightness = map(analogSliderValues[0], 0, 1023, 0, 255);
        }
        if (buttonValues[1] == 0 ) {
          stripBrightness = map(analogSliderValues[1], 0, 1023, 0, 255);
        }
        if (buttonValues[2] == 0 ) {
          stripHue = map(analogSliderValues[2], 0, 1023, 0, 255);
        }
      }
      break;
    default:
      break;
  }
}

void visualizer (int volValue) {
  fill_solid(ledArray[0], 8, CHSV( 0, 0, stickBgBrightness));
  fill_solid(ledArray[0], volValue, CHSV(stripHue, 255, 70));
  if (millis() - startMillis >= displayTime) {
//    fill_solid(ledArray[0], volValue, CRGB::Black);
    fill_solid(ledArray[0], 8, CHSV( 0, 0, stickBgBrightness));
  }
  FastLED.show();
}