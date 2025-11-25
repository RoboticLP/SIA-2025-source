#include <WiFi.h>
#include <WebServer.h>
#include "index.h"

// credentials for wifi
#define WiFi_SSID ""
#define WiFi_PASSWORD ""

#if __has_include("wifi_credentials.h")
  #include "wifi_credentials.h"
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
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Actual_IP = WiFi.softAPIP();
    WiFi.softAPConfig(PageIP, gateway, subnet);
    Serial.print("IP address: "); Serial.println(Actual_IP);
  }

  // once someone opens main address
  server.on("/", sendWebsite);

  // where website can request new data for refreshing
  server.on("/xml", sendXML);

  // add server listeners
  server.on("/BUTTON_0", handleButtonPress0);
  server.on("/BRIGHTNESS_SLIDER", handleBrightnessSlider);

  server.begin();
}

void loop() {
  // needs to be called every frame
  server.handleClient();
}

void handleButtonPress0() {
  LED0 = !LED0;

  digitalWrite(led0, LED0);
  server.send(200, "text/plain", ""); // send something so the esp won't think its dying and therefor won't reboot
}

void handleBrightnessSlider() {
  String t_state = server.arg("value");
  LED1_br = t_state.toInt();
  analogWrite(led1, LED1_br);
  server.send(200, "text/plain", "");
}

// send the html website to the client once they request it by opening xxx.xxx.xx.xx/
void sendWebsite() {
  server.send(200, "text/html", webpage_main);
}

// sends data to client to update its data
void sendXML() {
  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");
  // send Button0 state
  if (LED0) {
    strcat(XML, "<B0>1</B0>\n"); // send that LED is on
  } else {
    strcat(XML, "<B0>0</B0>\n"); // send that LED if off
  }

  // send slider value
  char sl_v_buffer[20]; // is big enough for string below (including number up to 3 digits)
  sprintf(sl_v_buffer, "<SL_V>%d</SL_V>\n", LED1_br);
  strcat(XML, sl_v_buffer);

  strcat(XML, "</Data>\n");
  server.send(200, "text/xml", XML);
}