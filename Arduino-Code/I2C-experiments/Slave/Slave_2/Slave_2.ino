#include <Wire.h>

int sensor1 = A0;
int sensor2 = 2;
int sensor3 = 3;
int sensor4 = 4;

int hits_goals = 0;

char message[50];

void setup() {  
  Serial.begin(9600);
  pinMode(sensor1, INPUT);

  Wire.begin(2);  // Arduino als I2C-Slave mit Adresse 2

  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
  //attachInterrupt(digitalPinToInterrupt(sensor1),hit_Goal,RISING);
}

void loop() {
  if(digitalRead(sensor1) == 1) hit_Goal();
  delay(200);
}

void hit_Goal(){
  hits_goals++;
  Serial.println("Hit registered Now: "+hits_goals);
}

void sendHits_GoalsData(){
  sprintf(message, "ht1:%d|",hits_goals);
  hits_goals = 0;
}

void sendData(){
  sendHits_GoalsData();
  Wire.write(message);
}

void requestEvent() {
  sendData();
  Serial.println("Daten gesendet");
}
