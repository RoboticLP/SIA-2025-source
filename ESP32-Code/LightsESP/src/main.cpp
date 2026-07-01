#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

class rgb;


// ——————— FUNKTIONS-PROTOTYPEN (FIX für deine Fehler) ———————
void receiveEvent(int);
void requestEvent();
void testButtonTriggered();
void swoopBallEffect(int leds[], int listLength, int priority,
                     uint8_t r, uint8_t g, uint8_t b,
                     boolean overwriteWithBlack);
void doOnePulse(uint8_t r, uint8_t g, uint8_t b);
void doOneSplitLoad(uint8_t r, uint8_t g, uint8_t b);
void randomTransition(int leds[], int listLength, int priority, String colorType);
void multipleSwoopsEffect(int priority, uint8_t r, uint8_t g, uint8_t b, int length, int distance);
void pulse(int leds[], int listLength, int priority, uint8_t r, uint8_t g, uint8_t b);
rgb randomRedColor();
String* splitString(String input, char splitter, int &count);
void processI2CData(String key, String value);
void handleSpeedChange(float speed);
void randomBlueAmbient(int leds[], int listLength, int priority);
void randomYellowAmbient(int leds[], int listLength, int priority);
void handleSlaveThreeHit();
void loadingEffect(int leds[], int listLength, int priority, uint8_t r, uint8_t g, uint8_t b);
void loadingSplitEffect(int priority, uint8_t r, uint8_t g, uint8_t b);
void doStarBurst(uint8_t r, uint8_t g, uint8_t b, uint8_t whiteChance);

#define PIN 23
#define NUMPIXELS 110
#define interrupt_1 15 //used for a button to test light effects, sounds etc.

// globale Variablen für die Kommunikation zum Nano
float globalEffectSpeed = 1.0;        // ranges from 0-2, multiplyer to in-/decrease speed, accesible in the webpanel
boolean globalEffectSpeedIsZero; //used to shut of effects while keeping the oriinal effect speed for later changes
//(is needed because otherwise you would need to divide by 0 when changing it later, see handleSpeedchange (at the bottom of the code))
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
int ballSwoopInts[40];
int ballOutLoadingInts[80];
uint8_t starBurstBright[NUMPIXELS];
bool    starBurstWhite[NUMPIXELS];
uint8_t starGOBright[NUMPIXELS];
bool    starGOWhite[NUMPIXELS];

// ——————————————————————————————————————————————————————SETUP——————————[...]
void setup()
{
  Serial.begin(9600);

  Wire.begin(5);                // I2C Slave Adresse 5
  Wire.onReceive(receiveEvent); // Master sendet Daten
  Wire.onRequest(requestEvent); // Master fragt Daten ab

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
  for (int i = 0; i<40; i++){
    ballSwoopInts[i] = NUMPIXELS  - 1 - i;
  }
  for (int i = 0; i < 80; i++){
    ballOutLoadingInts[i] = NUMPIXELS -41 - i;
  }
}

void testButtonTriggered()
{
  unsigned long now = millis();
  if (now - lastInterruptTime > 200)
  { // 200 ms entprellen
    totalTriggerAmountInRuntime++;
    writeConsoleThatTestButtonTriggered = 1;
    lastInterruptTime = now;
  }
}

int color = 0;

// —————————————————variables to handle lighting options in the loop——————————————————
long timeToShutOffPulseEffect;
rgb activePulseColor(255, 0, 0);
rgb pulseColor[5]= {rgb(255,45,4),rgb(120, 17, 120),rgb(28, 200, 94),rgb(255,255,0),rgb(255, 0, 255)};
bool buttonfast;
long timeToShutOffStarBurst;
rgb  activeStarColor(255, 255, 255);
uint8_t activeStarWhiteChance = 50;
long starBurstFlashUntil = 0;

#define PHASE_WAIT 0
#define PHASE_GAME 1
#define PHASE_GAME_2 2
#define PHASE_GAMEOVER 3
int gamePhase = PHASE_WAIT;   // Start: Warten

EffectState starBurstState    = EffectState(0, 35 / globalEffectSpeed);
EffectState starGameOverState = EffectState(0, 45 / globalEffectSpeed);
EffectState rainbowFlowState = EffectState(0, 22 / globalEffectSpeed);
uint16_t    rainbowPhase = 0;

rgb wavePalette[4] = { rgb(0,200,255), rgb(90,0,255), rgb(255,0,170), rgb(0,110,255) };


rgb currentWaveColor() {
  float t = fmod(millis() / 3000.0, 4.0);   // 3 s pro Farbe, 12 s Gesamtzyklus
  int i = (int)t, j = (i + 1) & 3;
  float f = t - i;
  rgb a = wavePalette[i], b = wavePalette[j];
  return rgb(a.r + (b.r - a.r) * f, a.g + (b.g - a.g) * f, a.b + (b.b - a.b) * f);
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

void starEffect(int leds[], int listLength, int priority,
                uint8_t r, uint8_t g, uint8_t b, uint8_t whiteChance,
                int spawnPerTick, uint8_t fadeAmount,
                EffectState &state, uint8_t bright[], bool whiteBuf[]) {
  if (millis() - state.lastProgressTimeStamp < state.timeBetweenProgress) return;
  if (priority >= PRIORITY_COUNT) return;
  long cooldown = state.timeBetweenProgress;

  for (int i = 0; i < listLength; i++) {                 // 1) ausfaden
    int idx = leds[i];
    if (idx < 0 || idx >= NUMPIXELS) continue;
    if (bright[idx] > fadeAmount) bright[idx] -= fadeAmount; else bright[idx] = 0;
  }
  for (int s = 0; s < spawnPerTick; s++) {                // 2) neue Sterne
    int pick = leds[random(listLength)];
    if (pick < 0 || pick >= NUMPIXELS) continue;
    bright[pick] = 255;
    whiteBuf[pick] = (random(100) < whiteChance);
  }
  for (int i = 0; i < listLength; i++) {                  // 3) nur leuchtende schreiben
    int idx = leds[i];
    if (idx < 0 || idx >= NUMPIXELS) continue;
    uint8_t br = bright[idx];
    if (br == 0) continue;
    LightState &ls = allLights[idx].prio[priority];
    if (whiteBuf[idx]) { ls.r = br; ls.g = br; ls.b = br; }
    else {
      ls.r = (uint16_t)r * br / 255;
      ls.g = (uint16_t)g * br / 255;
      ls.b = (uint16_t)b * br / 255;
    }
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 40;
  }
  state.lastProgressTimeStamp = millis();
}

rgb hueToRgb(uint16_t hue, uint8_t val) {
  uint32_t c = pixels.gamma32(pixels.ColorHSV(hue, 255, val));
  return rgb((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
}

// fliessender Regenbogen ueber den ganzen Strip
void rainbowFlow(int leds[], int listLength, int priority,
                 uint8_t val, uint16_t speedStep, uint8_t bands) {
  if (millis() - rainbowFlowState.lastProgressTimeStamp < rainbowFlowState.timeBetweenProgress) return;
  if (priority >= PRIORITY_COUNT) return;
  long cooldown = rainbowFlowState.timeBetweenProgress;
  for (int i = 0; i < listLength; i++) {
    int idx = leds[i];
    if (idx < 0 || idx >= NUMPIXELS) continue;
    uint16_t hue = rainbowPhase + (uint16_t)((uint32_t)i * 65535UL * bands / listLength);
    rgb col = hueToRgb(hue, val);
    LightState &ls = allLights[idx].prio[priority];
    ls.r = col.r; ls.g = col.g; ls.b = col.b;
    ls.on = true;
    ls.timeOfShutOff = millis() + cooldown + 50;
  }
  rainbowPhase += speedStep;   // Geschwindigkeit des Farbflusses
  rainbowFlowState.lastProgressTimeStamp = millis();
}

void doStarBurst(uint8_t r, uint8_t g, uint8_t b, uint8_t whiteChance) {
  timeToShutOffStarBurst = millis() + 600;
  starBurstFlashUntil    = millis() + 70;                       // NEU
  activeStarColor = rgb(r, g, b);
  activeStarWhiteChance = whiteChance;
  for (int i = 0; i < NUMPIXELS; i++) starBurstBright[i] = 0;   // NEU
}

//———————————————————————————————————————————————————————LOOP—————————
void loop() {
  bool burstActive = (millis() < timeToShutOffStarBurst);

  if (!burstActive) {
    switch (gamePhase) {
      case PHASE_WAIT: {
        // fester blauer Hintergrund auf prio 0 (unterste Ebene)
        setPixelsEqually(allLightInts, NUMPIXELS, 0, 40, 120, 0, 100);

        static rgb waitColor = randomRedColor();
        static unsigned long lastWaitColorChange = 0;
        if (millis() - lastWaitColorChange > 4000) {
          waitColor = randomRedColor();
          lastWaitColorChange = millis();
        }
        multipleSwoopsEffect(1, waitColor.r, waitColor.g, waitColor.b, 3, 5);   // prio 1 -> überschreibt Blau

        randomYellowAmbient(beachLightInts, 40, 2);                             // prio 2 -> Beach-LEDs
        break;
      }

      case PHASE_GAME:
        randomTransition(allLightInts, NUMPIXELS, 0, "specialblue");
        loadingEffect(ballOutLoadingInts, 80, 1, 255, 0, 255);
        swoopBallEffect(ballSwoopInts, 40, 1, 255, 255, 0, true);
        break;
        

      case PHASE_GAME_2: {   // eff:6 — Haupt-Spiel: fließende Wellen
        rgb wc = currentWaveColor();
        setPixelsEqually(allLightInts, NUMPIXELS, wc.r/10, wc.g/10, wc.b/10, 0, 60);  // dunkler Grund
        multipleSwoopsEffect(1, wc.r, wc.g, wc.b, 8, 6);                              // Wellen aus der Mitte
        break;
      }

      case PHASE_GAMEOVER:
        setPixelsEqually(allLightInts, NUMPIXELS, 45, 0, 0, 0, 120);
        starEffect(allLightInts, NUMPIXELS, 1, 255, 40, 40, 45,
                   4, 18, starGameOverState, starGOBright, starGOWhite);
        break;
    }
  } else {
    // Treffer-Burst: NUR Sterne sichtbar -> untere Prioritaeten ausblenden
    for (int i = 0; i < NUMPIXELS; i++) {
      allLights[i].prio[0].on = false;
      allLights[i].prio[1].on = false;
    }
    if (millis() < starBurstFlashUntil)
      setPixelsEqually(allLightInts, NUMPIXELS,
                       activeStarColor.r, activeStarColor.g, activeStarColor.b, 2, 60);
    starEffect(allLightInts, NUMPIXELS, 2,
               activeStarColor.r, activeStarColor.g, activeStarColor.b,
               activeStarWhiteChance, 9, 16,
               starBurstState, starBurstBright, starBurstWhite);
  }

  // ————————————————LIGHTMANAGER———————————————
  if(!globalEffectSpeedIsZero){
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
            r = led.prio[p].r; // wenn die höchste angeschaltete prio gefunden ist,
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
  }
  // ——————————————–Konsole schreiben (Serial.println geht nicht im Interrupt)————————————————
  if (writeConsoleThatTestButtonTriggered != -1)
  {
    writeConsoleThatTestButtonTriggered = -1;
    Serial.println(String("Triggered Test Button ") + String(" [") + totalTriggerAmountInRuntime + String("]"));
    rgb colorforpulse = pulseColor[random(5)];
    doOnePulse(colorforpulse.r,colorforpulse.g,colorforpulse.b);
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
  float brightness = random(50) / 100.0 + 0.5;
  uint8_t c = random(256);
  return rgb(0, c * brightness, 255 * brightness);
}

rgb randomYellowColorWithBrightness()
{
  float brightness = random(50) / 100.0 + 0.5;
  uint8_t c = random(106);
  uint8_t col= 150 + c;
  uint8_t whitepercentage = random (40);
  return rgb(255, col + (255-col)/100 * whitepercentage, 255/100 * whitepercentage);
}
rgb randomBlueColorWithWhiteLevel() {
  uint8_t c = random(256);
  float w = random(70) / 100.0;              // 0.0 – 0.69 als echter Anteil
  int r = (int)(255 * w);                    // Weißanteil im Rot
  int g = (int)(c + (255 - c) * w);          // Weißanteil im Grün
  if (r > 255) r = 255;
  if (g > 255) g = 255;
  return rgb(r, g, 255);
}

rgb randomRedColor()
{
  uint8_t c = random(160); // nur bis grün = 159 => kein richtiges gelb
  if (random(2) == 0)
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
EffectState pulseEffectState = EffectState(0, 30 / globalEffectSpeed);
EffectState loadingEffectState = EffectState(0, 30 / globalEffectSpeed);
EffectState loadingSplitEffectState = EffectState(0,30/globalEffectSpeed);
rgb colorAtStartOfPulse = rgb(0, 0, 0);
rgb randomTransitionColor = rgb(0, 0, 0);
rgb lastTransitionColor = rgb(0, 0, 0);

void loadingEffect(int leds[], int listLength, int priority, uint8_t r, uint8_t g, uint8_t b)
{
  long lastTime = loadingEffectState.lastProgressTimeStamp;
  long cooldown = loadingEffectState.timeBetweenProgress;
  if (millis() - loadingEffectState.lastProgressTimeStamp < loadingEffectState.timeBetweenProgress)
    return;

  int loadProgress = loadingEffectState.effectProgress;


  if(loadProgress < listLength){
    for (int i = 0; i < loadProgress; i++){
      if (leds[i] < 0 || leds[i] >= NUMPIXELS)
    continue;
      Led &led = allLights[leds[i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r;
      ls.g = g;
      ls.b = b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  else if(loadProgress < 2 * listLength){
    for (int i = 0; i < (2*listLength - loadProgress); i++){
      if (leds[i] < 0 || leds[i] >= NUMPIXELS)
    continue;
      Led &led = allLights[leds[i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r;
      ls.g = g;
      ls.b = b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  
  loadingEffectState.effectProgress++;
  if (loadProgress >= 2*NUMPIXELS)
  {
    loadingEffectState.effectProgress = 0;
    loadingEffectState.isActive = false;
  }
  loadingEffectState.lastProgressTimeStamp = millis();
}

void loadingSplitEffect(int priority, uint8_t r, uint8_t g, uint8_t b)
{
  long lastTime = loadingSplitEffectState.lastProgressTimeStamp;
  long cooldown = loadingSplitEffectState.timeBetweenProgress;
  if (millis() - loadingSplitEffectState.lastProgressTimeStamp < loadingSplitEffectState.timeBetweenProgress)
    return;

  int loadProgress = loadingSplitEffectState.effectProgress;

  if(loadProgress < NUMPIXELS/2){
    for (int i = 0; i < loadProgress; i++){
      Led &led = allLights[firstHalfLightInts[i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r;
      ls.g = g;
      ls.b = b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
    for (int i = 0; i < loadProgress; i++){
      Led &led = allLights[secondHalfLightInts[i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r;
      ls.g = g;
      ls.b = b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  else if(loadProgress < NUMPIXELS){
    for (int i = 0; i < (NUMPIXELS - loadProgress); i++){
      Led &led = allLights[firstHalfLightInts[i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r;
      ls.g = g;
      ls.b = b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
    for (int i = 0; i < (NUMPIXELS - loadProgress); i++){
      Led &led = allLights[secondHalfLightInts[i]];
      if (priority >= PRIORITY_COUNT)
        continue; // looking if the priority is higher than the max. for the led
      LightState &ls = led.prio[priority];

      ls.r = r;
      ls.g = g;
      ls.b = b;
      ls.on = true;
      ls.timeOfShutOff = millis() + cooldown + 10;
    }
  }
  
  loadingSplitEffectState.effectProgress++;
  if (loadProgress >= 2*NUMPIXELS)
  {
    loadingSplitEffectState.effectProgress = 0;
    loadingSplitEffectState.isActive = false;
  }
  loadingSplitEffectState.lastProgressTimeStamp = millis();
}

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
  blueAmbientEffectState.lastProgressTimeStamp = millis();
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

char command[20]; // global variable to hold the current command, can be used in the loop to trigger effects based on the last command

void receiveEvent(int howMany)
{
  int i = 0;

  String answer = "";
  while (Wire.available())
    answer += (char)Wire.read();

  Serial.print("Data recieved: ");
  Serial.println(answer);
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
}

void requestEvent()
{
  // Der Master hat abgefragt, ob wir noch aktiv sind
  // Antwort: 0xAA (170) = Status OK / Slave ist erreichbar
  Wire.write(0xAA);
  // Serial.println("Master polling received - Slave is reachable!");<f
}

void handleSpeedChange(float speed)
{
  // bei allen effectstates den cooldown mit dem alten speed multiplizieren und durch den neuen teilen
  if (speed != 0)
  {
    if (speed != globalEffectSpeed) {
      randomTransitionEffectState.timeBetweenProgress = randomTransitionEffectState
      .timeBetweenProgress * globalEffectSpeed / speed;
  
      swoopEffectState.timeBetweenProgress = swoopEffectState.
      timeBetweenProgress * globalEffectSpeed / speed;
  
      blueAmbientEffectState.timeBetweenProgress = blueAmbientEffectState.
      timeBetweenProgress * globalEffectSpeed / speed;
  
      multiSwoopEffectState.timeBetweenProgress = multiSwoopEffectState
      .timeBetweenProgress * globalEffectSpeed / speed;
  
      pulseEffectState.timeBetweenProgress = pulseEffectState.
      timeBetweenProgress * globalEffectSpeed / speed;
  
      yellowAmbientEffectState.timeBetweenProgress = yellowAmbientEffectState.
      timeBetweenProgress * globalEffectSpeed / speed;

      starBurstState.timeBetweenProgress    = starBurstState.timeBetweenProgress * globalEffectSpeed / speed;
      starGameOverState.timeBetweenProgress = starGameOverState.timeBetweenProgress * globalEffectSpeed / speed;
  
      loadingEffectState.timeBetweenProgress = loadingEffectState.timeBetweenProgress * globalEffectSpeed / speed;
      loadingSplitEffectState.timeBetweenProgress = loadingSplitEffectState.timeBetweenProgress * globalEffectSpeed / speed;

      rainbowFlowState.timeBetweenProgress = rainbowFlowState.timeBetweenProgress * globalEffectSpeed / speed;
  
      globalEffectSpeed = speed;
      globalEffectSpeedIsZero = false;
    }
  }
  else globalEffectSpeedIsZero = true;
}

void processI2CData(String key, String value) {
  key.trim();
  value.trim();
  float dataValueF = value.toFloat();
  int   dataValueI = value.toInt();

  if (key == "len") {
    overrideAllLightsOff = (dataValueI == 0); // 1=an, 0=aus

  } else if (key == "lsp") {
    handleSpeedChange(dataValueF);

  } else if (key == "eff") {
    switch (dataValueI) {
      case 1: doStarBurst(255, 220, 120, 55); break;  // Bumper/Tower -> weiß-gold
      case 2: doStarBurst(255, 140, 0,   35); break;  // Slingshot    -> orange
      case 3: doStarBurst(180, 0,   255, 35); break;  // Taster       -> lila
      case 4: gamePhase = PHASE_GAME;         break;  // In Game
      case 5: gamePhase = PHASE_GAMEOVER;     break;  // Verloren
      case 6: gamePhase = PHASE_GAME_2; break; // Game phase 2
      case 7: gamePhase = PHASE_WAIT;         break;  // Warten
      default: break;
    }
  }
}

// ———————nach Kommunikation mit dem Mega aufgerufen———————


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
