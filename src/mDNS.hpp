#pragma once
#include <Arduino.h>

/**
 *  MDNSManager – advertise an ESP32 over mDNS/DNS-SD
 *
 *  Usage:
 *      #include "MDNSManager.h"
 *      MDNSManager mdns("sensor", 80, "WX-AQ1", "1.0.3", POP);
 *
 *      // …after Wi-Fi is up:
 *      mdns.begin();     // prints:  mDNS up: http://esp32-42a1.local/
 */
class MDNSManager {
public:
    MDNSManager(const char *serviceType = "sensor",
                uint16_t    port        = 80,
                const char *model       = "WX-AQ1",
                const char *fw          = "1.0.0",
                const char *pop         = "abcd1234");

    /** Start (or restart) the mDNS service.
        @return true on success, false if MDNS.begin() failed.            */
    bool begin();
    void stop();
    /** Host name announced on the LAN, e.g. “esp32-42a1”.                */
    const String &hostname() const { return host_; }

private:
    String      host_;
    const char *service_;
    uint16_t    port_;
    const char *model_;
    const char *fw_;
    const char *pop_;
};
