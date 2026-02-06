#include <Wire.h>
#include <Arduino.h>

const int scoring = 6;
int SlingshotReader = 0;
int scoredTimes = 0;

char message[50];
//____________________________void Setup___________________________
void setup() {
  Wire.begin(2);  
  Wire.onRequest(requestEvent); 
 
  pinMode(scoring, INPUT_PULLUP);

}

//___________________________void Loop______________________________
void loop() {
  if(digitalRead(scoring) == LOW) {
    scoredTimes++;
  }
  if(scoredTimes > SlingshotReader) {
    Serial.println ("Slingshots wurden ausgelöst");
    Serial.println(scoredTimes);
    // code...
    scoredTimes = SlingshotReader;
  }
}
//___________________________void requestEvent_______________________
void requestEvent() {
  sprintf(message, "ht1:%d|", scoredTimes);
  Wire.write(message);
  Serial.println("Daten gesendet");
}