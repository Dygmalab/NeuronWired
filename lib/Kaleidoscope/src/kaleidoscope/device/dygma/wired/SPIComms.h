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

#pragma once

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include <Arduino.h>
#include <hardware/dma.h>
#include "hardware/spi.h"

namespace kaleidoscope::device::dygma::wired {
#define SIDE_nRESET_1  22  //19   // SWe 20220719: nRESET signal OUT to keyboard side 1; HIGH = running, LOW = reset
#define SIDE_nRESET_2  10  //12   // SWe 20220719: nRESET signal OUT to keyboard side 2; HIGH = running, LOW = reset

#define SPI_PORT1 spi0
#define SPI_SPEED (1000 * 1000)
#define SPI_MOSI1  20   //SPI-1 slave IN, we are slave
#define SPI_MISO1  23   //SPI-1 slave OUT, we are slave
#define SPI_CLK1   18  //was 22   //SPI-1 clock IN, we are slave (must be changed in HW to 18)
#define SPI_CS1    21   //SPI-1 chip select IN, we are slave
// SPI2 slave pins (SPI1 module at the RP2040)
#define SPI_PORT2 spi1
#define SPI_MOSI2   8   //SPI-2 slave IN, we are slave
#define SPI_MISO2  11   //SPI-2 slave OUT, we are slave
#define SPI_CLK2   14  //was 10   //SPI-2 clock IN, we are slave  (must be changed in HW to 14)
#define SPI_CS2     9   //SPI-2 chip select IN, we are slave1

class SPIComms {
 public:
  enum Side {
    LEFT,
    RIGHT,
  };
  explicit SPIComms(Side side);

  ~SPIComms();

  void initSPI();

  uint8_t writeTo(uint8_t *data, size_t length);

  uint8_t readFrom(uint8_t *data, size_t length);

  void disable();

  uint8_t crc_errors();

 private:
  enum SPI_COMUNICATIONS {
    SPI_CMD_DEFAULT,
    SPI_CMD_KEYS,
    SPI_CMD_VERSION,
    SPI_CMD_BATT_STATE,
    SPI_CMD_KEYSCAN_INTERVAL,
    SPI_CMD_LED_SET_BANK_TO,
  };

  struct Context {
    uint8_t cmd;
    uint8_t bit_arr;
    uint8_t sync;
    uint8_t size;
  };

  union Message {
    Context context;
    uint8_t buf[32];
  };

  struct spi_side {
    spi_inst *port;
    uint8_t mosi;
    uint8_t miso;
    uint8_t clock;
    uint8_t cs;
    uint32_t speed;
    Side side;
    uint8_t dma_irq;
    uint8_t reset_pin;
    int dma_tx;
    int dma_rx;
    dma_channel_config config_tx;
    dma_channel_config config_rx;
    Message tx_message;
    Message rx_message;
//    bool new_key;
//    std::queue<Message> tx_messages;
//    uint8_t last_keys[];
  };

  inline static spi_side
      spi_right{SPI_PORT1, SPI_MOSI1, SPI_MISO1, SPI_CLK1, SPI_CS1, SPI_SPEED, RIGHT, DMA_IRQ_1, SIDE_nRESET_1};
  inline static spi_side
      spi_left{SPI_PORT2, SPI_MOSI2, SPI_MISO2, SPI_CLK2, SPI_CS2, SPI_SPEED, LEFT, DMA_IRQ_0, SIDE_nRESET_2};

  Side side_;

  static void startDMA(spi_side &side);
  static void irqHandlerLeft();
  static void irqHandler(spi_side &side);
  static void irqHandlerRight();
  static void initSide(spi_side &side);
  static void disableSide(spi_side &side);
  static void updateHand(spi_side &side);
  static void updateLeds(spi_side &side);
};

}
#endif
