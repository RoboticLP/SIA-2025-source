# Pin Belegung
Hier werden alle Pin-Belegungen für die Nano und Mega Arduinos festgelegt.

## Navigation
> **Seiten**
- [Hauptseite](../README.md)
- [ESP32 Webserver](ESP32-Webserver.md)
- [Message IDs](error-codes.md)

> **Auf dieser Seite**
- [Mega](#mega)
- [Nano 2](#nano-2)
- [Nano 3](#nano-3)
- [Nano 4](#nano-4)
- [Nano 5](#nano-5)
---


## Interruptpins
> Information zu Interrupts auf der [Arduino Website](https://docs.arduino.cc/language-reference/funktionen/external-interrupts/attachInterrupt)

| Mega | Nanos |
|----------|----------|
| 2, 3, 18, 19, 20, 21   | 2, 3 |

## Datenkabel
| Nutzung | Mega | Nanos |
|----------|----------|----------|
| Data (SDA) | SDA | A4 |
| Clock (clk) | SCL | A5 |

## Allgeine benutzte Pins
> Wir verwenden für den Mega und alle Nanos die Pins **GND** und **VIN** (Volt in) Pin, sowie die Pins **A4** und **A5**, die für die Daten und die Clock stehen. [Informtionen](#datenkabel)

# Pin Belegung der einzelnen Arduinos
## Mega
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-mega2560-rev3)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 2 | Taster Finger 1 lesen | Nein | Ja | Taster | Digital | Input |
| ? | Flipper Finger 1 ausgeben | Ja | Nein | Spule | Digital | Output |
| 3 | Taster Finger 2 lesen | Nein | Ja | Taster | Digital | Input |
| ? | Flipper Finger 2 ausgeben | Ja | Nein | Spule | Digital | Output |
| 5 | Ball entlass in die Startvorrichtung | Nein | Nein | Servo | PWM |

## Nano 2
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 4 | Sensor für Ball Eject | Nein | Nein | Lichtschranke ? | Digital | Input |
| 5 | Spule für Ball Eject | Nein | Nein | Spule | Digital | Output |

## Nano 3
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

NICHT SICHER
| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 2 | Bumper Tower (3 Stück) | Ja | Ja | Spule | Digital | Input |
| 3 | Sensor für obere Rollbahn | Nein | Ja | Lichtschranke ? | Digital | Input |

## Nano 4
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)


| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| ? | Metallsensor1? | Nein | Nein | Sensor | Digital | Input |
| ? | Metallsensor2? | Nein | Nein | Sensor | Digital | Input |
| ? | Metallsensor3? | Nein | Nein | Sensor | Digital | Input |
| ? | LED1 | Nein | Nein | Ausgabe | Digital | Output |
| ? | LED2 | Nein | Nein | Ausgabe | Digital | Output |
| ? | LED3 | Nein | Nein | Ausgabe | Digital | Output |

## Nano 5
### Nano 5 ist für weitere Aktionen in der Zukunft geplant und wird im Falle vom Gebraucht genutzt.
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|

