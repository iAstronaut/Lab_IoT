#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>
#include <time.h>

// Schedule structure
struct Schedule {
    time_t off_time;
    time_t on_time;
    int schedule_on;
    int schedule_off;
};

class ScheduleManager {
public:
    ScheduleManager();
    void begin();
    void updateSchedule(const Schedule& newSchedule);
    Schedule getSchedule() const;
    void checkSchedule(bool& power_state);
    void setDefaultSchedule();

private:
    Schedule schedule;
    bool isInitialized;
};

#endif