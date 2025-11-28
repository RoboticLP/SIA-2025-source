#include <Wire.h>
#include "utils.h"

#define slave2 2
#define slave3 3
#define slave4 4
#define slave5 5
#define adminpanel 6

int moduleCount = 3;
int moduleSlaves[3] = {slave2, slave3, slave4}; // slave 5

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Wire.setClock(400000);

  Serial.begin(9600);
}

long updateBeginTime;

void loop() {
  updateBeginTime = millis();
  for (int i = 0; i < moduleCount; i++) {
    Wire.requestFrom(moduleSlaves[i], 20);

    // get all data from that slave and save it into 'answer'
    String answer = "";
    while (Wire.available()) {
      char c = Wire.read();
      answer += c;
    }

    // process data recieved by that slave
    int dataCount;
    String* data = splitString(answer, '|', dataCount);
    for (int j = 0; j < dataCount; j++) {
      if (data[j].indexOf(':') != -1) { // process substring if its a key with a value (ex.: ht1:1)
        int count;
        String* dataset = splitString(data[j], ':', count);
        String key = dataset[0];    // key from one of the datasets (ex.: hit1)
        String value = dataset[1];  // value of that key
        Serial.print(moduleSlaves[i]);
        Serial.print("Key: ");
        Serial.print(key);
        Serial.print(" Value: ");
        Serial.println(value);
        delete[] dataset;
      } else {  // if its just a piece of data, not a key with a value (ex.: err)
        // if we recieve err, reqeuest error data, process it and then continue
        Serial.print(moduleSlaves[i]);
        Serial.print("No key: ");
        Serial.println(data[j]);
      }
    }
    delete[] data;
  }
  Serial.print((millis() - updateBeginTime) / 1000.0); Serial.print(" Sekunden\n");

  // request admin panel less often with same logic as above - TESTING
  Wire.requestFrom(6, 30);

  // get all data from that slave and save it into 'answer'
  String answer = "";
  while (Wire.available()) {
    char c = Wire.read();
    answer += c;
  }
  Serial.print("ESP32: ");
  Serial.println(answer);
  
  delay(4000);
}