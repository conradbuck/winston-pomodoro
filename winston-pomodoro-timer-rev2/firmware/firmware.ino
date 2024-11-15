#include<FastLED.h>

#define NUM_LEDS 10

CRGB leds[NUM_LEDS];

void setup() {
	FastLED.addLeds<WS2812B, 2, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(50);
	delay(250);
}

void loop() {
	for(int i = 0; i < NUM_LEDS; i++) {
		leds[i] = CRGB(255,255,255);
		FastLED.show();
		fadeToBlackBy(leds, NUM_LEDS, 30);
		delay(150);

	}
}
