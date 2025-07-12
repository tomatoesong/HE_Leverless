#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

const char* ssid = "HE_Leverless-AP";
const char* password = "streetfighter6";

// Static IP configuration for Access Point
IPAddress local_ip(192, 168, 10, 1);  // ESP32 AP IP
IPAddress gateway(192, 168, 10, 1);   // Usually same as local_ip in AP mode
IPAddress subnet(255, 255, 255, 0);   // Standard subnet mask

WebServer server(80);

//define authenication
const char* base64Encoding = "c29uZ3RvbTk2Ojk2MDEwNQ==";  // base64encoding user:pass // songtom96:960105

const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 AP Button</title>
</head>
<body>
  <h2>ESP32 Access Point with Static IP</h2>
  <button onclick="buttonClicked()">Click Me</button>

  <script>
    function buttonClicked() {
      fetch('/button').then(res => {
        if (res.ok) alert("ESP32 received the click!");
      });
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html);
}

void handleButton() {
  Serial.println("Button clicked!");
  server.send(200, "text/plain", "OK");
}

void accessPointInit() {
  WiFi.softAPConfig(local_ip, gateway, subnet);

  // Start the access point
  WiFi.softAP(ssid, password);

  // Serial.println("Access Point Started");
  // Serial.print("Static IP Address: ");
  // Serial.println(WiFi.softAPIP());  // Should print: 192.168.10.1

  server.on("/", handleRoot);
  server.on("/button", handleButton);
  server.begin();
  rgbLedWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);
}

void wifiProtocol() {
  server.handleClient();
}