#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "Sensor.hpp"

class MqttManager {
public:
    MqttManager();
    void begin(const char* mqttServer, int mqttPort, const char* mqttTopic, const char* mqttUser, const char* mqttPass);
    void loop();
    void publishSensorData(const RadarSensor::Frame& frame);

private:
    const char* _mqttServer;
    int _mqttPort;
    const char* _mqttTopic;
    const char* _mqttUser;
    const char* _mqttPass;
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    bool _mqttConfigured = false;

    void reconnect();
};
