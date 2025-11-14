# SIA-2025-source
Alle Source-Codes der diesjährigen SIA.

## Resourcen
- [I2C Arduino Docs](https://docs.arduino.cc/learn/communication/wire/#arduino-i2c-pins)
- [SIA-Playlist YouTube von Adrian](https://youtube.com/playlist?list=PLDd5gBML2x-RDGElEF7yUXcqWyJdSLfL-&si=K51pTD4f3ktHvXSp)
- [sprintf (Formatierung von Strings)](#sprintf)

## I<sup>2</sup>C Infos
| Adressen | Gerät        | Aufgabe / Modul |
|----------|--------------|-----------------|
| (1)      | Arduino Mega | Master          |
| 2        | Arduino Nano |  |
| 3        | Arduino Nano |  |
| 4        | Arduino Nano |  |
| 5        | Arduino Nano |  |

#### Datenformat des Daten-Strings der Module (Nano's)
Jedes Modul (Arduino Nano) hat bis zu zwei Ziele (Targets) an sich angeschlossen, daher ```ht1``` und ```ht2``` für die Trefferanzahl von jedem Ziel seit der letzten Update-Anfrage.
Jedes Modul wird mehrmals pro Sekunde nach Updates gefragt, hier definieren wir wie die Antworten Formatiert sein müssen:

```c
ht1:%d|ht2:%d|err
```
*Die Reihenfolge der Keys+Werte ist egal*
|  Key-Name  | Wertetyp | Bedeutung                                            |
|------------|----------|------------------------------------------------------|
| ```\|```   | /        | Trennzeichen zwischen den Daten                      |
| ```ht1```  | int      | Zahl der Treffer (Hits) des **ersten** Ziels (Targets) seit dem letztem Update      |
| ```ht2```  | int      | Zahl der Treffer (Hits) des **zweiten** Ziels (Targets) seit dem letztem Update      |
| ```err```  | /        | Wird nur gesendet, **wenn** das Modul eine Fehlermeldung hat > dann kann die Fehlermeldung seperat abegefragt werden, damit normale Abfragen klein und schnell bleiben können |

## Datenformat des Admin-Panel Datenstrings (vom ESP32)
#### Idee
- seltener Abfragen **ob** er ein Update parat hat, und erst dann das Update abfragen.
- Error Meldungen nur alle Paar Sekunden schicken (und codiert? also *Zahl* bedeutet bestimmter Error, der erstm in der UI formuliert wird)

## sprintf (Formatierung von Strings)
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