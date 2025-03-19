#include <Arduino.h>
#include "task2.h"

void Task2(void *pvParameters) {
    while (1) {
        Serial.println("Hello from Task2");
        vTaskDelay(pdMS_TO_TICKS(1500));  // Delay 1500ms
    }
}
