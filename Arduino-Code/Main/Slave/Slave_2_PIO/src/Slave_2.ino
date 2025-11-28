#include <Wire.h>
#include <Arduino.h>

int sensor1 = 2; // NOT A2 !!!!!!!!!!!!!!!!!!!!
int sensor2 = 2;
int sensor3 = 3;
int sensor4 = 4;

int hits_goals = 0;

char message[50];

void setup() {
  Serial.begin(9600);
  pinMode(sensor1, INPUT_PULLUP);

  Wire.begin(2);  // Arduino als I2C-Slave mit Adresse 2

  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
  attachInterrupt(digitalPinToInterrupt(sensor1), hit_Goal, FALLING);
}

void loop() {
  // if(digitalRead(sensor1) == 0) hit_Goal();
  // delay(200);
}

void hit_Goal(){
  hits_goals++;
  Serial.print("Hit registered Now: ");
  Serial.print(hits_goals);
  Serial.print("\n");
}

void requestEvent() {
  sprintf(message, "ht1:%d|", hits_goals);
  hits_goals = 0;
  Wire.write(message);
  Serial.println("Daten gesendet");
}
