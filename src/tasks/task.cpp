#include <Arduino.h>
#include "task.h"

// Task priorities
#define WIFI_TASK_PRIORITY 3
#define MQTT_TASK_PRIORITY 2
#define DHT_TASK_PRIORITY 1

// Task stack sizes
#define WIFI_TASK_STACK_SIZE 4096
#define MQTT_TASK_STACK_SIZE 4096
#define DHT_TASK_STACK_SIZE 2048

// Function to create all tasks
void createTasks() {
    // Create WiFi task first (highest priority)
    xTaskCreate(
        TaskWiFi,           // Task function
        "WiFi",             // Task name
        WIFI_TASK_STACK_SIZE, // Stack size
        NULL,               // Task parameters
        WIFI_TASK_PRIORITY, // Priority
        NULL                // Task handle
    );

    // Create MQTT task
    xTaskCreate(
        TaskMQTT,           // Task function
        "MQTT",             // Task name
        MQTT_TASK_STACK_SIZE, // Stack size
        NULL,               // Task parameters
        MQTT_TASK_PRIORITY, // Priority
        NULL                // Task handle
    );

    // Create DHT20 sensor task (lowest priority)
    xTaskCreate(
        Task3,              // Task function
        "DHT20",            // Task name
        DHT_TASK_STACK_SIZE, // Stack size
        NULL,               // Task parameters
        DHT_TASK_PRIORITY,  // Priority
        NULL                // Task handle
    );
}
