# SIA-2025-source
Alle Source-Codes der diesjährigen SIA.

## Navigation
> **Seiten**
- [ESP32 Webserver](docs/ESP32-Webserver.md)
- [Pin-Belegung](docs/pins.md)
- [Error Codes](docs/error-codes.md)

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
| 5        | Arduino Nano | Extra Nano |
| 6        | ESP32        | WebServer       |

---

#### Datenformat des Daten-Strings der Module (Nano's)
Jedes Modul (Arduino Nano) hat bis zu zwei Ziele (Targets) an sich angeschlossen, daher ```ht1``` und ```ht2``` für die Trefferanzahl von jedem Ziel seit der letzten Update-Anfrage.
Jedes Modul wird mehrmals pro Sekunde nach Updates gefragt, hier definieren wir wie die Antworten Formatiert sein müssen:

```c
ht1:%d|ht2:%d|err
```
> *Die **Reihenfolge der Keys+Werte ist egal**, genau wie ihre **Vorhandenheit** - es werden nur erhaltene Daten verarbeitet und es sind keine speziellen Daten notwendig. Allerdings **muss** jeder versendete Key einen zugehörigen Wert haben*

|  Key-Name  | Wertetyp | Bedeutung                                            |
|------------|----------|------------------------------------------------------|
| ```\|```   | /        | Trennzeichen zwischen den Daten                      |
| ```ht1```  | int      | Zahl der Treffer (Hits) des **ersten** Ziels (Targets) seit dem letztem Update      |
| ```ht2```  | int      | Zahl der Treffer (Hits) des **zweiten** Ziels (Targets) seit dem letztem Update      |
| ```err```  | /        | Wird nur gesendet, **wenn** das Modul eine Fehlermeldung hat > dann kann die Fehlermeldung seperat direkt danach abegefragt werden(vor verarbeitung anderer slaves), damit normale Abfragen klein und schnell bleiben können |

> Da es von Arduino selbst keine eigene Lösung für das Splitten von Strings hat benutzen wir folgende eigene Lösung: [splitString](#splitstring)

---

## Datenformat des Admin-Panel Datenstrings (vom ESP32)
#### Idee
- seltener Abfragen **ob** er ein Update parat hat, und erst dann das Update abfragen.
- Error Meldungen nur alle Paar Sekunden schicken (und codiert? also *Zahl* bedeutet bestimmter Error, der erstm in der UI formuliert wird)

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
