# ESP32 Webserver
Die versendeten Daten und ihre Bedeutung.
Die ```index.html``` Datei ist nur zum lokalen Ausprobieren gedacht. Ihr Code wird später immer in die Variable in ```index.h``` eingefügt.
Zur Programmierung des ESP32 der Hochschule müsst ihr einen [Treiber](https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads) installieren.

## Navigation
> **Seiten**
- [Hauptseite](../README.md)
- [Pin-Belegung](pins.md)
- [Message IDs](error-codes.md)

> **Auf dieser Seite**
- [Methoden](#dokumentation-der-methoden)
- [Code Snippets](#code-snippets)
---

## WiFi Access Point
Sobald der ESP32 hochfährt und der WLAN-Modus nicht aktiv ist eröffnet er einen Access Point. Mit diesem können sich **maximal 4 Geräte** verbinden. Alle danach folgenden Geräte werden in eine Warteschlange gesetzt. Das Adminpanel ist immer erreichbar unter der lokalen IP  ```192.168.1.1``` im Browser des mit dem ESP32 verbunden Geräts. Der WLAN Modus ist nur aktiv wenn die ```wifi_credentials.h```-Datei mit den richtigen Variablen vorhanden ist.

## Funktionsweise der Buttons
Beim betätigen einer der Buttons wird eine PUT-Anfrage an die zugeordnete Endstelle gesendet (z.B. ```/BUTTON_0```). Wenn diese verständigt wird erkennt das der Server und schält in diesem Fall eine interne Variable um, die danach in digitalWrite für die LED verwendet wird. Diese Variable wird auch sofort mit der Methode ```broadcastSSE_update()``` an alle verbundenen Clients per SSE gesendet, damit alle auf dem gleichen Stand sind.

## Server-sent events (Aktualisierung der Client-Daten)
Die Clients verbinden sich nach laden der Website mit der Adresse /sse, der die SSE-Schnittstelle des Servers ist. SSE (Server-sent events) bedeutet, dass die Clients nicht aktiv den Server nach neuen Daten abfragen müssen, sondern dass der Server jegliche Daten sofort an die Clients senden kann, wann immer er will. Das verringert den Datenaustausch zwischen Client und Server auf das nötigste und macht die Daten-Updates der Clients fast sofortig, da nicht in einem bestimmten Takt abgefragt werden muss.

---

## Dokumentation der Methoden
### Server
- ```Arduino
    broadcastSSE_update()
Sendet alle in ihr definierten Daten an alle der verbundenen Clients zurück (z.B. Toggle states, Slider values, etc...)
- ```Arduino
    broadcastSSE_log(String logType, String logMessage, int logTimestamp)
Sendet eine log-Mitteilung an alle verbundenen Clients.
```logType``` "error" oder "info"
```logMessage``` die zu versendende Nachricht
```logTimestamp``` der Zeitpunkt von dem zu berichtenden Ereigniss (auf Arduinos z.B. durch ```millis()``` zu bekommen) 

### Client / Website
- ```javascript
    addLog(type, message, timestamp = new Date().toLocaleTimeString('de-DE'))
Fügt eine neue Log hinzu
```logType``` "error" oder "info" ODER ```logType.info``` oder ```logtype.error```
```logMessage``` die anzuzeigende Nachricht
```logTimestamp``` der Zeitpunkt von dem zu berichtenden Ereigniss (bei leer lassen wird aktuelle Zeit genommen)

#### Code Snippets
##### Section
```html
<div class="section-container">
    <div class="section-title">Beispiel Titel</div> <!-- Titel des Abschnitts -->
    
    <div class="section-subtitle">Beispiel Unterabschnitttitel</div> <!-- Titel eines Unterabschnitts -->
</div>
```

##### 2 Spalten in section-container
```html
<div class="controls-grid">
    ...
</div>
```

##### Toggle
```html
<div class="toggle-container">
    <div id="beispiel-toggle" class="toggle" onclick="toggleSetting(this);">
        <div class="toggle-thumb"></div>
    </div>
    Toggle text
</div>
```

Beispielmethode für Drück-Methode:
```javascript
function handleButtonPress0() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("PUT", "BUTTON_0", true);
    xhttp.send();
}
```

#### Slider
```html
<input type="range" id="brightness-slider" min="0" max="255" value="0" ininput="handleSliderInput(this.value)"\>
```
Die Methode ```handleSliderInput``` wird bei jeder veränderung des Sliders ausgeführt. Diese passt die Größe des farbigen Hintergrunds an, der den Wert des Sliders deutlicher macht. Zusätzlich wird alle 100ms der Wert des Sliders an den Server geschickt, solange dieser Verschoben wird. Der Server hat eine seperate Schnittstelle für den Slider, bei dem als URL-Parameter der Slider-Wert erwartet wird.

Beispielmethoden für Slider-Verarbeitung:
```javascript
let sliderTimeout;
function handleSliderInput(value) {
    requestAnimationFrame(() => updateSliderProgress()); // aktualisiert Größe des Hintergrunds (zur Vereinfachung hier weggelassen)
    clearTimeout(sliderTimeout);
    sliderTimeout = setTimeout(() => {
        handleUpdateSlider(value); // sendet Daten an Server mit "PUT" "BRIGHTNESS_SLIDER?value="+value
    }, 100);
}
```

#### Buttons
##### Primary Button Klasse (Primär)
```html
<button class="btn btn-primary" onclick="drueckMethodeHier()">KnopfTextHier</button>
```

##### Secondary Button Klasse (Sekundär)
```html
<button class="btn btn-secondary" onclick="drueckMethodeHier()">KnopfTextHier</button>
```

##### Danger Button Klasse (Gefahr)
```html
<button class="btn btn-danger" onclick="drueckMethodeHier()">KnopfTextHier</button>
```

##### Buttons verkleinern
```btn-small``` bei jedem anderen Button nutzen.
```html
<button class="btn btn-danger btn-small" onclick="drueckMethodeHier()">KnopfTextHier</button>
```
