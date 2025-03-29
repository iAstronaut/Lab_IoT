#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>
#include "../schedule/schedule.h"
#include "../tasks/schedule_task.h"

// MQTT Topics
#define TOPIC_TELEMETRY "v1/devices/me/telemetry"
#define TOPIC_RPC_REQUEST "v1/devices/me/rpc/request/+"
#define TOPIC_RPC_RESPONSE "v1/devices/me/rpc/response/"
#define TOPIC_ATTRIBUTES "v1/devices/me/attributes"
#define TOPIC_ATTRIBUTES_REQUEST "v1/devices/me/attributes/request/1"
#define TOPIC_ATTRIBUTES_RESPONSE "v1/devices/me/attributes/response/1"

// MQTT Configuration
#define MQTT_SERVER "app.coreiot.io"
#define MQTT_PORT 1883
#define MQTT_ACCESS_TOKEN "cShE3QTvAkrBcgOi58rJ"
#define MQTT_CLIENT_ID "esp32_device"

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
    void loop();
    bool isConnected();
    void setPowerState(bool state);
    bool getPowerState() const;
    void updateSchedule(const Schedule& newSchedule);
    Schedule getSchedule() const;

private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    ScheduleManager scheduleManager;
    bool power_state;
    Schedule schedule;
    unsigned long last_data_send_time;
    float sim_temperature;
    float sim_humidity;

    void connect();
    void reconnect();
    void callback(char* topic, byte* payload, unsigned int length);
    void handleRPC(const char* requestId, const char* message);
    void handleAttributes(const char* message);
    void generateSensorData();
    void sendTelemetryData(bool forceSend = false);
    void checkSchedule();
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

#endif