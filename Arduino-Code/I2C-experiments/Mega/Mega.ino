#include <Wire.h>

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)

  Serial.begin(9600);  // start serial for output
}

void loop() {
  Wire.requestFrom(6, 15);    // request 6 bytes from peripheral device #6 (arduino uno)
  Wire.requestFrom(6, 15);
  while (Wire.available()) { // peripheral may send less than requested
    char c = Wire.read(); // receive a byte as character

    Serial.print(c);         // print the character
  }
  Serial.print("\n");

  Wire.requestFrom(2, 18);
  while (Wire.available()) { // peripheral may send less than requested
    char c = Wire.read(); // receive a byte as character

    Serial.print(c);         // print the character
  }
  Serial.print("\n");

  delay(500);
}