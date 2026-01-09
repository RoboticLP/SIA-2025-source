# LCD Display Pin-Belegung

## Navigation
> **Seiten**
- [Hauptseite](../README.md)
- [Pin-Belegung](pins.md)
- [Message IDs](error-codes.md)

> **Auf dieser Seite**
- [Methoden](#dokumentation-der-methoden)
- [Code Snippets](#code-snippets)
---

## Übersicht
```
Pin-Nummern auf dem Display:
1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16
o  o  o  o  o  o  o  o  o  o  o  o  o  o  o  o
─────────────────────────────────────────────────
|                                               |
|                                               |
|                   Display                     |
|                                               |
|                                               |
|                                               |
─────────────────────────────────────────────────
```

## Pin-Konfiguration

| LCD Pin | Funktion | Arduino Mega | Beschreibung |
|---------|----------|--------------|--------------|
| 1       | VSS      | GND          | Ground |
| 2       | VDD      | 5V           | Stromversorgung |
| 3       | V0       | Pin 6        | Kontrast (über AdminPanel) |
| 4       | RS       | Pin 7        | Register Select (Data/Command) |
| 5       | RW       | GND          | Read/Write (auf GND = nur Schreiben) |
| 6       | E        | Pin 8        | Enable Signal |
| 7       | D0       | -            | Nicht verwendet (4-Bit Modus) |
| 8       | D1       | -            | Nicht verwendet (4-Bit Modus) |
| 9       | D2       | -            | Nicht verwendet (4-Bit Modus) |
| 10      | D3       | -            | Nicht verwendet (4-Bit Modus) |
| 11      | D4       | Pin 9        | Datenleitung |
| 12      | D5       | Pin 10       | Datenleitung |
| 13      | D6       | Pin 11       | Datenleitung |
| 14      | D7       | Pin 12       | Datenleitung |
| 15      | A        | 5V           | Hintergrundbeleuchtung + |
| 16      | K        | GND          | Hintergrundbeleuchtung - |

## Code-Konfiguration
```cpp
#include <LiquidCrystal.h>

// LCD Pin-Konfiguration: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
    lcd.begin(16, 2);  // 16 Zeichen, 2 Zeilen
    lcd.clear();
}
```

## Hinweise

- **4-Bit Modus**: Es werden nur die Datenleitungen D4-D7 verwendet (Pins 11-14 am LCD)
- **RW auf GND**: Das Display wird nur zum Schreiben verwendet, nicht zum Lesen
- **Kontrast**: V0 wird über einen Potentiometer an Pin 6 angeschlossen
- **Hintergrundbeleuchtung**: Pins 15 und 16 steuern die LED-Hintergrundbeleuchtung
