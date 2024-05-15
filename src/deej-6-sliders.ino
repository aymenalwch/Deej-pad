#include <SimpleRotary.h>
#include <EEPROM.h>
#include <FastLED.h>

#define RotaryCLK 4
#define RotaryDT 5
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
int stickBgBrightness = 30;
int stripBrightness = 255;
int stripHue = 0;

unsigned long startMillis;           //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long displayTime = 2000;   //the value is a number of milliseconds

const int NUM_SLIDERS = 6;
const int analogInputs[NUM_SLIDERS] = {A0,A1,A3,A4,A6,A7};
const int NUM_BUTTONS = 7;
//const int buttonInputs[NUM_BUTTONS] = {7,12,11,10,9,8,2,3,4,5,6};
const int buttonInputs[NUM_BUTTONS] = {7,12,11,10,9,8,6};

int analogSliderValues[NUM_SLIDERS];
int buttonValues[NUM_BUTTONS];
SimpleRotary rotary(RotaryCLK,RotaryDT,40);    //Setup media rotary encoder
byte RDir, PrevRDir;                        //Keep track of Rotary Encoder Direction
int keyPressed;

#define OFF 0
#define ON 1
#define NumCols 7
#define NumLayers 2
short Cols[NumCols] = {7,12,11,10,9,8,6};
int Layer1[NumLayers][NumCols] = {
  {1, 2, 3, 4, 5, 6, 7},
  {1, 2, 3, 4, 5, 6, 7}
};
short PressedCheck[NumLayers][NumCols] = { OFF };
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
//  updateMatrix()
  
  updateSerialValues();
  sendSerialData();           // Actually send data (all the time)

  fill_solid(ledArray[1], 5, CHSV( stripHue, 255, stripBrightness));   //4 is number of leds in the strip
//  fill_gradient(ledArray[1], 4, CHSV( 120, 255, 255), CHSV( 220, 255, 255),FORWARD_HUES );
  int brightness = analogRead(analogInputs[5]);
  brightness = map(brightness, 0, 1023, 0, 255);

  FastLED.show();
  delay(100);
}

// ------------------------------------------------------------------------------------------MORE
void updateSerialValues() {
  int visLength;
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int oldValue = analogSliderValues[i];
    analogSliderValues[i] = analogRead(analogInputs[i]);
    int newValue = analogSliderValues[i];
    int oldLength = map(oldValue, 0, 1023, 1, 8);
    int newLength = map(newValue, 0, 1023, 1, 8);
    if (oldLength != newLength) {
      visLength = newLength;
      startMillis = millis();
//      Serial.println("newLength "+ String(i) + String(newLength));
    }
  }
  visualizer(visLength);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonValues[i] = digitalRead(buttonInputs[i]);
    if (buttonValues[0] == 0 && buttonValues[5] == 0) {
      stickBgBrightness = map(analogSliderValues[0], 0, 1023, 0, 255);
    }
    if (buttonValues[3] == 0 && buttonValues[5] == 0) {
      stripBrightness = map(analogSliderValues[4], 0, 1023, 0, 255);
    }
    if (buttonValues[4] == 0 && buttonValues[5] == 0) {
      stripHue = map(analogSliderValues[5], 0, 1023, 0, 255);
    }
  }


  // Read the current state of CLK
  currentStateCLK = digitalRead(RotaryCLK);
    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(RotaryDT) != currentStateCLK) {
      counter --;
      currentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      counter ++;
      currentDir ="CW";
    }
  }
  
  // Remember last CLK state
  lastStateCLK = currentStateCLK;
  // Read the button state
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
  // Put in a slight delay to help debounce the reading
  delay(1);
  }

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
  Serial.println(keyPressed);
  Serial.print(String(currentDir));
  Serial.println(String(counter));
}
// --------------------------------------------------------------------------------------------
void updateMatrix() {
  int RowCnt = 0;                   //Keep track of scanned row - needs to be outside of the void loop so it isn't reset
  int ColCnt = 0;
  int LayerCnt = 0;
  while(ColCnt <= (NumCols - 1)) {
    if(digitalRead( Cols[ColCnt] ) == HIGH && PressedCheck[LayerCnt][ColCnt] == OFF) {
      switch(Layer1[LayerCnt][ColCnt]) {
        case 0:
        case 1:
          Serial.println("case 1 !");
          break;
        case 2:
          Serial.println("case 2 !");
          break;
        default:
          Serial.println("default pressed !");
      }
      PressedCheck[LayerCnt][ColCnt] = ON;   
      Serial.println( Layer1[LayerCnt][ColCnt] );  
    }
    else if(digitalRead( Cols[ColCnt] ) == LOW && PressedCheck[LayerCnt][ColCnt] == ON)
    {
      //Switch based on the switch released
      switch(Layer1[LayerCnt][ColCnt])
      {
        //Nothing for the tactile switch
        case 1:
                  Serial.println("case 1 releas !");
        case 2:
                  Serial.println("case 2 relase !");
        default:
          Serial.println("default released !");
      }
      PressedCheck[LayerCnt][ColCnt] = OFF;
    }
    ColCnt++;
  }
}
void simpleControl() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
      if (buttonValues[0] == 0 && buttonValues[5] == 0) {
        Serial.println("increase visual brightness !");
    }
  }
}
// -------------------------------------------------------------------------RGB
void LedMode (int Select) {
  CurrentLedMode = Select;
  switch(Select)
  {
    case 0:
      fill_solid(rawLeds, NUM_LEDS, CRGB::Black);
      break;
    case 1:
      fill_solid(rawLeds, NUM_LEDS, CRGB::White);
      break;
    case 2:
      fill_rainbow(ledArray[0], 8, gHue--, 12);   //vertical rainbow swipe
      break;
    case 3:
      visualizer(A2);
    case 5:
 //     EEPROM.write(1, CurrentLedMode);
 //     EEPROM.commit();
      break;
  }
}

void visualizer (int volValue) {
  fill_solid(ledArray[0], 8, CHSV( 0, 0, stickBgBrightness));
  fill_solid(ledArray[0], volValue, CHSV(0, 255, 70));
  if (millis() - startMillis >= displayTime) {
//    fill_solid(ledArray[0], volValue, CRGB::Black);
    fill_solid(ledArray[0], 8, CHSV( 0, 0, stickBgBrightness));
  }
  FastLED.show();
}
