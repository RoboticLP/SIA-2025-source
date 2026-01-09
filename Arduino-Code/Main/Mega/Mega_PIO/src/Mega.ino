#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "utils.h"
#include "GameStates.h"

// ───────────────────── LCD Pins ─────────────────────
// LCD Pin-Konfiguration: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

/*
RS → Pin 7
E → Pin 8
D4 → Pin 9
D5 → Pin 10
D6 → Pin 11
D7 → Pin 12
VSS → GND
VDD → 5V
V0 → Potentiometer (Kontrast)
RW → GND
A → 5V (Hintergrundbeleuchtung)
K → GND (Hintergrundbeleuchtung)*/

// ───────────────────── Adressen ─────────────────────
#define slave2      2
#define slave3      3
#define slave4      4
#define slave5      5
#define adminpanel  6

// ───────────────────── Globale Variablen ─────────────────────
bool ballInGame = false;

int moduleCount = 3;
int moduleSlaves[3] = { slave2, slave3, slave4 };

int multiplier = 100;
long points = 0;

long updateBeginTime;

GameState gameState     = WAIT_FOR_BALL;
GameState lastGameState = WAIT_FOR_BALL;

// ───────────────────── Setup / Loop ─────────────────────
void setup() {
    Wire.begin();
    Serial.begin(9600);

    // LCD initialisieren (16x2 Display)
    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("Flipper System");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");
    delay(2000);
    lcd.clear();

    setDebugMode(false); //Falls man Debug Modus will mache in admin panel
}

void loop() {
    checkGameState();
    printConnectionFromSlaves();

    Serial.println("Points: " + String(points));
    delay(4000);
}

// ───────────────────── Debug Modus ─────────────────────
void setDebugMode(bool enable) {
    if (enable) {
        gameState = DEBUG;
        lastGameState = DEBUG;
        Serial.println("DEBUG MODE AKTIVIERT");
        handleLCDDisplay();
    } else {
        gameState = WAIT_FOR_BALL;
        lastGameState = WAIT_FOR_BALL;
        Serial.println("DEBUG MODE BEENDET");
        handleLCDDisplay();
    }
}

// ───────────────────── Hauptlogik ─────────────────────
void printConnectionFromSlaves() {
    Serial.println("#######START CONSOLE#######");
    updateBeginTime = millis();

    String statusMessage = "";

    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        bool alive = isSlaveAlive(addr);

        statusMessage += "M" + String(addr) + ":" + String(alive ? 1 : 0) + "|";

        if (!alive) continue;

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
                handleModule(addr, dataset[0], dataset[1], statusMessage);
            }

            delete[] dataset;
        }
        delete[] data;
    }

    sendESP32ToAdminPanel(statusMessage);

    Serial.print((millis() - updateBeginTime) / 1000.0);
    Serial.println(" Sekunden");
    Serial.println("#######ENDE CONSOLE#######");
}

// ───────────────────── I2C Kommunikation ─────────────────────
bool isSlaveAlive(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

void sendESP32ToAdminPanel(String message) {
    if (!isSlaveAlive(adminpanel)) return;

    Wire.beginTransmission(adminpanel);
    Wire.write(message.c_str());
    Wire.endTransmission();
}

// ───────────────────── Modullogik ─────────────────────
void handleModule(int module, String key, String value, String &statusMessage) {
    switch (module) {
        case slave2: case slave3: case slave4:
            processSlaveData(key, value, module, statusMessage);
            break;
        default:
            break;
    }
}

void processSlaveData(String key, String value, int module, String &statusMessage) {
    int dataValue = value.toInt();

    if (key == "ht1") {
        points += dataValue * multiplier;
        Serial.println("Module " + String(module) + " ht1: " + dataValue);
    }
    else if (key == "ht2") {
        points += dataValue * multiplier;
        Serial.println("Module " + String(module) + " ht2: " + dataValue);
    }
    else if (key == "ht3") {
        Serial.println("Module " + String(module) + " ht3: " + dataValue);
    }
    else if (key == "err") {
        statusMessage += "err:" + value + "|";
        Serial.println("Module " + String(module) + " error: " + value);
    }
    else if (key == "text") {
        Serial.println("Module " + String(module) + " text: " + value);
    }
    else if (key == "ballingame") {
        ballInGame = (dataValue == 1);
    }
    else {
        Serial.println("Module " + String(module) + " unknown key: " + key);
    }
}

// ───────────────────── Game State Logik ─────────────────────
void checkGameState() {
    if (gameState == RESET || gameState == DEBUG) return;

    if (ballInGame) {
        gameState = IN_GAME;
    }
    else if (points > 0) {
        gameState = GAME_OVER;
    }
    else {
        gameState = WAIT_FOR_BALL;
    }

    if (gameState != lastGameState) {
        handleLCDDisplay();
        lastGameState = gameState;
    }

    if (gameState == GAME_OVER) {
        delay(10000);
        resetGame();
    }
}

// ───────────────────── Reset ─────────────────────
void resetGame() {
    gameState = RESET;

    points = 0;
    ballInGame = false;

    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        if (!isSlaveAlive(addr)) continue;

        Wire.beginTransmission(addr);
        Wire.write("resetGame");
        Wire.endTransmission();
    }

    delay(5000);
    gameState = WAIT_FOR_BALL;
    lastGameState = WAIT_FOR_BALL;
}

// ───────────────────── LCD Display ─────────────────────
void displayLCDDisplay(String line1, String line2) {
    lcd.clear();
    
    // Erste Zeile
    lcd.setCursor(0, 0);
    lcd.print(line1.substring(0, 16)); // Maximal 16 Zeichen
    
    // Zweite Zeile
    lcd.setCursor(0, 1);
    lcd.print(line2.substring(0, 16)); // Maximal 16 Zeichen
    
    Serial.println("LCD: " + line1 + " | " + line2);
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
            displayLCDDisplay("Game Over", "Resetting...");
            break;
        case DEBUG:
            displayLCDDisplay("DEBUG MODE", "Game Paused!");
            break;
    }
}

// ───────────────────── Hilfsfunktionen ─────────────────────
//Hier logik für die funktionen der sling shots, finger und verloren mit laser ding da
