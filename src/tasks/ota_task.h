#ifndef OTA_TASK_H
#define OTA_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mbedtls/sha256.h>
#include <mbedtls/md.h>
#include <esp_ota_ops.h>
#include <esp_secure_boot.h>

// OTA parameters
extern const char* OTA_API_KEY;
extern const char* FIRMWARE_VERSION;
extern const char* DEVICE_ID;
extern const uint32_t OTA_CHECK_INTERVAL;

// Hash verification parameters
#define HASH_BUF_SIZE 32
#define CHUNK_SIZE 1024

// Security parameters
extern const char* PUBLIC_KEY;

// Task handle
extern TaskHandle_t otaTaskHandle;

/**
 * Main OTA task function
 * Handles periodic checks for firmware updates
 */
void otaTask(void * parameter);

/**
 * Checks for available firmware updates
 * @return true if update was successful
 */
bool checkForUpdates();

/**
 * Handles the OTA update process
 * @param firmware_url URL to download firmware from
 * @param expectedSize Expected size of firmware
 * @param expected_hash Expected hash of firmware
 * @return true if update successful
 */
bool handleOTAUpdate(const char* firmware_url, int expectedSize, const char* expected_hash);

/**
 * Verifies firmware integrity and authenticity
 * @param firmware_data Pointer to firmware data
 * @param size Size of firmware
 * @param expected_hash Expected hash of firmware
 * @return true if verification passes
 */
bool verifyFirmware(const uint8_t* firmware_data, size_t size, const char* expected_hash);

/**
 * Compares version strings
 * @param newVersion New firmware version
 * @param currentVersion Current firmware version
 * @return true if newVersion is newer
 */
bool isNewerVersion(const char* newVersion, const char* currentVersion);

/**
 * Calculates the hash of a given data
 * @param data Pointer to data
 * @param size Size of data
 * @return Calculated hash as a String
 */
String calculateHash(const uint8_t* data, size_t size);

/**
 * Verifies if the calculated hash matches the expected hash
 * @param calculated_hash Calculated hash
 * @param expected_hash Expected hash
 * @return true if hashes match
 */
bool verifyHash(const String& calculated_hash, const char* expected_hash);

#endif // OTA_TASK_H