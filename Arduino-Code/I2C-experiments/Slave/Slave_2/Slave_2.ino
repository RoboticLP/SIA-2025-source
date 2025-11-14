#include <Wire.h>

int sensor1 = 1;
int sensor2 = 2;
int sensor3 = 3;
int sensor4 = 4;

int pointsToAdd = 0;

char message[50];

void setup() {  
  Serial.begin(9600);
  Wire.begin(2);  // Arduino als I2C-Slave mit Adresse 2
  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
}

void loop() {
  check();
  delay(1000);  // 1 Sekunde warten
  pointsToAdd += random(1, 10);  // Zufällige Punkte zwischen 1 und 10 hinzufügen
}

void check() {
  Serial.println("Points to add: " + String(pointsToAdd));  // String statt Verkettung
  pointsToAdd = 0;  // Punkte zurücksetzen
}

void requestEvent() {
  sprintf(message, "Points to add: %d", pointsToAdd);
  Wire.write(message);  // Nachricht an den Master senden
  pointsToAdd = 0;  // Punkte nach dem Senden zurücksetzen
}
