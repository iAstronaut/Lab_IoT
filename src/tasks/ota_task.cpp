#include "ota_task.h"

TaskHandle_t OTATask::taskHandle = NULL;
OTAClient* OTATask::otaClient = NULL;

void OTATask::start(const char* apiKey, const char* productId) {
    if (taskHandle != NULL) {
        return; // Task already running
    }

    otaClient = new OTAClient(apiKey, productId);
    otaClient->begin();

    xTaskCreate(
        taskFunction,
        "OTA Task",
        4096,
        NULL,
        1,
        &taskHandle
    );
}

void OTATask::stop() {
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }
    if (otaClient != NULL) {
        delete otaClient;
        otaClient = NULL;
    }
}

void OTATask::taskFunction(void* parameter) {
    while (true) {
        if (otaClient != NULL) {
            if (otaClient->checkForUpdates()) {
                Serial.println("New firmware available, starting update...");
                if (otaClient->performUpdate()) {
                    Serial.println("Update completed successfully");
                } else {
                    Serial.println("Update failed");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL));
    }
}