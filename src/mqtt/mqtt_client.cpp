#include "mqtt_client.h"
#include "../schedule/schedule.h"

// Global instance for callback
static MQTTClient* instance = nullptr;

// MQTT connection parameters
const int MAX_MQTT_RETRIES = 5;
const int MQTT_RETRY_DELAY = 5000; // 5 seconds

MQTTClient::MQTTClient()
    : mqttClient(espClient) {
    last_data_send_time = 0;
    sim_temperature = 25.0;
    sim_humidity = 50.0;
    connectionAttempts = 0;
    power_state = true;

    // Initialize schedule manager
    scheduleManager.begin();
}

void MQTTClient::begin() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
        this->callback(topic, payload, length);
    });
    instance = this;

    if (!connect()) {
        Serial.println("Failed to establish initial MQTT connection");
    }
}

bool MQTTClient::connect() {
    if (connectionAttempts >= MAX_MQTT_RETRIES) {
        Serial.println("Maximum MQTT connection attempts reached");
        return false;
    }

    Serial.print("Attempting MQTT connection... (");
    Serial.print(connectionAttempts + 1);
    Serial.print("/");
    Serial.print(MAX_MQTT_RETRIES);
    Serial.println(")");

    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_ACCESS_TOKEN, NULL)) {
        Serial.println("MQTT connected");
        connectionAttempts = 0;

        // Subscribe to topics
        if (!mqttClient.subscribe(TOPIC_RPC_REQUEST)) {
            Serial.println("Failed to subscribe to RPC topic");
            return false;
        }
        if (!mqttClient.subscribe(TOPIC_ATTRIBUTES_RESPONSE)) {
            Serial.println("Failed to subscribe to attributes topic");
            return false;
        }

        // Request shared attributes
        StaticJsonDocument<200> doc;
        doc["sharedKeys"] = "schedule";
        String payload;
        serializeJson(doc, payload);
        if (!mqttClient.publish(TOPIC_ATTRIBUTES_REQUEST, payload.c_str())) {
            Serial.println("Failed to publish attributes request");
            return false;
        }

        return true;
    } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        connectionAttempts++;
        delay(MQTT_RETRY_DELAY);
        return false;
    }
}

void MQTTClient::reconnect() {
    if (!mqttClient.connected()) {
        if (!connect()) {
            Serial.println("Failed to reconnect to MQTT");
            if (connectionAttempts >= MAX_MQTT_RETRIES) {
                Serial.println("Restarting ESP32 due to MQTT connection failure");
                ESP.restart();
            }
        }
    }
}

void MQTTClient::callback(char* topic, uint8_t* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.printf("Received message on %s: %s\n", topic, message);

    if (strncmp(topic, TOPIC_RPC_REQUEST, strlen(TOPIC_RPC_REQUEST)) == 0) {
        const char* requestId = topic + strlen(TOPIC_RPC_REQUEST);
        handleRPC(requestId, message);
    }
    else if (strcmp(topic, TOPIC_ATTRIBUTES_RESPONSE) == 0) {
        handleAttributes(message);
    }
}

void MQTTClient::handleRPC(const char* requestId, const char* message) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.println("Failed to parse RPC message");
        return;
    }

    const char* method = doc["method"];
    JsonObject params = doc["params"];

    StaticJsonDocument<200> response;

    if (strcmp(method, "getTemperatureReading") == 0) {
        response["temperature"] = sim_temperature;
    }
    else if (strcmp(method, "getHumidityReading") == 0) {
        response["humidity"] = sim_humidity;
    }
    else if (strcmp(method, "setPowerState") == 0) {
        bool newState = params["value"] | false;
        ScheduleTask::setPowerState(newState);
        response["power"] = newState;
    }
    else if (strcmp(method, "getPowerState") == 0) {
        response["power"] = ScheduleTask::getPowerState();
    }

    String responseStr;
    serializeJson(response, responseStr);

    char responseTopic[100];
    snprintf(responseTopic, sizeof(responseTopic), "%s%s", TOPIC_RPC_RESPONSE, requestId);
    mqttClient.publish(responseTopic, responseStr.c_str());
}

void MQTTClient::handleAttributes(const char* message) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("Failed to parse attributes message: ");
        Serial.println(error.c_str());
        return;
    }

    JsonObject shared = doc["shared"];
    if (shared.containsKey("schedule")) {
        JsonObject scheduleObj = shared["schedule"];

        // Validate schedule data
        if (!scheduleObj.containsKey("off_time") ||
            !scheduleObj.containsKey("on_time") ||
            !scheduleObj.containsKey("schedule_on") ||
            !scheduleObj.containsKey("schedule_off")) {
            Serial.println("Invalid schedule data received");
            return;
        }

        Schedule newSchedule;
        newSchedule.off_time = scheduleObj["off_time"];
        newSchedule.on_time = scheduleObj["on_time"];
        newSchedule.schedule_on = scheduleObj["schedule_on"];
        newSchedule.schedule_off = scheduleObj["schedule_off"];

        // Additional validation
        if (newSchedule.off_time < 0 || newSchedule.off_time > 23 ||
            newSchedule.on_time < 0 || newSchedule.on_time > 23) {
            Serial.println("Invalid time values in schedule");
            return;
        }

        if (!scheduleManager.updateSchedule(newSchedule)) {
            Serial.println("Failed to update schedule");
        }
    }
}

void MQTTClient::generateSensorData() {
    sim_temperature += random(-5, 6) / 10.0;
    sim_humidity += random(-10, 11) / 10.0;

    // Keep values in realistic ranges
    sim_temperature = constrain(sim_temperature, 15.0, 40.0);
    sim_humidity = constrain(sim_humidity, 30.0, 90.0);
}

void MQTTClient::sendTelemetryData(bool forceSend) {
    unsigned long current_time = millis();

    if ((ScheduleTask::getPowerState() || forceSend) && (current_time - last_data_send_time >= 15000)) {
        if (ScheduleTask::getPowerState()) {
            generateSensorData();
        } else {
            sim_temperature = 0;
            sim_humidity = 0;
        }

        StaticJsonDocument<200> doc;
        doc["temperature"] = sim_temperature;
        doc["humidity"] = sim_humidity;
        doc["power_state"] = ScheduleTask::getPowerState();

        Schedule currentSchedule = scheduleManager.getSchedule();
        doc["schedule_on"] = currentSchedule.schedule_on;
        doc["schedule_off"] = currentSchedule.schedule_off;

        String payload;
        serializeJson(doc, payload);
        mqttClient.publish(TOPIC_TELEMETRY, payload.c_str());

        last_data_send_time = current_time;
    }
}

void MQTTClient::loop() {
    reconnect();
    mqttClient.loop();
    if (ScheduleTask::getPowerState()) {
        sendTelemetryData();
    }
}

bool MQTTClient::isConnected() {
    return mqttClient.connected();
}

void MQTTClient::setPowerState(bool state) {
    ScheduleTask::setPowerState(state);
}

bool MQTTClient::getPowerState() const {
    return ScheduleTask::getPowerState();
}

void MQTTClient::updateSchedule(const Schedule& newSchedule) {
    scheduleManager.updateSchedule(newSchedule);
}

Schedule MQTTClient::getSchedule() const {
    return scheduleManager.getSchedule();
}