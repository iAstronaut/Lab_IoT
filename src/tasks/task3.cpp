#include <Arduino.h>
#include "task3.h"
#include "DHT20.h"
#include "mqtt.h"

// Declare DHT20 sensor
extern DHT20 DHT;

float latitude = 0.0;
float longitude = 0.0;

void Task3(void *pvParameters) {
    unsigned long lastSentTime = 0;
    unsigned long lastLocationTime = 0;
    const unsigned long SEND_INTERVAL = 10000;
    const unsigned long LOCATION_INTERVAL = 60000;
    while (1) {
        unsigned long currentTime = millis();

        // Read sensor data
        if (millis() - DHT.lastRead() >= 2000) {
            int status = DHT.read();

            if (status == DHT20_OK) {
                float temperature = DHT.getTemperature();
                float humidity = DHT.getHumidity();

                // Print sensor readings
                Serial.print("DHT20 Temperature: ");
                Serial.print(temperature, 1);
                Serial.println(" °C");

                Serial.print("DHT20 Humidity: ");
                Serial.print(humidity, 1);
                Serial.println(" %");

                if (currentTime - lastLocationTime >= LOCATION_INTERVAL) {

                    latitude = 0.0;
                    longitude = 0.0;
                    lastLocationTime = currentTime;
                }

                if (currentTime - lastSentTime >= SEND_INTERVAL) {
                    send_telemetry(temperature, humidity, latitude, longitude);
                    lastSentTime = currentTime;
                }
            } else {
                // Print error status
                Serial.print("Status: ");
                switch (status) {
                    case DHT20_ERROR_CHECKSUM:
                        Serial.println("Checksum error");
                        break;
                    case DHT20_ERROR_CONNECT:
                        Serial.println("Connect error");
                        break;
                    case DHT20_MISSING_BYTES:
                        Serial.println("Missing bytes");
                        break;
                    case DHT20_ERROR_BYTES_ALL_ZERO:
                        Serial.println("All bytes read zero");
                        break;
                    case DHT20_ERROR_READ_TIMEOUT:
                        Serial.println("Read time out");
                        break;
                    case DHT20_ERROR_LASTREAD:
                        Serial.println("Read too fast");
                        break;
                    default:
                        Serial.println("Unknown error");
                        break;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
