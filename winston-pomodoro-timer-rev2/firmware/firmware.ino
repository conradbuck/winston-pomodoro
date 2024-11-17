#include<FastLED.h>

#define NUM_LEDS 10

CRGB leds[NUM_LEDS];

void setup() {
	FastLED.addLeds<WS2812B, 2, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(255);
	delay(250);
}

void loop() {
	for(int i = 0; i < NUM_LEDS; i++) {
		leds[i] = CHSV(random8(),255,255);
		FastLED.show();
		fadeToBlackBy(leds, NUM_LEDS, 35);
		delay(100);

	}
}

/*
Rotary encoder algo
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
 */
