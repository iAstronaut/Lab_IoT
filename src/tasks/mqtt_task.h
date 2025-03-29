#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../mqtt/mqtt_client.h"

class MQTTTask {
public:
    static void start();
    static void stop();

private:
    static void taskFunction(void* parameter);
    static TaskHandle_t taskHandle;
    static MQTTClient* mqttClient;
    static const uint32_t STACK_SIZE = 8192;
};

#endif