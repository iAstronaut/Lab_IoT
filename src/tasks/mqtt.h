#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>

extern const char* mqtt_server;
extern const char* mqtt_user;
extern const int mqtt_port;

extern WiFiClient espClient;
extern PubSubClient client;

void setup_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void send_telemetry(float temperature, float humidity, float latitude, float longitude);
void TaskMQTT(void *pvParameters);

#endif
