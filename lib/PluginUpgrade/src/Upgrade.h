/* -*- mode: c++ -*-
 * kaleidoscope::plugin::LEDCapsLockLight -- Highlight CapsLock when active
 * Copyright (C) 2020  Dygma Lab S.L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Kaleidoscope.h>

namespace kaleidoscope {
namespace plugin {

class KeyScannerFlasher {
 public:
  enum Action {
    BEGIN            = 'B',
    INFO             = 'I',
    WRITE            = 'W',
    READ             = 'R',
    ERASE            = 'E',
    VALIDATE_PROGRAM = 'V',
    JUMP_ADDRESS     = 'J',
    FINISH           = 'F',
  };

  struct WriteAction {
    uint32_t addr;
    uint32_t size;
  };
  struct ReadAction {
    uint32_t addr;
    uint32_t size;
  };
  struct EraseAction {
    uint32_t addr;
    uint32_t size;
  };

  struct SealHeader {
    uint32_t deviceId;
    uint32_t version; /* Version of the Seal */
    uint32_t size;    /* Size of the Seal */
    uint32_t crc;     /* CRC of the Seal */
  };

  struct Seal {
    /* The header of the Seal */
    SealHeader header;

    /* The Seal */
    uint32_t programStart;
    uint32_t programSize;
    uint32_t programCrc;
    uint32_t programVersion;
  };

  struct InfoAction {
    uint32_t hardwareVersion;
    uint32_t flashStart;
    uint32_t validationSpaceStart;
    uint32_t programSpaceStart;
    uint32_t flashAvailable;
    uint32_t eraseAlignment;
    uint32_t writeAlignment;
    uint32_t maxTransmissionLength;
  };

  enum Side : uint8_t {
    RIGHT,
    LEFT,
  };

  bool getInfoFlasherKS(InfoAction &info_action);
  uint32_t sendWriteAction(WriteAction write_action, uint8_t *data);
  bool sendEraseAction(EraseAction erase_action);
  bool sendReadAction(ReadAction read_action);
  bool sendValidateProgram();
  bool sendBegin();
  bool sendFinish();
  bool sendJump(uint32_t address_to_jump);
  uint16_t readData(uint8_t *data, size_t size);
  void setSide(Side side);

  void setSideInfo(InfoAction info_action) {
    InfoAction &info = side_ ? infoLeft : infoRight;
    info             = info_action;
  };

  InfoAction getInfoAction() {
    InfoAction &info = side_ ? infoLeft : infoRight;
    return info;
  }

  uint32_t crc32(const void *ptr, uint32_t len);

 private:
  uint8_t address;
  InfoAction infoLeft{};
  InfoAction infoRight{};
  Side side_{0};
  uint32_t align(uint32_t val, uint32_t to);
  uint8_t readData(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit = false);
  uint8_t sendMessage(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit = false);
  uint8_t sendCommand(uint8_t address, uint8_t command, bool stopBit = false);
};

class Upgrade : public Plugin {
 public:
  // Kaleidoscope Focus library functions
  EventHandlerResult onFocusEvent(const char *command);
  // On Setup handler function
  EventHandlerResult onSetup();

  EventHandlerResult beforeReportingState();
  EventHandlerResult onKeyswitchEvent(Key &mapped_Key, KeyAddr key_addr, uint8_t key_state);

 private:
  KeyScannerFlasher key_scanner_flasher_{};
  bool activated = false;
  bool flashing  = false;
  uint16_t press_time{1};
  uint16_t pressed_time{0};
  bool serial_pre_activation = false;
  void resetSide(KeyScannerFlasher::Side side) const;
};

}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::Upgrade Upgrade;
