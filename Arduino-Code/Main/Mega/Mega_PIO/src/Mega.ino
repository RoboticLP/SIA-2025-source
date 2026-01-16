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

// ───────────────────── Timer ─────────────────────
auto timer = timer_create_default();

// ───────────────────── Globale Variablen ─────────────────────
int backlightPin = 6;

int moduleCount = 3;
int moduleSlaves[3] = { slave2, slave3, slave4 };


long points = 0;
long multiplier = 1.5;
int targets = 20; // Slave 4
int pointsBumper = 50; // Slave 3
int pointsSlingsshots = 50; // Mega

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

// ───────────────────── Setup / Loop ─────────────────────
void setBacklightPercent(int percent) {
    percent = constrain(percent, 0, 100);

    // Mindesthelligkeit, damit es sichtbar bleibt
    int pwm = map(percent, 0, 100, 10, 255);

    analogWrite(backlightPin, pwm);
}

void setup() {
    pinMode(backlightPin, OUTPUT);
   setBacklightPercent(1);

    Wire.begin();
    Serial.begin(9600);

    Serial.println("[BOOT] Flipper System startet");
    Serial.println("[DEBUG] Tippe 0 = Kugel eingeworfen | 1 = Kugel verloren");

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
    printConnectionFromSlaves();
    sendStatusToAdminPanel();
    reciveMessagesFromAdminPanel(); 

}

// ───────────────────── Debug Input über Serial ─────────────────────
void handleDebugInput() {
    if (!Serial.available()) return;

    char c = Serial.read();

    if (c == '0') {
        Serial.println("[DEBUG INPUT] Kugel eingeworfen");
        ballInGame = 1;
    }
    else if (c == '1') {
        Serial.println("[DEBUG INPUT] Kugel verloren");
        ballInGame = 0;
    }
}

// ───────────────────── Debug Modus ─────────────────────
void setDebugMode(bool enable) {
    if (enable) {
        gameState = DEBUG;
    } else {
        gameState = WAIT_FOR_BALL;
    }
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
    else if (gameState == IN_GAME && ballInGame == 0) {
        timer.cancel(pointsTask);
        startGameOver();
    }

    if (gameState != lastGameState) {
        handleLCDDisplay();
        lastGameState = gameState;
    }
}

// ───────────────────── Punkte automatisch erhöhen ─────────────────────
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

    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        if (!isSlaveAlive(addr)) continue;

        Wire.beginTransmission(addr);
        Wire.write("resetGame");
        Wire.endTransmission();
    }

    gameState = WAIT_FOR_BALL;
    lastGameState = WAIT_FOR_BALL;
    handleLCDDisplay();
    return false;
}

// ───────────────────── I2C Kommunikation ─────────────────────
bool isSlaveAlive(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

// ───────────────────── Status an ESP32 senden  ─────────────────────
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
    while (Wire.available()){
        answer += (char)Wire.read();
    }

    // Splitte die Antwort in einzelne Befehle
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

// ───────────────────── Verarbeite die Befehle ─────────────────────
void processESPData(String key, String value) {
    //HIER SCHAUEN OB DOUBLE
    double dataValue = value.toDouble();

    if (key == "mptl") {
        multiplier = dataValue;
    }
    else if (key == "pbu") {
        pointsBumper = dataValue;
    }
    else if (key == "psl") {
        pointsSlingsshots = dataValue;
    }
    else if (key == "tar") {
        targets = dataValue;
    }
    else {
        Serial.println("[ADMIN PANEL] Unbekannter Befehl: " + key);
    }
}

// ───────────────────── Slaves ─────────────────────
void printConnectionFromSlaves() {
    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        if (!isSlaveAlive(addr)) continue;

        Wire.requestFrom(addr, 50);
        String answer = "";

        while (Wire.available()) {
            answer += (char)Wire.read();
        }

        int dataCount;
        String* data = splitString(answer, '|', dataCount);

        for (int j = 0; j < dataCount; j++) {
            if (data[j].indexOf(':') == -1) continue;

            int count;
            String* dataset = splitString(data[j], ':', count);
            if (count == 2) {
                processSlaveData(dataset[0], dataset[1], addr, answer);
            }
            delete[] dataset;
        }
        delete[] data;
    }
}

void processSlaveData(String key, String value, int module, String &) {
    int dataValue = value.toInt();

    if ((key == "ht1" || key == "ht2") && gameState == IN_GAME) {
        if(module == slave3) {
            points += pointsBumper * multiplier;
        }
        else if(module == slave3) {
            points += pointsSlingsshots * multiplier;
        }
        points += dataValue * multiplier;
    }
    else if(key == "err") {
        Serial.println("[ERROR] Fehler von Modul " + String(module) + ": Code " + String(dataValue));
    }
    else if (key == "ballingame") {
        if(dataValue == 1)
            ballInGame = 1;
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
