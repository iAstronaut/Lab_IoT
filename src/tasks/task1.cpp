#include <Arduino.h>
#include "task1.h"

void Task1(void *pvParameters) {
    while (1) {
        Serial.println("Hello from Task1");
        vTaskDelay(pdMS_TO_TICKS(1000));  // Delay 1000ms
    }
}
