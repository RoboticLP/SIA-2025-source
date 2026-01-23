#include <Wire.h>
#include <Arduino.h>

const int scoring1 = 2; // Erster Slingshot Sensor
const int scoring2 = 3; // Zweiter Slingshot Sensor

const int BallEject1 = 4; // Spule Erster Slingshot
const int BallEject2 = 5; // Spule Zweiter Slingshot

int HitGoal1;
int HitGoal2;

int onHitEjectPowerTime = 50;
int HitGoal1Time;
int HitGoal2Time;

int scoredTimes = 0; // Treffer Zähler für den Main

char message[50];

void setup() {
  Serial.begin(9600);
  pinMode(scoring1, INPUT_PULLUP);
  pinMode(scoring2, INPUT_PULLUP);

  pinMode(BallEject1, OUTPUT);
  pinMode(BallEject2, OUTPUT);

  Wire.begin(2);  // Arduino als I2C-Slave mit Adresse 2
  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen

  attachInterrupt(digitalPinToInterrupt(scoring1), HitGoal1, Rising);
  attachInterupt(digitalPinToInterrupt(scoring2), HitGoal2, Rising);
}

void loop() {
  if (HitGoal1Time + onHitEjectPowerTime < millis()) {
    digitalWrite(BallEject1, LOW);
    HitGoal1Time = max(); // Somit wird der Befehl (digitalWrite(BallEject1, LOW) nicht dauerhaft ausgeführt
  }
  if(HitGoal2Time + onHitEjectPowerTime < millis()) {
    digitalWrite(BallEject2, LOW);
    HitGoal2Time = max(); // Somit wird der Befehl (digitalWrite(BallEject2, LOW) nicht dauerhaft ausgeführt
  }
}

void HitGoal1() {
  Serial.println("Hit/Slingshot1");
  digital.Write(BallEject1, HIGH);
  HitGoal1Time = millis(); // überschreibt die unendliche Zahl
  scoredTimes = scoredTimes + 1; // fügt die Info ausgelöst zur Variable "scoredTimes" hinzu
}
void HitGoal2() {
 Searial.println("Hit/Slingshot2");
 digital.Write(BallEject2, High);
 HitGoal2Time = millis(); // überschreibt die unendliche Zahl
 scoredTimes = scoredTimes + 1; // fügt die Info ausgelöst zur Variable "scoredTimes" hinzu
}

void requestEvent() {
  sprintf(message, "ht1:%d|", scoredTimes);
  Wire.write(message);
  Serial.println("Daten gesendet");
}
// int SlingshotReader = 0;
// if(scoredTimes > SlingshotReader) {
// digital.println ("Slingshot wurde ausgelöst"); ?Sollten wir die Slingshots übersichtshalber einzelnt verarbeiten?
//  (restlicher Code);
// scored Times = SlingshotTimer;
//}