# ESP32 Webserver
Die versendeten Daten und ihre Bedeutung.
Die ```test.html``` Datei ist nur zum lokalen Ausprobieren gedacht. Ihr Code wird später immer in die Variable in ```index.h``` eingefügt.

## Navigation
> **Seiten**
> [Hauptseite](../README.md)

> **Auf dieser Seite**
> [Methoden](#dokumentation-der-methoden)
> [Code Snippets](#code-snippets)
---

## WiFi Access Point
Sobald der ESP32 hochfährt und der WLAN-Modus nicht aktiv ist eröffnet er einen Access Point. Mit diesem können sich **maximal 4 Geräte** verbinden. Alle danach folgenden Geräte werden in eine Warteschlange gesetzt. Das Adminpanel ist immer erreichbar unter der lokalen IP  ```192.168.1.1``` im Browser des mit dem ESP32 verbunden Geräts. Der WLAN Modus ist nur aktiv wenn die ```wifi_credentials.h```-Datei mit den richtigen Variablen vorhanden ist.

## Funktionsweise der Buttons
Beim betätigen einer der Buttons wird eine PUT-Anfrage an die zugeordnete Endstelle gesendet (z.B. ```/BUTTON_0```). Wenn diese verständigt wird erkennt das der Server und schält in diesem Fall eine interne Variable um, die danach in digitalWrite für die LED verwendet wird. Diese Variable wird auch sofort mit der Methode ```broadcastSSE_update()``` an alle verbundenen Clients per SSE gesendet, damit alle auf dem gleichen Stand sind.

## Server-sent events (Aktualisierung der Client-Daten)

*erklärung folgt*

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
##### Toggle
```html
<div class="toggle-container">
  <div id="led-toggle" class="toggle" onclick="toggleSetting(this); drueckMethodeHier()">
        <div class="toggle-thumb"></div>
    </div>
</div>
```

Beispielmethode für Drück-Methode:
```javascript
function handleButtonPress0() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("PUT", "BUTTON_0", false);
    xhttp.send();
}
```

#### Slider
ist iwie komplizierterer shit

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