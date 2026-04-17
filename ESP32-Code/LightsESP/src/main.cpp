#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

class rgb;


// ——————— FUNKTIONS-PROTOTYPEN (FIX für deine Fehler) ———————
void receiveEvent(int);
void testButtonTriggered();
void doOnePulse(uint8_t r, uint8_t g, uint8_t b);
void randomTransition(int leds[], int listLength, int priority, String colorType);
void multipleSwoopsEffect(int priority, uint8_t r, uint8_t g, uint8_t b, int length, int distance);
void pulse(int leds[], int listLength, int priority, uint8_t r, uint8_t g, uint8_t b);
rgb randomRedColor();
String* splitString(String input, char splitter, int &count);
void processI2CData(String key, String value);
void handleSpeedChange(float speed);
void randomBlueAmbient(int leds[], int listLength, int priority);
void randomYellowAmbient(int leds[], int listLength, int priority);

#define PIN 23
#define NUMPIXELS 110
#define interrupt_1 15 //used for a button to test light effects, sounds etc.

// globale Variablen für die Kommunikation zum Nano
float globalEffectSpeed = 1.0;        // ranges from 0-2, multiplyer to in-/decrease speed, accesible in the webpanel
boolean overrideAllLightsOff = false; // used to deactivate all lights immediately, accesible in the webpanel

volatile int writeConsoleThatTestButtonTriggered = -1; // used for testing out individual sound- or lighteffects
int totalTriggerAmountInRuntime = 0;                   // used to bring the console messages for the button in order

volatile unsigned long lastInterruptTime = 0;

class rgb
{
public:
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  rgb() : r(0), g(0), b(0) {}

  rgb(uint8_t number1, uint8_t number2, uint8_t number3) : r(number1), g(number2), b(number3) {}
};

class LightState
{
public:
  bool on = false;
  uint8_t r = 0, g = 0, b = 0;
  long timeOfShutOff;
};
uint8_t PRIORITY_COUNT = 3;

class Led
{
public:
  uint8_t ledNumber;
  LightState prio[3];

  Led() : ledNumber(0) {}

  Led(uint8_t number) : ledNumber(number) {}
};

class EffectState
{
public:
  int effectProgress;
  long timeBetweenProgress;
  long lastProgressTimeStamp;
  boolean isActive;

  EffectState(int e, long t, boolean i) : effectProgress(e), timeBetweenProgress(t), isActive(i) {}
  EffectState(int e, long t) : effectProgress(e), timeBetweenProgress(t), isActive(false) {}
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool effectOngoing = false;

Led allLights[NUMPIXELS];
int allLightInts[NUMPIXELS];
int firstHalfLightInts[NUMPIXELS / 2];
int secondHalfLightInts[NUMPIXELS / 2];
int beachLightInts[40];

// ——————————————————————————————————————————————————————SETUP——————————————————————————————————————————————————————
void setup()
{
  Serial.begin(9600);

  Wire.begin(5);                // I2C Slave Adresse 5
  Wire.onReceive(receiveEvent); // Master sendet Daten

  delay(100); // let everything settle
  pixels.begin();
  pixels.setBrightness(90);

  pinMode(interrupt_1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_1), testButtonTriggered, FALLING);
  // –––Interrupt 2––– (Bumper 2)

  for (int i = 0; i < NUMPIXELS; i++) {
  pixels.setPixelColor(i, pixels.Color(255, 0, 0));
  }
  pixels.show();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    allLights[i] = Led(i);
    allLightInts[i] = i;
  }
  for (int i = 0; i < NUMPIXELS / 2; i++)
  {
    firstHalfLightInts[i] = NUMPIXELS / 2 - 1 - i;
  }
  for (int i = 0; i < NUMPIXELS / 2; i++)
  {
    secondHalfLightInts[i] = NUMPIXELS / 2 - 1 + i;
  }
  for (int i = 0; i<40; i++){
    if(i < 12)
    beachLightInts[i] = i;
    else beachLightInts[i] = NUMPIXELS + 12 - i;
  }
}

void testButtonTriggered()
{
  unsigned long now = millis();
  if (now - lastInterruptTime > 200)
  { // 200 ms entprellen
    doOnePulse(168, 0, 219);
    totalTriggerAmountInRuntime++;
    writeConsoleThatTestButtonTriggered = 1;
    lastInterruptTime = now;
  }
}

int color = 0;

// —————————————————variables to handle lighting options in the loop——————————————————
boolean ballIsOut = true;
long timeToShutOffPulseEffect;
rgb activePulseColor(255, 0, 0);

//———————————————————————————————————————————————————————LOOP——————————————————————————————————————————————————————————————————
void loop() {
  randomTransition(allLightInts,NUMPIXELS,0,"specialblue");
  if(ballIsOut){
    rgb lol = randomRedColor();
    multipleSwoopsEffect(1,lol.r,lol.g,lol.b,3,5);
    randomYellowAmbient(beachLightInts,40,2);
    //braun : 94 55 24
    //türkis: 0 197 219
  }
  else
  {
  }
  if (millis() < timeToShutOffPulseEffect)
  { // so kann ein vollständiger pulse-effekt durchlaufen werden, mithilfe eines passenden timestamps und der activepulsecolor
    pulse(allLightInts, NUMPIXELS, 2, activePulseColor.r, activePulseColor.g, activePulseColor.b);
  }

  // ————————————————LIGHTMANAGER———————————————
  if (!overrideAllLightsOff)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {

      Led &led = allLights[i]; //&--> Referenz, echtes Objekt statt Kopie
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;
      for (int p = PRIORITY_COUNT - 1; p >= 0; p--)
      {
        if (led.prio[p].on != false)
        {                    // nacheinander prioritäten abarbeiten, beginned bei der höchsten:
          r = led.prio[p].r; // wenn die höchste angeschaltene prio gefunden ist,
          g = led.prio[p].g; // werte übertragen und die for-schleife abbrechen.
          b = led.prio[p].b;
          if (led.prio[p].timeOfShutOff <= millis())
          {
            led.prio[p].on = false;
          }
          break;
        }
      }
      pixels.setPixelColor(i, pixels.Color(r, g, b)); // die übernommenen werte in den led-strip "eintragen"
    }
  }
  else
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); //alle Lichter auf Schwarz schalten
    }
  }
  pixels.show(); // nachdem alle pixel abgearbeitet wurden, den led-strip aktualisieren

  // ——————————————–Konsole schreiben (Serial.println geht nicht im Interrupt)————————————————
  if (writeConsoleThatTestButtonTriggered != -1)
  {
    writeConsoleThatTestButtonTriggered = -1;
    Serial.println(String("Triggered Test Button ") + String(" [") + totalTriggerAmountInRuntime + String("]"));
  }
}

void setPixelsEqually(int leds[], int listLength, int R, int G, int B, int priority, int duration)
{
  for (int number = 0; number < listLength; number++)
  {

    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];

    ls.r = R; // setting given rgb values for the given priority
    ls.g = G;
    ls.b = B;
    ls.on = true;
    ls.timeOfShutOff = millis() + duration;
  }
}

void setPixelsRandomRedAmbient(int leds[], int listLength, int priority, int duration)
{
  for (int number = 0; number < listLength; number++)
  {

    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];
    rgb color = randomRedColor();
    ls.r = color.r; // setting given rgb values for the given priority
    ls.g = color.g;
    ls.b = color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + duration;
  }
}

void setSpecificLed(int LED, int R, int G, int B, int priority, int duration)
{

  if (LED >= NUMPIXELS)
    return; // looking if the number extents the amount of numbers
  Led &led = allLights[LED];
  if (priority >= PRIORITY_COUNT)
    return; // looking if the priority is higher than the max. for the led
  LightState &ls = led.prio[priority];

  ls.r = R; // setting given rgb values for the given priority
  ls.g = G;
  ls.b = B;
  ls.on = true;
  ls.timeOfShutOff = millis() + duration;
}

rgb randomColor()
{
  uint8_t r = random(256);
  uint8_t g = random(256);
  uint8_t b = random(256);
  return rgb(r, g, b);
}

rgb randomBlueColorWithBrightness()
{
  float brightness = random(50) / 100 + 0.5;
  uint8_t c = random(256);
  return rgb(0, c * brightness, 255 * brightness);
}

rgb randomYellowColorWithBrightness()
{
  float brightness = random(50) / 100 + 0.5;
  uint8_t c = random(106);
  uint8_t col= 150 + c;
  uint8_t whitepercentage = random (40);
  return rgb(255, col + (255-col)/100 * whitepercentage, 255/100 * whitepercentage);
}
rgb randomBlueColorWithWhiteLevel()
{
  uint8_t c = random(256);
  uint8_t whitepercentage = random (70);

  return rgb(255/100 * whitepercentage, c + ((255-c)/100 * whitepercentage), 255);
}

rgb randomRedColor()
{
  uint8_t c = random(160); // nur bis grün = 159 => kein richtiges gelb
  if (random() < 0.5)
  {
    return rgb(255, 0, c);
  }
  else
    return rgb(255, c, 0);
}


// ——————————————————————EFFECTS————————————————
// EffectStates
EffectState swoopEffectState = EffectState(-5, 20 / globalEffectSpeed);           // progress -5 (startet außerhalb des strips); timeBetweenProgress 20 ms
EffectState multiSwoopEffectState = EffectState(0, 40 / globalEffectSpeed);       // progress -5 (startet außerhalb des strips); timeBetweenProgress 20 ms
EffectState blueAmbientEffectState = EffectState(0, 50 / globalEffectSpeed);
EffectState yellowAmbientEffectState = EffectState(0, 400 /globalEffectSpeed);
EffectState randomTransitionEffectState = EffectState(0, 50 / globalEffectSpeed); // progress 0;  timeBetweenProgress 20ms
EffectState pulseEffectState = EffectState(0, 15 / globalEffectSpeed);
rgb colorAtStartOfPulse = rgb(0, 0, 0);
rgb randomTransitionColor = rgb(0, 0, 0);
rgb lastTransitionColor = rgb(0, 0, 0);

void multipleSwoopsEffect(int priority, uint8_t r, uint8_t g, uint8_t b, int length, int distance)
{
  long lastTime = multiSwoopEffectState.lastProgressTimeStamp;
  long cooldown = multiSwoopEffectState.timeBetweenProgress;
  if (millis() - multiSwoopEffectState.lastProgressTimeStamp < multiSwoopEffectState.timeBetweenProgress)
    return;

  int swoopProgress = multiSwoopEffectState.effectProgress;

  int entlength = length + distance;

  for (int e = 0; e < (NUMPIXELS / 2) / entlength; e++)
  {
    int maxLight = swoopProgress + e * entlength;
    for (int i = 0; i < length; i++)
    {
      float mult = 1 - (float)i / length;
      if (maxLight - i < 0)
        continue;
      Led &led = allLights[firstHalfLightInts[maxLight - i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r * mult; // setting given rgb values for the given priority
      ls.g = g * mult;
      ls.b = b * mult;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  for (int e = 0; e < (NUMPIXELS / 2) / entlength; e++)
  {
    int maxLight = swoopProgress + e * entlength;
    for (int i = 0; i < length; i++)
    {
      float mult = 1 - (float)i / length;
      if (maxLight - i < 0)
        continue;
      Led &led = allLights[secondHalfLightInts[maxLight - i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r * mult; // setting given rgb values for the given priority
      ls.g = g * mult;
      ls.b = b * mult;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  multiSwoopEffectState.effectProgress++;
  if (swoopProgress >= length + distance)
  {
    multiSwoopEffectState.effectProgress = 0;
    multiSwoopEffectState.isActive = false;
  }
  multiSwoopEffectState.lastProgressTimeStamp = millis();
}

void doOnePulse(uint8_t r, uint8_t g, uint8_t b)
{
  timeToShutOffPulseEffect = millis() + pulseEffectState.timeBetweenProgress * 19;
  activePulseColor.r = r;
  activePulseColor.g = g;
  activePulseColor.b = b;
}

void pulse(int leds[], int listLength, int priority, uint8_t r, uint8_t g, uint8_t b)
{
  long lastTime = pulseEffectState.lastProgressTimeStamp;
  long cooldown = pulseEffectState.timeBetweenProgress;
  if (millis() - lastTime < cooldown)
    return;
  int effectProgress = pulseEffectState.effectProgress;

  if (effectProgress == 0)
  { // wenn der Effekt beginnt, wird die prio 0 vom ersten licht in der liste als startpunkt für die "pulse-transistion" genommen
    Led exampleLight = allLights[leds[0]];
    LightState exampleLightState = exampleLight.prio[0];
    colorAtStartOfPulse.r = exampleLightState.r;
    colorAtStartOfPulse.g = exampleLightState.g;
    colorAtStartOfPulse.b = exampleLightState.b;
  }

  int multiplyer;
  if (effectProgress <= 10)
    multiplyer = effectProgress;
  else
    multiplyer = 20 - effectProgress;
  //---> 0,1,2,3,4,5,6 bis 19 effectprogress wird zu 0,1,2,3,...8,9,10,9,8,...,2,1,0 multiplyer --> pulsieren

  for (int number = 0; number < listLength; number++)
  {
    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];
    int diffR = r - colorAtStartOfPulse.r;
    int diffG = g - colorAtStartOfPulse.g;
    int diffB = b - colorAtStartOfPulse.b;

    ls.r = colorAtStartOfPulse.r + diffR / 10 * multiplyer;
    ls.g = colorAtStartOfPulse.g + diffG / 10 * multiplyer;
    ls.b = colorAtStartOfPulse.b + diffB / 10 * multiplyer;
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 10;
  }

  pulseEffectState.effectProgress++;
  if (effectProgress > 19)
  {
    pulseEffectState.effectProgress = 0;
    pulseEffectState.isActive = false;
    // Serial.println("Pulse-Effekt komplett durchgelaufen");
  }
  pulseEffectState.lastProgressTimeStamp = millis();
}

void randomTransition(int leds[], int listLength, int priority, String colorType)
{
  long lastTime = randomTransitionEffectState.lastProgressTimeStamp;
  long cooldown = randomTransitionEffectState.timeBetweenProgress;
  if (millis() - lastTime < cooldown)
    return;
  for (int number = 0; number < listLength; number++)
  {
    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];
    int difr = randomTransitionColor.r - ls.r;
    int difg = randomTransitionColor.g - ls.g;
    int difb = randomTransitionColor.b - ls.b;
    if (abs(difr) < 3 && abs(difb) < 3 && abs(difg) < 3)
    {
      // ———————————Zielfarbe erreicht——————————
      // Serial.println("switched color for the transition ambient!");
      ls.r = randomTransitionColor.r;
      ls.g = randomTransitionColor.g;
      ls.b = randomTransitionColor.b;

      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 50; 
      //lichter weiterhin anmachen--> beugt bugs vor (lichter gehen aus), 
      //die entstehen wenn zufällig die selbe oder eine ähnliche farbe random wieder gepickt wird

      lastTransitionColor = randomTransitionColor;
      if (colorType.equalsIgnoreCase("blue"))
        randomTransitionColor = randomBlueColorWithBrightness();
      else if (colorType.equalsIgnoreCase("red"))
        randomTransitionColor = randomRedColor();
        else if (colorType.equalsIgnoreCase("specialblue"))
        randomTransitionColor = randomBlueColorWithWhiteLevel();
      else
        randomTransitionColor = randomColor();
      continue;
    }
    int stepR = (randomTransitionColor.r - lastTransitionColor.r) * 0.05;
    int stepG = (randomTransitionColor.g - lastTransitionColor.g) * 0.05;
    int stepB = (randomTransitionColor.b - lastTransitionColor.b) * 0.05;

    // Mindestschritt erzwingen
    if (stepR == 0 && difr != 0)
      stepR = (difr > 0) ? 1 : -1;
    if (stepG == 0 && difg != 0)
      stepG = (difg > 0) ? 1 : -1;
    if (stepB == 0 && difb != 0)
      stepB = (difb > 0) ? 1 : -1;

    int newR = ls.r + stepR;
    int newG = ls.g + stepG;
    int newB = ls.b + stepB;

    // sichertstellen, das die Werte nicht "überlaufen (z.b.245 + 13 --> 255 anstatt 245 + 13 --> 3)"
    if ((stepR > 0 && newR > randomTransitionColor.r) ||
        (stepR < 0 && newR < randomTransitionColor.r))
      newR = randomTransitionColor.r;

    if ((stepG > 0 && newG > randomTransitionColor.g) ||
        (stepG < 0 && newG < randomTransitionColor.g))
      newG = randomTransitionColor.g;

    if ((stepB > 0 && newB > randomTransitionColor.b) ||
        (stepB < 0 && newB < randomTransitionColor.b))
      newB = randomTransitionColor.b;

    ls.r = newR; // farbwerte setzen
    ls.g = newG;
    ls.b = newB;

    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 50;
  }
  randomTransitionEffectState.lastProgressTimeStamp = millis();
}

void randomBlueAmbient(int leds[], int listLength, int priority)
{
  long lastTime = blueAmbientEffectState.lastProgressTimeStamp;
  long cooldown = blueAmbientEffectState.timeBetweenProgress;
  if (millis() - blueAmbientEffectState.lastProgressTimeStamp < blueAmbientEffectState.timeBetweenProgress)
    return;

  for (int number = 0; number < listLength; number++)
  {
    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];
    rgb color = randomBlueColorWithBrightness();
    ls.r = color.r; // setting given rgb values for the given priority
    ls.g = color.g;
    ls.b = color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 50;
  }
}

void randomYellowAmbient(int leds[], int listLength, int priority)
{
  long lastTime = yellowAmbientEffectState.lastProgressTimeStamp;
  long cooldown = yellowAmbientEffectState.timeBetweenProgress;
  if (millis() - yellowAmbientEffectState.lastProgressTimeStamp < yellowAmbientEffectState.timeBetweenProgress)
    return;

  for (int number = 0; number < listLength; number++)
  {
    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];
    rgb color = randomYellowColorWithBrightness();
    ls.r = color.r; // setting given rgb values for the given priority
    ls.g = color.g;
    ls.b = color.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 50;
  }
  yellowAmbientEffectState.lastProgressTimeStamp = millis();
}

void swoopBallEffect(int leds[], int listLength, int priority, uint8_t r, uint8_t g, uint8_t b, boolean overwriteWithBlack)
{
  long lastTime = swoopEffectState.lastProgressTimeStamp;
  long cooldown = swoopEffectState.timeBetweenProgress;
  if (millis() - swoopEffectState.lastProgressTimeStamp < swoopEffectState.timeBetweenProgress)
    return;

  int swoopProgress = swoopEffectState.effectProgress;
  if (swoopProgress > listLength)
  {
    swoopEffectState.effectProgress = 0;
    swoopEffectState.isActive = false;
    Serial.println("Swoop-Effekt komplett durchgelaufen");
  }

  for (int number = 0; number < listLength; number++)
  {
    if (leds[number] >= NUMPIXELS)
      continue; // looking if the number extents the amount of numbers
    Led &led = allLights[leds[number]];
    if (priority >= PRIORITY_COUNT)
      continue; // looking if the priority is higher than the max. for the led
    LightState &ls = led.prio[priority];

    int dif = abs(number - swoopProgress);
    float multiplyer = 1 - dif * 0.17; // vom mittelpunkt des effektes aus wird jede led 10% schwächer
    if (multiplyer <= 0)
      multiplyer = 0;
    rgb color = rgb(r * multiplyer, g * multiplyer, b * multiplyer); // final color, some shade of red
    if (multiplyer != 0 || overwriteWithBlack == true)
    {                 // if the values would be 0 0 0 and overwrite... is turned off, the values aren't applied
      ls.r = color.r; // setting given rgb values for the given priority
      ls.g = color.g;
      ls.b = color.b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  swoopEffectState.effectProgress++;
  if (swoopProgress > listLength)
  {
    swoopEffectState.effectProgress = -5;
    swoopEffectState.isActive = false;
    Serial.println("Swoop-Effekt komplett durchgelaufen");
  }
  swoopEffectState.lastProgressTimeStamp = millis();
}

// ——————————let all pixels in an array go through one rainbow loop——————————————
void rainBow(int leds[], int listLength)
{
  effectOngoing = true;
  int rLevel = 255;
  int gLevel = 0;
  int bLevel = 0;
  int delayMS = 3;

  for (int i = 0; i <= 255; i++)
  { // g --> 255
    gLevel = i;
    setPixelsEqually(leds, listLength, rLevel, gLevel, bLevel, 0, 100);
    delay(delayMS);
  }
  for (int level = 255; level >= 0; level--)
  { // r --> 0
    rLevel = level;
    setPixelsEqually(leds, listLength, rLevel, gLevel, bLevel, 0, 100);
    delay(delayMS);
  }
  for (int i = 0; i <= 255; i++)
  { // b --> 255
    bLevel = i;
    setPixelsEqually(leds, listLength, rLevel, gLevel, bLevel, 0, 100);
    delay(delayMS);
  }
  for (int level = 255; level >= 0; level--)
  { // g --> 0
    gLevel = level;
    setPixelsEqually(leds, listLength, rLevel, gLevel, bLevel, 0, 100);
    delay(delayMS);
  }
  for (int i = 0; i <= 255; i++)
  { // r --> 255
    rLevel = i;
    setPixelsEqually(leds, listLength, rLevel, gLevel, bLevel, 0, 100);
    delay(delayMS);
  }
  for (int level = 255; level >= 0; level--)
  { // b --> 0
    bLevel = level;
    setPixelsEqually(leds, listLength, rLevel, gLevel, bLevel, 0, 100);
    delay(delayMS);
  }
  effectOngoing = false;
}

void receiveEvent(int howMany)
{
  int i = 0;

  String answer = "";
  while (Wire.available())
    answer += (char)Wire.read();

  int dataCount;
  String *data = splitString(answer, '|', dataCount);

  for (int j = 0; j < dataCount; j++)
  {
    if (data[j].indexOf(':') == -1)
      continue;

    int count;
    String *dataset = splitString(data[j], ':', count);

    if (count == 2)
    {
      processI2CData(dataset[0], dataset[1]);
    }

    delete[] dataset;
  }
  delete[] data;

  // // Kommando auswerten
  // if (strcmp(command, "slaveTwo") == 0)
  // {
  //   Serial.println("slaveTwo got hit...");
  //   handleSlaveTwoHit();
  // }
  // if (strcmp(command, "slaveThree") == 0)
  // {
  //   Serial.println("slaveThree (Bumpers) got hit...");
  //   handleSlaveThreeHit();
  // }
  // if (strcmp(command, "slaveFour") == 0)
  // {
  //   Serial.println("slaveFour (Targets) got hit...");
  //   handleSlaveFourHit();
  // }
  // if (strcmp(command, "ballOut") == 0)
  // {
  //   Serial.println("the ball went out!");
  //   handleBallOut();
  // }
  // if (strcmp(command, "ballIn") == 0)
  // {
  //   Serial.println("the ball entered the playing field!");
  //   handleBallIn();
  // }
  // if (strcmp(command, "switchAllLights") == 0)
  // {
  //   handleLightsSwitchedON_OFF();
  // }
}

void processI2CData(String key, String value) {
  float dataValueF = value.toFloat();
  int dataValueI = value.toInt();
  if (key == "len") {
    overrideAllLightsOff = dataValueI == 1 ? false : true; // true -> aus; false -> an
  } else if (key == "lsp") {
    handleSpeedChange(dataValueF);
  } // else if (key == "KEYHIER") // TODO
}

// ———————nach Kommunikation mit dem Mega aufgerufen———————
void handleBallIn()
{
  ballIsOut = false;
}

void handleBallOut()
{
  ballIsOut = true;
}

void handleSlaveTwoHit()
{
  doOnePulse(255, 0, 0);
}

void handleSlaveThreeHit()
{
  doOnePulse(0, 255, 0);
}

void handleSlaveFourHit()
{
  doOnePulse(0, 0, 255);
}

void handleSpeedChange(float speed)
{
  // bei allen effectstates den cooldown mit dem alten speed multiplizieren und durch den neuen teilen
  if (speed != 0)
  {
    randomTransitionEffectState.timeBetweenProgress = randomTransitionEffectState.timeBetweenProgress * globalEffectSpeed / speed;
    swoopEffectState.timeBetweenProgress = swoopEffectState.timeBetweenProgress * globalEffectSpeed / speed;
    blueAmbientEffectState.timeBetweenProgress = blueAmbientEffectState.timeBetweenProgress * globalEffectSpeed / speed;
  }
  globalEffectSpeed = speed;
}

String* splitString(String input, char splitter, int &count) {
  // Count how many splits we'll have
  count = 1;
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == splitter) {
      count++;
    }
  }
  
  // Create array to hold the substrings
  String* result = new String[count];
  
  // Split the string
  int index = 0;
  int lastPos = 0;
  
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == splitter) {
      result[index] = input.substring(lastPos, i);
      index++;
      lastPos = i + 1;
    }
  }
  
  // Add the last substring
  result[index] = input.substring(lastPos);
  
  return result;
}