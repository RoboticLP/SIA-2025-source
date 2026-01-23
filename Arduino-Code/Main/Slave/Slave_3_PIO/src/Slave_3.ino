#include <Arduino.h>
#include <Wire.h>
#include <Arduino.h>
#define BumperLight_1 6
#define BumperLight_2 7
#define BumperLight_3 8
#define interrupt_1 2
#define interrupt_2 3
#define normal_pin_3 4

volatile int writeConsoleThatBumperTriggered = -1;

volatile unsigned long lastInterruptTime[3] = {0,0,0};

volatile int totalTriggerAmountInRuntime = 0; //hauptsächlich zum debuggen, um neue nachrichten im serial erkennen zu können


int light[3] = {BumperLight_1,BumperLight_2,BumperLight_3};
long lightDuration = 2000;
boolean lightActive[3] = {false,false,false};
unsigned long lightOffTime[3]; //hier wird der cooldown für die lichter gespeichert (als millis() timestamp)

String error_module3 = ""; // String that is send to the master with I2C  in case an error occurs

char requestEventAnswer[50]; //text thats 50 bytes long to send the master module

// ───────────────────── Globale Variablen ─────────────────────
volatile int hitpoints = 0;

char message[50];
char command[20];   // für empfangene Kommandos

// ───────────────────── Bumper Methods ─────────────────────
//each bumper needs an extra method, because attachInterrupt doesn't allow methods with parameters
void triggerBumperOne(){ 
    unsigned long now = millis();
    if (now - lastInterruptTime[0] > 200) { // 200 ms entprellen
    hitpoints++;
    lightActive[0] = true;
    totalTriggerAmountInRuntime++; //wird in der Nachricht zu Serial benutzt
    writeConsoleThatBumperTriggered = 1; //volatile um eine Nachricht zum Serial zu triggern
    lastInterruptTime[0] = now;
    }
}

void triggerBumperTwo(){
    unsigned long now = millis();
    if (now - lastInterruptTime[1] > 200) { // 200 ms entprellen
    hitpoints++;
    lightActive[1] = true;
    totalTriggerAmountInRuntime++; //wird in der Nachricht zu Serial benutzt
    writeConsoleThatBumperTriggered = 2; //volatile um eine Nachricht zum Serial zu triggern
    lastInterruptTime[1] = now;
    }
}

void triggerBumperThree(){
    unsigned long now = millis();
    if (now - lastInterruptTime[2] > 200) { // 200 ms entprellen
    hitpoints++;
    lightActive[2] = true;
    totalTriggerAmountInRuntime++; //wird in der Nachricht zu Serial benutzt
    writeConsoleThatBumperTriggered = 3; //volatile um eine Nachricht zum Serial zu triggern
    lastInterruptTime[2] = now;
    }
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
    attachInterrupt(digitalPinToInterrupt(interrupt_1), triggerBumperOne, FALLING);
    //–––Interrupt 2––– (Bumper 2)
    pinMode(interrupt_2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interrupt_2), triggerBumperTwo, FALLING);
    //–––Pin 4–– (Bumper 3)
    pinMode(normal_pin_3, INPUT_PULLUP);
    
    //––– Bumper Lights –––
    for(int i = 0;i < 3; i++) pinMode(light[i],OUTPUT);
    
}

// ───────────────────── Loop ─────────────────────
void loop() {
    //——————————————–Konsole schreiben (Serial.println geht nicht im Interrupt)————————————————
    if(writeConsoleThatBumperTriggered != -1){
        int dummy = writeConsoleThatBumperTriggered; //um sicherzustellen, dass die nachricht nicht mehrmals gesendet wird
        writeConsoleThatBumperTriggered = -1;
        Serial.println(String("Triggered Bumper ") + dummy + String(" [") + totalTriggerAmountInRuntime + String("]"));
    }

    //–––––––Bumper 3 Abfragen, da es keinen dritten Interrupt-Pin gibt–––––––
    static bool lastState = HIGH; // static --> wird nicht jedesmal neu gesetzt
    bool currentState = digitalRead(normal_pin_3);

    if (lastState == HIGH && currentState == LOW) {
        triggerBumperThree();
    }
    lastState = currentState;

    //––––––––Lichter––––––––
    for(int i = 0; i<3; i++){
        //Abfragen, ob eins der Interrupts oder Bumper 3 eine Lichteraktivierung angefragt hat
        if(lightActive[i]){
            digitalWrite(light[i], HIGH);
            lightOffTime[i] = millis() + lightDuration;
            lightActive[i] = false;
        }
        //Abfragen, ob ein Licht seinen Timestamp erreicht hat und ausgeschaltet werden muss
        if (digitalRead(light[i]) == HIGH && millis() > lightOffTime[i]) { //turn off light if millis() timestamp is reached
            digitalWrite(light[i], LOW);
        }
    }
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
        Serial.println("resetting...");
    }
}
