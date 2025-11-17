#include <Wire.h>

#define slave2 2
#define slave3 3
#define slave4 4
#define slave5 5
#define adminpanel 6

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

    // get all data from that slave and save it into 'answer'
    String answer = "";
    while (Wire.available()) {
      char c = Wire.read();
      answer += c;
    }

    // process data recieved by that slave
    int dataCount;
    String* data = splitString(test, '|', dataCount);
    for (int j = 0; j < dataCount; j++) {
      if (data[j].indexOf(':') != -1) { // process substring if its a key with a value (ex.: ht1:1)
        int count;
        String* dataset = splitString(data[j], ':', count);
        String key = dataset[0];    // key from one of the datasets (ex.: hit1)
        String value = dataset[1];  // value of that key
        Serial.print("Key: ");
        Serial.print(key);
        Serial.print(" Value: ");
        Serial.print(value); 
        Serial.print("\n");
        delete[] dataset;
      } else {  // if its just a piece of data, not a key with a value (ex.: err)
        // if we recieve err, reqeuest error data, process it and then continue
        Serial.println(data[j]);
      }
    }
    delete[] data;
    Serial.print(" NEXT SLAVE\n");
  }
  Serial.print((millis() - updateBeginTime) / 1000.0); Serial.print(" Sekunden\n");


  // request admin panel less often with same logic as above
  delay(2000);
}

/* Utility functions */

/* since Arduino doesn't have a builtin way to split Strings, this exists
Usage:
int dataCount;
String* segments = splitString(inputString, '|', dataCount);
for (int j = 0; j < dataCount; j++) {
  segments[j]; // to access substrings
  // process substrings here
}
delete[] segments; // delete array to prevent memory leaks
*/
String* splitString(String input, char splitter, int &count) {
  // Count how many splits we'll have
  count = 1;
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == splitter) {
      count++;
    }
  }
  
  // Create array to hold the substrings
  String* result = new String[count];
  
  // Split the string
  int index = 0;
  int lastPos = 0;
  
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == splitter) {
      result[index] = input.substring(lastPos, i);
      index++;
      lastPos = i + 1;
    }
  }
  
  // Add the last substring
  result[index] = input.substring(lastPos);
  
  return result;
}