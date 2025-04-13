#ifndef OTA_TASK_H
#define OTA_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// OTA parameters
extern const char* OTA_API_KEY;
extern const char* FIRMWARE_VERSION;
extern const char* DEVICE_ID;
extern const uint32_t OTA_CHECK_INTERVAL;

// Task handle
extern TaskHandle_t otaTaskHandle;

// Function declarations
void otaTask(void * parameter);
bool checkForUpdates();
bool handleOTAUpdate(const char* firmware_url, int expectedSize);

#endif // OTA_TASK_H