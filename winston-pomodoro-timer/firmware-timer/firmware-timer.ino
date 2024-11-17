#include<FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 10
#define BRIGHTNESS 100

#define LEFT 8
#define RIGHT 9
#define PUSH 10

int longPressDuration = 1000;

CRGB leds[NUM_LEDS];

int delta = 0;
int buttonState = 0;

int timerState = 0;
//0 idle - timer over
//1 set work timer
//2 set rest timer
//3 set num cycles
//4 running work timer
//5 running rest timer
//6 pause timer
//7 finish timer

//state specifc variables
//0
bool fadeDir = 0;
int fadeVal = 0;
int fadeSpeed = 10;
//1
int workInc = 5; //how much each click of the knob increases time in minutes
int workTimeLeft = 0;
//2
int restInc = 5; //increase in time between click in minutes
int restTimeLeft = 0;

//utility variables
bool exitDetected = 0;
unsigned long difference;
int index = 0;
bool firstRun = 0;
bool workTimer = 0;

int rangeNumber(int num, int lower, int upper) { //range a number between lower (inclusive) and upper (exclusive)
  if(num < lower) {
    return lower;
  } else if(num >= upper) {
    return upper - 1;
  } else {
    return num;
  }
  
}

boolean delayTime(uint16_t delayamnt) {
  difference = millis();
  while( millis() - difference < delayamnt) {
    delta = rotary();
    buttonState = pollButton();
    if(delta != 0 || buttonState != 0) {
      exitDetected = 1;
      return true;
    }
  }
  exitDetected = 0;
  return false;
}


uint8_t lrmem = 3;
int lrsum = 0;
int rotary()
{
  static int8_t TRANS[] = {0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0};
  int8_t l, r;

  l = digitalRead(LEFT);
  r = digitalRead(RIGHT);
  lrmem = ((lrmem & 0x03) << 2) + 2 * l + r;
  lrsum = lrsum + TRANS[lrmem];
  /* encoder not in the neutral state */
  if (lrsum % 4 != 0) return (0);
  /* encoder in the neutral state */

  if (lrsum == 4) {
    lrsum = 0;
    return (1);
  }
  if (lrsum == -4) {
    lrsum = 0;
    return (-1);
  }
  /* lrsum > 0 if the impossible transition */
  lrsum = 0;
  return (0);
}

int pollButton() { //returns 0 if no push, 1 if push, 2 if long push
  if(!digitalRead(PUSH)) {
    while(!digitalRead(PUSH)) {
      if(millis() - difference > longPressDuration) {
        return 2;
      }
    }
    delay(20);
    return 1;
  } else {
    return 0;
  }
}

CRGB backgroundColor = CRGB::White;
CRGB workTimeSet = CHSV(165, 180, 255);
CRGB workTimeRun = CHSV(165, 255, 255);
CRGB restTimeSet = CHSV(12, 180, 255);
CRGB restTimeRun = CHSV(12, 255, 255);

void setup() {
	pinMode(LEFT, INPUT_PULLUP);

	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
	delay(500);
	FastLED.setBrightness(BRIGHTNESS);
	FastLED.show();
  difference = millis();
  
  timerState = 0;
  firstRun = 1;
}

void loop() {
  if(exitDetected == 0) {
    delta = rotary();
    buttonState = pollButton();
  }
  exitDetected = 0;
	switch(timerState) {
		case 0: //idle state
      if(firstRun) {
        firstRun = 0;
      }
      if(buttonState == 2) {
        //long press registered
        //change backgroundColor mode??
        while(!digitalRead(PUSH)) {}
        delay(20);
        
      } else if(buttonState == 1) {
        timerState = 1;
        firstRun = 1;
        FastLED.setBrightness(BRIGHTNESS);
        FastLED.show();
      } else {
        fill_solid(leds, NUM_LEDS, backgroundColor);
        if(fadeDir) {
          //fade up
          fadeVal++;
          if(fadeVal > 255) {
            fadeVal = 255;
            fadeDir = 0;
          }
        } else {
          //fade down
          fadeVal--;
          if(fadeVal < 0) {
            fadeVal = 0;
            fadeDir = 1;
          }
        }
        FastLED.setBrightness(fadeVal);
        FastLED.show();
        delayTime(fadeSpeed);
      }
			break;
		case 1: //set work timer
      if(firstRun) {
        firstRun = 0;
        index = NUM_LEDS / 2 - 1;
      }
      if(buttonState == 2) {
        //long press registered
        
        while(!digitalRead(PUSH)) {}
        delay(20);
        
      } else if(buttonState == 1) {
        timerState = 2;
        firstRun = 1;
      } else {
         index += delta;
         index = rangeNumber(index, 0, NUM_LEDS);
         fill_solid(leds, NUM_LEDS, backgroundColor);
         for(int i = 0; i <= index; i++) {
          leds[i] = workTimeSet;
         }
         workTimeLeft = workInc * index; //length of timer in mins
         FastLED.show();
      }
      break;
    case 2: //set rest timer
      if(firstRun) {
        firstRun = 0;
        index = NUM_LEDS / 2 - 1;
      }
      if(buttonState == 2) {
        //long press registered
        
        while(!digitalRead(PUSH)) {}
        delay(20);
        
      } else if(buttonState == 1) {
        timerState = 4;
        firstRun = 1;
      } else {
         index += delta;
         index = rangeNumber(index, 0, NUM_LEDS);
         fill_solid(leds, NUM_LEDS, backgroundColor);
         for(int i = 0; i <= index; i++) {
          leds[i] = restTimeSet;
         }
         restTimeLeft = restInc * index; //length of timer in mins
         FastLED.show();
      }
      break;
    case 3: //set num cycles
      
      break;
    case 4: //running work timer
      if(firstRun) {
        workTimer = 1;
        firstRun = 0;
      }
      if(buttonState = 1) { //pause timer
        timerState = 6;
      }
      if(millis() % 60*1000 == 0) {
        workTimeLeft -= 1;
      }
      if(workTimeLeft == 0) {
        timerState = 5;
        firstRun = 1;
      }
      fill_solid(leds, NUM_LEDS, backgroundColor);
      for(int i = 0; i < (workTimeLeft / workInc); i++) {
        leds[i] = workTimeRun;
      }
      FastLED.show();
      break;
    case 5: //running rest timer
      if(firstRun) {
        workTimer = 0;
        firstRun = 0;
      }
      if(buttonState = 1) { //pause timer
        timerState = 6;
      }
      if(millis() % 60*1000 == 0) {
        workTimeLeft -= 1;
      }
      if(workTimeLeft == 0) {
        timerState = 7;
      }
      fill_solid(leds, NUM_LEDS, backgroundColor);
      for(int i = 0; i < (workTimeLeft / workInc); i++) {
        leds[i] = workTimeRun;
      }
      FastLED.show();
      break;
    case 6: //pause timer
      if(firstRun) {
        firstRun = 0;
      }
      if(buttonState = 1) {

        if(workTimer) { //return to work timer
          timerState = 4;
        } else { //return to rest timer
          timerState = 5;
        }
      } else {
        
      }
      
      break;
    case 7: //finish timer
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();
      delay(150);
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.show();
      delay(150);
      timerState = 0;
      break;
    
	}
}
