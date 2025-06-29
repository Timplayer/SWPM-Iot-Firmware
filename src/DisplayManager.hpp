#pragma once
#include <SSD1306.h>       // ThingPulse SSD1306 driver
#include <qrcodeoled.h>     // Lightweight QR generator

class DisplayManager {
public:
  DisplayManager();

  void begin();                           // init screen
  void showQR(const char* payload);       // provisioning QR
  void showStatus(const String& msg);       // e.g. “Wi-Fi OK!”
private:
  SSD1306   oled_;
  QRcodeOled qrcode_;
};
