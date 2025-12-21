#include <Arduino.h>
#include <Wire.h>

int hits_goals = 0;

int ballingame = 0; // Status, ob Ball im Spiel ist

int gameSensor = 3; // Pin für den Sensor

char message[50];

void setup() {
  Serial.begin(9600);

  Wire.begin(4);  // Arduino als I2C-Slave mit Adresse 4

  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
  pinMode(gameSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(gameSensor), ballInGame, FALLING);
  handleReset();
}

void loop() {

}

void handleReset(){
  ballingame = 0;
  hits_goals = 0;
}

void ballInGame(){
  ballingame = 1;
}

void requestEvent() {
  sprintf(message, "ballingame:%d|err:33", ballingame);
  Wire.write(message);  // sende Nachricht an Master
}
