#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUMPIXELS 12

class LightState {
public:
  bool on = false;
  uint8_t r = 0, g = 0, b = 0;
};

class Led {
public:
  uint8_t ledNumber;
  static const uint8_t PRIORITY_COUNT = 3;
  LightState prio[PRIORITY_COUNT];

  Led() : ledNumber(0) {}

  Led(uint8_t number) : ledNumber(number) {}
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool effectOngoing = false;

Led allLights[NUMPIXELS];
int allLightInts[NUMPIXELS];

void setup() {
   Serial.begin(9600);
  delay(100);              // let everything settle
  pixels.begin();
  pixels.setBrightness(10);

  for(int i = 0; i < NUMPIXELS; i++){
    allLights[i] = Led(i);
    allLightInts[i] = i;
  }
}

int color = 0;

void loop() {
  color++;
  if(color > 255) color = 0;
  setPixelsEqually(allLightInts,NUMPIXELS,color,color,0,0);
  //————————————————LIGHTMANAGER———————————————
  for(int i = 0; i < NUMPIXELS; i++){

    Led& led = allLights[i]; //&--> Referenz, echtes Objekt statt Kopie
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    for(int i = led.PRIORITY_COUNT -1; i >= 0; i--){
      if(led.prio[i].on != false){ //nacheinander prioritäten abarbeiten, beginned bei der höchsten:
        r = led.prio[i].r; //wenn die höchste angeschaltene prio gefunden ist,
        g = led.prio[i].g; //werte übertragen und die for-schleife abbrechen.
        b = led.prio[i].b;
        break;
      }
    }
    pixels.setPixelColor(i,pixels.Color(r,g,b)); //die übernommenen werte in den led-strip "eintragen"
  }
  pixels.show(); //nachdem alle pixel abgearbeitet wurden, den led-strip aktualisieren
}



void setPixelsEqually(int leds[], int listLength, int R, int G, int B, int priority){
  for(int number = 0; number < listLength; number ++){

    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    
    ls.r= R; //setting given rgb values for the given priority
    ls.g= G;
    ls.b= B;
    ls.on = true;
  }
}

//———————————————————let every pixel fade in and out twice, like a heartbeat—————————————
void heartBeatTwice(){
  effectOngoing = true;
  Serial.println("yo");
  for(int i = 0; i < 2; i++){
    for(int level = 0; level < 255; level ++){
      for(int number = 0; number < NUMPIXELS; number ++){
        pixels.setPixelColor (number, pixels.Color(level,100,0));
      }
      pixels.show();
      delay(1);
    }
    for(int level = 255; level >= 0; level --){
      for(int number = 0; number < NUMPIXELS; number ++){
        pixels.setPixelColor (number, pixels.Color(level,100,0));
      }
      pixels.show();
      delay(1);
    }
    delay(100);
  }
  effectOngoing = false;
}


//——————————let all pixels in an array go through one rainbow loop——————————————
void rainBow(int leds[], int listLength){
  effectOngoing = true;
  int rLevel = 255;
  int gLevel = 0;
  int bLevel = 0;
  int delayMS = 3;

  for(int i = 0; i <= 255; i ++){ //g --> 255
    gLevel = i;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0);
    delay(delayMS);
  }
  for(int level = 255; level >= 0; level --){ //r --> 0
    rLevel = level;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0);
      delay(delayMS);
  }
  for(int i = 0; i <= 255; i ++){ //b --> 255
    bLevel = i;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0);
    delay(delayMS);
  }
  for(int level = 255; level >= 0; level --){ //g --> 0
    gLevel = level;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0);
      delay(delayMS);
  }
  for(int i = 0; i <= 255; i ++){ //r --> 255
    rLevel = i;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0);
    delay(delayMS);
  }
  for(int level = 255; level >= 0; level --){ //b --> 0
    bLevel = level;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0);
    delay(delayMS);
  }
  effectOngoing = false;
}
