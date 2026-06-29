#include <Arduino.h>
#include <Wire.h>

// ───────────────────── Pins / Konstanten ─────────────────────
#define interrupt_1 2          // Bumper-Sensor (LOW-aktiv, INPUT_PULLUP)
const unsigned long CONFIRM_MS  = 5;    // Pegel nach 5 ms bestätigen
const unsigned long DEBOUNCE_MS = 200;  // Mindestabstand zwischen Treffern

// ───────────────────── Globale Variablen ─────────────────────
volatile bool          edgePending      = false;  // Flanke erkannt, noch nicht bestätigt
volatile unsigned long edgeTime         = 0;      // Zeitpunkt der Flanke
volatile int           hitpoints        = 0;      // bestätigte Treffer
unsigned long          lastHitTime      = 0;      // letzter gezählter Treffer

char message[50];
char command[20];

// ───────────────────── Interrupt: nur Flanke merken ─────────────────────
void triggerBumper() {
    edgePending = true;
    edgeTime    = millis();
}

// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(3);                 // I2C Slave Adresse 3
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    pinMode(interrupt_1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interrupt_1), triggerBumper, FALLING);
}

// ───────────────────── Loop: Pegel bestätigen ─────────────────────
void loop() {
    if (edgePending && (millis() - edgeTime > CONFIRM_MS)) {
        edgePending = false;

        // Nur zählen, wenn der Pin nach CONFIRM_MS noch LOW ist (echter Treffer)
        // und der letzte Treffer lange genug her ist (Entprellung).
        if (digitalRead(interrupt_1) == LOW &&
            (millis() - lastHitTime > DEBOUNCE_MS)) {
            hitpoints++;
            lastHitTime = millis();
        }
        // Pin schon wieder HIGH -> war nur ein Stoerspike -> verwerfen
    }
}

// ───────────────────── Hilfsfunktionen ─────────────────────
void handleReset() {
    noInterrupts();
    hitpoints   = 0;
    edgePending = false;
    interrupts();
}

// ───────────────────── I2C Callbacks ─────────────────────
void requestEvent() {
    int toSend;
    noInterrupts();          // hitpoints wird im ISR geaendert -> atomar lesen/ruecksetzen
    toSend    = hitpoints;
    hitpoints = 0;
    interrupts();

    snprintf(message, sizeof(message), "bth:%d|", toSend);
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
