#include <Arduino.h>
#include <Wire.h>


volatile int hitpoints = 0;

const int taster = 2;          // Taster (LOW-aktiv)

char message[100];


// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(4);                 // I2C Slave Adresse 4
    Wire.onRequest(requestEvent);  // Anfrage vom Master
    Wire.onReceive(recieveEvent); // Empfang vom Master
    pinMode(taster, INPUT_PULLUP);

    handleReset();
}

// ───────────────────── Loop ─────────────────────
void loop() {
  checkTaster();
}

// ───────────────────── Taster funktion ─────────────────────
void checkTaster(){
    // Taster gedrückt?
    if (digitalRead(taster) == LOW) {
        delay(20); // Entprellen

        if (digitalRead(taster) == LOW) {
            hitpoints++;

            // Warten bis losgelassen
            while (digitalRead(taster) == LOW);
        }
    }
}

// ───────────────────── Hilfsfunktionen ─────────────────────
void handleReset() {
    hitpoints = 0;
    //Hier message das reset fertig bei dem module evt zu adminpanel? // nope - adiii
}

// ───────────────────── I2C Callback ─────────────────────
void requestEvent() {
    snprintf(message, sizeof(message),
             "tah:%d|",
             hitpoints);

    
    Wire.write(message);
    hitpoints = 0;
}

char command[20];
void recieveEvent(int numBytes) {

  if (strcmp(command, "resetGame") == 0) {
      handleReset();
    }
}
