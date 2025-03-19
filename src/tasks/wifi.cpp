#include "wifi.h"
#include <WiFi.h>

const char* ssid = "TECHNEST-5G";   // SSID của Wi-Fi
const char* password = "Technest2025"; // Password của Wi-Fi

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
}
