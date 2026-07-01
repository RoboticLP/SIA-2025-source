#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "utils.h"
#include "GameStates.h"
#include <arduino-timer.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// ───────────────────── LCD Pins ─────────────────────
LiquidCrystal lcd(A7, A8, A9, A10, A11, A12);

// ───────────────────── DF PLayer Mini ─────────────────────
SoftwareSerial mySoftwareSerial(10,11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
bool dfplayerInitialized = false;

// ───────────────────── Adressen ─────────────────────
#define slave2      2
#define slave3      3
#define slave4      4
#define light      5
#define adminpanel  6

// ───────────────────── Globale Variablen ─────────────────────
auto timer = timer_create_default();
int backlightPin = A6;

int singalForBallStart = A3; // Knopf der gedrückt wird wenn ball in startvorrichtung ist
int ballLostSensor = A4; // Pin für ball lost sensor (done)
int outputFinger = A5; // Pin um Finger zu aktivieren oder deaktivieren
int ballInStart = A13; // Ball instartvorrichtung lassen

int moduleCount = 4;
int moduleSlaves[4] = { slave2, slave3, slave4, light };

long points = 0;
float multiplier = 1.00;
int pointsTargets = 20;
int pointsBumper = 50;
int pointsSlingsshots = 60;

float lightSpeed = 1.0;
int lightState = 1;

int volume = 15;

int ballInGame = 0;

// new: counters for rotating start song
int ballEntryCounter = 0; // increments every time a ball enters the game
int songStartOffset = 0;  // advances every BALLS_PER_SONG_INCREMENT entries
const int BALLS_PER_SONG_INCREMENT = 3; // change every 5th ball entry
const int SONGS_IN_FOLDER = 2; // number of songs in folder 2; adjust if different#
bool pointsOver1000 = false;
bool sillyMode = false;

GameState gameState     = WAIT_FOR_BALL;
GameState lastGameState = RESET;

// ───────────────────── Timer-Handles ─────────────────────
Timer<>::Task gameOverTask;
Timer<>::Task resetTask;

// ───────────────────── LED Effect 4 Auto-Off ─────────────────────
unsigned long ledEffect4StartTime = 0;
bool ledEffect4Active = false;
const unsigned long LED_EFFECT_4_DURATION = 5000; // 5 Sekunden

// ───────────────────── Forward Declarations ─────────────────────
void handleLCDDisplay();
void checkGameState();
void startGameOver();
bool finishGameOver(void *);
void printConnectionFromSlaves();
void sendStatusToAdminPanel();
void reciveMessagesFromAdminPanel();
void sendErrorToESP(int);
void checkLEDEffect4Timeout();

// ───────────────────── Backlight ─────────────────────
void setBacklightPercent(int percent) {
    percent = constrain(percent, 0, 100);
    int pwm = map(percent, 0, 100, 10, 255);
    analogWrite(backlightPin, pwm);
}

// ───────────────────── Setup / Loop ─────────────────────
void setup() {
    pinMode(backlightPin, OUTPUT); // Hintergrund ist Pin 6
    pinMode(ballLostSensor, INPUT_PULLUP); // Ball Lost Sensor ist Pin 4
    pinMode(outputFinger, OUTPUT); // Output für Finger ist Pin 5
    pinMode(singalForBallStart, INPUT_PULLUP); // Knopf für Ball Start ist Pin 3
    pinMode(ballInStart, OUTPUT); // Ball in Startvorrichtung lassen ist Pin 13

    //setBacklightPercent(30);
    

    Wire.begin();
    Serial.begin(9600);
    mySoftwareSerial.begin(9600);
    dfplayerInitialized = myDFPlayer.begin(mySoftwareSerial);
    if (!dfplayerInitialized) {
        Serial.println("DFPlayer init failed");
    }
    Serial.println("Flipper System Starting...");

    lcd.begin(16, 2);
    analogWrite(backlightPin, 50); // ich werde jeden von euch finden der es wagt, nur zu denken diese Zeile zu verändern
    lcd.clear();
    lcd.print("Flipper System");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");
    delay(500);
    if (digitalRead(singalForBallStart) == LOW) { // enable troll mode logic
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Silly troll mode:");
        lcd.setCursor(0, 1);
        lcd.print("Activated ;)");
        sillyMode = true;
        if(dfplayerInitialized) {
            myDFPlayer.playFolder(3, 1); // start that song
            delay(15000);
        }
    }
    delay(500);
    lcd.clear();
    if(dfplayerInitialized) {
        if (sillyMode == false) {
            myDFPlayer.playFolder(1, 1);
            myDFPlayer.loop(101);
        } else {
            myDFPlayer.playFolder(1, 2);
            myDFPlayer.loop(102);
        }
    }
    myDFPlayer.volume(volume);
    

    setDebugMode(false);
    sendLEDEffect(7);
}

void loop() {
    timer.tick();
    checkGameState();
    checkFingers();
    checkBallLost();
    sendFingerUpdate();
    checkBallInStart();
    checkLEDEffect4Timeout();
    
    // Slave-Kommunikation nur alle 100 Milli Sekunden
    static unsigned long lastSlaveCheck = 0;
    if (millis() - lastSlaveCheck >= 800) {
        printConnectionFromSlaves();
        lastSlaveCheck = millis();
    }
    // Data-Kommunikation nur alle 2 Sekunden
    static unsigned long lastDataCheck = 0;
    if (millis() - lastDataCheck >= 2000) {
        sendStatusToAdminPanel();
        reciveMessagesFromAdminPanel();
        sendLEDUpdates();
        myDFPlayer.volume(volume);
        lastDataCheck = millis();
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

        lastGameState = gameState;
    }
}

// ───────────────────── Game Over / Reset ─────────────────────
void startGameOver() {
    if(dfplayerInitialized) {
        if (sillyMode == true) {
            myDFPlayer.playFolder(3, 3); // faaaahhh;
        }
    }
    sendLEDEffect(5);
    lastGameState = IN_GAME;
    gameState = GAME_OVER;
    handleLCDDisplay();
    gameOverTask = timer.in(5000, finishGameOver);
}

bool finishGameOver(void *) {
    if(dfplayerInitialized) {
        if (sillyMode == false) {
            myDFPlayer.playFolder(1, 1);
            myDFPlayer.loop(101);
        } else {
            myDFPlayer.playFolder(1, 2);
            myDFPlayer.loop(102);
        }
    }
    gameState = RESET;
    handleLCDDisplay();
    resetTask = timer.in(2000, resetGame);
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

void reciveMessagesFromAdminPanel() { // receiving alternating data strings
    if (!isSlaveAlive(adminpanel)) return;
    Wire.requestFrom(adminpanel, 100);
    String answer = "";
    while (Wire.available()) answer += (char)Wire.read();

    Serial.print("Admin Panel Message: ");
    Serial.println(answer);
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
        lightSpeed = dataValueF/100;
    }else if(key == "len"){
        lightState= dataValueI;
    }else if(key == "vol"){
        volume = dataValueI;
        myDFPlayer.volume(volume);
    }else{
        sendErrorToESP(411);
    }
}

void sendLEDUpdates() {
    if (!isSlaveAlive(light)) return;
    String statusMessage = "lsp:" + String(lightSpeed) + "|len:" + String(lightState) + "|";
    Wire.beginTransmission(light);
    Wire.write(statusMessage.c_str());
    Wire.endTransmission();
}

void sendLEDEffect(int effectCode) {
    Serial.println("Sending LED Effect: "+String(effectCode));
    if (!isSlaveAlive(light)) return;
    Serial.println("Sending LED Effect: "+String(effectCode));
    String statusMessage = "eff:" + String(effectCode)+"|";
    Wire.beginTransmission(light);
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
    String collectedSlaveData = ""; // for debugging purposes, not used in the current implementation
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

        collectedSlaveData += "Slave " + String(addr) + ": " + answer + "\n";

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
                // Serial.println(answer);
                processSlaveData(dataset[0], dataset[1], addr);
            }

            delete[] dataset;
        }
        delete[] data;
    }

    Serial.println(collectedSlaveData);
    //Serial.print("Verarbeitung dauerte: ");
    //Serial.print((millis() - updateBeginTime) / 1000.0);
    //Serial.println(" Sekunden");
}

void processSlaveData(String key, String value, int module) {
    int dataValue = value.toInt();
    auto addPoints = [&](const char* type, int basePoints) {
        if (gameState != IN_GAME) return;
        if(points > 1000 && !pointsOver1000) {
            pointsOver1000 = true;
            if (dfplayerInitialized) {
                if (sillyMode == false) {
                    myDFPlayer.advertise(1);
                } else {
                    myDFPlayer.advertise(2); // play the silly sound if silly mode is active
                }
            }
            sendLEDEffect(8);
        }
        points += dataValue * multiplier * basePoints;
        handleLCDDisplay();
    };

    if(key == "bth"){ 
        addPoints("Bumper Hits",   pointsBumper);
        if(dataValue > 0 && gameState == IN_GAME){
            sendLEDEffect(1);
            //myDFPlayer.advertise(1);
        }
    }
    else if(key == "ssh"){
        addPoints("Slingshot Hits", pointsSlingsshots);
        if(dataValue > 0 && gameState == IN_GAME)
            sendLEDEffect(2);
    } 
    else if(key == "tah"){
         addPoints("Target Hits", pointsTargets);
         if(dataValue > 0 && gameState == IN_GAME)
            sendLEDEffect(3);
    }
    else if (key == "err") {
        sendErrorToESP(dataValue);
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
            displayLCDDisplay("Flipper bereit", "<- druecken");
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
    pointsOver1000 = false;
    lastGameState = RESET;
    sendLEDEffect(7);
    handleLCDDisplay();
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
    if(gameState == IN_GAME && digitalRead(ballLostSensor) == HIGH){
        ballInGame = 0;
        startGameOver();
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
    if(digitalRead(singalForBallStart) == LOW && gameState == WAIT_FOR_BALL){
        digitalWrite(ballInStart, HIGH);
        if(ballInGame == 0 && gameState == WAIT_FOR_BALL) {
            ballInGame = 1;
            lastGameState = WAIT_FOR_BALL;
            gameState = IN_GAME;
            sendLEDEffect(4);
            ledEffect4StartTime = millis();
            ledEffect4Active = true;

            // increment ball entry counter and advance song offset every X entries
            ballEntryCounter++;
            if (ballEntryCounter % BALLS_PER_SONG_INCREMENT == 0) {
                songStartOffset = (songStartOffset + 1) % SONGS_IN_FOLDER;
            }

            // determine song index (1-based) inside folder 2
            int songToPlay = (songStartOffset % SONGS_IN_FOLDER) + 1;

            if(dfplayerInitialized) {
                if (sillyMode == false) {
                    // Play and loop the selected track from folder 2.
                    // DFRobotDFPlayerMini uses absolute track numbers for loop(), typically folder*100 + trackIndex
                    int absoluteTrack = 200 + songToPlay; // folder 2 -> 200 + track
                    myDFPlayer.playFolder(2, songToPlay); // start that song
                    myDFPlayer.loop(absoluteTrack); // loop that specific track
                } else {
                    myDFPlayer.playFolder(3, 2); // play the silly song if silly mode is active
                    myDFPlayer.loop(302); // loop the silly song
                }
            }
        }
    }else{
        digitalWrite(ballInStart, LOW);
    }
}

// ───────────────────── LED Effect 4 Auto-Off ─────────────────────
void checkLEDEffect4Timeout() {
    if (gameState == IN_GAME && ledEffect4Active && (millis() - ledEffect4StartTime >= LED_EFFECT_4_DURATION)) {
        sendLEDEffect(6);
        ledEffect4Active = false;
    }
}
