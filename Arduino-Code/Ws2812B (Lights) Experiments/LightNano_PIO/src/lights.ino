#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>


#define PIN 6
#define NUMPIXELS 24

//globale Variablen für die Lichter
float globalEffectSpeed = 1.0; //ranges from 0-2, multiplyer to in-/decrease speed, accesible in the webpanel
boolean overrideAllLightsOff = false; //used to deactivate all lights immediately, accesible in the webpanel

class rgb {
  public:
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  rgb() : r(0), g(0), b(0) {}

  rgb(uint8_t number1,uint8_t number2,uint8_t number3) : r(number1), g(number2), b(number3) {}
};

class LightState {
public:
  bool on = false;
  uint8_t r = 0, g = 0, b = 0;
  long timeOfShutOff;
};

class Led {
public:
  uint8_t ledNumber;
  static const uint8_t PRIORITY_COUNT = 3;
  LightState prio[PRIORITY_COUNT];

  Led() : ledNumber(0) {}

  Led(uint8_t number) : ledNumber(number) {}
};

class EffectState {
public:
  int effectProgress;
  long timeBetweenProgress;
  long lastProgressTimeStamp;

  EffectState(int e, long t) : effectProgress(e), timeBetweenProgress(t) {}
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool effectOngoing = false;

Led allLights[NUMPIXELS];
int allLightInts[NUMPIXELS];

void setup() {
  Serial.begin(9600);

  Wire.begin(3);                  // I2C Slave Adresse 3
  Wire.onReceive(receiveEvent);   // Master sendet Daten



  delay(100);              // let everything settle
  pixels.begin();
  pixels.setBrightness(200);

  for(int i = 0; i < NUMPIXELS; i++){
    allLights[i] = Led(i);
    allLightInts[i] = i;
  }
}

int color = 0;

void loop() {
  //if (millis() - lastUpdate >= 150) {
  //  lastUpdate = millis();
  //  setPixelsRandomBlueAmbient(allLightInts,NUMPIXELS,0,310);
  //}
  swoopBallEffect(allLightInts,NUMPIXELS,2,255,0,255);

  //————————————————LIGHTMANAGER———————————————
  if(!overrideAllLightsOff){
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
        if(led.prio[i].timeOfShutOff <= millis()){
          led.prio[i].on = false;
        }
        break;
      }
    }
    pixels.setPixelColor(i,pixels.Color(r,g,b)); //die übernommenen werte in den led-strip "eintragen"
  }
  }
  else{
    for(int i = 0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i,pixels.Color(0,0,0)); //die übernommenen werte in den led-strip "eintragen"
  }
  }
  pixels.show(); //nachdem alle pixel abgearbeitet wurden, den led-strip aktualisieren
}



void setPixelsEqually(int leds[], int listLength, int R, int G, int B, int priority, int duration){
  for(int number = 0; number < listLength; number ++){

    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    
    ls.r= R; //setting given rgb values for the given priority
    ls.g= G;
    ls.b= B;
    ls.on = true;
    ls.timeOfShutOff = millis() + duration;
  }
}

void setPixelsRandomRedAmbient(int leds[], int listLength, int priority, int duration){
  for(int number = 0; number < listLength; number ++){

    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    rgb color = randomRedColor();
    ls.r= color.r; //setting given rgb values for the given priority
    ls.g= color.g;
    ls.b= color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + duration;
  }
}

void setPixelsRandomBlueAmbient(int leds[], int listLength, int priority, int duration){
  for(int number = 0; number < listLength; number ++){

    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    rgb color = randomBlueColor();
    ls.r= color.r; //setting given rgb values for the given priority
    ls.g= color.g;
    ls.b= color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + duration;
  }
}

void setSpecificLed(int LED, int R, int G, int B, int priority, int duration){

    if(LED >= NUMPIXELS) return; //looking if the number extents the amount of numbers
    Led& led = allLights[LED];
    if(priority >= led.PRIORITY_COUNT) return; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    
    ls.r= R; //setting given rgb values for the given priority
    ls.g= G;
    ls.b= B;
    ls.on = true;
    ls.timeOfShutOff = millis() + duration;
}

rgb randomColor(){
  uint8_t r = random(256);
  uint8_t g = random(256);
  uint8_t b = random(256);
  return rgb(r,g,b);
}

rgb randomBlueColor(){
  uint8_t c = random(256);
  return rgb(0,c,255);
}

rgb randomRedColor(){
  uint8_t c = random(160); //nur bis grün = 159 => kein richtiges gelb
  return rgb(255,c,0);
}



//——————————————————————EFFECTS————————————————
//EffectStates
EffectState swoopEffectState = EffectState(-5,20 / globalEffectSpeed); //progress 0; timeBetweenProgress 100 ms

void swoopBallEffect(int leds[], int listLength, int priority,uint8_t r,uint8_t g,uint8_t b){
  long lastTime = swoopEffectState.lastProgressTimeStamp;
  long cooldown = swoopEffectState.timeBetweenProgress;
  if(millis() - swoopEffectState.lastProgressTimeStamp < swoopEffectState.timeBetweenProgress) return;

  int swoopProgress = swoopEffectState.effectProgress;
  if(swoopProgress > listLength) {
      swoopEffectState.effectProgress = 0;
      Serial.println("Swoop-Effekt komplett durchgelaufen");
  }

  for(int number = 0; number < listLength; number ++){
    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];

    int dif = abs(number - swoopProgress);
    float multiplyer = 1 - dif * 0.17; //vom mittelpunkt des effektes aus wird jede led 10% schwächer
    if(multiplyer <= 0) multiplyer = 0;
    rgb color = rgb(r * multiplyer,g * multiplyer, b * multiplyer); //final color, some shade of red
    
    ls.r= color.r; //setting given rgb values for the given priority
    ls.g= color.g;
    ls.b= color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 10;
  } 
  swoopEffectState.effectProgress++;
  if(swoopProgress > listLength) {
      swoopEffectState.effectProgress = -5;
      Serial.println("Swoop-Effekt komplett durchgelaufen");
  }
  swoopEffectState.lastProgressTimeStamp = millis();
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
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0,100);
    delay(delayMS);
  }
  for(int level = 255; level >= 0; level --){ //r --> 0
    rLevel = level;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0,100);
      delay(delayMS);
  }
  for(int i = 0; i <= 255; i ++){ //b --> 255
    bLevel = i;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0,100);
    delay(delayMS);
  }
  for(int level = 255; level >= 0; level --){ //g --> 0
    gLevel = level;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0,100);
      delay(delayMS);
  }
  for(int i = 0; i <= 255; i ++){ //r --> 255
    rLevel = i;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0,100);
    delay(delayMS);
  }
  for(int level = 255; level >= 0; level --){ //b --> 0
    bLevel = level;
    setPixelsEqually(leds,listLength,rLevel,gLevel,bLevel,0,100);
    delay(delayMS);
  }
  effectOngoing = false;
}

void receiveEvent(int howMany) {
    int i = 0;

    while (Wire.available() && i < sizeof(command) - 1) {
        command[i++] = Wire.read();
    }
    command[i] = '\0'; // String terminieren

    // Kommando auswerten
    if (strcmp(command, "resetGame") == 0) {
        handleReset();
        Serial.println("resetting...");
    }
}
