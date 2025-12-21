#include <Arduino.h>
#include <Wire.h>


int hits_goals = 0;

char message[50];

void setup() {
  Serial.begin(9600);

  Wire.begin(4);  // Arduino als I2C-Slave mit Adresse 2

  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
}

void loop() {
  
}


void requestEvent() {
  sprintf(message, "ht1:%d", hits_goals);
  hits_goals = 0;
  Wire.write(message);
}