#include <Arduino.h>
#include <Wire.h>

#define interrupt_1 2

volatile int hitpoints = 0;
volatile unsigned long lastInterruptTime = 0;

char message[50];
char command[20];

// ───────────────────── Interrupt Methode ─────────────────────
void triggerBumper() {
    unsigned long now = millis();

    // Entprellen (200 ms)
    if (now - lastInterruptTime > 200) {
        hitpoints++;
        lastInterruptTime = now;
    }
}

// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    // I2C Slave Adresse 3
    Wire.begin(3);

    // I2C Events
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    // Pin 2 als Interrupt Eingang
    pinMode(interrupt_1, INPUT_PULLUP);

    // Interrupt aktivieren
    attachInterrupt(
        digitalPinToInterrupt(interrupt_1),
        triggerBumper,
        FALLING
    );
}

// ───────────────────── Loop ─────────────────────
void loop() {
    // nichts mehr nötig
}

// ───────────────────── Hilfsfunktionen ─────────────────────
void handleReset() {
    hitpoints = 0;
}

// ───────────────────── I2C Callback ─────────────────────
void requestEvent() {
    snprintf(message, sizeof(message),
             "bth:%d|",
             hitpoints);

    // Nach dem Senden zurücksetzen
    hitpoints = 0;

    Wire.write(message);
}

void receiveEvent(int howMany) {

    int i = 0;

    while (Wire.available() && i < sizeof(command) - 1) {
        command[i++] = Wire.read();
    }

    command[i] = '\0';

    if (strcmp(command, "resetGame") == 0) {
        handleReset();
    }
}