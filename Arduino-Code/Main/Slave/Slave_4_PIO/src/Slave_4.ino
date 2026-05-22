#include <Arduino.h>
#include <Wire.h>

// ───────────────────── Globale Variablen ─────────────────────
volatile int ballingame = 0;
volatile bool ballReported = false; // wurde schon gemeldet?
volatile int hitpoints = 0;

const int taster = 2;          // Taster (LOW-aktiv)
const int gameSensor = 3;      // Ballsensor (Interrupt)

char message[100];


// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(4);                 // I2C Slave Adresse 4
    Wire.onRequest(requestEvent);  // Anfrage vom Master
    Wire.onReceive(recieveEvent); // Empfang vom Master

    pinMode(gameSensor, INPUT_PULLUP);
    pinMode(taster, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(gameSensor), ballInGameISR, FALLING);

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
    ballingame = false;
    ballReported = false;
    hitpoints = 0;
    //Hier message das reset fertig bei dem module evt zu adminpanel? // nope - adiii
}

// ISR → so kurz wie möglich!
void ballInGameISR() {
    Serial.println("Ball Sensor ausgelöst!");
    if (!ballReported) {
        Serial.println("Ball im Spiel erkannt, sende Update an Master...");
        ballingame = 1;
        ballReported = true;
    }
}

// ───────────────────── I2C Callback ─────────────────────
void requestEvent() {
    int len = 0;

    // Ball-Event NUR EINMAL senden
    len += snprintf(message + len, sizeof(message) - len,
                        "ballingame:%d|", ballingame);

    // Hitpoints der targets immer senden
    len += snprintf(message + len, sizeof(message) - len,
                    "tah:%d|", hitpoints);

    
    Wire.write(message);
    hitpoints = 0;
    ballingame = 0;
    ballReported = false;
}

char command[20];
void recieveEvent(int numBytes) {

  if (strcmp(command, "resetGame") == 0) {
      handleReset();
    }
}
