#include <Wire.h>
#include <Arduino.h>

int scoring = 6; //Taster Pin
int SlingshotReader = 0; //Auswertung
int scoredTimes = 0; //Auswertung

unsigned long TasterZeit = 0; // Zeit beim drücken ders Tasters
int TasterGedrueckt = 0; // If-Clause Bestätigung

int entprellZeit = 200; // 200 ms


char message[50];
//____________________________void Setup___________________________
void setup() {
  Wire.begin(2);  
  Wire.onRequest(requestEvent); 
  Serial.begin(9600);
 
  pinMode(scoring, INPUT_PULLUP);

}

//___________________________void Loop______________________________
void loop() {


if(digitalRead(scoring) == LOW) {
TasterZeit = millis();                // aktualisiert die Tasterzeit
Serial.println("1");
TasterGedrueckt = TasterGedrueckt + 1;  // registriert das Signal und speichert es als Variable
}
if(millis() - TasterZeit > entprellZeit && TasterGedrueckt == 1) {
TasterGedrueckt = 0;                   // zurücksetzen 
Serial.println("2");
scoredTimes = scoredTimes + 1;
}

if(scoredTimes > SlingshotReader) {
    Serial.println ("Slingshots wurden ausgelöst");
    Serial.println(scoredTimes);
    // code...
    SlingshotReader = scoredTimes;
 } }
//___________________________void requestEvent_______________________
void requestEvent() {
  sprintf(message, "ssh:%d|", scoredTimes);
  Wire.write(message);
  Serial.println();//Daten gesendet
}