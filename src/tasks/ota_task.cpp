#include "ota_task.h"
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

TaskHandle_t otaTaskHandle = NULL;

// OTA parameters
const int MAX_HTTP_RETRIES = 3;
const int HTTP_RETRY_DELAY = 5000; // 5 seconds
const int MIN_FIRMWARE_SIZE = 100000; // Minimum firmware size (100KB)
const int MAX_FIRMWARE_SIZE = 2000000; // Maximum firmware size (2MB)

void otaTask(void * parameter) {
    Serial.println("OTA Task started");

    while(1) {
        if(WiFi.status() == WL_CONNECTED) {
            if(checkForUpdates()) {
                Serial.println("Update completed successfully");
            }
        } else {
            Serial.println("WiFi not connected, skipping update check");
        }
        vTaskDelay(pdMS_TO_TICKS(OTA_CHECK_INTERVAL));
    }
}

bool checkForUpdates() {
    HTTPClient http;
    bool updateResult = false;
    int retryCount = 0;

    // Create URL for update check
    String url = "https://otadrive.com/deviceapi/update?";
    url += "k=" + String(OTA_API_KEY);
    url += "&v=" + String(FIRMWARE_VERSION);
    url += "&s=" + String(DEVICE_ID);
    url += "&p=ESP32";  // Platform
    url += "&b=GENERIC"; // Board type

    Serial.println("Checking for updates...");
    Serial.println("URL: " + url);

    while (retryCount < MAX_HTTP_RETRIES) {
        http.begin(url);
        int httpCode = http.GET();

        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Response: " + payload);

            if(payload.indexOf("update available") >= 0) {
                Serial.println("Update available!");

                // Parse response to get new version
                StaticJsonDocument<200> doc;
                DeserializationError error = deserializeJson(doc, payload);

                if(!error) {
                    const char* newVersion = doc["version"] | "1.0.0.2";
                    int firmwareSize = doc["size"] | 0;

                    // Validate firmware size
                    if (firmwareSize < MIN_FIRMWARE_SIZE || firmwareSize > MAX_FIRMWARE_SIZE) {
                        Serial.println("Invalid firmware size");
                        break;
                    }

                    // Create download URL
                    String downloadUrl = "https://otadrive.com/deviceapi/download?";
                    downloadUrl += "k=" + String(OTA_API_KEY);
                    downloadUrl += "&v=" + String(newVersion);
                    downloadUrl += "&s=" + String(DEVICE_ID);

                    updateResult = handleOTAUpdate(downloadUrl.c_str(), firmwareSize);
                    break;
                } else {
                    Serial.print("Failed to parse update response: ");
                    Serial.println(error.c_str());
                }
            } else {
                Serial.println("No updates available");
                break;
            }
        } else {
            Serial.printf("Update check failed, error: %d\n", httpCode);
            retryCount++;
            if (retryCount < MAX_HTTP_RETRIES) {
                Serial.println("Retrying in 5 seconds...");
                vTaskDelay(pdMS_TO_TICKS(HTTP_RETRY_DELAY));
            }
        }

        http.end();
    }

    if (retryCount >= MAX_HTTP_RETRIES) {
        Serial.println("Maximum retry attempts reached");
    }

    return updateResult;
}

bool handleOTAUpdate(const char* firmware_url, int expectedSize) {
    HTTPClient http;
    bool success = false;
    int retryCount = 0;

    Serial.println("Starting firmware update...");
    Serial.println("Download URL: " + String(firmware_url));

    while (retryCount < MAX_HTTP_RETRIES) {
        http.begin(firmware_url);
        int httpCode = http.GET();

        if(httpCode == HTTP_CODE_OK) {
            int contentLength = http.getSize();
            Serial.printf("Update size: %d bytes\n", contentLength);

            // Validate content length
            if (contentLength != expectedSize) {
                Serial.println("Content length mismatch");
                break;
            }

            if (Update.begin(contentLength)) {
                Serial.println("Starting download...");

                // Get tcp stream
                WiFiClient * stream = http.getStreamPtr();

                // Read data and write to Update
                size_t written = Update.writeStream(*stream);

                if (written == contentLength) {
                    Serial.println("Written : " + String(written) + " successfully");
                } else {
                    Serial.println("Written only : " + String(written) + "/" + String(contentLength));
                    break;
                }

                if (Update.end()) {
                    if (Update.isFinished()) {
                        Serial.println("Update successfully completed");
                        success = true;
                        // Wait a moment before restarting
                        vTaskDelay(pdMS_TO_TICKS(2000));
                        ESP.restart();
                    } else {
                        Serial.println("Update not finished");
                    }
                } else {
                    Serial.println("Error Occurred: " + String(Update.getError()));
                }
            } else {
                Serial.println("Not enough space to begin OTA");
            }
            break;
        } else {
            Serial.printf("HTTP download failed, error: %d\n", httpCode);
            retryCount++;
            if (retryCount < MAX_HTTP_RETRIES) {
                Serial.println("Retrying in 5 seconds...");
                vTaskDelay(pdMS_TO_TICKS(HTTP_RETRY_DELAY));
            }
        }

        http.end();
    }

    if (retryCount >= MAX_HTTP_RETRIES) {
        Serial.println("Maximum download retry attempts reached");
    }

    return success;
}