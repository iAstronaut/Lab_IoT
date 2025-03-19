#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>

extern const char* ssid;
extern const char* password;

void setup_wifi();
void TaskWiFi(void *pvParameters);

#endif
