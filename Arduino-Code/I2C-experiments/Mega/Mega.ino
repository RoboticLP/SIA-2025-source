#include <Wire.h>

#define slave2 2
#define slave3 3
#define slave4 4
#define slave5 5

int slaves[4] = {slave2, slave3, slave4, slave5};

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Wire.setClock(400000);

  Serial.begin(9600);
}

long updateBeginTime;

void loop() {
  updateBeginTime = millis();
  for (int i = 0; i < 4; i++) {
    Wire.requestFrom(slaves[i], 20);
    while (Wire.available()) { // peripheral may send less than requested
      char c = Wire.read(); // receive a byte as character

      Serial.print(c);         // print the character
    }
    Serial.print(" NEXT SLAVE\n");
  }
  Serial.print((millis() - updateBeginTime) / 1000.0); Serial.print(" Sekunden\n");

  delay(2000);
}