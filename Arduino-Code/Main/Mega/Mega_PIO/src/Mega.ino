#include <Arduino.h>
#include <Wire.h>
#include "utils.h"

#define slave2 2
#define slave3 3
#define slave4 4
#define slave5 5
#define adminpanel 6

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

void printConnectionFromSlaves(){
  Serial.println("#######START CONSOLE#######");
  updateBeginTime = millis();

  for (int i = 0; i < moduleCount; i++) {
    // Anfrage an das Slave-Modul senden
    Wire.requestFrom(moduleSlaves[i], 50);

    // Alle Daten vom Slave in einen String sammeln
    String answer = "";
    while (Wire.available()) {
      char c = Wire.read();
      answer += c;
    }

    // Daten aufteilen und verarbeiten
    int dataCount;
    String* data = splitString(answer, '|', dataCount);  // Antwort in Teile splitten

    // Jedes Datenstück bearbeiten
    for (int j = 0; j < dataCount; j++) {
      if (data[j].indexOf(':') != -1) {  // Prüfen, ob es sich um ein Key-Value Paar handelt
        int count;
        String* dataset = splitString(data[j], ':', count);
        String key = dataset[0];  // Schlüssel (z.B. ht1, ht2)
        String value = dataset[1];  // Wert des Schlüssels

        // Verarbeiten des Key-Value Paares für dieses Modul
        handleModule(moduleSlaves[i], key, value); 

        // Speicher freigeben
        delete[] dataset;
      }
    }
    delete[] data;  // Speicher freigeben
  }

  // Zeit ausgeben, wie lange die Verarbeitung dauerte
  Serial.print((millis() - updateBeginTime) / 1000.0);
  Serial.print(" Sekunden\n");

  // Anfrage an das Admin-Panel senden
  Wire.requestFrom(adminpanel, 50);  // Anfrage an Admin Panel (Größe 50)
  Serial.print("ESP32: ");
  String adminAnswer = "";
  while (Wire.available()) {
    char c = Wire.read();
    adminAnswer += c;
  }
  Serial.println(adminAnswer);

  Serial.println("#######ENDE CONSOLE#######");
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

    //HIER ERROR AN ESP
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" text: ");
    Serial.println(value);
  }

  else {
    Serial.print("Module ");
    Serial.print(module);
    Serial.print(" received unknown key: ");
    Serial.println(key);
  }
}
