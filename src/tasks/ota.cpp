#include <Arduino.h>
#include "ota.h"
#include <HTTPClient.h>
#include <Update.h>
#include <SPIFFS.h>

#define FIRMWARE_URL "http://your-ota-server.com/firmware.bin"
#define FIRMWARE_VERSION_URL "http://your-ota-server.com/version.txt"

void TaskOTA(void *pvParameters) {
    while (1) {
        HTTPClient http;
        WiFiClient client;

        // Check for new firmware version
        http.begin(client, FIRMWARE_VERSION_URL);
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String newVersion = http.getString();
            String currentVersion = "1.0.0"; // Replace with your current version

            if (newVersion != currentVersion) {
                Serial.println("New firmware available. Starting update...");

                // Download firmware
                http.begin(client, FIRMWARE_URL);
                httpCode = http.GET();

                if (httpCode == HTTP_CODE_OK) {
                    int contentLength = http.getSize();
                    if (contentLength > 0) {
                        if (Update.begin(contentLength)) {
                            size_t written = Update.writeStream(http.getStream());
                            if (written == contentLength) {
                                Serial.println("Written : " + String(written) + " successfully");
                            } else {
                                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
                            }
                        } else {
                            Serial.println("No space left for update");
                        }
                    } else {
                        Serial.println("Content-Length was 0 or not set");
                    }
                } else {
                    Serial.println("Failed to download firmware");
                }
            }
        }

        http.end();
        vTaskDelay(pdMS_TO_TICKS(300000)); // Check every 5 minutes
    }
}