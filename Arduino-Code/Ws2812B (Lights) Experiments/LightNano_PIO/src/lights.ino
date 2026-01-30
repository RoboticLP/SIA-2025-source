#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUMPIXELS 1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
   Serial.begin(9600);
  delay(100);              // let everything settle
  pixels.begin();
  pixels.setBrightness(255);

  pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  pixels.show();
}

int time = 0;
void loop() {
  switchlight();
  delay(1000);
}

boolean lightOn = false;
void switchlight(){
  if(lightOn){
  pixels.setPixelColor (0, pixels.Color(10,10,10));
  Serial.println("lightsSwitchedON");
  lightOn = false;
  }
  else {
    pixels.setPixelColor (0, pixels.Color(255, 255, 255));
    Serial.println("lightsSwitchedOFF");
    lightOn = true;
  }
  pixels.show();
}

