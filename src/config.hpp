#pragma once
#include <cstdint>

// ---------- Wi-Fi / BLE provisioning ----------
constexpr char  POP[]          = "abcd1234"; // proof-of-possession
constexpr char  SERVICE_NAME[] = "PROV_123"; // must start with "PROV_"
constexpr bool  RESET_PROV     = false;      // keep creds after first boot

// ---------- OLED ----------
constexpr uint8_t I2C_ADDR = 0x3C;           // 0x3C or 0x3D
constexpr int     SDA_PIN  = 21;
constexpr int     SCL_PIN  = 22;