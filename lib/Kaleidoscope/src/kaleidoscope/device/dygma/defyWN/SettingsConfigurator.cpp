/* -*- mode: c++ -*-
 * kaleidoscope::device::dygma::Wired -- Kaleidoscope device plugin for Dygma Wired
 * Copyright (C) 2017-2019  Keyboard.io, Inc
 * Copyright (C) 2017-2019  Dygma Lab S.L.
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

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include "kaleidoscope/Runtime.h"
#include <Kaleidoscope-FocusSerial.h>
#include <pico/stdlib.h>
#include "SettingsConfigurator.h"
#include "Communications_protocol.h"
#include "Communications.h"
#include "kaleidoscope/plugin/EEPROM-Settings.h"

namespace kaleidoscope {
namespace device {
namespace dygma {
namespace defyWN {

#ifndef WIRED_FIRMWARE_VERSION
#define WIRED_FIRMWARE_VERSION "<unknown>"
#endif

EventHandlerResult SettingsConfigurator::onFocusEvent(const char *command) {
  if (::Focus.handleHelp(command,
                         PSTR("hardware.version\n"
                              "hardware.side_power\n"
                              "hardware.side_ver\n"
                              "hardware.sled_ver\n"
                              "hardware.sled_current\n"
                              "hardware.layout\n"
                              "hardware.joint\n"
                              "hardware.keyscan\n"
                              "hardware.crc_errors\n"
                              "hardware.firmware\n"
                              "hardware.chip_id\n"
                              "hardware.settings.printConfig\n"
                              "hardware.settings.underGlow\n"
                              "hardware.settings.ledDriver\n"
                              "hardware.settings.spiSpeed\n"
                              "hardware.settings.cpuSpeed\n"
                              "hardware.settings.ledDriverPullUp")))
    return EventHandlerResult::OK;

  if (strncmp_P(command, PSTR("hardware."), 9) != 0)
    return EventHandlerResult::OK;

  if (strcmp_P(command + 9, PSTR("version")) == 0) {
    ::Focus.send("Dygma Wired");
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (strcmp_P(command + 9, PSTR("firmware")) == 0) {
    ::Focus.send(WIRED_FIRMWARE_VERSION);
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (strcmp_P(command + 9, PSTR("chip_id")) == 0) {
    ::Focus.send(Runtime.device().getChipID().c_str());
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (strcmp_P(command + 9, PSTR("side_power")) == 0) {
    if (::Focus.isEOL()) {
      ::Focus.send(Runtime.device().side.getPower());
      return EventHandlerResult::EVENT_CONSUMED;
    } else {
      uint8_t power = Runtime.serialPort().parseInt();
      Runtime.device().side.setPower(power);
      return EventHandlerResult::EVENT_CONSUMED;
    }
  }

  enum {
    PRINT_CONFIG,
    ALIVE_INTERVAL,
    UNDERGLOW,
    LED_DRIVER,
    CPU_CLOCK,
    SPI_CLOCK,
    LED_DRIVER_PULL_UP,
  } sub_command;


  if (strncmp(command, "hardware.settings.", 18) != 0) {
    return EventHandlerResult::OK;
  }

  if (strcmp(command + 18, "printConfig") == 0)
    sub_command = PRINT_CONFIG;
  else if (strcmp(command + 18, "aliveInterval") == 0)
    sub_command = ALIVE_INTERVAL;
  else if (strcmp(command + 18, "underGlow") == 0)
    sub_command = UNDERGLOW;
  else if (strcmp(command + 18, "ledDriver") == 0)
    sub_command = LED_DRIVER;
  else if (strcmp(command + 18, "spiSpeed") == 0)
    sub_command = SPI_CLOCK;
  else if (strcmp(command + 18, "cpuSpeed") == 0)
    sub_command = CPU_CLOCK;
  else if (strcmp(command + 18, "ledDriverPullUp") == 0)
    sub_command = LED_DRIVER_PULL_UP;
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
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT || side == Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      KeyScannerSettings &ks = side == Communications_protocol::KEYSCANNER_DEFY_RIGHT ? right_settings : left_settings;
      char string[280];
      snprintf(string, sizeof(string), "This is the configuration\n crc: %lu\n pull_up_config: %lu\n cpu_speed: %lu\n spi_speed_base: %lu\n spi_speed_variation: %lu\n pooling_rate_base: %lu\n pooling_rate_variation: %lu\n underGlow_enabled: %i\n led_driver_enabled: %i\n", ks.crc, ks.pull_up_config, ks.cpu_speed, ks.spi_speed_base, ks.spi_speed_variation, ks.pooling_rate_base, ks.pooling_rate_variation, ks.underGlow_enabled, ks.led_driver_enabled);
      ::Focus.send(string);
    }
    if (side == Communications_protocol::Devices::NEURON_DEFY_WIRED) {
      ::Serial.printf("Neuron CPU is at: %lu\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY));
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
      ::Serial.println("The time alive interval needs to be greater than 1ms and less than 500ms");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t variation = Runtime.serialPort().parseInt();
    if (variation > 100) {
      ::Serial.println("The time alive interval variation needs to be less than 100ms");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Communications_protocol::Packet packet{};
    packet.header.command = Communications_protocol::SET_ALIVE_INTERVAL;
    packet.header.size    = sizeof(uint32_t) * 2;
    memcpy(&packet.data[0], &base, sizeof(uint32_t));
    memcpy(&packet.data[sizeof(uint32_t)], &variation, sizeof(uint32_t));
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      ::Serial.println("Sending alive interval to left side base");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT;
    }
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      ::Serial.println("Sending alive interval to right side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT;
    }
    Communications.sendPacket(packet);
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
      ::Serial.println("The spi speed needs to be greater than 0.5MHz and less than 7Mhz");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    if (::Focus.isEOL()) return EventHandlerResult::EVENT_CONSUMED;
    uint32_t variation = Runtime.serialPort().parseInt();
    if (variation > 2000000) {
      ::Serial.println("The spi speed variation needs to be less than 2Mhz");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Communications_protocol::Packet packet;
    packet.header.command = Communications_protocol::SET_SPI_SPEED;
    packet.header.size    = sizeof(uint32_t) * 2;
    memcpy(&packet.data[0], &base, sizeof(uint32_t));
    memcpy(&packet.data[sizeof(uint32_t)], &variation, sizeof(uint32_t));
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      ::Serial.println("Sending spi speed to left side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT;
    }
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      ::Serial.println("Sending spi speed to right side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT;
    }
    Communications.sendPacket(packet);
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
      ::Serial.println("The time cpu speed needs to be greater than 45MHz and less than 133Mhz");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Communications_protocol::Packet packet;
    packet.header.command = Communications_protocol::SET_CLOCK_SPEED;
    packet.header.size    = sizeof(uint32_t);
    memcpy(&packet.data[0], &cpu_speed, sizeof(uint32_t));
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT) {
      ::Serial.println("Setting cpuSpeed in left side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT;
      Communications.sendPacket(packet);
    }
    if (side == Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT) {
      ::Serial.println("Setting cpuSpeed in right side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT;
      Communications.sendPacket(packet);
    }
    if (side == Communications_protocol::Devices::NEURON_DEFY_WIRED) {
      ::Serial.println("Setting cpuSpeed in neuron");
      config_.validation = 0x4321;
      config_.cpuSpeed   = cpu_speed;
      Runtime.storage().put(cpu_base_, config_);
      Runtime.storage().commit();
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
      ::Serial.println("The underglow must be 0 for off or 1 for on");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Packet packet;
    packet.header.command = SET_ENABLE_UNDERGLOW;
    packet.header.size    = sizeof(uint8_t);
    memcpy(&packet.data[0], &enabled, sizeof(uint8_t));
    if (side == Devices::KEYSCANNER_DEFY_LEFT) {
      ::Serial.println("Setting underglow in left side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT;
    }
    if (side == Devices::KEYSCANNER_DEFY_RIGHT) {
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT;
      ::Serial.println("Setting underglow in right side");
    }
    Communications.sendPacket(packet);
  } break;
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
      ::Serial.println("The ledDriver must be 0 for off or 1 for on");
      return EventHandlerResult::EVENT_CONSUMED;
    }
    Packet packet;
    packet.header.command = SET_ENABLE_LED_DRIVER;
    packet.header.size    = sizeof(uint8_t);
    memcpy(&packet.data[0], &enabled, sizeof(uint8_t));
    if (side == Devices::KEYSCANNER_DEFY_LEFT) {
      ::Serial.println("Setting ledDriver in left side");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT;
    }
    if (side == Devices::KEYSCANNER_DEFY_RIGHT) {
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT;
      ::Serial.println("Setting ledDriver in right side");
    }
    Communications.sendPacket(packet);
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
    r.D3    = 0;
    Packet packet;
    packet.header.command = SET_LED_DRIVER_PULLUP;
    packet.header.size    = sizeof(uint8_t);
    memcpy(&packet.data[0], &r.data, sizeof(uint8_t));
    if (side == Devices::KEYSCANNER_DEFY_LEFT) {
      ::Serial.println("Setting ledDriver pullup in left side ");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_LEFT;
    }
    if (side == Devices::KEYSCANNER_DEFY_RIGHT) {
      ::Serial.println("Setting ledDriver pullup in right side ");
      packet.header.device = Communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT;
    }
    Communications.sendPacket(packet);
    break;
  }
  }
  return EventHandlerResult::OK;
}
EventHandlerResult SettingsConfigurator::onSetup() {
  cpu_base_ = ::EEPROMSettings.requestSlice(sizeof(config_));
  Runtime.storage().get(cpu_base_, config_);
  if (config_.validation != 0x4321) {
    config_.validation = 0x4321;
    config_.cpuSpeed   = 120000;
    Runtime.storage().put(cpu_base_, config_);
    Runtime.storage().commit();
  }
  set_sys_clock_khz(config_.cpuSpeed, true);
  Communications.callbacks.bind(IS_ALIVE, [this](Packet p) {
    KeyScannerSettings &ks = p.header.device == Communications_protocol::KEYSCANNER_DEFY_RIGHT ? right_settings : left_settings;
    memcpy(&ks, p.data, sizeof(ks));
  });
  return EventHandlerResult::OK;
}

}  // namespace defyWN
}  // namespace dygma
}  // namespace device
}  // namespace kaleidoscope

kaleidoscope::device::dygma::defyWN::SettingsConfigurator SettingsConfigurator;

#endif
