#ifndef SCHEDULE_TASK_H
#define SCHEDULE_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "../schedule/schedule.h"

class ScheduleTask {
public:
    static bool start();
    static void stop();
    static bool getPowerState();
    static void setPowerState(bool state);

private:
    static void taskFunction(void* parameter);
    static TaskHandle_t taskHandle;
    static ScheduleManager* scheduleManager;
    static bool power_state;
    static SemaphoreHandle_t powerStateMutex;
    static const uint32_t STACK_SIZE = 4096;
    static const uint32_t CHECK_INTERVAL = 1000; // Check every second
};

#endif