#include <Wire.h>
#include <Arduino.h>

int scoring = 2;     // Taster Pin
int scoredTimes = 0; // Auswertung

unsigned long TasterZeit = 0; // Zeit beim drücken ders Tasters
int TasterGedrueckt = 0;      // If-Clause Bestätigung

int entprellZeit = 200; // 200 ms

char command[20]; // für empfangene Kommandos

char message[50];
//____________________________void Setup___________________________
void setup()
{
  Wire.begin(2);
  Wire.onRequest(requestEvent); // Master fragt Daten an
  Wire.onReceive(recieveEvent); // Master sendet Daten

  Serial.begin(9600);

  pinMode(scoring, INPUT_PULLUP);
}

//___________________________void Loop______________________________
void loop()
{

  unsigned long now = millis();

  if (digitalRead(scoring) == LOW && !TasterGedrueckt)
  {
    TasterGedrueckt = 1;
    TasterZeit = now;
  }

  if (TasterGedrueckt && now - TasterZeit > entprellZeit)
  {
    scoredTimes++;
    TasterGedrueckt = 0;

    while (digitalRead(scoring) == LOW)
    {
      TasterGedrueckt = 0; // zurücksetzen
      scoredTimes = scoredTimes + 1;
    }
  }
}

void handleReset()
{
  scoredTimes = 0;
}
//___________________________void requestEvent_______________________
void requestEvent()
{
  sprintf(message, "ssh:%d|", scoredTimes);
  Wire.write(message);
  scoredTimes = 0;
}

void recieveEvent(int numBytes)
{
  int i = 0;

  while (Wire.available() && i < sizeof(command) - 1)
  {
    command[i++] = Wire.read();
  }

  command[i] = '\0';

  if (strcmp(command, "resetGame") == 0)
  {
    handleReset();
  }
}
