#ifndef OTA_CLIENT_H
#define OTA_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>

class OTAClient {
public:
    OTAClient(const char* apiKey, const char* productId);
    void begin();
    bool checkForUpdates();
    bool performUpdate();
    String getCurrentVersion();
    String getDeviceId();

private:
    const char* apiKey;
    const char* productId;
    const char* baseUrl = "https://api.otadrive.com/v1";
    String deviceId;
    String currentVersion;
    String latestVersion;
    String firmwareUrl;

    bool downloadFirmware();
    bool verifyUpdate();
    String getChipId();
};

#endif