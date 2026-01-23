#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "utils.h"
#include "GameStates.h"
#include <arduino-timer.h>

// ───────────────────── LCD Pins ─────────────────────
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// ───────────────────── Adressen ─────────────────────
#define slave2      2
#define slave3      3
#define slave4      4
#define adminpanel  6

// ───────────────────── Globale Variablen ─────────────────────
auto timer = timer_create_default();
int backlightPin = 6;
int ballLostSensor = A0;

int moduleCount = 3;
int moduleSlaves[3] = { slave2, slave3, slave4 };

long points = 0;
double multiplier = 1.5;
int targets = 20;
int pointsBumper = 50;
int pointsSlingsshots = 50;

int ballInGame = 0;

GameState gameState     = WAIT_FOR_BALL;
GameState lastGameState = WAIT_FOR_BALL;

// ───────────────────── Timer-Handles ─────────────────────
Timer<>::Task gameOverTask;
Timer<>::Task resetTask;
Timer<>::Task pointsTask;

// ───────────────────── Forward Declarations ─────────────────────
void handleLCDDisplay();
void checkGameState();
void startGameOver();
bool finishGameOver(void *);
bool finishReset(void *);
bool addRandomPoints(void *);
void printConnectionFromSlaves();
void handleDebugInput();
void sendStatusToAdminPanel();
void reciveMessagesFromAdminPanel();
bool playMusic(void *);

// ───────────────────── Backlight ─────────────────────
void setBacklightPercent(int percent) {
    percent = constrain(percent, 0, 100);
    int pwm = map(percent, 0, 100, 10, 255);
    analogWrite(backlightPin, pwm);
}

// ───────────────────── Setup / Loop ─────────────────────
void setup() {
    pinMode(backlightPin, OUTPUT);
    pinMode(ballLostSensor, INPUT);

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
    handleDebugInput();
    checkGameState();
    checkBallLost();  
    
    // Slave-Kommunikation nur alle 2 Sekunden
    static unsigned long lastSlaveCheck = 0;
    if (millis() - lastSlaveCheck >= 2000) {
        printConnectionFromSlaves();
        sendStatusToAdminPanel();
        reciveMessagesFromAdminPanel();
        lastSlaveCheck = millis();
    }
}

// ───────────────────── Debug Input ─────────────────────
void handleDebugInput() {
    if (!Serial.available()) return;
    char c = Serial.read();
    if (c == '0') ballInGame = 1;
    else if (c == '1') ballInGame = 0;
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

    if (gameState == WAIT_FOR_BALL && ballInGame == 1) {
        gameState = IN_GAME;
        pointsTask = timer.every(1000, addRandomPoints);
    }

    if (gameState != lastGameState) {
        handleLCDDisplay();
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
    resetTask = timer.in(5000, finishReset);
    return false;
}

bool finishReset(void *) {
    points = 0;
    ballInGame = 0;
    gameState = WAIT_FOR_BALL;
    lastGameState = WAIT_FOR_BALL;
    handleLCDDisplay();
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
    //Serial.println("Admin Panel says: " + answer);
}

// ───────────────────── Slaves ─────────────────────
long updateBeginTime;
void printConnectionFromSlaves() {
    updateBeginTime = millis();

    String statusMessage = "gs:" + String(gameState) + "|pts:" + String(points) + "|";

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
            Serial.println("Module " + String(addr) + " keine Antwort");
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
                // Key-Value Pair verarbeiten (z.B. "ht1:5" oder "balllost:1")
                processSlaveData(dataset[0], dataset[1], addr, statusMessage);
            }

            delete[] dataset;
        }
        delete[] data;
    }

    //Serial.print("Verarbeitung dauerte: ");
    //Serial.print((millis() - updateBeginTime) / 1000.0);
    //Serial.println(" Sekunden");
}

void processSlaveData(String key, String value, int module, String &statusMessage) {
    int dataValue = value.toInt();
    // Verarbeitung der einzelnen Keys
    if (key == "ht1") {
        if (gameState == IN_GAME) {
            points += dataValue * multiplier;
            Serial.println(">>> Module " + String(module) + " HIT 1: +" + String(dataValue * multiplier) + " Punkte | Total: " + String(points));
            handleLCDDisplay();
        }
    }
    else if (key == "ballingame") {
        if(ballInGame == 0 && dataValue == 1 && gameState == WAIT_FOR_BALL) {
            ballInGame = 1;
            Serial.println(">>> Module " + String(module) + " meldet Kugel im Spiel.");
        }
    }
    else if (key == "err") {
        statusMessage += "err:" + value + "|";
        Serial.println("!!! Module " + String(module) + " ERROR: " + value + " !!!");
    }
    else {
        Serial.println("Module " + String(module) + " Unbekannter Key: " + key + " = " + value);
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

void checkBallLost(){
    int sensorValue = analogRead(ballLostSensor);
    if(sensorValue < 250 && ballInGame == 1 && gameState == IN_GAME){
        timer.cancel(pointsTask);
        startGameOver();
    }
    
}
