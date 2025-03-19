#include <Arduino.h>
#include "mqtt.h"

void TaskMQTT(void *pvParameters) {
    // Setup MQTT
    setup_mqtt();

    while (1) {
        // Maintain MQTT connection
        if (!client.connected()) {
            Serial.println("MQTT connection lost. Reconnecting...");
            setup_mqtt();
        }

        // Process MQTT messages
        client.loop();

        // Small delay to prevent watchdog issues
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}