#include<FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 10
#define BRIGHTNESS 50

#define LEFT 8
#define RIGHT 9
#define PUSH 10

int lengthOfSecond = 5; //length of a second in ms

int longPressDuration = 1000;

uint8_t workHue = 165;
uint8_t restHue = 12;

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
int fadeSpeed = 5;
//1
int workInc = 5; //how much each click of the knob increases time in minutes
int workTimeLeft = 0;
int workTimeTot = 0;
//2
int restInc = 5; //increase in time between click in minutes
int restTimeLeft = 0;
int restTimeTot = 0;
//3
int remainingCycles = 0;

//utility variables
bool exitDetected = 0;
int secondCount = 0;
unsigned long timerPace;
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
  while(millis() - difference < delayamnt) {
    delta = rotary();
    buttonState = pollButton();
    //if(delta != 0 || buttonState != 0) {
    if(buttonState != 0) {
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
  difference = millis();
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

double averageColor(int index) {
  return (1.0 * leds[index].r + 1.0 * leds[index].g + 1.0 * leds[index].b) / 3;
}


void printLEDstate() {
  Serial.print("[");
  for(int i = 0; i < NUM_LEDS; i++) {
    Serial.print(averageColor(i));
    Serial.print("], [");
  }
  Serial.print("];");
  Serial.println();
}

CRGB backgroundColor = CRGB::White;
CRGB workTimeSet = CHSV(workHue, 180, 255);
CRGB workTimeRun = CHSV(workHue, 255, 255);
CRGB restTimeSet = CHSV(restHue, 180, 255);
CRGB restTimeRun = CHSV(restHue, 255, 255);

void setup() {
  Serial.begin(115200);
	pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(PUSH, INPUT_PULLUP);
  
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
	delay(500);
	FastLED.setBrightness(BRIGHTNESS);
	FastLED.show();
  difference = millis();
  timerPace = millis();
  
  timerState = 0;
  firstRun = 1;
}

void loop() {
  if(exitDetected == 0) {
    delta = rotary();
    buttonState = pollButton();
  }
  exitDetected = 0;
  
  if(timerState == 6) {
    //Serial.println("timer state 6 at the top");
    
    //delay(350);
  }
  if(timerState == 0) { //idle state
    if(firstRun) {
      fadeVal = 128;
      fadeDir = 0;
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
        if(fadeVal > BRIGHTNESS) {
          fadeVal = BRIGHTNESS;
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

      //switch this to delayTime for non-blocking
      delay(fadeSpeed * 1.0 * (255 / BRIGHTNESS));
    }
  } else if(timerState == 1) { //set work timer
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
       workTimeLeft = workInc * (index + 1); //length of timer in mins
       workTimeTot = workTimeLeft;
       FastLED.show();
    }
  } else if(timerState == 2) { //set rest timer
    if(firstRun) {
      firstRun = 0;
      index = NUM_LEDS / 2 - 1;
    }
    if(buttonState == 2) {
      //long press registered
      
      while(!digitalRead(PUSH)) {}
      delay(20);
      
    } else if(buttonState == 1) {
      timerState = 3;
      firstRun = 1;
    } else {
       index += delta;
       index = rangeNumber(index, 0, NUM_LEDS);
       fill_solid(leds, NUM_LEDS, backgroundColor);
       for(int i = 0; i <= index; i++) {
        leds[i] = restTimeSet;
       }
       restTimeLeft = restInc * (index + 1); //length of timer in mins
       restTimeTot = restTimeLeft;
       FastLED.show();
    }
  } else if(timerState == 3) { //set num cycles
    if(firstRun) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
    if(delta != 0 || firstRun) {
      remainingCycles += delta;
      if(remainingCycles <= 0) {
        remainingCycles = 1;
      } else if(remainingCycles > NUM_LEDS) {
        remainingCycles = NUM_LEDS;
      }
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      for(int i = 0; i < remainingCycles; i++) {
        leds[i] = backgroundColor;
      }
      FastLED.show();
    }
    firstRun = 0;
    if(buttonState == 1) {
      timerState = 4;
      firstRun = 1;
      Serial.print("desired cycles: ");
      Serial.println(remainingCycles);
      Serial.print("work time in mins: ");
      Serial.println(workTimeTot);
      Serial.print("rest time in mins: ");
      Serial.println(restTimeTot);
      return;
    }
    
  } else if(timerState == 4) { //running work timer
    double percentLeft;
    int currLED = 0;
    bool newFraction = 0;
    if(firstRun) {
      Serial.println("mode 4");
      newFraction = 1;
      percentLeft = (restTimeLeft * 1.0) / (restTimeTot * 1.0) * 100.0;
      workTimer = 1;
      timerPace = millis();
      firstRun = 0;
    }
    if(buttonState == 1) { //pause timer
      timerState = 6;
      firstRun = 1;
      return;
    }
    if(millis() - timerPace >= lengthOfSecond) {
      timerPace = millis();
      secondCount++;
      if(secondCount >= 60){
        workTimeLeft -= 1;
        newFraction = 1;
        secondCount = 0;
      }
      percentLeft = (workTimeLeft * 1.0) / (workTimeTot * 1.0) * 100.0;
    }
    if(workTimeLeft == 0) {
      timerState = 5;
      firstRun = 1;
      return;
    }
    if(newFraction) {
      fill_solid(leds, NUM_LEDS, backgroundColor);
      while(percentLeft >= 10) {
        leds[currLED] = workTimeRun;
        percentLeft -= 10;
        currLED++;
      }
      if(percentLeft != 0) {
        leds[currLED] = CHSV(workHue, 255, 255 - (10 * (10 - percentLeft)));
      }
      //printLEDstate();
      FastLED.show();
    }
    
  } else if(timerState == 5) { //running rest timer
    double percentLeft;
    int currLED = 0;
    bool newFraction = 0;
    if(firstRun) {
      Serial.println("mode 5");
      newFraction = 1;
      percentLeft = (restTimeLeft * 1.0) / (restTimeTot * 1.0) * 100.0;
      workTimer = 0;
      firstRun = 0;
      timerPace = millis();
    }
    if(buttonState == 1) { //pause timer
      timerState = 6;
      firstRun = 1;
      return;
    }
    if(millis() - timerPace >= lengthOfSecond) {
      timerPace = millis();
      secondCount++;
      if(secondCount >= 60){
        restTimeLeft -= 1;
        newFraction = 1;
        secondCount = 0;
      }
      percentLeft = (restTimeLeft * 1.0) / (restTimeTot * 1.0) * 100.0;
    }
    if(restTimeLeft == 0) {
      firstRun = 1;
      remainingCycles--;
      if(remainingCycles > 0) {
        workTimeLeft = workTimeTot;
        restTimeLeft = restTimeTot;
        timerState = 4;
      } else {
        timerState = 7;
      }
      return;
    }
    if(newFraction) {
      fill_solid(leds, NUM_LEDS, backgroundColor);
      while(percentLeft >= 10) {
        leds[currLED] = restTimeRun;
        percentLeft -= 10;
        currLED++;
      }
      if(percentLeft != 0) {
        leds[currLED] = CHSV(restHue, 255, 255 - (10 * (10 - percentLeft)));
      }
      //printLEDstate();
      FastLED.show();
    }
  } else if(timerState == 6) { //pause timer
    if(firstRun) {
      firstRun = 0;
      Serial.println("mode 6");
    }
    
    if(buttonState == 1) {
      firstRun = 1;
      FastLED.setBrightness(BRIGHTNESS);
      FastLED.show();
      if(workTimer) { //return to work timer
        timerState = 4;
      } else { //return to rest timer
        timerState = 5;
      }
      return;
    }
    if(fadeDir) {
      //fade up
      fadeVal++;
      if(fadeVal > BRIGHTNESS) {
        fadeVal = BRIGHTNESS;
        fadeDir = 0;
      }
    } else {
      //fade down
      fadeVal--;
      if(fadeVal < 10) {
        fadeVal = 10;
        fadeDir = 1;
      }
    }
    FastLED.setBrightness(fadeVal);
    FastLED.show();

    //switch this to delayTime for non-blocking
    delay(fadeSpeed * 1.0 * (255 / BRIGHTNESS));
  } else if(timerState == 7) { //finish timer
    if(firstRun) {
      firstRun = 0;
    }
    difference = millis();
    Serial.println("end time");
    uint8_t indicies[NUM_LEDS];
    for(int i = 0; i < NUM_LEDS; i++) {
      indicies[i] = random8();
    }
    while(millis() - difference < 10000) {
      for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(indicies[i], 255, 255);
        indicies[i]++;
      }
      FastLED.show();
      delay(5);
    }
    timerState = 0;
    firstRun = 1;
  } else { //unknown
    Serial.print("Unexpected timerState: ");
    Serial.println(timerState);
  }
 
}
