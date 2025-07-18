#include "Config.hpp"
#include "DisplayManager.hpp"

constexpr uint8_t I2C_ADDR = 0x3C;           // 0x3C or 0x3D
constexpr int     SDA_PIN  = 21;
constexpr int     SCL_PIN  = 22;

DisplayManager::DisplayManager() :
  oled_(I2C_ADDR, SDA_PIN, SCL_PIN),
  qrcode_(&oled_) {}

void DisplayManager::begin() {
  oled_.init();
  oled_.flipScreenVertically();         // looks nicer on many panels
  oled_.setBrightness(0xFF);
}

void DisplayManager::showQR(const char* payload) {
  oled_.clear();
  qrcode_.init();                       // black background, white QR
  qrcode_.create(payload);              // auto-scales to fit 128×64
}

void DisplayManager::showStatus(const String& msg) {
  oled_.clear();
  oled_.drawString(0, 0, msg.c_str());
  oled_.display();
}
