#include <Arduino.h>
#include <Wire.h>
#include "DHT20.h"
#include "tasks/task.h"  // Include the task header to initialize all tasks
#include <WiFi.h>
#include "tasks/ota_task.h"
#include "tasks/schedule_task.h"
#include "tasks/mqtt_task.h"

// DHT20 Sensor
// DHT20 DHT;

// Define I2C pins (Modify based on your board)
#define I2C_SDA 11  // Change to match your hardware
#define I2C_SCL 12  // Change to match your hardware

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// OTADrive credentials
const char* otadriveApiKey = "85e7d3e5-05ab-46ad-bd6b-fbbe3492491f";
const char* otadriveProductId = "34000";

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
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize MQTT
    // initializeMQTT();

    // Create Tasks (task.cpp will handle this)
    createTasks();

    // Start OTA task
    OTATask::start(otadriveApiKey, otadriveProductId);

    // Start schedule task
    ScheduleTask::start();
    MQTTTask::start();
}

void loop() {
    // Main loop is empty as we're using FreeRTOS tasks
    delay(1000);
}
