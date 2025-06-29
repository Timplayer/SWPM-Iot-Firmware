#pragma once
#include <WiFiProv.h>

#include "mDNS.hpp"
#include "DisplayManager.hpp"
#include "Webserver.hpp"

class ProvisioningManager {
public:
  explicit ProvisioningManager(DisplayManager& display,MDNSManager& mdns, WebServerManager& webserver);

  void begin();               // start BLE provisioning (if needed)
private:
  static void onWiFiEvent(arduino_event_t* event);
  void drawQR();              // helper

  static ProvisioningManager* self_; // for static callback glue

  DisplayManager& display_;
  MDNSManager& mdns_;    
  WebServerManager& webserver_; // optional, not used here

};
