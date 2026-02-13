#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>


#define PIN 6
#define NUMPIXELS 24

//globale Variablen für die Kommunikation zum Nano
float globalEffectSpeed = 1.0; //ranges from 0-2, multiplyer to in-/decrease speed, accesible in the webpanel
boolean overrideAllLightsOff = false; //used to deactivate all lights immediately, accesible in the webpanel
char command[50]; //für empfangene Kommdandos

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
  boolean isActive;

  EffectState(int e, long t, boolean i) : effectProgress(e), timeBetweenProgress(t),isActive(i) {}
  EffectState(int e, long t) : effectProgress(e), timeBetweenProgress(t),isActive(false) {}
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool effectOngoing = false;

Led allLights[NUMPIXELS];
int allLightInts[NUMPIXELS];

void setup() {
  Serial.begin(9600);

  Wire.begin(5);                  // I2C Slave Adresse 5
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
  randomTransition(allLightInts,NUMPIXELS,2,"red");

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

//———————nach Kommunikation mit dem Mega aufgerufen———————
void handleReset(){

}

void handleSpeedChange(float speed){
  //bei allen effectstates den cooldown mit dem alten speed multiplizieren und durch den neuen teilen
  randomTransitionEffectState.timeBetweenProgress = randomTransitionEffectState.timeBetweenProgress*globalEffectSpeed/speed;
  swoopEffectState.timeBetweenProgress = swoopEffectState.timeBetweenProgress*globalEffectSpeed/speed;
  blueAmbientEffectState.timeBetweenProgress = blueAmbientEffectState.timeBetweenProgress*globalEffectSpeed/speed;
  globalEffectSpeed = speed;
}

void handleLightsOff(boolean off){
  overrideAllLightsOff = off;
}

void handleBallIn(){

}

void handleBallOut(){

}

void handleSlaveTwoHit(){

}

void handleSlaveThreeHit(){

}

void handleSlaveFourHit(){

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

rgb randomBlueColorWithBrightness(){
  float brightness = random(50)/100 + 0.5;
  uint8_t c = random(256);
    return rgb(0,c*brightness,255*brightness);
}

rgb randomRedColor(){
  uint8_t c = random(160); //nur bis grün = 159 => kein richtiges gelb
  if(random() < 0.5){
    return rgb(255,0,c);
  }
  else return rgb(255,c,0);
}


//——————————————————————EFFECTS————————————————
//EffectStates
EffectState swoopEffectState = EffectState(-5,20 / globalEffectSpeed); //progress -5 (startet außerhalb des strips); timeBetweenProgress 20 ms
EffectState blueAmbientEffectState = EffectState(0,50 / globalEffectSpeed); //progress 0; timeBetweenProgress 50 ms
EffectState randomTransitionEffectState = EffectState(0,50 / globalEffectSpeed); //progress 0;  timeBetweenProgress 20ms
rgb randomTransitionColor = rgb(0,0,0);
rgb lastTransitionColor = rgb(0,0,0);

void randomTransition(int leds[], int listLength, int priority, String colorType){
  long lastTime = randomTransitionEffectState.lastProgressTimeStamp;
  long cooldown = randomTransitionEffectState.timeBetweenProgress;
  if(millis() - lastTime < cooldown) return;
  for(int number = 0; number < listLength; number ++){
    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    int difr= randomTransitionColor.r - ls.r;
    int difg = randomTransitionColor.g - ls.g;
    int difb = randomTransitionColor.b - ls.b;
    if(abs(difr) < 3 && abs(difb) < 3 && abs(difg) < 3){
      //———————————Zielfarbe erreicht——————————
      Serial.println("switched color for the transition ambient!");
      ls.r = randomTransitionColor.r;
      ls.g = randomTransitionColor.g;
      ls.b = randomTransitionColor.b;
      lastTransitionColor = randomTransitionColor;
      if(colorType.equalsIgnoreCase("blue"))randomTransitionColor = randomBlueColorWithBrightness();
      else if(colorType.equalsIgnoreCase("red"))randomTransitionColor = randomRedColor();
      else randomTransitionColor = randomColor();
      continue;
    }
    int stepR = (randomTransitionColor.r - lastTransitionColor.r)*0.05;
    int stepG = (randomTransitionColor.g - lastTransitionColor.g)*0.05;
    int stepB = (randomTransitionColor.b - lastTransitionColor.b)*0.05;

    // Mindestschritt erzwingen
    if(stepR == 0 && difr != 0) stepR = (difr > 0) ? 1 : -1;
    if(stepG == 0 && difg != 0) stepG = (difg > 0) ? 1 : -1;
    if(stepB == 0 && difb != 0) stepB = (difb > 0) ? 1 : -1;

    int newR = ls.r + stepR;
    int newG = ls.g + stepG;
    int newB = ls.b + stepB;

    //sichertstellen, das die Werte nicht "überlaufen (z.b.245 + 13 --> 255 anstatt 245 + 13 --> 3)"
    if((stepR > 0 && newR > randomTransitionColor.r) ||
      (stepR < 0 && newR < randomTransitionColor.r))
      newR = randomTransitionColor.r;

    if((stepG > 0 && newG > randomTransitionColor.g) ||
      (stepG < 0 && newG < randomTransitionColor.g))
      newG = randomTransitionColor.g;

    if((stepB > 0 && newB > randomTransitionColor.b) ||
      (stepB < 0 && newB < randomTransitionColor.b))
      newB = randomTransitionColor.b;

    ls.r = newR; //farbwerte setzen
    ls.g = newG;
    ls.b = newB;

    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 50;
  }
  randomTransitionEffectState.lastProgressTimeStamp = millis();
}

void randomBlueAmbient(int leds[], int listLength, int priority){
  long lastTime = blueAmbientEffectState.lastProgressTimeStamp;
  long cooldown = blueAmbientEffectState.timeBetweenProgress;
  if(millis() - blueAmbientEffectState.lastProgressTimeStamp < blueAmbientEffectState.timeBetweenProgress) return;

  for(int number = 0; number < listLength; number ++){
    if(leds[number] >= NUMPIXELS) continue; //looking if the number extents the amount of numbers
    Led& led = allLights[leds[number]];
    if(priority >= led.PRIORITY_COUNT) continue; //looking if the priority is higher than the max. for the led
    LightState& ls = led.prio[priority];
    rgb color = randomBlueColorWithBrightness();
    ls.r= color.r; //setting given rgb values for the given priority
    ls.g= color.g;
    ls.b= color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 50;
  }
}

void swoopBallEffect(int leds[], int listLength, int priority,uint8_t r,uint8_t g,uint8_t b){
  long lastTime = swoopEffectState.lastProgressTimeStamp;
  long cooldown = swoopEffectState.timeBetweenProgress;
  if(millis() - swoopEffectState.lastProgressTimeStamp < swoopEffectState.timeBetweenProgress) return;

  int swoopProgress = swoopEffectState.effectProgress;
  if(swoopProgress > listLength) {
      swoopEffectState.effectProgress = 0;
      swoopEffectState.isActive = false;
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
      swoopEffectState.isActive = false;
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
        Serial.println("resetting...");
    }
}
