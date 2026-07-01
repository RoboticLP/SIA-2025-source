#include <Arduino.h>
#include <Wire.h>

const int scoring = 2;
const unsigned long DEBOUNCE_MS    = 200;
const uint8_t       CONFIRM_COUNT  = 5;    // so viele LOW-Messungen am Stueck = echt
const unsigned long CONFIRM_GAP_US = 300;  // Abstand zwischen den Pruef-Messungen

volatile int  scoredTimes = 0;
bool          pressed     = false;
unsigned long lastHitTime = 0;

char message[50];
char command[20];

void setup() {
    Serial.begin(9600);
    Wire.begin(2);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    pinMode(scoring, INPUT_PULLUP);
}

// prueft, ob der Pin WIRKLICH gedrueckt ist (nicht nur Spulen-Stoerimpuls)
bool reallyPressed() {
    for (uint8_t k = 0; k < CONFIRM_COUNT; k++) {
        if (digitalRead(scoring) != LOW) return false;
        delayMicroseconds(CONFIRM_GAP_US);
    }
    return true;
}

void loop() {
    bool isLow = (digitalRead(scoring) == LOW);

    if (isLow) {
        if (!pressed && (millis() - lastHitTime > DEBOUNCE_MS)) {
            if (reallyPressed()) {
                scoredTimes++;
                lastHitTime = millis();
                pressed = true;
            }
        }
    } else {
        pressed = false;
    }
}

void handleReset() {
    pressed     = false;
    lastHitTime = 0;
    scoredTimes = 0;
}

void requestEvent() {
    int toSend  = scoredTimes;
    scoredTimes = 0;
    snprintf(message, sizeof(message), "ssh:%d|", toSend);
    Wire.write(message);
}

void receiveEvent(int howMany) {
    int i = 0;
    while (Wire.available() && i < (int)sizeof(command) - 1) {
        command[i++] = Wire.read();
    }
    command[i] = '\0';
    if (strcmp(command, "resetGame") == 0) {
        handleReset();
    }
}