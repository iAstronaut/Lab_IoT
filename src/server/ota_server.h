#ifndef OTA_SERVER_H
#define OTA_SERVER_H

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class OTAServer {
public:
    OTAServer();
    void begin();
    void handleLoop();

private:
    AsyncWebServer server;
    String getChipId();
    void handleVersion(AsyncWebServerRequest *request);
    void handleFirmware(AsyncWebServerRequest *request);
    void handleUpdate(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);
    void handleUpload(AsyncWebServerRequest *request);
    void handleRoot(AsyncWebServerRequest *request);
    void handleDeviceInfo(AsyncWebServerRequest *request);
    bool checkFirmwareVersion(const char* version);
    const char* getUploadPage();
};

#endif