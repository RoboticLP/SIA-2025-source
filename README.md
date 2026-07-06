# SIA-2025-source
Alle Source-Codes der diesjährigen SIA.

## Navigation
> **Seiten**
- [ESP32 Webserver](docs/ESP32-Webserver.md)
- [Pin-Belegung](docs/pins.md)
- [ID-Definitions](docs/id-definitions.md)

> **Auf dieser Seite**
- [I2C Arduino Docs](https://docs.arduino.cc/learn/communication/wire/#arduino-i2c-pins)
- [SIA-Playlist YouTube von Adrian](https://youtube.com/playlist?list=PLDd5gBML2x-RDGElEF7yUXcqWyJdSLfL-&si=K51pTD4f3ktHvXSp)
- [sprintf (Formatierung von Strings)](#sprintf)
---

## I<sup>2</sup>C Infos
| Adressen | Gerät        | Aufgabe / Modul |
|----------|--------------|-----------------|
| (1)      | Arduino Mega | Master          |
| 2        | Arduino Nano | Modul 1 |
| 3        | Arduino Nano | Modul 2 |
| 4        | Arduino Nano | Modul 3 |
| 5        | ESP32        | WS2812B LEDs |
| 6        | ESP32        | WebServer       |

---

#### Datenformat des Daten-Strings der Module (Nano's)
Die Module senden derzeit folgende Key-Values im I²C-String an den Mega-Master:

- `ssh:%d|` für Slingshot-Hits vom Slave 2
- `bth:%d|` für Bumper-Hits vom Slave 3/4
- `err:%s|` für Fehlercodes

> *Die **Reihenfolge der Keys+Werte ist egal**, genau wie ihre **Vorhandenheit** - es werden nur erhaltene Daten verarbeitet und es sind keine speziellen Daten notwendig. Allerdings **muss** jeder versendete Key einen zugehörigen Wert haben*

|  Key-Name  | Wertetyp | Bedeutung                                            |
|------------|----------|------------------------------------------------------|
| ```\|```   | /        | Trennzeichen zwischen den Daten                      |
| ```bth```  | int      | Zahl der Treffer (Hits) der Bumper Tower seit dem letzem Update |
| ```ssh```  | int      | Zahl der Treffer (Hits) der Sling Shots seit dem letzem Update |
| ```err```  | String (Error-code) | Wird nur versendet wenn ein Error vorhanden ist. Sendet dann einen Error-code der vom ESP32 verarbeitet wird. |

> Da es von Arduino selbst keine eigene Lösung für das Splitten von Strings hat benutzen wir folgende eigene Lösung: [splitString](#splitstring)

---

## Datenformat des Admin-Panel Datenstrings (vom ESP32)
Hilfreiche Resourcen:
- [Bedeutungen der Message-IDs](docs/id-definitions.md)

##### Mega > ESP

```c
M2:%d|M3:%d|M4:%d|M5:%d|err:%d
```

| Key-Name | Wertetyp | Bedeutung |
|----------|----------|-----------|
| ```\|``` | /        | Trennzeichen zwischen den Daten |
| ```M2``` | int | Status von Modul 2 (1 = erreichbar, 0 = nicht erreichbar) |
| ```M3``` | int | Status von Modul 3 (1 = erreichbar, 0 = nicht erreichbar) |
| ```M4``` | int | Status von Modul 4 (1 = erreichbar, 0 = nicht erreichbar) |
| ```M5``` | int | Status von Modul 5 (1 = erreichbar, 0 = nicht erreichbar) |
| ```err``` | int | Fehlercode, der an das Frontend gemeldet wird |

---

##### ESP > Mega

Das ESP32 sendet seine Einstellungen in zwei alternierenden I²C-Paketen:

```c
mtpl:%.2f|pbu:%d|psl:%d|pta:%d
```

```c
len:%d|lsp:%d|vol:%d|rst:%d
```

| Key-Name | Wertetyp | Bedeutung |
|----------|----------|-----------|
| ```\|``` | /        | Trennzeichen zwischen den Daten |
| ```mtpl``` | float (2 Kommastellen)  | Der aktuelle Punkte-Multiplier. |
| ```pbu``` | int | Trefferpunktzahl für die Bumper-tower |
| ```psl``` | int | Trefferpunktzahl für die Slingshots |
| ```pta``` | int | Trefferpunktzahl für die Targets |
| ```len``` | int | "1" wenn Lichter an sein sollen, "0" wenn aus |
| ```lsp``` | int | Geschwindigkeit der Lichteffekte von 0-200 |
| ```vol``` | int | Lautstärke des DFPlayers |
| ```rst``` | bit | "1" wenn Spiel resetted werden soll |

---

#### Datenformat Lichteffekte von Mega zu Nano (5)
Das Mega sendet zwei verschiedene Light-Daten an den Nano 5:

- `lsp:%d|len:%d|` für Lichtgeschwindigkeit und Ein/Aus-Zustand
- `eff:%d|` für einen einzelnen Lichteffektcode

|  Key-Name  | Wertetyp | Bedeutung |
|------------|----------|-----------|
| ```\|```   | /        | Trennzeichen zwischen den Daten                      |
| ```lsp```  | int      | Lightspeed, vom ESP32 |
| ```len```  | int      | Light-State (1 = an, 0 = aus) |
| ```eff```  | int      | Lichteffekt-ID, die in Nano 5 als separates Paket ausgelöst wird |

---

## sprintf
Formatierung von Strings
- [Online Tutorial für sprintf](https://www.programmingelectronics.com/sprintf-arduino/)

**Code Beispiel**
```c
char varBuffer[länge];
sprintf(varBuffer, "der String: die %s", "Formatierung");
```
| Format-Specifier | Datentyp |
|------------------|----------|
|```%d```          | Integer (decimal) |
|```%u```          | Unsigned Integer |
|```%ld```         | Long Integer |
|```%lu```         | Unsigned Long |
|```%f```          | Float (funktioniert nicht auf AVR Arduino!) |
|```%s```          | String |
|```%c```          | Character |

---

## splitString
**Anwendung**
```cpp
int datenAnzahl;  // gibt die länge des Arrays an
String* arrayName = splitString(zuSplittenderString, trennzeichen, datenAnzahl);  // splitString(String input, char splitter, int &count)
for (int i = 0; i < datenAnzahl; i++) { // die Schleife zum durchlaufen der sub-Strings
  arrayName[i]; // so greift man auf einen sub-String zu
  // hier können wir die sub-Strings verarbeiten
}
delete[] arrayName; // das Array wieder löschen um memory leaks zu verhindern
```

<details>
<summary>Die Funktion zum kopieren (ist auch in utils.h im Mega Source vorhanden)</summary>

```cpp
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
```

</details>

---
## Interrupt-Benutzung
- Beim Nano sind Pin 2 und 3 Interrupts --> D2 und D3 auf dem Board
>**Anwendung im Setup**
```cpp
pinMode(interrupt_1, INPUT_PULLUP); // oder INPUT
attachInterrupt(digitalPinToInterrupt(interrupt_1), ISR, RISING);
//dabei ist ISR eine Methode, z.B. triggerBumperOne, diese wird bei dem Event (Hier RISING) aufgerufen
//die Methode darf KEINE Parameter haben, und wird ohne Klammern in attachInterrupt() geschrieben
```
