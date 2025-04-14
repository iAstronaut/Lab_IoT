#include "ota_task.h"
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <mbedtls/sha256.h>

TaskHandle_t otaTaskHandle = NULL;

// OTA parameters
const int MAX_HTTP_RETRIES = 3;
const int HTTP_RETRY_DELAY = 5000; // 5 seconds
const int MIN_FIRMWARE_SIZE = 100000; // Minimum firmware size (100KB)
const int MAX_FIRMWARE_SIZE = 2000000; // Maximum firmware size (2MB)

// Security parameters
const char* PUBLIC_KEY = "-----BEGIN PUBLIC KEY-----\n"
                        "YOUR_PUBLIC_KEY_HERE\n"
                        "-----END PUBLIC KEY-----\n";

/**
 * OTA Task - Handles firmware updates over-the-air
 * Implements:
 * - Secure firmware verification using SHA-256
 * - Fail-safe dual partition update
 * - Automatic rollback on failed updates
 * - Resumable downloads for network interruptions
 */
void otaTask(void * parameter) {
    Serial.println("OTA Task started");

    // Initialize secure elements if available
    if(esp_secure_boot_enabled()) {
        Serial.println("Secure boot is enabled");
    }

    while(1) {
        if(WiFi.status() == WL_CONNECTED) {
            if(checkForUpdates()) {
                Serial.println("Update completed successfully");
                // Wait for restart to take effect
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        } else {
            Serial.println("WiFi not connected, skipping update check");
        }
        vTaskDelay(pdMS_TO_TICKS(OTA_CHECK_INTERVAL));
    }
}

String calculateHash(const uint8_t* data, size_t size) {
    uint8_t hash[HASH_BUF_SIZE];
    mbedtls_sha256_context ctx;

    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 for SHA-256, 1 for SHA-224
    mbedtls_sha256_update(&ctx, data, size);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    // Convert hash to hex string
    char hash_str[HASH_BUF_SIZE * 2 + 1];
    for(int i = 0; i < HASH_BUF_SIZE; i++) {
        sprintf(&hash_str[i * 2], "%02x", hash[i]);
    }
    hash_str[HASH_BUF_SIZE * 2] = '\0';

    return String(hash_str);
}

bool verifyHash(const String& calculated_hash, const char* expected_hash) {
    if (!expected_hash) {
        Serial.println("Error: Expected hash is null");
        return false;
    }

    // Convert both hashes to lowercase for comparison
    String calc_hash_lower = calculated_hash;
    calc_hash_lower.toLowerCase();

    String expected_hash_str = String(expected_hash);
    expected_hash_str.toLowerCase();

    // Compare hashes
    if (calc_hash_lower.equals(expected_hash_str)) {
        Serial.println("Hash verification successful");
        return true;
    } else {
        Serial.println("Hash verification failed");
        Serial.println("Calculated: " + calc_hash_lower);
        Serial.println("Expected: " + expected_hash_str);
        return false;
    }
}

bool verifyFirmware(const uint8_t* firmware_data, size_t size, const char* expected_hash) {
    if (!firmware_data || size == 0) {
        Serial.println("Invalid firmware data");
        return false;
    }

    // Calculate hash of firmware data
    String calculated_hash = calculateHash(firmware_data, size);

    // Verify hash
    return verifyHash(calculated_hash, expected_hash);
}

bool checkForUpdates() {
    HTTPClient http;
    bool updateResult = false;
    int retryCount = 0;

    String url = "https://otadrive.com/deviceapi/update?";
    url += "k=" + String(OTA_API_KEY);
    url += "&v=" + String(FIRMWARE_VERSION);
    url += "&s=" + String(DEVICE_ID);
    url += "&p=ESP32";
    url += "&b=GENERIC";

    Serial.println("Checking for updates...");
    Serial.println("URL: " + url);

    while (retryCount < MAX_HTTP_RETRIES) {
        http.begin(url);

        // Add security headers
        http.addHeader("X-Device-ID", DEVICE_ID);
        http.addHeader("X-Firmware-Version", FIRMWARE_VERSION);

        int httpCode = http.GET();

        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Response: " + payload);

            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if(!error) {
                const char* newVersion = doc["version"] | "1.0.0.2";
                int firmwareSize = doc["size"] | 0;
                const char* firmwareHash = doc["hash"] | "";  // Get hash from server response

                // Validate firmware size
                if (firmwareSize < MIN_FIRMWARE_SIZE || firmwareSize > MAX_FIRMWARE_SIZE) {
                    Serial.println("Invalid firmware size");
                    break;
                }

                // Version control check
                if (!isNewerVersion(newVersion, FIRMWARE_VERSION)) {
                    Serial.println("Firmware version is not newer than current version");
                    break;
                }

                String downloadUrl = "https://otadrive.com/deviceapi/download?";
                downloadUrl += "k=" + String(OTA_API_KEY);
                downloadUrl += "&v=" + String(newVersion);
                downloadUrl += "&s=" + String(DEVICE_ID);

                Serial.println("Firmware hash from server: " + String(firmwareHash));
                updateResult = handleOTAUpdate(downloadUrl.c_str(), firmwareSize, firmwareHash);
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

/**
 * Handles the OTA update process with resume capability
 * @param firmware_url URL to download firmware from
 * @param expectedSize Expected size of firmware
 * @param signature Firmware signature for verification
 * @return true if update successful
 */
bool handleOTAUpdate(const char* firmware_url, int expectedSize, const char* signature) {
    HTTPClient http;
    bool success = false;
    int retryCount = 0;
    size_t downloadedSize = 0;

    Serial.println("Starting firmware update...");
    Serial.println("Download URL: " + String(firmware_url));

    // Prepare update partition
    const esp_partition_t* update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        Serial.println("Failed to find update partition");
        return false;
    }

    while (retryCount < MAX_HTTP_RETRIES && downloadedSize < expectedSize) {
        http.begin(firmware_url);

        // Add range header for resume capability
        if (downloadedSize > 0) {
            http.addHeader("Range", "bytes=" + String(downloadedSize) + "-");
        }

        int httpCode = http.GET();

        if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_PARTIAL_CONTENT) {
            WiFiClient* stream = http.getStreamPtr();
            uint8_t buffer[1024] = {0};
            int bytesRead = 0;

            // Read and verify firmware in chunks
            while ((bytesRead = stream->readBytes(buffer, sizeof(buffer))) > 0) {
                if (verifyFirmware(buffer, bytesRead, signature)) {
                    // Write verified chunk to flash
                    if (esp_ota_write(update_partition, buffer, bytesRead) != ESP_OK) {
                        Serial.println("Failed to write firmware chunk");
                        break;
                    }
                    downloadedSize += bytesRead;

                    // Print progress
                    Serial.printf("Progress: %d%%\n", (downloadedSize * 100) / expectedSize);
                } else {
                    Serial.println("Firmware verification failed");
                    break;
                }
            }

            if (downloadedSize == expectedSize) {
                if (esp_ota_end() == ESP_OK && esp_ota_set_boot_partition(update_partition) == ESP_OK) {
                    Serial.println("Update successfully completed");
                    success = true;
                    break;
                }
            }
        } else {
            Serial.printf("HTTP download failed, error: %d\n", httpCode);
            retryCount++;
            if (retryCount < MAX_HTTP_RETRIES) {
                Serial.println("Retrying download...");
                vTaskDelay(pdMS_TO_TICKS(HTTP_RETRY_DELAY));
            }
        }

        http.end();
    }

    if (!success) {
        Serial.println("Update failed, rolling back...");
        esp_ota_mark_app_invalid_rollback_and_reboot();
    }

    return success;
}

/**
 * Compares version strings
 * @param newVersion New firmware version
 * @param currentVersion Current firmware version
 * @return true if newVersion is newer
 */
bool isNewerVersion(const char* newVersion, const char* currentVersion) {
    // Implement version comparison logic here
    // This is a simple implementation - enhance based on your versioning scheme
    return strcmp(newVersion, currentVersion) > 0;
}