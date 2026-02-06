# 1 "C:\\Users\\Mariu\\AppData\\Local\\Temp\\tmpmf12u71r"
#include <Arduino.h>
# 1 "C:/Users/Mariu/Documents/Schule/SIA-2025-source/Arduino-Code/Main/Slave/Slave_2_PIO/src/Slave_2.ino"
#include <Wire.h>
#include <Arduino.h>


char message[50];
void setup();
void loop();
void requestEvent();
#line 7 "C:/Users/Mariu/Documents/Schule/SIA-2025-source/Arduino-Code/Main/Slave/Slave_2_PIO/src/Slave_2.ino"
void setup() {
  Wire.begin(2);
  Wire.onRequest(requestEvent);
  const int scoring = 6;
  pinMode(scoring, INPUT_PULLUP);
  int SlingshotReader = 0;
  int scoredTimes = 0;
}


void loop() {
  if(digitalRead(scoring) = LOW) {
    scoredTimes = scoredTimes + 1;
  }
  if(scoredTimes > SlingshotReader) {
    digital.println ("Slingshots wurden ausgelöst");
    digital.println(scoredTimes);

    scoredTimes = SlingshotReader;
  }
}

void requestEvent() {
  sprintf(message, "ht1:%d|", scoredTimes);
  Wire.write(message);
  Serial.println("Daten gesendet");
}