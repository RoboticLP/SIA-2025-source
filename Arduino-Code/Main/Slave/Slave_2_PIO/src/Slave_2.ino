#include <Arduino.h>
#include <Wire.h>

int ballEjectRead = 4;
int ballEjectOUT = 5;

int waitTime = 3000; // Zeit in ms bis der Ball ausgeworfen wird

int hits_goals = 0;

char message[50];

void setup() {
  Serial.begin(9600);
  pinMode(ballEjectRead, INPUT_PULLUP);
  pinMode(ballEjectOUT, OUTPUT);

  Wire.begin(2);  // Arduino als I2C-Slave mit Adresse 2

  Wire.onRequest(requestEvent);  // registriere den Event für Datenanforderungen
}

void loop() {
  if(digitalRead(ballEjectRead) == LOW) // NOCH SCHAUEN OB LOW ODER HIGH
    handleBallEject();
}

void handleBallEject(){
  Serial.println("Ball eingegangen, warte "+String(waitTime)+"ms bis Auswurf");
  //LICHT UND SOUND EINFÜGEN
  hits_goals++;
  delay(waitTime);
  digitalWrite(ballEjectOUT, HIGH); // Auswurf einschalten
  delay(500); //Kurz warten damit der Ball auch wirklich ausgeworfen wird
  digitalWrite(ballEjectOUT, LOW); // Auswurf wieder ausschalten
  Serial.println("Ball ausgeworfen");
}

void requestEvent() {
  sprintf(message, "ht1:%d", hits_goals);
  hits_goals = 0;
  Wire.write(message);
}
