/* -*- mode: c++ -*-
 * Kaleidoscope-EEPROM-Settings -- Basic EEPROM settings plugin for Kaleidoscope.
 * Copyright (C) 2017, 2018, 2019  Keyboard.io, Inc
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

#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-FocusSerial.h>
#include <pico/stdlib.h>
#include "kaleidoscope/plugin/EEPROM-Settings/crc.h"
#include "kaleidoscope/layers.h"

namespace kaleidoscope {
namespace plugin {

struct EEPROMSettings::settings EEPROMSettings::settings_;
bool EEPROMSettings::is_valid_;
bool EEPROMSettings::sealed_;
uint16_t EEPROMSettings::next_start_ = sizeof(EEPROMSettings::settings);

EventHandlerResult EEPROMSettings::onSetup() {
  Runtime.storage().begin(8192);
  Runtime.storage().get(0, settings_);

  /* If the version is undefined, set up sensible defaults. */
  if (settings_.version == VERSION_UNDEFINED) {
    if (settings_.default_layer == 127 &&
        settings_.ignore_hardcoded_layers) {
      /* If both of these are set, that means that the EEPROM is uninitialized,
         and setting sensible defaults is safe. If either of them is not at it's
         uninitialized state, we do not override them, to avoid overwriting user
         settings. */
      settings_.ignore_hardcoded_layers = false;
      settings_.default_layer           = 0;
    }

    /* If the version is undefined, we'll set it to our current one. */
    settings_.version = VERSION_CURRENT;

    /* Ideally, we'd save the defaults set here on the first write, but we are
     * not able to catch all writes yet. For the sake of consistency, if we
     * encounter a firmware with no version defined, we'll set sensible
     * defaults. */
    Runtime.storage().put(0, settings_);
    Runtime.storage().commit();
  }

  //  if(settings_.validation!=0x421563){
  //    settings_.cpuSpeed=125000;
  //    settings_.validation=0x421563;
  //    Runtime.storage().put(0, settings_);
  //    Runtime.storage().commit();
  //  }
  //
  //  set_sys_clock_khz(settings_.cpuSpeed, true);


  return EventHandlerResult::OK;
}

EventHandlerResult EEPROMSettings::beforeEachCycle() {
  if (!sealed_)
    seal();

  return EventHandlerResult::OK;
}

bool EEPROMSettings::isValid(void) {
  return is_valid_;
}

uint16_t EEPROMSettings::crc(void) {
  if (sealed_)
    return settings_.crc;
  return 0;
}

uint8_t EEPROMSettings::default_layer(uint8_t layer) {
  if (layer < layer_count) {
    Layer.move(layer);
    settings_.default_layer = layer;
  }

  /*
   * We set default_layer to IGNORE_HARDCODED_LAYER (instead of `value`) because
   * due to compatibility reasons, we might get passed 0xff, yet, we want to set
   * a different value to signal an explicit "no default".
   */
  if (layer >= IGNORE_HARDCODED_LAYER) {
    settings_.default_layer = IGNORE_HARDCODED_LAYER;
  }
  update();
  return settings_.default_layer;
}

void EEPROMSettings::ignoreHardcodedLayers(bool value) {
  settings_.ignore_hardcoded_layers = value;
  if (settings_.default_layer > IGNORE_HARDCODED_LAYER)
    settings_.default_layer = IGNORE_HARDCODED_LAYER;
  update();
}

void EEPROMSettings::seal(void) {
  sealed_ = true;

  CRC.finalize();

  if (settings_.version != VERSION_CURRENT) {
    is_valid_ = false;
    return;
  }

  if (settings_.crc == 0xffff) {
    settings_.crc = CRC.crc;
    update();
  } else if (settings_.crc != CRC.crc) {
    return;
  }

  /* If we have a default layer set, switch to it.
   *
   * We check if the layer is smaller than IGNORE_HARDCODED_LAYER (0x7e),
   * because we want to avoid setting a default layer in two cases:
   *
   * - When the EEPROM is uninitialized (0x7f)
   * - When such layer switching is explicitly turned off (0x7e)
   */
  if (settings_.default_layer < IGNORE_HARDCODED_LAYER)
    Layer.move(settings_.default_layer);
}

uint16_t EEPROMSettings::requestSlice(uint16_t size) {
  if (sealed_)
    return 0;

  uint16_t start = next_start_;
  next_start_ += size;

  CRC.update((const void *)&size, sizeof(size));

  return start;
}

void EEPROMSettings::invalidate(void) {
  is_valid_ = false;
}

uint16_t EEPROMSettings::used(void) {
  return next_start_;
}

void EEPROMSettings::update(void) {
  Runtime.storage().put(0, settings_);
  Runtime.storage().commit();
  is_valid_ = true;
}

/** Focus **/
EventHandlerResult FocusSettingsCommand::onFocusEvent(const char *command) {
  enum {
    DEFAULT_LAYER,
    IS_VALID,
    GET_VERSION,
    PRINT_CONFIG,
    ALIVE_INTERVAL,
    UNDERGLOW,
    LED_DRIVER,
    CPU_CLOCK,
    SPI_CLOCK,
    LED_DRIVER_PULL_UP,
    CRC,
  } sub_command;

  if (::Focus.handleHelp(command, PSTR("settings.defaultLayer\n"
                                       "settings.valid?\n"
                                       "settings.version\n"
                                       "settings.printConfig\n"
                                       "settings.aliveInterval\n"
                                       "settings.underGlow\n"
                                       "settings.ledDriver\n"
                                       "settings.spiSpeed\n"
                                       "settings.cpuSpeed\n"
                                       "settings.ledDriverPullUp\n"
                                       "settings.crc")))
    return EventHandlerResult::OK;

  if (strncmp_P(command, PSTR("settings."), 9) != 0)
    return EventHandlerResult::OK;

  if (strcmp_P(command + 9, PSTR("defaultLayer")) == 0)
    sub_command = DEFAULT_LAYER;
  else if (strcmp_P(command + 9, PSTR("valid?")) == 0)
    sub_command = IS_VALID;
  else if (strcmp_P(command + 9, PSTR("version")) == 0)
    sub_command = GET_VERSION;
  else if (strcmp_P(command + 9, PSTR("printConfig")) == 0)
    sub_command = PRINT_CONFIG;
  else if (strcmp_P(command + 9, PSTR("underGlow")) == 0)
    sub_command = UNDERGLOW;
  else if (strcmp_P(command + 9, PSTR("ledDriver")) == 0)
    sub_command = LED_DRIVER;
  else if (strcmp_P(command + 9, PSTR("aliveInterval")) == 0)
    sub_command = ALIVE_INTERVAL;
  else if (strcmp_P(command + 9, PSTR("spiSpeed")) == 0)
    sub_command = SPI_CLOCK;
  else if (strcmp_P(command + 9, PSTR("cpuSpeed")) == 0)
    sub_command = CPU_CLOCK;
  else if (strcmp_P(command + 9, PSTR("ledDriverPullUp")) == 0)
    sub_command = LED_DRIVER_PULL_UP;
  else if (strcmp_P(command + 9, PSTR("crc")) == 0)
    sub_command = CRC;
  else
    return EventHandlerResult::OK;
  switch (sub_command) {
  case PRINT_CONFIG: {
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (side != 1 && side != 2 && side != 3) {
      ::Serial.println("need a side and this need to be either 1 for left side 2 for right side or 3 for neuron");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Runtime.device().printConfigLeftHand();
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Runtime.device().printConfigRightHand();
    }
    if (side == Side_communications_protocol::Devices::NEURON_DEFY_WIRED) {
      Serial.printf("Neuron CPU is at: %lu\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY));
    }
    break;
  }
  case ALIVE_INTERVAL: {
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (side != 1 && side != 2) {
      ::Serial.println("need a side and this need to be either 1 for left side 2 for right side");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t base = Runtime.serialPort().parseInt();
    if (base < 1 || base >= 500) {
      Serial.println("The time alive interval needs to be greater than 1ms and less than 500ms");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t variation = Runtime.serialPort().parseInt();
    if (variation > 100) {
      Serial.println("The time alive interval variation needs to be less than 100ms");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_ALIVE_INTERVAL;
    packet.context.size    = sizeof(uint32_t) * 2;
    memcpy(&packet.data[0], &base, sizeof(uint32_t));
    memcpy(&packet.data[sizeof(uint32_t)], &variation, sizeof(uint32_t));
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Serial.printf("Sending alive interval to left side base %lu and variation %lu\n", base, variation);
      Runtime.device().sendPacketLeftHand(packet);
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Serial.printf("Sending alive interval to right side base %lu and variation %lu\n", base, variation);
      Runtime.device().sendPacketRightHand(packet);
    }
    break;
  }
  case SPI_CLOCK: {
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (side != 1 && side != 2) {
      ::Serial.println("need a side and this need to be either 1 for left side 2 for right side");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t base = Runtime.serialPort().parseInt();
    if (base < 500000 || base >= 7000000) {
      Serial.println("The spi speed needs to be greater than 0.5MHz and less than 7Mhz");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t variation = Runtime.serialPort().parseInt();
    if (variation > 2000000) {
      Serial.println("The spi speed variation needs to be less than 2Mhz");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_SPI_SPEED;
    packet.context.size    = sizeof(uint32_t) * 2;
    memcpy(&packet.data[0], &base, sizeof(uint32_t));
    memcpy(&packet.data[sizeof(uint32_t)], &variation, sizeof(uint32_t));
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Serial.printf("Sending spi speed to left side base %lu and variation %lu\n", base, variation);
      Runtime.device().sendPacketLeftHand(packet);
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Serial.printf("Sending spi speed to right side base %lu and variation %lu\n", base, variation);
      Runtime.device().sendPacketRightHand(packet);
    }
    break;
  }
  case CPU_CLOCK: {
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (side != 1 && side != 2 && side != 3) {
      ::Serial.println("need a side and this need to be either 1 for left side 2 for right side or 3 for neuron");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t cpu_speed = Runtime.serialPort().parseInt();
    if (cpu_speed < 45000 || cpu_speed >= 133000) {
      Serial.println("The time cpu speed needs to be greater than 45MHz and less than 133Mhz");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_CLOCK_SPEED;
    packet.context.size    = sizeof(uint32_t);
    memcpy(&packet.data[0], &cpu_speed, sizeof(uint32_t));
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Serial.printf("Setting cpuSpeed in left side to %lu\n", cpu_speed);
      Runtime.device().sendPacketLeftHand(packet);
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Serial.printf("Setting cpuSpeed in right side to %lu\n", cpu_speed);
      Runtime.device().sendPacketRightHand(packet);
    }
    if (side == Side_communications_protocol::Devices::NEURON_DEFY_WIRED) {
      Serial.printf("Setting cpuSpeed in neuron to %lu\n", cpu_speed);
      Runtime.device().settings.setCPUSpeed(cpu_speed);
      ::EEPROMSettings.update();
    }
    break;
  }
  case UNDERGLOW: {
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (side != 1 && side != 2) {
      ::Serial.println("need a side and this need to be either 1 for left side 2 for right side");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint8_t enabled = Runtime.serialPort().parseInt();
    if (enabled != 1 && enabled != 0) {
      Serial.println("The underglow must be 0 for off or 1 for on");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_ENABLE_UNDERGLOW;
    packet.context.size    = sizeof(uint8_t);
    memcpy(&packet.data[0], &enabled, sizeof(uint8_t));
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Serial.printf("Setting underglow in left side to %i\n", enabled);
      Runtime.device().sendPacketLeftHand(packet);
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Serial.printf("Setting underglow in right side to %i\n", enabled);
      Runtime.device().sendPacketRightHand(packet);
    }
    break;
  }
  case LED_DRIVER: {
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (side != 1 && side != 2) {
      ::Serial.println("need a side and this need to be either 1 for left side 2 for right side");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint8_t enabled = Runtime.serialPort().parseInt();
    if (enabled != 1 && enabled != 0) {
      Serial.println("The ledDriver must be 0 for off or 1 for on");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_ENABLE_LED_DRIVER;
    packet.context.size    = sizeof(uint8_t);
    memcpy(&packet.data[0], &enabled, sizeof(uint8_t));
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Serial.printf("Setting ledDriver in left side to %i\n", enabled);
      Runtime.device().sendPacketLeftHand(packet);
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Serial.printf("Setting ledDriver in right side to %i\n", enabled);
      Runtime.device().sendPacketRightHand(packet);
    }
    break;
  }
  case LED_DRIVER_PULL_UP: {
    union Register {
      struct {
        uint8_t CSPUR : 3;
        uint8_t D3 : 1;
        uint8_t SWPDR : 3;
        uint8_t PHC : 1;
      };
      uint8_t data;
    };

    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t side = Runtime.serialPort().parseInt();
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    Register r{};
    r.PHC = Runtime.serialPort().parseInt();
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    r.SWPDR = Runtime.serialPort().parseInt();
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    r.CSPUR = Runtime.serialPort().parseInt();
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_LED_DRIVER_PULLUP;
    packet.context.size    = sizeof(uint8_t);
    memcpy(&packet.data[0], &r.data, sizeof(uint8_t));
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      Serial.printf("Setting ledDriver pullup in left side to %i\n", r.data);
      Runtime.device().sendPacketLeftHand(packet);
    }
    if (side == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      Serial.printf("Setting  ledDriver pullup in right side to %i\n", r.data);
      Runtime.device().sendPacketRightHand(packet);
    }
    break;
  }
  case DEFAULT_LAYER: {
    if (::Focus.isEOL()) {
      ::Focus.send(::EEPROMSettings.default_layer());
    } else {
      uint8_t layer;
      ::Focus.read(layer);
      ::EEPROMSettings.default_layer(layer);
    }
    break;
  }
  case IS_VALID:
    ::Focus.send(::EEPROMSettings.isValid());
    break;
  case GET_VERSION:
    ::Focus.send(::EEPROMSettings.version());
    break;
  case CRC:
    ::Focus.sendRaw(::CRC.crc, F("/"), ::EEPROMSettings.crc());
    break;
  }

  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult FocusEEPROMCommand::onFocusEvent(const char *command) {
  enum {
    CONTENTS,
    FREE,
  } sub_command;

  if (::Focus.handleHelp(command, PSTR("eeprom.contents\neeprom.free")))
    return EventHandlerResult::OK;

  if (strcmp_P(command, PSTR("eeprom.contents")) == 0)
    sub_command = CONTENTS;
  else if (strcmp_P(command, PSTR("eeprom.free")) == 0)
    sub_command = FREE;
  else
    return EventHandlerResult::OK;

  switch (sub_command) {
  case CONTENTS: {
    if (::Focus.isEOL()) {
      for (uint16_t i = 0; i < Runtime.storage().length(); i++) {
        uint8_t d = Runtime.storage().read(i);
        ::Focus.send(d);
      }
    } else {
      for (uint16_t i = 0; i < Runtime.storage().length() && !::Focus.isEOL(); i++) {
        uint8_t d;
        ::Focus.read(d);
        Runtime.storage().update(i, d);
      }
    }

    break;
  }
  case FREE:
    ::Focus.send(Runtime.storage().length() - ::EEPROMSettings.used());
    break;
  }

  return EventHandlerResult::EVENT_CONSUMED;
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::EEPROMSettings EEPROMSettings;
kaleidoscope::plugin::FocusSettingsCommand FocusSettingsCommand;
kaleidoscope::plugin::FocusEEPROMCommand FocusEEPROMCommand;
