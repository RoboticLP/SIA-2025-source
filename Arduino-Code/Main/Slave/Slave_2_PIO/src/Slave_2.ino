#include <Wire.h>
#include <Arduino.h>

int scoring = 6;
int SlingshotReader = 0;
int scoredTimes = 0;

bool tasterZustand = HIGH;   // stabiler Zustand
bool letzterGelesenerWert = HIGH;   // letzter Rohwert
unsigned long letzteZeit = 0;

int entprellZeit = 50; // 50 ms


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

bool gelesen = digitalRead(scoring);

 // Änderung erkannt?
  if (gelesen != letzterGelesenerWert) {
    letzteZeit = millis(); // Zeit merken
    letzterGelesenerWert = gelesen;
    Serial.println("1.Schritt");
  
  // Ist der Wert lange genug stabil?
  if ((millis() - letzteZeit) > entprellZeit) {
    Serial.println("2.Schritt");
    // Nur reagieren, wenn stabiler Zustand neu ist
    if (tasterZustand != gelesen) {
    tasterZustand = gelesen;
    Serial.println("3.Schritt");
    // Aktion beim Drücken (LOW wegen INPUT_PULLUP)
      if (tasterZustand == LOW) {
        scoredTimes = scoredTimes + 1;
      }}}}
  if(scoredTimes > SlingshotReader) {
    Serial.println ("Slingshots wurden ausgelöst");
    Serial.println(scoredTimes);
    // code...
    SlingshotReader = scoredTimes;
  }
  }
//___________________________void requestEvent_______________________
void requestEvent() {
  sprintf(message, "ssh:%d|", scoredTimes);
  Wire.write(message);
  Serial.println();//Daten gesendet
}