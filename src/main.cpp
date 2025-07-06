
#include "Config.hpp"            // your constants
#include "DisplayManager.hpp"    // wrapped OLED driver
#include "ProvisioningManager.hpp"
#include "mDNS.hpp"           // mDNS/DNS-SD
#include "Sensor.hpp"
#include "Webserver.hpp"      // tiny HTTP server
#include "MqttManager.hpp"
#include "Config.hpp"         // new Config class
#include <WiFi.h>            // Wi-Fi client

DisplayManager      display;
Config              config; // Global config object

MDNSManager mdns("sensor",          // service type
                 80,                // port
                 "Test1234",      // e.g. "WX-AQ1"
                 "1.0.3",        // e.g. "1.0.3"
                 "abcd1234");    // e.g. "abcd1234"

WebServerManager webserver(80); // HTTP server on port 80
ProvisioningManager prov(display, mdns, webserver, config);
RadarSensor sensor(Serial2, Serial, &display);
MqttManager mqtt; // MqttManager will be initialized with config later

void setup()
{
  Serial.begin(115200);
  display.begin();
  config.load(); // Load configuration at startup
  mqtt.begin(config.mqttServer.c_str(), config.mqttPort, config.mqttTopic.c_str(), config.mqttUser.c_str(), config.mqttPass.c_str()); // Initialize MqttManager with loaded config
  prov.begin();
  sensor.begin();
  sensor.setMqttManager(&mqtt);

  webserver.setSensorCallback([&]() {
    // Return a JSON string with sensor data
    String json = "{\"is_person\":";
    json += sensor.getIsPerson() ? "true" : "false";
    json += "}";
    return json;
  });

  webserver.setMqttConfigCallback([&](String server, int port, String topic, String user, String pass) {
    config.mqttServer = server;
    config.mqttPort = port;
    config.mqttTopic = topic;
    config.mqttUser = user;
    config.mqttPass = pass;
    config.save();
    mqtt.begin(config.mqttServer.c_str(), config.mqttPort, config.mqttTopic.c_str(), config.mqttUser.c_str(), config.mqttPass.c_str());
  });

  char fw[32];
  if (sensor.queryFirmwareVersion(fw, sizeof(fw)))
    Serial.printf("FW : %s\n", fw);

    static const uint8_t CMD_REPORT_MODE[] PROGMEM =
     { 0xFD,0xFC,0xFB,0xFA,  0x08,0x00,  0x12,0x00, 0x00,0x00, 0x04,0x00,0x00,0x00,  0x04,0x03,0x02,0x01 };

  Serial2.write(CMD_REPORT_MODE, sizeof(CMD_REPORT_MODE));
}

void loop()
{

      static uint32_t last = 0;
      if (millis() - last > 1000) {
          last = millis();

          if(webserver.reset){
            Serial.println("Reset!");
            webserver.stop();
            WiFi.disconnect(false, true);
            wifi_prov_mgr_reset_provisioning();
            mdns.stop();
            ESP.restart();
            return;
          }

          if (WiFi.status() == WL_CONNECTED) {
            mqtt.loop();
            sensor.loop();
            Serial.println("Wi-Fi OK – running main app");
          }else{
            Serial.println("Wi-Fi not connected yet");
          }
      }
}
