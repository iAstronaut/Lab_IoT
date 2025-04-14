#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "../schedule/schedule.h"

// MQTT Topics
#define TOPIC_TELEMETRY "v1/devices/me/telemetry"
#define TOPIC_ATTRIBUTES_REQUEST "v1/devices/me/attributes/request/1"
#define TOPIC_ATTRIBUTES_RESPONSE "v1/devices/me/attributes/response/+"
#define TOPIC_RPC_REQUEST "v1/devices/me/rpc/request/+"
#define TOPIC_RPC_RESPONSE "v1/devices/me/rpc/response/"

// MQTT Configuration
#define MQTT_SERVER "demo.thingsboard.io"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32_Client"
#define MQTT_ACCESS_TOKEN "YOUR_ACCESS_TOKEN"

// Schedule structure
struct Schedule {
    time_t off_time;
    time_t on_time;
    int schedule_on;
    int schedule_off;
};

class MQTTClient {
public:
    MQTTClient();
    void begin();
    bool connect();
    void reconnect();
    void loop();
    bool isConnected();
    void setPowerState(bool state);
    bool getPowerState() const;
    void updateSchedule(const Schedule& newSchedule);
    Schedule getSchedule() const;

private:
    void callback(char* topic, uint8_t* payload, unsigned int length);
    void handleRPC(const char* requestId, const char* message);
    void handleAttributes(const char* message);
    void generateSensorData();
    void sendTelemetryData(bool forceSend = false);

    WiFiClient espClient;
    PubSubClient mqttClient;
    unsigned long last_data_send_time;
    float sim_temperature;
    float sim_humidity;
    int connectionAttempts;
    ScheduleManager scheduleManager;
    bool power_state;
    Schedule schedule;
};

#endif