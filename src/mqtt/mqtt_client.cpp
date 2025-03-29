#include "mqtt_client.h"
#include "../schedule/schedule.h"

// Global instance for callback
static MQTTClient* instance = nullptr;

MQTTClient::MQTTClient()
    : mqttClient(espClient) {
    last_data_send_time = 0;
    sim_temperature = 25.0;
    sim_humidity = 50.0;

    // Initialize schedule manager
    scheduleManager.begin();
}

void MQTTClient::begin() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    instance = this;
    connect();
}

void MQTTClient::connect() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_ACCESS_TOKEN, NULL)) {
            Serial.println("connected");
            mqttClient.subscribe(TOPIC_RPC_REQUEST);
            mqttClient.subscribe(TOPIC_ATTRIBUTES_RESPONSE);

            // Request shared attributes
            StaticJsonDocument<200> doc;
            doc["sharedKeys"] = "schedule";
            String payload;
            serializeJson(doc, payload);
            mqttClient.publish(TOPIC_ATTRIBUTES_REQUEST, payload.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void MQTTClient::reconnect() {
    if (!mqttClient.connected()) {
        connect();
    }
}

void MQTTClient::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->callback(topic, payload, length);
    }
}

void MQTTClient::callback(char* topic, byte* payload, unsigned int length) {
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
        bool newState = params.as<bool>();
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
        Serial.println("Failed to parse attributes message");
        return;
    }

    JsonObject shared = doc["shared"];
    if (shared.containsKey("schedule")) {
        JsonObject scheduleObj = shared["schedule"];
        Schedule newSchedule;
        newSchedule.off_time = scheduleObj["off_time"];
        newSchedule.on_time = scheduleObj["on_time"];
        newSchedule.schedule_on = scheduleObj["schedule_on"];
        newSchedule.schedule_off = scheduleObj["schedule_off"];
        scheduleManager.updateSchedule(newSchedule);
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

    if ((ScheduleTask::getPowerState() || forceSend) && (current_time - last_data_send_time >= 60000)) {
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