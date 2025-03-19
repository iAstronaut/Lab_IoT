#include <Arduino.h>
#include "wifi.h"

void TaskWiFi(void *pvParameters) {
    // Setup WiFi
    setup_wifi();

    while (1) {
        // Check WiFi connection status
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi connection lost. Reconnecting...");
            WiFi.reconnect();
        }

        // Delay for a short time to prevent watchdog issues
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}