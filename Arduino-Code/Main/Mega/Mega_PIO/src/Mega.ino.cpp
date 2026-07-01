# 1 "C:\\Users\\fkirc\\AppData\\Local\\Temp\\tmpzh8s28e4"
#include <Arduino.h>
# 1 "C:/Users/fkirc/Documents/GitHub/SIA-2025-source/Arduino-Code/Main/Mega/Mega_PIO/src/Mega.ino"
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "utils.h"
#include "GameStates.h"
#include <arduino-timer.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


LiquidCrystal lcd(A7, A8, A9, A10, A11, A12);


SoftwareSerial mySoftwareSerial(10,11);
DFRobotDFPlayerMini myDFPlayer;
bool dfplayerInitialized = false;


#define slave2 2
#define slave3 3
#define slave4 4
#define light 5
#define adminpanel 6


auto timer = timer_create_default();
int backlightPin = A6;

int singalForBallStart = A3;
int ballLostSensor = A4;
int outputFinger = A5;
int ballInStart = A13;

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


int ballEntryCounter = 0;
int songStartOffset = 0;
const int BALLS_PER_SONG_INCREMENT = 3;
const int SONGS_IN_FOLDER = 2;
bool pointsOver1000 = false;

GameState gameState = WAIT_FOR_BALL;
GameState lastGameState = RESET;


Timer<>::Task gameOverTask;
Timer<>::Task resetTask;


unsigned long ledEffect4StartTime = 0;
bool ledEffect4Active = false;
const unsigned long LED_EFFECT_4_DURATION = 5000;


void handleLCDDisplay();
void checkGameState();
void startGameOver();
bool finishGameOver(void *);
void printConnectionFromSlaves();
void sendStatusToAdminPanel();
void reciveMessagesFromAdminPanel();
void sendErrorToESP(int);
void checkLEDEffect4Timeout();
void setBacklightPercent(int percent);
void setup();
void loop();
void setDebugMode(bool enable);
bool isSlaveAlive(uint8_t address);
void processESPData(String key, String value);
void sendLEDUpdates();
void sendLEDEffect(int effectCode);
void sendErrorToESP(int errorCode);
void processSlaveData(String key, String value, int module);
void displayLCDDisplay(String line1, String line2);
void checkFingers();
bool resetGame(void *);
void checkBallLost();
void sendFingerUpdate();
void checkBallInStart();
#line 81 "C:/Users/fkirc/Documents/GitHub/SIA-2025-source/Arduino-Code/Main/Mega/Mega_PIO/src/Mega.ino"
void setBacklightPercent(int percent) {
    percent = constrain(percent, 0, 100);
    int pwm = map(percent, 0, 100, 10, 255);
    analogWrite(backlightPin, pwm);
}


void setup() {
    pinMode(backlightPin, OUTPUT);
    pinMode(ballLostSensor, INPUT_PULLUP);
    pinMode(outputFinger, OUTPUT);
    pinMode(singalForBallStart, INPUT_PULLUP);
    pinMode(ballInStart, OUTPUT);




    Wire.begin();
    Serial.begin(9600);
    mySoftwareSerial.begin(9600);
    dfplayerInitialized = myDFPlayer.begin(mySoftwareSerial);
    if (!dfplayerInitialized) {
        Serial.println("DFPlayer init failed");
    }
    Serial.println("Flipper System Starting...");

    lcd.begin(16, 2);
    analogWrite(backlightPin, 50);
    lcd.clear();
    lcd.print("Flipper System");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");
    delay(1500);
    lcd.clear();
    if(dfplayerInitialized) {
        myDFPlayer.loopFolder(1);
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


    static unsigned long lastSlaveCheck = 0;
    if (millis() - lastSlaveCheck >= 800) {
        printConnectionFromSlaves();
        lastSlaveCheck = millis();
    }

    static unsigned long lastDataCheck = 0;
    if (millis() - lastDataCheck >= 2000) {
        sendStatusToAdminPanel();
        reciveMessagesFromAdminPanel();
        sendLEDUpdates();
        myDFPlayer.volume(volume);
        lastDataCheck = millis();
    }
}


void setDebugMode(bool enable) {
    gameState = enable ? DEBUG : WAIT_FOR_BALL;
    lastGameState = gameState;
    handleLCDDisplay();
}


void checkGameState() {
    if (gameState == DEBUG || gameState == RESET) return;

    if (gameState != lastGameState) {
        handleLCDDisplay();

        lastGameState = gameState;
    }
}


void startGameOver() {
    if(dfplayerInitialized) {
        myDFPlayer.loopFolder(1);
    }
    sendLEDEffect(5);
    lastGameState = IN_GAME;
    gameState = GAME_OVER;
    handleLCDDisplay();
    gameOverTask = timer.in(5000, finishGameOver);
}

bool finishGameOver(void *) {
    gameState = RESET;
    handleLCDDisplay();
    resetTask = timer.in(2000, resetGame);
    return false;
}


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


long updateBeginTime;
void printConnectionFromSlaves() {
    String collectedSlaveData = "";
    updateBeginTime = millis();

    for (int i = 0; i < moduleCount; i++) {
        int addr = moduleSlaves[i];
        bool alive = isSlaveAlive(addr);
        if (!alive) {
            continue;
        }


        Wire.requestFrom(addr, 50);

        String answer = "";
        while (Wire.available()) {
            answer += (char)Wire.read();
        }

        collectedSlaveData += "Slave " + String(addr) + ": " + answer + "\n";

        if (answer.length() == 0) {
            continue;
        }


        int dataCount;
        String* data = splitString(answer, '|', dataCount);

        for (int j = 0; j < dataCount; j++) {
            if (data[j].indexOf(':') == -1) continue;

            int count;
            String* dataset = splitString(data[j], ':', count);

            if (count == 2) {


                processSlaveData(dataset[0], dataset[1], addr);
            }

            delete[] dataset;
        }
        delete[] data;
    }

    Serial.println(collectedSlaveData);



}

void processSlaveData(String key, String value, int module) {
    int dataValue = value.toInt();
    auto addPoints = [&](const char* type, int basePoints) {
        if (gameState != IN_GAME) return;
        if(points > 1000 && !pointsOver1000) {
            pointsOver1000 = true;
            myDFPlayer.advertise(1);
            sendLEDEffect(8);
        }
        points += dataValue * multiplier * basePoints;
        handleLCDDisplay();
    };

    if(key == "bth"){
        addPoints("Bumper Hits", pointsBumper);
        if(dataValue > 0 && gameState == IN_GAME){
            sendLEDEffect(1);

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


            ballEntryCounter++;
            if (ballEntryCounter % BALLS_PER_SONG_INCREMENT == 0) {
                songStartOffset = (songStartOffset + 1) % SONGS_IN_FOLDER;
            }


            int songToPlay = (songStartOffset % SONGS_IN_FOLDER) + 1;

            if(dfplayerInitialized) {


                int absoluteTrack = 200 + songToPlay;
                myDFPlayer.playFolder(2, songToPlay);
                myDFPlayer.loop(absoluteTrack);
            }
        }
    }else{
        digitalWrite(ballInStart, LOW);
    }
}


void checkLEDEffect4Timeout() {
    if (gameState == IN_GAME && ledEffect4Active && (millis() - ledEffect4StartTime >= LED_EFFECT_4_DURATION)) {
        sendLEDEffect(6);
        ledEffect4Active = false;
    }
}