#include "config.hpp"
#include "ProvisioningManager.hpp"
#include "DisplayManager.hpp"
#include <WiFi.h>
#include "wifi_provisioning/manager.h"   // wifi_prov_mgr_reset_sm_state_on_failure()
#include "esp_wifi.h"   

ProvisioningManager* ProvisioningManager::self_ = nullptr;

ProvisioningManager::ProvisioningManager(DisplayManager& display, MDNSManager& mdns, WebServerManager& webserver)
  : display_(display), mdns_(mdns), webserver_(webserver) { self_ = this; }

void ProvisioningManager::begin() {

  WiFi.onEvent(onWiFiEvent);



  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 4000) {
    delay(10);
  }

  if (WiFi.status() == WL_CONNECTED) {  // connected → skip BLE
      display_.showStatus("Wi-Fi OK!");
      return;
  }

  Serial.println("Starting BLE provisioning...");
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE,
                            WIFI_PROV_SCHEME_HANDLER_NONE,
                            WIFI_PROV_SECURITY_1,
                            POP, SERVICE_NAME, /*softAP key*/ nullptr,
                            nullptr,
                            RESET_PROV);
  WiFiProv.printQR(SERVICE_NAME, POP, "ble");

  display_.showStatus("Starting BLE provisioning...");
  drawQR();
}

void ProvisioningManager::drawQR() {
  // Build the JSON payload expected by Espressif’s mobile apps
  static char payload[120];
  snprintf(payload, sizeof(payload),
          "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}",
          SERVICE_NAME, POP);
  display_.showQR(payload);
}

void ProvisioningManager::onWiFiEvent(arduino_event_t* event)
{
  switch (event->event_id) {
    case ARDUINO_EVENT_PROV_START:
      self_->drawQR();
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      self_->display_.showStatus("Wi-Fi OK!");
      self_->mdns_.begin();
      self_->webserver_.begin();
      break;
    
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
      auto* disc = &event->event_info.wifi_sta_disconnected;
      Serial.printf("PROV STA disconnected, reason = %d", disc->reason);

      bool authError =
            disc->reason == WIFI_REASON_AUTH_EXPIRE          || 
            disc->reason == WIFI_REASON_AUTH_FAIL            ||  
            disc->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||
            disc->reason == WIFI_REASON_NO_AP_FOUND;

      if (authError) {
        //WiFi.disconnect(false, true);
        wifi_prov_mgr_reset_sm_state_on_failure();
        //wifi_prov_mgr_reset_provisioning();
        wifi_config_t cfg = {};
        esp_wifi_set_config(WIFI_IF_STA, &cfg);
      }
      break;
    }
    default:
      break;
  }
}
