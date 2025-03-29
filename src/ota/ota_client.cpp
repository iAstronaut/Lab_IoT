#include "ota_client.h"
#include <ArduinoJson.h>

#define CURRENT_FIRMWARE_VERSION "1.0.0"

OTAClient::OTAClient(const char* apiKey, const char* productId)
    : apiKey(apiKey), productId(productId) {
    deviceId = getChipId();
    currentVersion = CURRENT_FIRMWARE_VERSION;
}

void OTAClient::begin() {
    Serial.println("OTA Client initialized");
    Serial.printf("Device ID: %s\n", deviceId.c_str());
    Serial.printf("Current Version: %s\n", currentVersion.c_str());
}

String OTAClient::getChipId() {
    String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
    ChipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
    return ChipIdHex;
}

bool OTAClient::checkForUpdates() {
    HTTPClient http;
    String url = String(baseUrl) + "/products/" + productId + "/firmware/latest";

    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + apiKey);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            latestVersion = doc["version"].as<String>();
            firmwareUrl = doc["url"].as<String>();

            if (latestVersion != currentVersion) {
                Serial.printf("New firmware available: %s\n", latestVersion.c_str());
                return true;
            }
        }
    }

    http.end();
    return false;
}

bool OTAClient::downloadFirmware() {
    HTTPClient http;

    http.begin(firmwareUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        if (contentLength > 0) {
            if (Update.begin(contentLength)) {
                Serial.println("Starting firmware update...");
                return true;
            } else {
                Serial.println("No space for update");
                return false;
            }
        }
    }

    http.end();
    return false;
}

bool OTAClient::performUpdate() {
    if (!downloadFirmware()) {
        return false;
    }

    HTTPClient http;
    http.begin(firmwareUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient* client = http.getStreamPtr();

        size_t written = Update.writeStream(*client);
        Serial.printf("Written: %d\n", written);

        if (Update.end()) {
            Serial.println("Update completed successfully");
            if (Update.isRunning()) {
                Serial.println("Rebooting...");
                ESP.restart();
            }
            return true;
        } else {
            Serial.printf("Update failed: %s\n", Update.errorString());
            return false;
        }
    }

    http.end();
    return false;
}

String OTAClient::getCurrentVersion() {
    return currentVersion;
}

String OTAClient::getDeviceId() {
    return deviceId;
}