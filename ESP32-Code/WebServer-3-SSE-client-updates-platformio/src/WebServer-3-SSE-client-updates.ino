#include <WiFi.h>
#include <WebServer.h>
#include "index.h"

// credentials for wifi
#define WiFi_SSID ""
#define WiFi_PASSWORD ""

#if __has_include("wifi_credentials.h")
  // #include "wifi_credentials.h"
#endif


// credentials for the access point
#define AP_SSID "NeuerESP32"
#define AP_PASSWORD "12345678"


// Defining PINS
#define led0 2
#define led1 14

// Variables
bool useWiFi = false; // easier for debugging if on, if not it will create a AP
bool LED0 = false;
int LED1_br = 0;

// XML array size needs to be bigger than the expected size
char XML[150];

// create the actual webserver
WebServer server(80);

// WiFi settings for the local access point
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress Actual_IP;

// SSE Clients
int maxSSEClients = 4;
WiFiClient sseClients[4];
bool sseClientsConnected[4] = {false, false, false, false};

void setup() {
  Serial.begin(115200);

  // Setting pin modes
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);

  // LED0 = false;
  // digitalWrite(led, LED0);

  if (WiFi_SSID != "" && WiFi_PASSWORD != "") {
    WiFi.begin(WiFi_SSID, WiFi_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("Connected! IP address: "); Serial.println(WiFi.localIP());
    Actual_IP = WiFi.localIP();
  } else {
    // start access point and print out the IP address
    WiFi.softAPConfig(PageIP, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Actual_IP = WiFi.softAPIP();
    Serial.print("IP address: "); Serial.println(Actual_IP);
  }

  // once someone opens main address
  server.on("/", sendWebsite);

  // where website can request new data for refreshing
  // server.on("/xml", sendXML);

  // SSE connection
  server.on("/sse", handleSSEConnect);

  // add server listeners
  server.on("/BUTTON_0", handleButtonPress0);
  server.on("/BRIGHTNESS_SLIDER", handleBrightnessSlider);

  server.begin();
}

void loop() {
  // needs to be called every frame
  server.handleClient();

  handleSSEClients();
}

void handleButtonPress0() {
  LED0 = !LED0;

  digitalWrite(led0, LED0);
  server.send(200, "text/plain", ""); // send something so the esp won't think its dying and therefor won't reboot
  broadcastSSE_update();
}

void handleBrightnessSlider() {
  String t_state = server.arg("value");
  LED1_br = t_state.toInt();
  analogWrite(led1, LED1_br);
  server.send(200, "text/plain", "");
  broadcastSSE_update();
}

// send the html website to the client once they request it by opening xxx.xxx.xx.xx/
void sendWebsite() {
  server.send(200, "text/html", webpage_main);
}

void handleSSEConnect() {
  Serial.println("SSE connect request recieved");
  WiFiClient client = server.client();

  int slot = -1;
  for (int i = 0; i < maxSSEClients; i++) {
    if (sseClientsConnected[i] == false) {
      slot = i;
      Serial.print("it's slot #"); Serial.println(slot);
      break;
    }
  }

  if (slot == -1) {
    Serial.println("Max sse clients reached, rejecting connection");
    client.println("HTTP/1.1 503 Service Unavailable");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println("");

    return;
  }

  sseClientsConnected[slot] = true;
  sseClients[slot] = client;

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/event-stream");
  client.println("Cache-Control: no-cache");
  client.println("Connection: keep-alive");
  client.println("");
  client.flush();

  client.print("keepalive\n\n");
  client.flush();
  Serial.println("SSE answer sent. Sending first update packet");
  broadcastSSE_update(); // first update to set all data to current
  broadcastSSE_log("info", "Ein neuer Client hat sich verbunden", millis()); // nur testweise hier
}

void broadcastSSE_update() {
  String xmlData = "<?xml version='1.0'?><Data>";
  xmlData += "<B0>" + String(LED0 ? "1" : "0") + "</B0>";
  xmlData += "<SL_V>" + String(LED1_br) + "</SL_V>";
  xmlData += "</Data>";

  for (int i = 0; i < maxSSEClients; i++) {
    if (sseClientsConnected[i] && sseClients[i].connected()) {
      // SSE Format: "data: INHALT\n\n"
      sseClients[i].print("data: ");
      sseClients[i].print(xmlData);
      sseClients[i].print("\n\n");
      // sseClients[i].flush();
    }
  }
}

//** sendet log an alle aktuell verbundenen clients
//** logType: String - der Typ ("error" oder "info")
//** logMessage: String - die zu versendende Nachricht (max 95 Zeichen)
//** logTimestamp: int - der Zeitpunkt des ereignisses, der auf dem Adminpanel angezeigt wird (max 7 Stellen)
void broadcastSSE_log(String logType, String logMessage, int logTimestamp) {
  String xmlData = "<?xml version='1.0'?><Data><log>";
  xmlData += "<logType>" + logType + "</logType>";
  xmlData += "<logMessage>" + logMessage + "</logMessage>";
  xmlData += "<logTimestamp>" + String(logTimestamp) + "</logTimestamp>";
  xmlData += "</log></Data>";

  // send data to all clients
  for (int i = 0; i < maxSSEClients; i++) {
    if (sseClientsConnected[i] && sseClients[i].connected()) {
      // SSE Format: "data: INHALT\n\n"
      sseClients[i].print("data: ");
      sseClients[i].print(xmlData);
      sseClients[i].print("\n\n");
      sseClients[i].flush();
    }
  }
}

// runs every frame
unsigned long lastKeepAlive = 0;
void handleSSEClients() {
  for (int i = 0; i < maxSSEClients; i++) {
    if (sseClientsConnected[i] == true && sseClients[i].connected()) {
      if (millis() - lastKeepAlive > 30000) {
        WiFiClient client = sseClients[i];

        client.print("keepalive\n\n");
        client.flush();
        Serial.println("keepalive sent");
        lastKeepAlive = millis();
      }
    } else if (sseClientsConnected[i] == true && sseClients[i].connected() != true) {
      Serial.print("SSE connection lost for client #"); Serial.println(i);
      sseClientsConnected[i] = false;
      sseClients[i].stop();
    }
  }
}