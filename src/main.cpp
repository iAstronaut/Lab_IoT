#include <Arduino.h>
#include <Wire.h>
#include "DHT20.h"
#include "tasks/task.h"  // Include the task header to initialize all tasks

// DHT20 Sensor
// DHT20 DHT;

// Define I2C pins (Modify based on your board)
#define I2C_SDA 11  // Change to match your hardware
#define I2C_SCL 12  // Change to match your hardware

void setup() {
    Serial.begin(115200);
    // Wire.begin(I2C_SDA, I2C_SCL);  // Initialize I2C with specific pins

    // Initialize DHT20
    // if (!DHT.begin()) {
    //     Serial.println("Failed to initialize DHT20 sensor!");
    //     while (1);
    // }
    // Serial.println("DHT20 sensor initialized.");

    // Initialize WiFi


    // Initialize MQTT
    // initializeMQTT();

    // Create Tasks (task.cpp will handle this)
    createTasks();
}

void loop() {
    // Empty: FreeRTOS handles tasks
}
