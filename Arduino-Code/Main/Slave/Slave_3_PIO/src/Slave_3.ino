#include <Arduino.h>
#include <Wire.h>
#include <Arduino.h>
#define BumperLight_1 6
#define BumperLight_2 7
#define BumperLight_3 8
#define interrupt_1 2
#define interrupt_2 3
#define normal_pin_3 4

int numberOfBumpers = 3;
int light[3] = {BumperLight_1,BumperLight_2,BumperLight_3};
long lightDuration = 1000;

boolean didBumperThreeTriggerAlready = false; //Da es nur 2 Interrupt-Pins gibt, muss Bumper 3 mit einem normalen Pin und dieser Variable gesteuert werden

int bumperHits = 0;

String error_module3 = ""; // String that is send to the master with I2C  in case an error occurs

char requestEventAnswer[50]; //text thats 50 bytes long to send the master module

// ───────────────────── Globale Variablen ─────────────────────
volatile int hitpoints = 0;

char message[50];
char command[20];   // für empfangene Kommandos

// ───────────────────── Bumper Methods ─────────────────────
//each bumper needs an extra method, because attachInterrupt doesn't allow methods with parameters
void triggerBumperOne(){ 
    hitpoints++;
}

void triggerBumperTwo(){
    hitpoints++;
    
}

void triggerBumperThree(){
    hitpoints++;
}


// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(3);                  // I2C Slave Adresse 3
    Wire.onRequest(requestEvent);   // Master fordert Daten an
    Wire.onReceive(receiveEvent);   // Master sendet Daten

    handleReset();
    //–––Interrupt 1––– (Bumper 1)
    pinMode(interrupt_1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interrupt_1), triggerBumperOne, RISING);
    //–––Interrupt 2––– (Bumper 2)
    pinMode(interrupt_1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interrupt_2), triggerBumperTwo, RISING);
    //–––Pin 4–– (Bumper 3)
    pinMode(normal_pin_3, INPUT_PULLUP);
    
    //––– Bumper Lights –––
    for(int i = 0;i < 3; i++) pinMode(light[i],OUTPUT);
    
}

// ───────────────────── Loop ─────────────────────
void loop() {
    //Bumper 3 Abfragen, da es keinen dritten Interrupt-Pin gibt
    if(didBumperThreeTriggerAlready) return;
    if(digitalRead(normal_pin_3)){
        didBumperThreeTriggerAlready = true;
        triggerBumperThree;
    }
    else didBumperThreeTriggerAlready = false;
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
