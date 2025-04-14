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

// WiFi connection parameters
const int MAX_WIFI_RETRIES = 10;
const int WIFI_RETRY_DELAY = 5000; // 5 seconds

bool connectToWiFi() {
    int retryCount = 0;

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && retryCount < MAX_WIFI_RETRIES) {
        delay(WIFI_RETRY_DELAY);
        Serial.print("Attempting WiFi connection... (");
        Serial.print(retryCount + 1);
        Serial.print("/");
        Serial.print(MAX_WIFI_RETRIES);
        Serial.println(")");
        retryCount++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi after maximum retries");
        return false;
    }

    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup...");

    // Initialize WiFi with retry mechanism
    if (!connectToWiFi()) {
        Serial.println("Failed to initialize WiFi. Restarting in 5 seconds...");
        delay(5000);
        ESP.restart();
    }

    // Initialize DHT20
    // if (!DHT.begin()) {
    //     Serial.println("Failed to initialize DHT20 sensor!");
    //     while (1);
    // }
    // Serial.println("DHT20 sensor initialized.");

    // Create Tasks
    createTasks();

    // Start OTA task
    if (!OTATask::start(otadriveApiKey, otadriveProductId)) {
        Serial.println("Failed to start OTA task");
    }

    // Start schedule task
    if (!ScheduleTask::start()) {
        Serial.println("Failed to start Schedule task");
    }

    // Start MQTT task
    if (!MQTTTask::start()) {
        Serial.println("Failed to start MQTT task");
    }

    Serial.println("Setup completed successfully");
}

void loop() {
    // Check WiFi connection periodically
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        if (!connectToWiFi()) {
            Serial.println("Failed to reconnect to WiFi. Restarting...");
            ESP.restart();
        }
    }

    delay(1000);
}
