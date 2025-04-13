#include "schedule_task.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

TaskHandle_t ScheduleTask::taskHandle = NULL;
ScheduleManager* ScheduleTask::scheduleManager = NULL;
bool ScheduleTask::power_state = true;
SemaphoreHandle_t ScheduleTask::powerStateMutex = NULL;

bool ScheduleTask::start() {
    if (taskHandle != NULL) {
        Serial.println("Schedule task already running");
        return false;
    }

    // Create mutex for power state synchronization
    powerStateMutex = xSemaphoreCreateMutex();
    if (powerStateMutex == NULL) {
        Serial.println("Failed to create power state mutex");
        return false;
    }

    // Create schedule manager
    scheduleManager = new ScheduleManager();
    if (scheduleManager == NULL) {
        Serial.println("Failed to create schedule manager");
        vSemaphoreDelete(powerStateMutex);
        return false;
    }

    if (!scheduleManager->begin()) {
        Serial.println("Failed to initialize schedule manager");
        delete scheduleManager;
        scheduleManager = NULL;
        vSemaphoreDelete(powerStateMutex);
        return false;
    }

    // Create task
    BaseType_t xReturned = xTaskCreate(
        taskFunction,
        "Schedule Task",
        STACK_SIZE,
        NULL,
        1,
        &taskHandle
    );

    if (xReturned != pdPASS) {
        Serial.println("Failed to create schedule task");
        delete scheduleManager;
        scheduleManager = NULL;
        vSemaphoreDelete(powerStateMutex);
        return false;
    }

    Serial.println("Schedule task started successfully");
    return true;
}

void ScheduleTask::stop() {
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }

    if (scheduleManager != NULL) {
        delete scheduleManager;
        scheduleManager = NULL;
    }

    if (powerStateMutex != NULL) {
        vSemaphoreDelete(powerStateMutex);
        powerStateMutex = NULL;
    }
}

void ScheduleTask::taskFunction(void* parameter) {
    while (true) {
        if (scheduleManager != NULL) {
            bool currentPowerState;

            // Get power state with mutex protection
            if (xSemaphoreTake(powerStateMutex, portMAX_DELAY) == pdTRUE) {
                currentPowerState = power_state;
                xSemaphoreGive(powerStateMutex);
            } else {
                Serial.println("Failed to get power state mutex");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // Check schedule
            if (!scheduleManager->checkSchedule(currentPowerState)) {
                Serial.println("Failed to check schedule");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL));
    }
}

bool ScheduleTask::getPowerState() {
    bool state = false;
    if (powerStateMutex != NULL) {
        if (xSemaphoreTake(powerStateMutex, portMAX_DELAY) == pdTRUE) {
            state = power_state;
            xSemaphoreGive(powerStateMutex);
        }
    }
    return state;
}

void ScheduleTask::setPowerState(bool state) {
    if (powerStateMutex != NULL) {
        if (xSemaphoreTake(powerStateMutex, portMAX_DELAY) == pdTRUE) {
            power_state = state;
            xSemaphoreGive(powerStateMutex);
        }
    }
}