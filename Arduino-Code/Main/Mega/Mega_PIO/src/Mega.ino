#include <Arduino.h>
#include <Wire.h>
#include "utils.h"

#define slave2 2
#define slave3 3
#define slave4 4
#define slave5 5
#define adminpanel 6

bool ballInGame = false;

int moduleCount = 3;  // Anzahl der Module (Slaves)
int moduleSlaves[3] = {slave2, slave3, slave4};  // Array mit Slave-Adressen

int multiplier = 100;
long points = 0;

void setup() {
  Wire.begin();        // I2C Bus starten (Master)
  Serial.begin(9600);
}

long updateBeginTime;

void loop() {
  // Slaves abfragen und alle erhaltenen Daten anzeigen
  printConnectionFromSlaves();
  Serial.println("Points: "+String(points));  // Gesamtpunkte ausgeben
  delay(4000);  // 4 Sekunden warten
}

void printConnectionFromSlaves() {
  Serial.println("#######START CONSOLE#######");
  updateBeginTime = millis();

  for (int i = 0; i < moduleCount; i++) {
    int addr = moduleSlaves[i];

    // 🔍 Prüfen ob Slave existiert
    if (!isSlaveAlive(addr)) {
      continue;
    }
    int alive = isSlaveAlive(addr) ? 1 : 0;
    char aliveChar = alive ? '1' : '0';
    sendESP32ToAdminPanel("M" + String(addr) + ":" +aliveChar+"|");


    Wire.requestFrom(addr, 50);

    String answer = "";
    while (Wire.available()) {
      answer += (char)Wire.read();
    }

    int dataCount;
    String* data = splitString(answer, '|', dataCount);

    for (int j = 0; j < dataCount; j++) {
      if (data[j].indexOf(':') != -1) {
        int count;
        String* dataset = splitString(data[j], ':', count);

        if (count == 2) {
          handleModule(addr, dataset[0], dataset[1]);
        }

        delete[] dataset;
      }
    }
    delete[] data;
  }

  Serial.print((millis() - updateBeginTime) / 1000.0);
  Serial.println(" Sekunden");

  if (isSlaveAlive(adminpanel)) {
  Wire.requestFrom(adminpanel, 50);
  String adminAnswer = "";
  while (Wire.available()) {
    adminAnswer += (char)Wire.read();
  }
  Serial.print("ESP32: ");
  Serial.println(adminAnswer);
} else {
  Serial.println("Adminpanel nicht erreichbar");
}

  Serial.println("#######ENDE CONSOLE#######");
}

void sendESP32ToAdminPanel(String message) {
  if (!isSlaveAlive(adminpanel)) {
    return;
  }

  Wire.beginTransmission(adminpanel);
  Wire.write(message.c_str());
  Wire.endTransmission();
}



bool isSlaveAlive(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}


// Diese Funktion verarbeitet jedes empfangene Key-Value Paar für das gegebene Modul
void handleModule(int module, String key, String value) {
  switch (module) {
    case slave2:  // Slave 2
      processSlaveData(key, value, module);
      break;
    case slave3:  // Slave 3
      processSlaveData(key, value, module);
      break;
    case slave4:  // Slave 4
      processSlaveData(key, value, module);
      break;
    default:
      break;
  }
}

// Diese Funktion verarbeitet die spezifischen Daten für jedes Modul
void processSlaveData(String key, String value, int module) {
  int dataValue = value.toInt();  // Wert in eine Ganzzahl umwandeln
  
  // Beispielhafte Verarbeitungslogik je nach Schlüssel
  if (key == "ht1") {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" ht1: ");
    Serial.println(dataValue);
    // Addiere den Wert zu den Punkten
    points += dataValue * multiplier;
  }
  else if (key == "ht2") {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" ht2: ");
    Serial.println(dataValue);
    // Addiere den Wert zu den Punkten
    points += dataValue * multiplier;
  }
  else if (key == "ht3") {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" ht3: ");
    Serial.println(dataValue);
  }
  else if (key == "err") {

    //HIER ERROR AN ESP
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" error: ");
    Serial.println(value);
  }
  else if (key == "text") {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" text: ");
    Serial.println(value);
  }
  else if (key == "ballingame") {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" ballInGame: ");
    Serial.println(value);
    ballInGame = (dataValue == 1);
  }

  else {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" received unknown key: ");
    Serial.println(key);
  }
}
