#include "schedule_task.h"

TaskHandle_t ScheduleTask::taskHandle = NULL;
ScheduleManager* ScheduleTask::scheduleManager = NULL;
bool ScheduleTask::power_state = true;

void ScheduleTask::start() {
    if (taskHandle != NULL) {
        return; // Task already running
    }

    scheduleManager = new ScheduleManager();
    scheduleManager->begin();

    xTaskCreate(
        taskFunction,
        "Schedule Task",
        STACK_SIZE,
        NULL,
        1,
        &taskHandle
    );
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
}

void ScheduleTask::taskFunction(void* parameter) {
    while (true) {
        if (scheduleManager != NULL) {
            scheduleManager->checkSchedule(power_state);
        }
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL));
    }
}

bool ScheduleTask::getPowerState() {
    return power_state;
}

void ScheduleTask::setPowerState(bool state) {
    power_state = state;
}