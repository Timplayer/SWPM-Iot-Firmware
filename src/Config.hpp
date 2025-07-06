#pragma once
#include <Arduino.h>

//constexpr char  POP[]          = "abcd1234"; // proof-of-possession
//constexpr char  SERVICE_NAME[] = "PROV_123"; // must start with "PROV_"
//constexpr bool  RESET_PROV     = false;      // keep creds after first boot

class Config {
public:
    String mqttServer;
    int mqttPort;
    String mqttTopic;
    String mqttUser;
    String mqttPass;

    String serviceName = "PROV_123"; // Default service name
    String pop = "abcd1234";        // Default proof of possession
    bool resetProv = false;         // Reset provisioning after first boot

    Config();
    void load();
    void save();
};
