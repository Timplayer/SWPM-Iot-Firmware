#include "Sensor.hpp"
#include "MqttManager.hpp"

const uint8_t RadarSensor::DET_HDR[4]  = { 0xF4, 0xF3, 0xF2, 0xF1 };
const uint8_t RadarSensor::DET_TAIL[4] = { 0xF8, 0xF7, 0xF6, 0xF5 };
const uint8_t RadarSensor::CTRL_HDR[4] = { 0xFD, 0xFC, 0xFB, 0xFA };
const uint8_t RadarSensor::CTRL_TAIL[4]= { 0x04, 0x03, 0x02, 0x01 };

/* ---------------- ctor / begin ------------------------------- */
RadarSensor::RadarSensor(HardwareSerial& radarPort,
                         Stream&         dbgPort,
                         DisplayManager* disp)
  : _radar(radarPort), _dbg(dbgPort), _display(disp) {}

void RadarSensor::begin(uint32_t baud)
{
  _radar.begin(baud);
}

/* ---------------- public helpers ----------------------------- */
bool RadarSensor::queryFirmwareVersion(char* out, uint8_t maxLen)
{
  static const uint8_t CMD_FW[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x02,0x00,  0x00,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  uint8_t buf[64] = {0};
  uint8_t idx = writeCmd(CMD_FW, sizeof(CMD_FW), buf, sizeof(buf));
  if (idx < 22) return false;

  uint16_t payloadLen = buf[4] | (uint16_t(buf[5]) << 8);
  uint16_t verLen = buf[10] | (uint16_t(buf[11]) << 8);
  if (verLen == 0 || verLen > maxLen-1 || verLen > payloadLen-6) return false;

  memcpy(out, &buf[12], verLen);
  out[verLen] = '\0';
  return true;
}

bool RadarSensor::querySerialNumber(uint16_t& serial)
{
  static const uint8_t CMD_SN[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x02,0x00,  0x11,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  uint8_t buf[32];
  uint8_t len = writeCmd(CMD_SN, sizeof(CMD_SN), buf, sizeof(buf));

  if (len < 16 || buf[7] != 0x11 || buf[8] != 0x01) return false;
  if (buf[9] || buf[10]) return false;

  serial = buf[13] | (uint16_t(buf[14]) << 8);
  return true;
}

bool RadarSensor::readConfig()
{
  /* …(unchanged – just wrapped) */
  static const uint8_t CMD_CONFIG_DISTANCE_MIN[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x04,0x00,  0x08,0x00, 0x00,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  static const uint8_t CMD_CONFIG_DISTANCE_MAX[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x04,0x00,  0x08,0x00, 0x01,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  static const uint8_t CMD_CONFIG_ACTIVE_FRAME[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x04,0x00,  0x08,0x00, 0x02,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  static const uint8_t CMD_CONFIG_INACTIVE_FRAMES[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x04,0x00,  0x08,0x00, 0x03,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  static const uint8_t CMD_CONFIG_DELAY[] PROGMEM =
     { CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
       0x04,0x00,  0x08,0x00, 0x04,0x00,
       CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3] };

  uint8_t buf[32];
  writeCmd(CMD_CONFIG_DISTANCE_MIN, sizeof(CMD_CONFIG_DISTANCE_MIN), buf, sizeof(buf));
  writeCmd(CMD_CONFIG_DISTANCE_MAX, sizeof(CMD_CONFIG_DISTANCE_MAX), buf, sizeof(buf));
  writeCmd(CMD_CONFIG_ACTIVE_FRAME, sizeof(CMD_CONFIG_ACTIVE_FRAME), buf, sizeof(buf));
  writeCmd(CMD_CONFIG_INACTIVE_FRAMES, sizeof(CMD_CONFIG_INACTIVE_FRAMES), buf, sizeof(buf));
  uint8_t len = writeCmd(CMD_CONFIG_DELAY, sizeof(CMD_CONFIG_DELAY), buf, sizeof(buf));

  return len > 0;
}

bool RadarSensor::readRegisterWord(uint16_t chip, uint16_t addr, uint16_t& value)
{
  uint8_t cmd[16] = {
    CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
    0x06,0x00,
    0x02,0x00,
    uint8_t(chip & 0xFF), uint8_t(chip >> 8),
    uint8_t(addr & 0xFF), uint8_t(addr >> 8),
    CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3]
  };

  uint8_t buf[32]; uint8_t len = sizeof(buf);
  if (!sendCmd(cmd, sizeof(cmd), buf, len)) return false;

  if (len < 16 || buf[7] != 0x02 || buf[8] != 0x01) return false;
  if (buf[9] || buf[10]) return false;

  value = buf[11] | (uint16_t(buf[12]) << 8);
  return true;
}

bool RadarSensor::readParameter(uint16_t id, uint32_t& val)
{
  uint8_t cmd[14] = {
    CTRL_HDR[0],CTRL_HDR[1],CTRL_HDR[2],CTRL_HDR[3],
    0x04,0x00,
    0x08,0x00,
    uint8_t(id & 0xFF), uint8_t(id >> 8),
    CTRL_TAIL[0],CTRL_TAIL[1],CTRL_TAIL[2],CTRL_TAIL[3]
  };

  uint8_t buf[40]; uint8_t len = sizeof(buf);
  if (!sendCmd(cmd, sizeof(cmd), buf, len)) return false;

  if (len < 18 || buf[7] != 0x08 || buf[8] != 0x01) return false;
  if (buf[9] || buf[10]) return false;

  val =  buf[11] |
        (uint32_t(buf[12]) << 8) |
        (uint32_t(buf[13]) << 16) |
        (uint32_t(buf[14]) << 24);
  return true;
}

bool RadarSensor::setReportMode(uint32_t mode)
{
  /* build frame:
   *   HDR           4
   *   LEN           2   (0x0008)
   *   CMD           2   (0x0012)
   *   PAYLOAD       4   (uint32 mode)
   *   TAIL          4
   *   --------------
   *                 16 bytes total
   */
  uint8_t cmd[16] = {
    CTRL_HDR[0], CTRL_HDR[1], CTRL_HDR[2], CTRL_HDR[3],
    0x08, 0x00,                         // length = 8
    0x12, 0x00,                         // command 0x0012
    uint8_t(mode & 0xFF),
    uint8_t((mode >> 8)  & 0xFF),
    uint8_t((mode >> 16) & 0xFF),
    uint8_t((mode >> 24) & 0xFF),
    CTRL_TAIL[0], CTRL_TAIL[1], CTRL_TAIL[2], CTRL_TAIL[3]
  };

  uint8_t buf[32];
  uint8_t len = writeCmd(cmd, sizeof(cmd), buf, sizeof(buf));

  /* expect at least header+len+cmd+status+tail = 16 bytes
     status is buf[9] | (buf[10] << 8) → must be 0 */
  return len >= 16 &&
         buf[7]  == 0x12 &&  // echoed command LSB
         buf[8]  == 0x01 &&  //   "     "     MSB (0x0001 = answer)
         buf[9]  == 0x00 &&  // status LSB
         buf[10] == 0x00;    // status MSB
}

/* ---------------- loop / stream parser ----------------------- */
void RadarSensor::loop()
{
  parseStream();
}

void RadarSensor::parseStream()
{
  while (_radar.available()) {
    uint8_t b = _radar.read();

    switch (_state) {
      case S::FIND_HDR:
        if (b == DET_HDR[_hdrIdx]) {
          if (++_hdrIdx == 4) { _hdrIdx = 0; _state = S::LEN_L; }
        } else _hdrIdx = 0;
        break;

      case S::LEN_L: _payLen = b; _state = S::LEN_H; break;

      case S::LEN_H:
        _payLen |= uint16_t(b) << 8;
        if (_payLen != DETECT_PAYLOAD) _state = S::FIND_HDR;
        else { _payCnt = 0; _state = S::PAYLOAD; }
        break;

      case S::PAYLOAD:
        _payload[_payCnt++] = b;
        if (_payCnt >= _payLen) { _tailIdx = 0; _state = S::FIND_TAIL; }
        break;

      case S::FIND_TAIL:
        if (b == DET_TAIL[_tailIdx]) {
          if (++_tailIdx == 4) {
            Frame rf;
            rf.targetPresent = _payload[0];
            rf.distance_mm   = _payload[1] | (uint16_t(_payload[2]) << 8);
            for (uint8_t i = 0; i < 16; ++i) {
              rf.energy[i] = _payload[3 + i * 2] |
                             (uint16_t(_payload[4 + i * 2]) << 8);
            }
            handleFrame(rf);
            _state = S::FIND_HDR;
          }
        } else _state = S::FIND_HDR;
        break;
    }
  }
}

void RadarSensor::handleFrame(const Frame& rf)
{
  if (_mqttManager) {
    _mqttManager->publishSensorData(rf);
  }

  _dbg.print(F("Target: "));
  _dbg.print(rf.targetPresent ? F("YES") : F("no "));
  _dbg.print(F("  Dist: "));
  _dbg.print(rf.distance_mm);
  _dbg.print(F(" mm  Energies:"));
  for (uint8_t i = 0; i < 16; ++i) { _dbg.print(' '); _dbg.print(rf.energy[i]); }
  _dbg.println();

  is_person = rf.targetPresent;

  if (_display) {
    _display->showStatus(String("Target: ") +
                         (rf.targetPresent ? "YES" : "no ") +
                         "  Dist: " + String(rf.distance_mm) + " mm");
  }
}

/* ---------------- low-level helpers -------------------------- */
bool RadarSensor::sendCmd(const uint8_t* frame, uint8_t frameLen,
                          uint8_t* resp, uint8_t& respLen,
                          uint16_t timeout)
{
  _radar.write(frame, frameLen);
  unsigned long t0 = millis();
  uint8_t idx = 0;

  while ((millis() - t0) < timeout && idx < respLen) {
    if (_radar.available()) {
      resp[idx++] = _radar.read();
      if (idx >= 4 &&
          resp[idx - 4] == CTRL_TAIL[0] &&
          resp[idx - 3] == CTRL_TAIL[1] &&
          resp[idx - 2] == CTRL_TAIL[2] &&
          resp[idx - 1] == CTRL_TAIL[3])
      {
        respLen = idx;
        return true;          // tail reached
      }
    }
  }
  respLen = idx;
  return false;               // timeout
}

uint8_t RadarSensor::writeCmd(const uint8_t* frame, uint8_t frameLen,
                              uint8_t* buf, uint8_t bufLen)
{
  _radar.write(frame, frameLen);
  _radar.flush();

  uint8_t idx = 0;
  unsigned long t0 = millis();

  while ((millis() - t0) < CMD_TIMEOUT_MS && idx < bufLen) {
    if (_radar.available()) {
      buf[idx++] = _radar.read();
      if (idx >= 4 &&
          buf[idx - 4] == CTRL_TAIL[0] &&
          buf[idx - 3] == CTRL_TAIL[1] &&
          buf[idx - 2] == CTRL_TAIL[2] &&
          buf[idx - 1] == CTRL_TAIL[3])
      {
        break;                // full frame read
      }
    }
  }

  for (uint8_t i = 0; i < idx; ++i) {
    _dbg.printf("%02X ", buf[i]);
  }
  _dbg.println();

  return idx;
}
