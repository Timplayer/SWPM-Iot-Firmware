#include "Config.hpp"
#include <EEPROM.h>

// Define the size of the EEPROM storage
#define EEPROM_SIZE 512

Config::Config() {
    // Initialize with default values
    mqttServer = "";
    mqttPort = 1883;
    mqttTopic = "";
    mqttUser = "tim";
    mqttPass = "tim";
}

void Config::load() {
    EEPROM.begin(EEPROM_SIZE);
    // Read MQTT Server
    char serverBuffer[100];
    EEPROM.get(0, serverBuffer);
    mqttServer = String(serverBuffer);

    // Read MQTT Port
    EEPROM.get(100, mqttPort);

    // Read MQTT Topic
    char topicBuffer[100];
    EEPROM.get(104, topicBuffer);
    mqttTopic = String(topicBuffer);

    // Read MQTT User
    char userBuffer[100];
    EEPROM.get(204, userBuffer);
    mqttUser = String(userBuffer);

    // Read MQTT Pass
    char passBuffer[100];
    EEPROM.get(304, passBuffer);
    mqttPass = String(passBuffer);

    EEPROM.end();
}

void Config::save() {
    EEPROM.begin(EEPROM_SIZE);
    // Write MQTT Server
    EEPROM.put(0, mqttServer.c_str());

    // Write MQTT Port
    EEPROM.put(100, mqttPort);

    // Write MQTT Topic
    EEPROM.put(104, mqttTopic.c_str());

    // Write MQTT User
    EEPROM.put(204, mqttUser.c_str());

    // Write MQTT Pass
    EEPROM.put(304, mqttPass.c_str());

    EEPROM.commit();
    EEPROM.end();
}
