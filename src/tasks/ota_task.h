#ifndef OTA_TASK_H
#define OTA_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../ota/ota_client.h"

class OTATask {
public:
    static void start(const char* apiKey, const char* productId);
    static void stop();

private:
    static void taskFunction(void* parameter);
    static TaskHandle_t taskHandle;
    static OTAClient* otaClient;
    static const uint32_t CHECK_INTERVAL = 300000; // 5 minutes
};

#endif