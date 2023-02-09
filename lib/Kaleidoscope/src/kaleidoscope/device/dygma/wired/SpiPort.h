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

#ifndef SPICOMUNICATIONS_H_
#define SPICOMUNICATIONS_H_

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include <Arduino.h>
#include <hardware/dma.h>
#include "hardware/spi.h"
#include "Side_communications_protocol.h"

namespace kaleidoscope::device::dygma::wired {
using namespace Side_communications_protocol;
#define SIDE_nRESET_1 22  //19   // SWe 20220719: nRESET signal OUT to keyboard side 1; HIGH = running, LOW = reset
#define SIDE_nRESET_2 10  //12   // SWe 20220719: nRESET signal OUT to keyboard side 2; HIGH = running, LOW = reset

#define SPI_PORT_0    spi0
#define SPI_SPEED     (4000 * 1000)
#define SPI_MOSI_0    20
#define SPI_MISO_0    23
#define SPI_CLK_0     18
#define SPI_CS_0      21

#define SPI_PORT_1    spi1
#define SPI_MOSI_1    8
#define SPI_MISO_1    11
#define SPI_CLK_1     14
#define SPI_CS_1      9

class SpiPort {
 public:
  explicit SpiPort(bool side);

  void initCommunications();

  uint8_t writeTo(uint8_t *data, size_t length);

  bool sendPacket(Side_communications_protocol::Packet *data);

  uint8_t readFrom(uint8_t *data, size_t length);

  void disable();

  uint8_t crc_errors();
  virtual ~SpiPort();
  void irq();
  queue_t txMessages;
  queue_t rxMessages;
  Devices sideCommunications;

  void printConfig();

 private:
  void initInterrupt();
  void startDMA();
  void disableSide();

  struct Spi_settings {
    spi_inst *port;
    uint8_t mosi;
    uint8_t miso;
    uint8_t clock;
    uint8_t cs;
    uint32_t speed;
    uint8_t reset;
    uint8_t irq;
    uint8_t dmaIndexTx;
    uint8_t dmaIndexRx;
    dma_channel_config channelConfigTx;
    dma_channel_config channelConfigRx;
    Packet txMessage;
    Packet rxMessage;
  };

  struct StartInfo {
    uint32_t crc;
    uint32_t pull_up_config;
    uint32_t cpu_speed;
    uint32_t spi_speed_base;
    uint32_t spi_speed_variation;
    uint32_t pooling_rate_base;
    uint32_t pooling_rate_variation;
    uint8_t led_driver_enabled;
    uint8_t underGlow_enabled;
  }config;

  Spi_settings spiSettings;
  bool portUSB;
  uint32_t lasTimeCommunication = 2000;
};

extern SpiPort spi_0;
extern SpiPort spi_1;

}  // namespace kaleidoscope::device::dygma::wired
#endif
#endif  //NEURONWIRED_SRC_SPICOMUNICATIONS_H_
