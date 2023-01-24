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

#include "Upgrade.h"
#include "kaleidoscope/plugin/FocusSerial.h"

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
  WIRE_.beginTransmission(address);
  WIRE_.write(command);
  return WIRE_.endTransmission(stopBit);
}

uint8_t KeyScannerFlasher::sendMessage(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit) {
  WIRE_.beginTransmission(address);
  WIRE_.write(message, lenMessage);
  return WIRE_.endTransmission(stopBit);
}

uint8_t KeyScannerFlasher::readData(uint8_t address, uint8_t *message, uint32_t lenMessage, bool stopBit) {
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
void KeyScannerFlasher::setSide(bool side) {
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

bool KeyScannerFlasher::sendWriteAction(KeyScannerFlasher::WriteAction write_action) {
  if (sendCommand(address, Action::WRITE)) return false;
  if (sendMessage(address, (uint8_t *)&write_action, sizeof(write_action))) return false;
  return true;
}

bool KeyScannerFlasher::sendReadAction(KeyScannerFlasher::ReadAction read_action) {
  if (sendCommand(address, Action::READ)) return false;
  if (sendMessage(address, (uint8_t *)&read_action, sizeof(read_action))) return false;
  return true;
}
bool KeyScannerFlasher::sendMessage(uint8_t *data, size_t size) {
  if (sendMessage(address, data, size)) return false;
  return true;
}
uint16_t KeyScannerFlasher::readData(uint8_t *data, size_t size) {
  return readData(address, data, size);
}
bool KeyScannerFlasher::sendGo(uint32_t address_to_jump) {
  if (sendCommand(address, Action::GO)) return false;
  if (sendMessage(address, (uint8_t *)&address_to_jump, sizeof(address_to_jump))) return false;
  return true;
}
bool KeyScannerFlasher::sendValidateSealAction(KeyScannerFlasher::ValidateSealAction validate_seal_action) {
  if (sendCommand(address, Action::VALIDATE_SEAL)) return false;
  if (sendMessage(address, (uint8_t *)&validate_seal_action, sizeof(validate_seal_action))) return false;
  uint8_t done = false;
  readData(address, (uint8_t *)&done, sizeof(done));
  return done;
}

EventHandlerResult Upgrade::onFocusEvent(const char *command) {
  if (::Focus.handleHelp(command,
                         PSTR(
                           "upgrade.start\n"
                           "upgrade.isReady\n"
                           "upgrade.neuron\n"
                           "upgrade.end\n"
                           "upgrade.keyscanner.side\n"
                           "upgrade.keyscanner.getInfo\n"
                           "upgrade.keyscanner.sendErase\n"
                           "upgrade.keyscanner.read\n"
                           "upgrade.keyscanner.sendWrite\n"
                           "upgrade.keyscanner.sendValidateSeal\n"
                           "upgrade.keyscanner.sendGo")))
    return EventHandlerResult::OK;
  //TODO set numbers ot PSTR

  if (strncmp_P(command, PSTR("upgrade."), 8) != 0)
    return EventHandlerResult::OK;


  if (strcmp_P(command + 8, PSTR("start")) == 0) {
    serial_pre_activation = true;
  }

  if (strcmp_P(command + 8, PSTR("neuron")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    Runtime.rebootBootloader();
  }

  if (strcmp_P(command + 8, PSTR("isReady")) == 0) {
    ::Focus.send(flashing);
  }

  if (strcmp_P(command + 8, PSTR("end")) == 0) {
    serial_pre_activation = false;
    activated             = false;
    pressed_time          = 0;
  }

  if (strncmp_P(command + 8, PSTR("keyscanner."), 11) != 0)
    return EventHandlerResult::OK;

  if (strcmp_P(command + 8 + 11, PSTR("side")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    uint8_t side;
    ::Focus.read(side);
    key_scanner_flasher_.setSide(side);
  }

  if (strcmp_P(command + 8 + 11, PSTR("getInfo")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;

    KeyScannerFlasher::InfoAction info{};
    //Get Info
    Runtime.device().side.prepareForFlash();
    if (!key_scanner_flasher_.getInfoFlasherKS(info)) {
      return EventHandlerResult::ERROR;
    }
    Focus.send(info.hardwareVersion);
    Focus.send(info.flashStart);
    Focus.send(info.validationSpaceStart);
    Focus.send(info.validationSpaceSize);
    Focus.send(info.flashSize);
    Focus.send(info.eraseAlignment);
    Focus.send(info.writeAlignment);
    Focus.send(info.maxDataLength);
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendErase")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;

    KeyScannerFlasher::EraseAction erase{};
    readStruct(erase);
    if (!key_scanner_flasher_.sendEraseAction(erase)) {
      return EventHandlerResult::ERROR;
    }
  }

  if (strcmp_P(command + 8 + 11, PSTR("read")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    KeyScannerFlasher::ReadAction read_action{static_cast<uint32_t>(Runtime.serialPort().parseInt()), static_cast<uint32_t>(Runtime.serialPort().parseInt())};
    if (!key_scanner_flasher_.sendReadAction(read_action)) {
      return EventHandlerResult::ERROR;
    }
    //If for NRF this code does not work we can change this to just struct of ReadAction and sizeof(Seal?) array :)
    uint8_t data[256];
    if (key_scanner_flasher_.readData(data, read_action.size) != read_action.size) {
      return EventHandlerResult::ERROR;
    }
    auto *data2 = reinterpret_cast<uint32_t *>(data);
    for (int i = 0; i < read_action.size / sizeof(read_action.size); ++i) {
      ::Focus.send(data2[i]);
    }
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendWrite")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;
    KeyScannerFlasher::WriteAction write_action{static_cast<uint32_t>(Runtime.serialPort().parseInt()), static_cast<uint32_t>(Runtime.serialPort().parseInt())};
    if (!key_scanner_flasher_.sendWriteAction(write_action)) {
      return EventHandlerResult::ERROR;
    }
    uint8_t data[256];
    watchdog_update();
    Runtime.serialPort().read(data, write_action.size);

    if (!key_scanner_flasher_.sendMessage(data, write_action.size)) {
      Serial.printf("Shit");
      return EventHandlerResult::ERROR;
    }
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendValidateSeal")) == 0) {
    if (!flashing) return EventHandlerResult::ERROR;

    KeyScannerFlasher::ValidateSealAction validateSeal{};
    readStruct(validateSeal);
    key_scanner_flasher_.sendValidateSealAction(validateSeal);
  }

  if (strcmp_P(command + 8 + 11, PSTR("sendGo")) == 0) {
    uint32_t to_jump;
    readStruct(to_jump);
    if (!key_scanner_flasher_.sendGo(to_jump)) {
      return EventHandlerResult::ERROR;
    }
  }

  return EventHandlerResult::EVENT_CONSUMED;
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
