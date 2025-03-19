#include "mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* mqtt_server = "app.coreiot.io";
const char* mqtt_user = "cShE3QTvAkrBcgOi58rJ";  // Device token
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_mqtt() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_user, "")) {
      Serial.println("Connected to MQTT");
      // Subscribe to telemetry topic like in Python
      client.subscribe("v1/devices/me/telemetry");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Create a null-terminated string from payload
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Received message: ");
  Serial.println(message);
}

void send_telemetry(float temperature, float humidity, float latitude, float longitude) {
  StaticJsonDocument<200> doc;
  // Round values to 2 decimal places like in Python
  doc["temperature"] = round(temperature * 100.0) / 100.0;
  doc["humidity"] = round(humidity * 100.0) / 100.0;
  doc["latitude"] = round(latitude * 100.0) / 100.0;
  doc["longitude"] = round(longitude * 100.0) / 100.0;

  char buffer[256];
  serializeJson(doc, buffer);

  // Print sent data like in Python
  Serial.print("Sent data: ");
  Serial.println(buffer);

  client.publish("v1/devices/me/telemetry", buffer);
}
