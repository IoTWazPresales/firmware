#ifndef GROWLIGHT_H
#define GROWLIGHT_H
#include <FastLED.h>
#include <GrowLight.h>
#define NUM_LEDS 1
//#define DATA_PIN 13

CRGB leds[NUM_LEDS];
int hue = 0;

GrowLight::GrowLight(AsyncWebServer* server) : _server(server), _lastReading(0) {
    _state.grlight = -1; // Initialize the pH value
}

float GrowLight::getGrLight() const {
    return _state.grlight;
}

void GrowLight::begin() {
  // _state.temperature = 999;
  // SerialOTA.println(F("Init temperature sensor"));
  Serial.println("Starting Light...");
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  emit();
 
}

void GrowLight::loop() {
 
}

void GrowLight::emit() {
    // Color cycle effect
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
 
  // Update the LED strip with the new colors
  FastLED.show();
 
  // Increase the hue value for the next cycle
  hue++;
 
  // Wait for a moment before the next update
  FastLED.delay(30);
  
}

#endif