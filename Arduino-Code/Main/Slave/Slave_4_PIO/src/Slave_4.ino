#include <Arduino.h>
#include <Wire.h>

int hitpoints = 0;

int ballingame = 0; // Status, ob Ball im Spiel ist

int taster = 2; // Pin für die Taster
int gameSensor = 3; // Pin für den Sensor

char message[50];

void setup() {
  Serial.begin(9600);

  Wire.begin(4);  // Arduino als I2C-Slave mit Adresse 4

  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
  pinMode(gameSensor, INPUT_PULLUP);
  pinMode(taster, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(gameSensor), ballInGame, FALLING);
  handleReset();
}

void loop() {
  if(digitalRead(taster) == LOW){ //schauen ob low active
    while (digitalRead(taster) == LOW){
      continue;
    }
    hitpoints++;
  }
}

void handleReset(){
  ballingame = 0;
  hitpoints = 0;
}

void ballInGame(){
  ballingame = 1;
}

void requestEvent() {
  sprintf(message, "ballingame:%d|ht1:%d", ballingame, hitpoints);
  hitpoints = 0; // nach dem Senden der Hitpoints zurücksetzen
  Wire.write(message);  // sende Nachricht an Master
}
