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

#ifndef _SPISLAVE_H
#define _SPISLAVE_H

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include <Arduino.h>
#include <hardware/dma.h>
#include "hardware/spi.h"
#include "KeyScanner_communications_protocol.h"

using namespace Communications_protocol;
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

//TODO: Refactor SPI class to have a port parameter
class SPISlave {
 public:
  explicit SPISlave(bool side);

  void init();

  virtual ~SPISlave();
  queue_t tx_messages_;
  queue_t rx_messages_;
  void irq();
  Devices device;

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
  Spi_settings spiSettings;
  bool portUSB;
};

extern SPISlave port0;
extern SPISlave port1;

#endif
#endif  //NEURONWIRED_SRC_SPICOMUNICATIONS_H_
