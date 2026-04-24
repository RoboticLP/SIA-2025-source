#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "utils.h"
#include "GameStates.h"
#include <arduino-timer.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// ───────────────────── LCD Pins ─────────────────────
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// ───────────────────── DF PLayer Mini ─────────────────────
SoftwareSerial mySoftwareSerial(10,11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// ───────────────────── Adressen ─────────────────────
#define slave2      2
#define slave3      3
#define slave4      4
#define slave5      5
#define adminpanel  6

// ───────────────────── Globale Variablen ─────────────────────
auto timer = timer_create_default();
int backlightPin = 13;

int singalForBallStart = 3; // Knopf der gedrückt wird wenn ball in startvorrichtung ist muss
int ballLostSensor = 4; // Pin für ball lost sensor (done)
int outputFinger = 5; // Pin um Finger zu aktivieren oder deaktivieren
int ballInStart = 6; // Ball instartvorrichtung lassen

int moduleCount = 4;
int moduleSlaves[4] = { slave2, slave3, slave4, slave5 };

long points = 0;
float multiplier = 1.00;
int pointsTargets = 20;
int pointsBumper = 50;
int pointsSlingsshots = 50;

float lightSpeed = 1.0;
int lightState = 1;

int ballInGame = 0;

GameState gameState     = WAIT_FOR_BALL;
GameState lastGameState = WAIT_FOR_BALL;

// ───────────────────── Timer-Handles ─────────────────────
Timer<>::Task gameOverTask;
Timer<>::Task resetTask;

// ───────────────────── Forward Declarations ─────────────────────
void handleLCDDisplay();
void checkGameState();
void startGameOver();
bool finishGameOver(void *);
bool addRandomPoints(void *);
void printConnectionFromSlaves();
void sendStatusToAdminPanel();
void reciveMessagesFromAdminPanel();
void sendErrorToESP(int);

// ───────────────────── Backlight ─────────────────────
void setBacklightPercent(int percent) {
    percent = constrain(percent, 0, 100);
    int pwm = map(percent, 0, 100, 10, 255);
    analogWrite(backlightPin, pwm);
}

// ───────────────────── Setup / Loop ─────────────────────
void setup() {
    pinMode(backlightPin, OUTPUT); // Hintergrund ist Pin 13
    pinMode(ballLostSensor, INPUT_PULLUP); // Ball Lost Sensor ist Pin 4
    pinMode(outputFinger, OUTPUT); // Output für Finger ist Pin 5
    pinMode(singalForBallStart, INPUT_PULLUP); // Knopf für Ball Start ist Pin 3
    pinMode(ballInStart, OUTPUT); // Ball in Startvorrichtung lassen ist Pin 6
    attachInterrupt(digitalPinToInterrupt(singalForBallStart), checkBallInStart, CHANGE); // PRÜFEN!!!

    setBacklightPercent(30);

    Wire.begin();
    Serial.begin(9600);
    Serial.println("Flipper System Starting...");

    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("Flipper System");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");
    delay(1500);
    lcd.clear();

    setDebugMode(false);
}

void loop() {
    timer.tick();
    checkGameState();
    checkFingers();
    checkBallLost();
    sendFingerUpdate();
    
    // Slave-Kommunikation nur alle 2 Sekunden
    static unsigned long lastSlaveCheck = 0;
    if (millis() - lastSlaveCheck >= 2000) {
        printConnectionFromSlaves();
        sendStatusToAdminPanel();
        reciveMessagesFromAdminPanel();
        sendLEDUpdates();
        lastSlaveCheck = millis();
    }
}

// ───────────────────── Debug Modus ─────────────────────
void setDebugMode(bool enable) {
    gameState = enable ? DEBUG : WAIT_FOR_BALL;
    lastGameState = gameState;
    handleLCDDisplay();
}

// ───────────────────── Game State Logik ─────────────────────
void checkGameState() {
    if (gameState == DEBUG || gameState == RESET) return;

    if (gameState != lastGameState) {
        handleLCDDisplay();
        
        // MUSIC
        if (gameState == IN_GAME) {
            myDFPlayer.loopFolder(2);
        } else {
            myDFPlayer.loopFolder(1);
        }

        lastGameState = gameState;
    }
}

// ───────────────────── Punkte ─────────────────────
bool addRandomPoints(void *) {
    if (gameState != IN_GAME) return false;
    points += random(50, 150);
    handleLCDDisplay();
    return true;
}

// ───────────────────── Game Over / Reset ─────────────────────
void startGameOver() {
    gameState = GAME_OVER;
    handleLCDDisplay();
    gameOverTask = timer.in(10000, finishGameOver);
}

bool finishGameOver(void *) {
    gameState = RESET;
    handleLCDDisplay();
    resetTask = timer.in(5000, resetGame);
    return false;
}

// ───────────────────── I2C ─────────────────────
bool isSlaveAlive(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

void sendStatusToAdminPanel() {
    if (!isSlaveAlive(adminpanel)) return;
    String statusMessage = "";
    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        bool alive = isSlaveAlive(addr);
        statusMessage += "M" + String(addr) + ":" + String(alive ? 1 : 0) + "|";
    }
    Wire.beginTransmission(adminpanel);
    Wire.write(statusMessage.c_str());
    Wire.endTransmission();
}

void reciveMessagesFromAdminPanel() {
    if (!isSlaveAlive(adminpanel)) return;
    Wire.requestFrom(adminpanel, 50);
    String answer = "";
    while (Wire.available()) answer += (char)Wire.read();

        int dataCount;
        String* data = splitString(answer, '|', dataCount);

        for (int j = 0; j < dataCount; j++) {
            if (data[j].indexOf(':') == -1) continue;

            int count;
            String* dataset = splitString(data[j], ':', count);

            if (count == 2) {
                processESPData(dataset[0], dataset[1]);
            }

            delete[] dataset;
        }
        delete[] data;
}

void processESPData(String key, String value) {
    float dataValueF = value.toFloat();
    int dataValueI = value.toInt();
    if (key == "mtpl") {
        multiplier = dataValueF;
    }else if(key == "pbu"){
        pointsBumper = dataValueI;
    }else if(key == "psl"){
        pointsSlingsshots = dataValueI;
    }else if(key == "pta"){
        pointsTargets = dataValueI;
    }else if(key == "rst"){
        if(dataValueI == 1){
            resetTask = timer.in(0, resetGame);
        }
    }else if(key == "lsp"){
        lightSpeed = dataValueF;
    }else if(key == "len"){
        lightSpeed = dataValueI;
    }else{
        sendErrorToESP(411);
    }
}

void sendLEDUpdates() {
    if (!isSlaveAlive(slave5)) return;
    String statusMessage = "lsp:" + String(lightSpeed) + "|len:" + String(lightState) + "|";
    Wire.beginTransmission(slave5);
    Wire.write(statusMessage.c_str());
    Wire.endTransmission();
}

void sendLEDEffect(int effectCode) {
    if (!isSlaveAlive(slave5)) return;
    String statusMessage = "eff:" + String(effectCode)+"|";
    Wire.beginTransmission(slave5);
    Wire.write(statusMessage.c_str());
    Wire.endTransmission();
}

void sendErrorToESP(int errorCode) {
    if (!isSlaveAlive(adminpanel)) return;
    String statusMessage = "err:" + String(errorCode)+"|";
    Wire.beginTransmission(adminpanel);
    Wire.write(statusMessage.c_str());
    Wire.endTransmission();
}

// ───────────────────── Slaves ─────────────────────
long updateBeginTime;
void printConnectionFromSlaves() {
    updateBeginTime = millis();

    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        bool alive = isSlaveAlive(addr);
        if (!alive) {
            continue;
        }

        // Daten von Slave anfordern
        Wire.requestFrom(addr, 50);

        String answer = "";
        while (Wire.available()) {
            answer += (char)Wire.read();
        }

        if (answer.length() == 0) {
            continue;
        }

        // Antwort aufteilen und verarbeiten
        int dataCount;
        String* data = splitString(answer, '|', dataCount);

        for (int j = 0; j < dataCount; j++) {
            if (data[j].indexOf(':') == -1) continue;

            int count;
            String* dataset = splitString(data[j], ':', count);

            if (count == 2) {
                // Key-Value Pair verarbeiten (z.B. "ssh:5" oder "balllost:1")
                processSlaveData(dataset[0], dataset[1], addr);
            }

            delete[] dataset;
        }
        delete[] data;
    }

    //Serial.print("Verarbeitung dauerte: ");
    //Serial.print((millis() - updateBeginTime) / 1000.0);
    //Serial.println(" Sekunden");
}

void processSlaveData(String key, String value, int module) {
    int dataValue = value.toInt();
    // Verarbeitung der einzelnen Keys
    //Bumper Tower Hits
    auto addPoints = [&](const char* type, int basePoints) {
        if (gameState != IN_GAME) return;
        points += dataValue * multiplier * basePoints;
        handleLCDDisplay();
    };

    if(key == "bth"){ 
        addPoints("Bumper Hits",   pointsBumper);
        sendLEDEffect(1);
    }
    else if(key == "ssh") addPoints("Slingshot Hits", pointsSlingsshots);
    else if(key == "tah") addPoints("Target Hits",    pointsTargets);
    else if(key == "ballingame") {
        if(ballInGame == 0 && dataValue == 1 && gameState == WAIT_FOR_BALL) {
            ballInGame = 1;
            gameState = IN_GAME;
        }
    }
    else if (key == "err") {
        sendErrorToESP(dataValue);
        Serial.println("!!! Module " + String(module) + " ERROR: " + value + " !!!");
    }
    else {
        sendErrorToESP(410+module);
    }
}

// ───────────────────── LCD ─────────────────────
void displayLCDDisplay(String line1, String line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(line2.substring(0, 16));
}

void handleLCDDisplay() {
    switch (gameState) {
        case WAIT_FOR_BALL:
            displayLCDDisplay("Flipper bereit", "Kugel einwerfen");
            break;
        case IN_GAME:
            displayLCDDisplay("Punkte:", String(points));
            break;
        case GAME_OVER:
            displayLCDDisplay("Game Over", "Score: " + String(points));
            break;
        case RESET:
            displayLCDDisplay("Resetting...", "Bitte warten");
            break;
        case DEBUG:
            displayLCDDisplay("DEBUG MODE", "Game Paused!");
            break;
    }
}


void checkFingers(){
    if(gameState == IN_GAME){
        digitalWrite(outputFinger,1);
    }else
        digitalWrite(outputFinger,0);
}

bool resetGame(void *) {
    points = 0;
    ballInGame = 0;
    gameState = WAIT_FOR_BALL;
    lastGameState = WAIT_FOR_BALL;
    for(int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        if (isSlaveAlive(addr)) {
            Wire.beginTransmission(addr);
            Wire.write("resetGame");
            Wire.endTransmission();
        }
    }
    return false;
}


//Point Methods
void checkBallLost(){
    if(gameState == IN_GAME && digitalRead(ballLostSensor) == LOW){
        ballInGame = 0;
    }
}

void sendFingerUpdate(){
    if(gameState == IN_GAME){
        digitalWrite(outputFinger,1);
    }else{
        digitalWrite(outputFinger,0);
    }
}

void checkBallInStart(){
    if(digitalRead(singalForBallStart) == LOW){
        digitalWrite(ballInStart, HIGH);
    }else{
        digitalWrite(ballInStart, LOW);
    }
}
