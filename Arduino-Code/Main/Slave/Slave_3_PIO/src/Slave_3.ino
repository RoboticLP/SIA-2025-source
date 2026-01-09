#include <Arduino.h>
#include <Wire.h>
#include <Arduino.h>
#define Spule_1 6
#define Spule_2 7
#define Spule_3 8
#define interrupt_1 2
#define interrupt_2 3

int numberOfBumpers = 3;
int bumper[3] = {Spule_1,Spule_2,Spule_3};

int bumperHits = 0;

String error_module3 = "";

char requestEventAnswer[50]; //text thats 50 bytes long to send the master module

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
  //Bumper 1
  
}

//method to trigger a bumper, executed by the interrupt pins in  void loop
void triggerBumper(int number){
  hitpoints++;
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
