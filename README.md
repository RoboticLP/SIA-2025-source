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
Jedes Modul (Arduino Nano) hat bis zu zwei Ziele (Targets) an sich angeschlossen, daher ```ht1``` und ```ht2``` für die Trefferanzahl von jedem Ziel seit der letzten Update-Anfrage.
Jedes Modul wird mehrmals pro Sekunde nach Updates gefragt, hier definieren wir wie die Antworten Formatiert sein müssen:

```c
bth:%d|ssh:%d|tah:%d|ballingame:%d|err:%s
```
> *Die **Reihenfolge der Keys+Werte ist egal**, genau wie ihre **Vorhandenheit** - es werden nur erhaltene Daten verarbeitet und es sind keine speziellen Daten notwendig. Allerdings **muss** jeder versendete Key einen zugehörigen Wert haben*

|  Key-Name  | Wertetyp | Bedeutung                                            |
|------------|----------|------------------------------------------------------|
| ```\|```   | /        | Trennzeichen zwischen den Daten                      |
| ```bth```  | int      | Zahl der Treffer (Hits) der Bumper Tower seit dem letzem Update |
| ```ssh```  | int      | Zahl der Treffer (Hits) der Sling Shots seit dem letzem Update |
| ```tah```  | int      | Zahl der Treffer (Hits) der Targets seit dem letzem Update |
| ```ballingame``` | int | Meldet die Kugel im Game (Sensor hat Kugel erkannt) |
| ```err```  | String (Error-code) | Wird nur versendet wenn Error vorhanden ist. Sendet dann einen Error-code der dann vom ESP32 verarbeitet wird. |

> Da es von Arduino selbst keine eigene Lösung für das Splitten von Strings hat benutzen wir folgende eigene Lösung: [splitString](#splitstring)

---

## Datenformat des Admin-Panel Datenstrings (vom ESP32)
Hilfreiche Resourcen:
- [Bedeutungen der Message-IDs](docs/id-definitions.md)

##### Mega > ESP

```c
gs:%d|err:%d
```

| Key-Name | Wertetyp | Bedeutung |
|----------|----------|-----------|
| ```\|``` | /        | Trennzeichen zwischen den Daten |
| ```gs``` | int | [Gamestate](docs/id-definitions.md#gamestate-id) (aktueller Zustand des Flippers) |
| ```err``` | int | Id eines Logs/Errors für das Frontend |

---

##### ESP > Mega

```c
mtpl:%.2f|pbu:%d|psl:%d|pta:%d|len:%.2f|lsp:%.2f|rst:%d
```

| Key-Name | Wertetyp | Bedeutung |
|----------|----------|-----------|
| ```\|``` | /        | Trennzeichen zwischen den Daten |
| ```mtpl``` | float (2 Kommastellen)  | Der aktuelle Punkte-Multiplier. *Wird intern für I²C in String umgewandelt* |
| ```pbu``` | int | Trefferpunktzahl für die Bumper-tower |
| ```psl``` | int | Trefferpunktzahl für die Slingshots |
| ```pta``` | int | Trefferpunktzahl für die Targets |
| ```len``` | bit | "1" wenn Lichter an sein sollen, "0" wenn aus|
| ```lsp``` | float (2 Kommastellen) | Geschwindigkeit der Lichteffekte von 0-200% (0.0-2.0) |
| ```rst``` | bit | "1" wenn Spiel resetted werden soll |

---

#### Datenformat Lichteffekte von Mega zu Nano (5)
Wir senden die Effekte in Integer, wo der Nano 5 dann weis bei welchem Inegerwert er welchen Effekt abspielen soll. So weis er immer welches Ziel getroffen wurde und zu was er einen Effekt spielen soll.

|  Key-Name  | Wertetyp | Bedeutung |
|------------|----------|-----------|
| ```\|```   | /        | Trennzeichen zwischen den Daten                      |
| ```lsp```  | float    | Lightspeed, vom ESP32 |
| ```len```  | int      | Light-State (Sagt ob die Lichter an oder aus sein sollen) |
| ```eff```  | int      | **WIP** [Lichteffekt-ID](docs/id-definitions.md#gamestate-id) |

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
