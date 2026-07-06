# Pin Belegung
Hier werden alle Pin-Belegungen für die Nano und Mega Arduinos festgelegt.

**ALLE PINS WERDEN BEI Dx ANGESCHLOSSEN ALSO BEI EINEM D PIN**

## Navigation
> **Seiten**
- [Hauptseite](../README.md)
- [ESP32 Webserver](ESP32-Webserver.md)
- [ID-Definitions](id-definitions.md)

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
| A3 | Ball In Start Knopf | Nein | Nein | Button | Analog | Input_Pullup |
| A4 | Ball Lost Schranke | Nein | Nein | Digitalschranke | Analog | Input_Pullup |
| A5 | Enable/Disable Fingers While InGame | Nein | Nein | Finger-Ausgang | Analog | Output |
| A6 | Background-Light PWM | Nein | Nein | LED-Hintergrundbeleuchtung | Analog | Output |
| A13 | Ball entlass in die Startvorrichtung | Ja | Nein | Spule | Digital | Output |

## Nano 2
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 2 | Slingshot Sensor | Nein | Nein | Taster | Digital | Input_Pullup |

Ball Eject ist akutell ausgenommen.

## Nano 3
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 2 | Bumper Tower Sensor | Nein | Ja | Taster | Digital | Input_Pullup |

## Nano 4
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 2 | Bumper/Trigger Sensor | Nein | Ja | Taster/Schalter | Digital | Input_Pullup |

## Nano 5
### aktuell keine Verwendung
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/arduino-nano)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|


## ESP 2
### ESP 2 ist für die Lichter zuständig
> Information zur Pin verarbeitung [hier](https://devboards.info/boards/esp32)

| Pins | Nutzung | Relais | Interrupt | Bauteil | Signal | PinMode |
|----------|----------|----------|----------|----------|----------|----------|
| 23 | Datenoutput | Nein | Nein | WS2812B Lichtstreifen DIN | Digital | Output |
