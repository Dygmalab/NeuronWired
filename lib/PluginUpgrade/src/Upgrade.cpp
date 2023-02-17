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

#include <hardware/dma.h>
#include "Upgrade.h"
#include "Wire.h"
#include "kaleidoscope/plugin/FocusSerial.h"
#include "../../EEPROM/src/EEPROM.h"
#include "LedModeSerializable.h"
#define WIRE_ Wire1

namespace kaleidoscope {
namespace plugin {

#define sendStruct(X)                   \
  for (int i = 0; i < sizeof(X); ++i) { \
    ::Focus.send(((uint8_t *)&(X))[i]); \
  }
#define readStruct(X)                   \
  for (int i = 0; i < sizeof(X); ++i) { \
    ::Focus.read(((uint8_t *)&(X))[i]); \
  }

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t KeyScannerFlasher::sendCommand(uint8_t address, uint8_t command, bool stopBit) {
  watchdog_update();
  WIRE_.beginTransmission(address);
  WIRE_.write(command);
  return WIRE_.endTransmission(stopBit);
}

uint8_t KeyScannerFlasher::sendMessage(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit) {
  watchdog_update();
  WIRE_.beginTransmission(address);
  WIRE_.write(message, lenMessage);
  return WIRE_.endTransmission(stopBit);
}

uint8_t KeyScannerFlasher::readData(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit) {
  watchdog_update();
  WIRE_.requestFrom(address, lenMessage, false);
  return WIRE_.readBytes(message, lenMessage);
}

uint32_t KeyScannerFlasher::align(uint32_t val, uint32_t to) {
  uint32_t r = val % to;
  return r ? val + (to - r) : val;
}

uint32_t KeyScannerFlasher::crc32(const void *ptr, uint32_t len) {
  uint32_t dummy_dest, crc;

  int channel          = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  channel_config_set_sniff_enable(&c, true);

  // Seed the CRC calculation
  dma_hw->sniff_data = 0xffffffff;

  // Mode 1, then bit-reverse the result gives the same result as
  dma_sniffer_enable(channel, 0x1, true);
  dma_hw->sniff_ctrl |= DMA_SNIFF_CTRL_OUT_REV_BITS;

  dma_channel_configure(channel, &c, &dummy_dest, ptr, len / 4, true);

  dma_channel_wait_for_finish_blocking(channel);

  // Read the result before resetting
  crc = dma_hw->sniff_data ^ 0xffffffff;

  dma_sniffer_disable();
  dma_channel_unclaim(channel);

  return crc;
}

void KeyScannerFlasher::setSide(Side side) {
  side_                      = side;
  uint8_t left_boot_address  = Runtime.device().side.left_boot_address;
  uint8_t right_boot_address = Runtime.device().side.right_boot_address;
  address                    = side ? left_boot_address : right_boot_address;
}

bool KeyScannerFlasher::getInfoFlasherKS(InfoAction &info_action) {
  if (sendCommand(address, Action::INFO)) return false;
  readData(address, (uint8_t *)&info_action, sizeof(info_action));
  return true;
}

bool KeyScannerFlasher::sendEraseAction(KeyScannerFlasher::EraseAction erase_action) {
  if (sendCommand(address, Action::ERASE)) return false;
  if (sendMessage(address, (uint8_t *)&erase_action, sizeof(erase_action))) return false;
  uint8_t done = false;
  readData(address, (uint8_t *)&done, sizeof(done));
  return done;
}

uint32_t KeyScannerFlasher::sendWriteAction(KeyScannerFlasher::WriteAction write_action, uint8_t *data) {
  if (sendCommand(address, Action::WRITE)) {
    return 0;
  }
  if (sendMessage(address, (uint8_t *)&write_action, sizeof(write_action))) {
    return 0;
  }

  sleep_us(500);
  if (sendMessage(address, data, write_action.size)) {
    return 0;
  }

  uint32_t crc32 = 0;
  readData(address, (uint8_t *)&crc32, sizeof(crc32));
  return crc32;
}

bool KeyScannerFlasher::sendReadAction(KeyScannerFlasher::ReadAction read_action) {
  if (sendCommand(address, Action::READ)) return false;
  if (sendMessage(address, (uint8_t *)&read_action, sizeof(read_action))) return false;
  return true;
}

uint16_t KeyScannerFlasher::readData(uint8_t *data, size_t size) {
  return readData(address, data, size);
}
bool KeyScannerFlasher::sendJump(uint32_t address_to_jump) {
  if (sendCommand(address, Action::JUMP_ADDRESS)) return false;
  if (sendMessage(address, (uint8_t *)&address_to_jump, sizeof(address_to_jump))) return false;
  return true;
}

bool KeyScannerFlasher::sendValidateProgram() {
  if (sendCommand(address, Action::VALIDATE_PROGRAM)) return false;
  uint8_t done = false;
  readData(address, (uint8_t *)&done, sizeof(done));
  return done;
}

bool KeyScannerFlasher::sendBegin() {
  if (sendCommand(address, Action::BEGIN)) return false;
  return true;
}

bool KeyScannerFlasher::sendFinish() {
  if (sendCommand(address, Action::FINISH)) return false;
  return true;
}

EventHandlerResult Upgrade::onFocusEvent(const char *command) {
  if (::Focus.handleHelp(command,
                         PSTR(
                           "upgrade.start\n"
                           "upgrade.neuron\n"
                           "upgrade.end\n"
                           "upgrade.keyscanner.beginRight\n"  //Choose the side Right
                           "upgrade.keyscanner.beginLeft\n"   //Choose the side Left
                           "upgrade.keyscanner.getInfo\n"     //Version, and CRC, and is connected and start address, program is OK
                           "upgrade.keyscanner.sendWrite\n"   //Write //{Address size DATA crc} Check if we are going to support --? true false
                           "upgrade.keyscanner.validate\n"    //Check validity
                           "upgrade.keyscanner.finish\n"      //Finish bootloader
                           "upgrade.keyscanner.sendStart")))  //Start main application and check validy //true false

    return EventHandlerResult::OK;
  //TODO set numbers ot PSTR

  if (strncmp_P(command, PSTR("upgrade."), 8) != 0)
    return EventHandlerResult::OK;


  if (strcmp_P(command + 8, PSTR("start")) == 0) {
    serial_pre_activation = true;
  }

  if (strcmp_P(command + 8, PSTR("neuron")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    EEPROM.erase();
    Runtime.rebootBootloader();
  }

  if (strcmp_P(command + 8, PSTR("isReady")) == 0) {
    ::Focus.send(flashing);
  }

  if (strcmp_P(command + 8, PSTR("end")) == 0) {
    serial_pre_activation = false;
    activated             = false;
    flashing              = false;
    pressed_time          = 0;
  }

  if (strncmp_P(command + 8, PSTR("keyscanner."), 11) != 0)
    return EventHandlerResult::OK;

  if (strcmp_P(command + 8 + 11, PSTR("beginRight")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    key_scanner_flasher_.setSide(KeyScannerFlasher::RIGHT);
    resetSide(KeyScannerFlasher::LEFT);
    resetSide(KeyScannerFlasher::RIGHT);
    bool right_side = key_scanner_flasher_.sendBegin();
    if (right_side) {
      Focus.send(true);
      return EventHandlerResult::EVENT_CONSUMED;
    }

    Focus.send(false);
    return EventHandlerResult::ERROR;
  }

  if (strcmp_P(command + 8 + 11, PSTR("beginLeft")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    key_scanner_flasher_.setSide(KeyScannerFlasher::LEFT);
	resetSide(KeyScannerFlasher::RIGHT);
	resetSide(KeyScannerFlasher::LEFT);
    bool right_side = key_scanner_flasher_.sendBegin();
    if (right_side) {
      Focus.send(true);
      return EventHandlerResult::EVENT_CONSUMED;
    }

    Focus.send(false);
    return EventHandlerResult::ERROR;
  }

  if (strcmp_P(command + 8 + 11, PSTR("getInfo")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    KeyScannerFlasher::InfoAction info{};
    if (!key_scanner_flasher_.getInfoFlasherKS(info)) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    key_scanner_flasher_.setSideInfo(info);

    KeyScannerFlasher::ReadAction read{info.validationSpaceStart, sizeof(KeyScannerFlasher::Seal)};
    KeyScannerFlasher::Seal seal{};
    key_scanner_flasher_.sendReadAction(read);
    if (key_scanner_flasher_.readData((uint8_t *)&seal, sizeof(KeyScannerFlasher::Seal)) != sizeof(KeyScannerFlasher::Seal)) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    Focus.send(info.hardwareVersion);
    Focus.send(info.flashStart);
    Focus.send(seal.programVersion);
    Focus.send(seal.programCrc);
    Focus.send(true);
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendRead")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    Runtime.device().side.prepareForFlash();
    KeyScannerFlasher::InfoAction info{};
    if (!key_scanner_flasher_.getInfoFlasherKS(info)) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    key_scanner_flasher_.setSideInfo(info);

    KeyScannerFlasher::ReadAction read{info.validationSpaceStart, sizeof(KeyScannerFlasher::Seal)};
    KeyScannerFlasher::Seal seal{};
    key_scanner_flasher_.sendReadAction(read);
    if (key_scanner_flasher_.readData((uint8_t *)&seal, sizeof(KeyScannerFlasher::Seal)) != sizeof(KeyScannerFlasher::Seal)) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    Focus.send(info.hardwareVersion);
    Focus.send(info.flashStart);
    Focus.send(seal.programVersion);
    Focus.send(seal.programCrc);
    Focus.send(true);
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendWrite")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    struct {
      KeyScannerFlasher::WriteAction write_action;
      uint8_t data[256];
      uint32_t crc32Transmission;
    } packet;
    watchdog_update();
    Serial.readBytes((uint8_t *)&packet, sizeof(packet));
    watchdog_update();
    auto info_action = key_scanner_flasher_.getInfoAction();

    uint32_t crc32InMemory = key_scanner_flasher_.crc32(packet.data, packet.write_action.size);
    if (packet.crc32Transmission != crc32InMemory) {
      ::Focus.send(false);
      return EventHandlerResult::ERROR;
    }

    if (packet.write_action.addr % info_action.eraseAlignment == 0) {
      KeyScannerFlasher::EraseAction erase_action{packet.write_action.addr, info_action.eraseAlignment};
      if (!key_scanner_flasher_.sendEraseAction(erase_action)) {
        ::Focus.send(false);
        return EventHandlerResult::ERROR;
      }
    }
    uint32_t crcKeyScannerCalculation = key_scanner_flasher_.sendWriteAction(packet.write_action, packet.data);
    if (crcKeyScannerCalculation != packet.crc32Transmission) {
      ::Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    ::Focus.send(true);
  }

  if (strcmp_P(command + 8 + 11, PSTR("validate")) == 0) {
    if (!key_scanner_flasher_.sendValidateProgram()) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    Focus.send(true);
  }

  if (strcmp_P(command + 8 + 11, PSTR("finish")) == 0) {
    if (!key_scanner_flasher_.sendFinish()) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    Focus.send(true);
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendStart")) == 0) {
    auto info_action = key_scanner_flasher_.getInfoAction();
    if (!key_scanner_flasher_.sendValidateProgram()) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    if (!key_scanner_flasher_.sendJump(info_action.programSpaceStart)) {
      Focus.send(false);
      return EventHandlerResult::ERROR;
    }
    Focus.send(true);
  }

  return EventHandlerResult::EVENT_CONSUMED;
}
void Upgrade::resetSide(KeyScannerFlasher::Side side) const {
  Runtime.device().side.prepareForFlash();
  if (side == KeyScannerFlasher::RIGHT) {
    Runtime.device().side.resetRight();
    return;
  }
  Runtime.device().side.resetLeft();
}
EventHandlerResult Upgrade::onSetup() {
  return EventHandlerResult::OK;
}
EventHandlerResult Upgrade::onKeyswitchEvent(Key &mapped_Key, KeyAddr key_addr, uint8_t key_state) {
  if (!serial_pre_activation)
    return EventHandlerResult::OK;

  if (!key_addr.isValid() || (key_state & INJECTED) != 0) {
    return EventHandlerResult::OK;
  }

  if (activated && key_addr.col() == 0 && key_addr.row() == 0 && keyToggledOff(key_state)) {
    activated    = false;
    pressed_time = 0;
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (key_addr.col() == 0 && key_addr.row() == 0 && keyToggledOn(key_state)) {
    activated    = true;
    pressed_time = Runtime.millisAtCycleStart();
    ;
    return EventHandlerResult::EVENT_CONSUMED;
  }

  return EventHandlerResult::OK;
}
EventHandlerResult Upgrade::beforeReportingState() {
  if (flashing) return EventHandlerResult::OK;
  if (!serial_pre_activation)
    return EventHandlerResult::OK;
  if (!activated)
    return EventHandlerResult::OK;
  if (Runtime.hasTimeExpired(pressed_time, press_time)) {
    flashing = true;
    return EventHandlerResult::OK;
  }
  return EventHandlerResult::OK;
}
}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::Upgrade Upgrade;
