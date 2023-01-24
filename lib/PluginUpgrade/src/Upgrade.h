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
    INFO          = 'I',
    WRITE         = 'W',
    READ          = 'R',
    ERASE         = 'E',
    VALIDATE_SEAL = 'V',
    GO            = 'G',
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

  struct ValidateSealAction {
    uint32_t addr;
    uint32_t size;
  };

  struct InfoAction {
    uint32_t hardwareVersion;
    uint32_t flashStart;
    uint32_t validationSpaceStart;
    uint32_t validationSpaceSize;
    uint32_t flashSize;
    uint32_t eraseAlignment;
    uint32_t writeAlignment;
    uint32_t maxDataLength;
  };

  bool getInfoFlasherKS(InfoAction &info_action);
  bool sendWriteAction(WriteAction write_action);
  bool sendEraseAction(EraseAction erase_action);
  bool sendReadAction(ReadAction read_action);
  bool sendValidateSealAction(ValidateSealAction validate_seal_action);
  bool sendGo(uint32_t address_to_jump);
  bool sendMessage(uint8_t *data,size_t size);
  uint16_t readData(uint8_t *data,size_t size);
  void setSide(bool side);

 private:
  uint8_t address;
  uint32_t crc32(const void *ptr, uint32_t len);
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
  bool flashing = false;
  uint16_t press_time{1500};
  uint16_t pressed_time{0};
  bool serial_pre_activation = false;
};

}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::Upgrade Upgrade;
