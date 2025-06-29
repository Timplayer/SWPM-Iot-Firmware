#include "mDNS.hpp"
#include <ESPmDNS.h>
#include <WiFi.h>

MDNSManager::MDNSManager(const char *serviceType,
                         uint16_t    port,
                         const char *model,
                         const char *fw,
                         const char *pop)
: service_(serviceType),
  port_   (port),
  model_  (model),
  fw_     (fw),
  pop_    (pop)
{
    /* Make host name unique: “esp32-<last-3-bytes-of-MAC>” */
    uint32_t macLast3 = (uint32_t)ESP.getEfuseMac() & 0xFFFFFF;
    char buf[20];
    sprintf(buf, "esp32-%06X", macLast3);
    host_ = String(buf);
}

bool MDNSManager::begin()
{
    if (!MDNS.begin(host_.c_str())) {
        Serial.println("⚠️  mDNS init failed");
        return false;
    }

    MDNS.addService("sensor", "tcp", 80);
    MDNS.addServiceTxt("sensor", "tcp", "info", "mDNS test");


    Serial.printf("mDNS up: http://%s.local/\n", host_.c_str());
    return true;
}

void MDNSManager::stop(){}