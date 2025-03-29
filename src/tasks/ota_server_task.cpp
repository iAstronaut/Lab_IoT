#include <Arduino.h>
#include "ota_server_task.h"
#include "../server/ota_server.h"

OTAServer otaServer;

void TaskOTAServer(void *pvParameters) {
    otaServer.begin();

    while (1) {
        otaServer.handleLoop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}