#include <Arduino.h>
#include <Wire.h>

// ───────────────────── Globale Variablen ─────────────────────
volatile int hitpoints = 0;

char message[50];
char command[20];   // für empfangene Kommandos

// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(3);                  // I2C Slave Adresse 3
    Wire.onRequest(requestEvent);   // Master fordert Daten an
    Wire.onReceive(receiveEvent);   // Master sendet Daten

    handleReset();
}

// ───────────────────── Loop ─────────────────────
void loop() {

}


// ───────────────────── Hilfsfunktionen ─────────────────────
void handleReset() {
    hitpoints = 0;
}

// ───────────────────── I2C Callback ─────────────────────
void requestEvent() {
    // String bauen
    snprintf(message, sizeof(message),
             "ht1:%d|",
             hitpoints);

    hitpoints = 0; // nach dem Senden zurücksetzen

    Wire.write(message);
}

//testen
void receiveEvent(int howMany) {
    int i = 0;

    while (Wire.available() && i < sizeof(command) - 1) {
        command[i++] = Wire.read();
    }
    command[i] = '\0'; // String terminieren

    // Kommando auswerten
    if (strcmp(command, "resetGame") == 0) {
        handleReset();
    }
}
