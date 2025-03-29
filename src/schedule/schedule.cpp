#include "schedule.h"

ScheduleManager::ScheduleManager() : isInitialized(false) {
    // Initialize schedule with default values
    schedule.off_time = 0;
    schedule.on_time = 0;
    schedule.schedule_on = 0;
    schedule.schedule_off = 0;
}

void ScheduleManager::begin() {
    setDefaultSchedule();
    isInitialized = true;
}

void ScheduleManager::setDefaultSchedule() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get local time");
        return;
    }

    // Set off time to 21:51 today
    timeinfo.tm_hour = 22;
    timeinfo.tm_min = 05;
    timeinfo.tm_sec = 0;
    schedule.off_time = mktime(&timeinfo);

    // Set on time to 5 minutes after off time
    schedule.on_time = schedule.off_time + 300;
}

void ScheduleManager::updateSchedule(const Schedule& newSchedule) {
    schedule = newSchedule;
    isInitialized = true;
}

Schedule ScheduleManager::getSchedule() const {
    return schedule;
}

void ScheduleManager::checkSchedule(bool& power_state) {
    if (!isInitialized) {
        setDefaultSchedule();
    }

    time_t current_time = time(nullptr);

    bool is_in_off_period = current_time >= schedule.off_time && current_time < schedule.on_time;
    bool is_after_on_time = current_time >= schedule.on_time;

    if (is_in_off_period && power_state) {
        power_state = false;
        Serial.printf("Schedule: Power OFF at %ld\n", current_time);
    }
    else if (is_after_on_time && !power_state) {
        power_state = true;
        Serial.printf("Schedule: Power ON at %ld\n", current_time);
    }
}