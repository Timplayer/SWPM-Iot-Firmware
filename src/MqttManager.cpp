#include "MqttManager.hpp"

MqttManager::MqttManager()
    : _mqttClient(_wifiClient) {}

void MqttManager::begin(const char* mqttServer, int mqttPort, const char* mqttTopic, const char* mqttUser, const char* mqttPass) {
    if (mqttServer && strlen(mqttServer) > 0) {
        _mqttServer = mqttServer;
        _mqttPort = mqttPort;
        _mqttTopic = mqttTopic;
        _mqttUser = mqttUser;
        _mqttPass = mqttPass;
        _mqttClient.setServer(_mqttServer, _mqttPort);
        _mqttConfigured = true;
    } else {
        _mqttConfigured = false;
    }
}

void MqttManager::loop() {
    if (_mqttConfigured && WiFi.status() == WL_CONNECTED) {
        if (!_mqttClient.connected()) {
            reconnect();
        }
        _mqttClient.loop();
    }
}

void MqttManager::publishSensorData(const RadarSensor::Frame& frame) {
    if (_mqttClient.connected()) {
        String payload = "{\"targetPresent\":";
        payload += frame.targetPresent ? "true" : "false";
        payload += ",\"distance_mm\":";
        payload += frame.distance_mm;
        payload += "}";
        _mqttClient.publish(_mqttTopic, payload.c_str());
    }
}

void MqttManager::reconnect() {
        Serial.print("Attempting MQTT connection...");
        if (_mqttUser && strlen(_mqttUser) > 0) {
            if (_mqttClient.connect("ESP32Client", _mqttUser, _mqttPass)) {
                Serial.println("connected");
            } else {
                Serial.print("failed, rc=");
                Serial.print(_mqttClient.state());
                Serial.println(" try again in 5 seconds");
            }
        } else {
            if (_mqttClient.connect("ESP32Client")) {
                Serial.println("connected");
            } else {
                Serial.print("failed, rc=");
                Serial.print(_mqttClient.state());
                Serial.println(" try again in 5 seconds");
            }
        }
    
}

