#include <Arduino.h>
#include <Wire.h>

// ───────────────────── Pins / Konstanten ─────────────────────
const int taster = 2;                    // Bumper-Sensor (LOW-aktiv, INPUT_PULLUP)
const unsigned long CONFIRM_MS  = 5;     // Pegel muss CONFIRM_MS am Stueck LOW sein
const unsigned long DEBOUNCE_MS = 200;   // Mindestabstand zwischen Treffern

// ───────────────────── Globale Variablen ─────────────────────
volatile int  hitpoints   = 0;           // bestaetigte Treffer (im I2C-ISR gelesen)
bool          pressed      = false;       // aktueller bestaetigter Zustand
unsigned long lowSince     = 0;           // seit wann ist der Pin LOW
unsigned long lastHitTime  = 0;           // letzter gezaehlter Treffer

char message[50];
char command[20];

// ───────────────────── Setup ─────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(3);                 // I2C Slave Adresse 3
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    pinMode(taster, INPUT_PULLUP);
}

// ───────────────────── Loop: Pegel pollen ─────────────────────
void loop() {
    bool isLow = (digitalRead(taster) == LOW);

    if (isLow) {
        if (lowSince == 0) {
            lowSince = millis();           // gerade LOW geworden -> Zeit merken
        }
        // Pin lange genug LOW + noch nicht als Treffer gewertet + entprellt?
        if (!pressed &&
            (millis() - lowSince > CONFIRM_MS) &&
            (millis() - lastHitTime > DEBOUNCE_MS)) {
            hitpoints++;
            lastHitTime = millis();
            pressed = true;                // erst wieder zaehlen nach Loslassen
        }
    } else {
        lowSince = 0;                      // HIGH -> zuruecksetzen (war Glitch oder losgelassen)
        pressed  = false;
    }
}

// ───────────────────── Hilfsfunktionen ─────────────────────
void handleReset() {
    hitpoints = 0;
}

// ───────────────────── I2C Callbacks ─────────────────────
void requestEvent() {
    int toSend;
    toSend    = hitpoints;
    hitpoints = 0;

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
