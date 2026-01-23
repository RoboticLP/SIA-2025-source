#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define DATAPIN 6;
#define NUMPIXELS 20;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATAPIN, NEO_GRB, NEO_KHZ800)

void setup() {
  pixels.begin();
}

void loop() {
}

