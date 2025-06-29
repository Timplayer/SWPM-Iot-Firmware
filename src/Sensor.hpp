#pragma once
#include <Arduino.h>
#include "DisplayManager.hpp"

/**
 *  -------------------------------------------------------------
 *  RadarSensor – high-level helper for an XY-MOTION radar module
 *
 *  * Handles all control-frame commands (FW, SN, register, parameter)
 *  * Parses the streaming distance/energy frames
 *  * Exposes a simple public API (begin, loop, query…)
 *
 *  Radar module is expected on the serial port passed to the ctor
 *  Debug output goes to the second Stream passed to the ctor
 *  -------------------------------------------------------------
 */
class RadarSensor
{
public:
  /// Result structure of a streaming frame
  struct Frame {
    bool     targetPresent;
    uint16_t distance_mm;
    uint16_t energy[16];
  };

  /**
   * @param radarPort  serial connected to the radar (default = Serial2)
   * @param dbgPort    serial for debug logging  (default = Serial)
   * @param disp       pointer to an optional DisplayManager (may be nullptr)
   */
  explicit RadarSensor(HardwareSerial& radarPort  = Serial2,
                       Stream&         dbgPort    = Serial,
                       DisplayManager* disp       = nullptr);

  /// Initialise serial ports, display, etc.
  void begin(uint32_t baud = RADAR_BAUD);

  /// Must be called regularly inside loop(); handles stream parsing
  void loop();

  /* ---- control-frame helpers -------------------------------- */
  bool queryFirmwareVersion(char* out, uint8_t maxLen);
  bool querySerialNumber(uint16_t& serial);
  bool readConfig();
  bool readRegisterWord(uint16_t chip, uint16_t addr, uint16_t& value);
  bool readParameter(uint16_t id, uint32_t& val);
  /**
   * Change the radar’s reporting mode (command 0x0012)
   *
   * @param mode  value written to the radar (e.g. 0x00000004 = report-only
   *              when target present, 0x00000064 = normal continuous mode)
   * @return true on ACK, false on timeout or error status
   */
  bool setReportMode(uint32_t mode);

  bool getIsPerson() const
  {
    return is_person;
  }
private:
  /* fixed tokens */
  static const uint8_t DET_HDR[4];
  static const uint8_t DET_TAIL[4];
  static const uint8_t CTRL_HDR[4];
  static const uint8_t CTRL_TAIL[4];

  static constexpr uint32_t RADAR_BAUD     = 115200;
  static constexpr uint16_t CMD_TIMEOUT_MS = 1000;
  static constexpr uint16_t DETECT_PAYLOAD = 35;     // 1+2+32

  /* ---- low-level helpers ------------------------------------ */
  bool sendCmd(const uint8_t* frame, uint8_t frameLen,
               uint8_t* resp, uint8_t& respLen,
               uint16_t timeout = CMD_TIMEOUT_MS);

  uint8_t writeCmd(const uint8_t* frame, uint8_t frameLen,
                   uint8_t* buf, uint8_t bufLen);

  void parseStream();        ///< consumes Serial2 bytes → Frame
  void handleFrame(const Frame& rf);

  /* ---- streaming parser state ------------------------------- */
  enum class S : uint8_t { FIND_HDR, LEN_L, LEN_H, PAYLOAD, FIND_TAIL };

  S        _state   = S::FIND_HDR;
  uint8_t  _hdrIdx  = 0, _tailIdx = 0;
  uint16_t _payLen  = 0, _payCnt  = 0;
  uint8_t  _payload[DETECT_PAYLOAD];

  /* ---- I/O dependencies ------------------------------------- */
  HardwareSerial& _radar;
  Stream&         _dbg;
  DisplayManager* _display;

  bool is_person = false;
};
