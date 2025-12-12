# 1 "C:\\Users\\fkirc\\AppData\\Local\\Temp\\tmpbe0oa6c6"
#include <Arduino.h>
# 1 "C:/Users/fkirc/Documents/GitHub/SIA-2025-source/Arduino-Code/Main/Slave/Slave_4_PIO/src/Slave_4.ino"
#include <Arduino.h>
#include <Wire.h>
void setup();
void loop();
void requestEvent();
#line 4 "C:/Users/fkirc/Documents/GitHub/SIA-2025-source/Arduino-Code/Main/Slave/Slave_4_PIO/src/Slave_4.ino"
void setup() {
  Wire.begin(4);

  Wire.onRequest(requestEvent);
}

void loop() {
  delay(100);
}





void requestEvent() {

  Wire.write("text:Hallo");

}