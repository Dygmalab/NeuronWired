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
#define SIZE_TRANSFER    40

class SpiPort {
public:

  enum SPI_COMMUNICATION {
	IS_ALIVE = 1,
	HAS_KEYS,
	SET_MODE_LED,
	SYNC_MODE_LED,
	UPDATE_LED_BANK,
  };

  struct Context {
	uint8_t cmd;
	uint8_t bit_arr;
	uint8_t size;
  };

  union Message {
	struct {
	  Context context;
	  uint8_t data[SIZE_TRANSFER - sizeof(context)];
	};
	uint8_t buf[SIZE_TRANSFER];
  };

  static_assert(sizeof(Message)==SIZE_TRANSFER);

  explicit SpiPort(bool side);

  void initCommunications();

  uint8_t writeTo(uint8_t *data, size_t length);

  bool sendMessage(Message *data);

  uint8_t readFrom(uint8_t *data, size_t length);

  void disable();

  uint8_t crc_errors();
  virtual ~SpiPort();
  void irq();
  queue_t txMessages;
  queue_t rxMessages;

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
	Message txMessage;
	Message rxMessage;
  };

  Spi_settings spiSettings;
  bool portUSB;
  uint32_t lasTimeCommunication;
};

extern SpiPort portLeft;
extern SpiPort portRight;

}
#endif
#endif //NEURONWIRED_SRC_SPICOMUNICATIONS_H_
