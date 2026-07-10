#include <Arduino.h>
#include <Wire.h>

#define TASTER_PIN 2
const unsigned long DEBOUNCE_MS = 50;

volatile int  hitpoints   = 0;
unsigned long lastHitTime = 0;
bool          wasPressed  = false;
char message[50];
char command[20];

void setup() {
    Serial.begin(9600);
    Wire.begin(4);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    pinMode(TASTER_PIN, INPUT); // extern auf GND gezogen oder INPUT_PULLDOWN falls verfügbar
}

void loop() {
    bool pressed = digitalRead(TASTER_PIN) == HIGH;

    if (pressed && !wasPressed && (millis() - lastHitTime > DEBOUNCE_MS)) {
        hitpoints++;
        lastHitTime = millis();
    }
    wasPressed = pressed; // erst weiter wenn losgelassen
}

void requestEvent() {
    int toSend;
    noInterrupts();
    toSend    = hitpoints;
    hitpoints = 0;
    interrupts();
    snprintf(message, sizeof(message), "tah:%d|", toSend);
    Wire.write(message);
}

void receiveEvent(int howMany) {
    int i = 0;
    while (Wire.available() && i < (int)sizeof(command) - 1)
        command[i++] = Wire.read();
    command[i] = '\0';
    if (strcmp(command, "resetGame") == 0) {
        noInterrupts(); hitpoints = 0; interrupts();
    }
}
