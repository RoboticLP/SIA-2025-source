#include <Wire.h>
#include <Arduino.h>

int scoring = 6; //Taster Pin
int SlingshotReader = 0; //Auswertung
int scoredTimes = 0; //Auswertung

unsigned long TasterZeit = 0; // Zeit beim drücken ders Tasters
int TasterGedrueckt = 0; // If-Clause Bestätigung

int entprellZeit = 200; // 200 ms

char command[20];   // für empfangene Kommandos


char message[50];
//____________________________void Setup___________________________
void setup() {
  Wire.begin(2);  
  Wire.onRequest(requestEvent);  // Master fragt Daten an
  Wire.onReceive(recieveEvent);  // Master sendet Daten
  
  Serial.begin(9600);
 
  pinMode(scoring, INPUT_PULLUP);

}

//___________________________void Loop______________________________
void loop() {

unsigned long now = millis();

if(digitalRead(scoring) == LOW) {
TasterZeit = now;                // aktualisiert die Tasterzeit
TasterGedrueckt = 1;  // registriert das Signal und speichert es als Variable
}
if(now - TasterZeit > entprellZeit && TasterGedrueckt == 1) {
TasterGedrueckt = 0;                   // zurücksetzen 
scoredTimes = scoredTimes + 1;
}

if(scoredTimes > SlingshotReader) {
    Serial.println ("Slingshots wurden ausgelöst");
    Serial.println(scoredTimes);
    // code...
    SlingshotReader = scoredTimes;
 } }

void handleReset() {
scoredTimes = 0;
SlingshotReader = 0;
}
//___________________________void requestEvent_______________________
void requestEvent() {
  sprintf(message, "ssh:%d|", scoredTimes);
  Wire.write(message);
  Serial.println();//Daten gesendet
}
void recieveEvent(int numBytes) {

  if (strcmp(command, "resetGame") == 0) {
      handleReset();
      Serial.println("resetting...");
    }
}
