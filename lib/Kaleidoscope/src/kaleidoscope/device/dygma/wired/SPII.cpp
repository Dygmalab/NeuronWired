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

#include <pico/time.h>
#include "SpiPort.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

namespace kaleidoscope::device::dygma::wired {

SpiPort portLeft(0);
SpiPort portRight(1);

void __no_inline_not_in_flash_func(dma_irq_1_handler)() {
  portRight.irq();
}

void __no_inline_not_in_flash_func(dma_irq_0_handler)() {
  portLeft.irq();
}

void SpiPort::startDMA() {
  channel_config_set_transfer_data_size(&spiSettings.channelConfigTx, DMA_SIZE_8);
  channel_config_set_dreq(&spiSettings.channelConfigTx, spi_get_dreq(spiSettings.port, true));

  channel_config_set_transfer_data_size(&spiSettings.channelConfigRx, DMA_SIZE_8);
  channel_config_set_dreq(&spiSettings.channelConfigRx, spi_get_dreq(spiSettings.port, false));
  channel_config_set_read_increment(&spiSettings.channelConfigRx, false);
  channel_config_set_write_increment(&spiSettings.channelConfigRx, true);

  dma_channel_configure(spiSettings.dmaIndexTx, &spiSettings.channelConfigTx,
						&spi_get_hw(spiSettings.port)->dr, // write address
						spiSettings.txMessage.buf, // read address
						SIZE_TRANSFER, // element count (each element is of size transfer_data_size)
						false); // don't start yet
  dma_channel_configure(spiSettings.dmaIndexRx, &spiSettings.channelConfigRx,
						spiSettings.rxMessage.buf, // write address
						&spi_get_hw(spiSettings.port)->dr, // read address
						SIZE_TRANSFER, // element count (each element is of size transfer_data_size)
						false); // don't start yet
  dma_start_channel_mask((1u << spiSettings.dmaIndexTx) | (1u << spiSettings.dmaIndexRx));
}

void SpiPort::initInterrupt() {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_init(spiSettings.port, spiSettings.speed);
  spi_set_format(spiSettings.port, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
  spi_set_slave(spiSettings.port, true);
  gpio_set_function(spiSettings.mosi, GPIO_FUNC_SPI);
  gpio_set_function(spiSettings.miso, GPIO_FUNC_SPI);
  gpio_set_function(spiSettings.clock, GPIO_FUNC_SPI);
  gpio_set_function(spiSettings.cs, GPIO_FUNC_SPI);

  spiSettings.dmaIndexTx = dma_claim_unused_channel(true);
  spiSettings.dmaIndexRx = dma_claim_unused_channel(true);
  spiSettings.channelConfigTx = dma_channel_get_default_config(spiSettings.dmaIndexTx);
  spiSettings.channelConfigRx = dma_channel_get_default_config(spiSettings.dmaIndexRx);
  irq_set_exclusive_handler(spiSettings.irq, portUSB ? dma_irq_1_handler : dma_irq_0_handler);
  irq_set_enabled(spiSettings.irq, true);
  portUSB ? dma_channel_set_irq1_enabled(spiSettings.dmaIndexRx, true) : dma_channel_set_irq0_enabled(spiSettings
																										  .dmaIndexRx,
																									  true);
  startDMA();
}

SpiPort::SpiPort(bool side) : portUSB(side) {
  if (side) {
	spiSettings = {SPI_PORT1, SPI_MOSI1, SPI_MISO1, SPI_CLK1, SPI_CS1, SPI_SPEED, SIDE_nRESET_1, DMA_IRQ_1};
  } else {
	spiSettings = {SPI_PORT2, SPI_MOSI2, SPI_MISO2, SPI_CLK2, SPI_CS2, SPI_SPEED, SIDE_nRESET_2, DMA_IRQ_0};
  }
  gpio_init(spiSettings.reset);
  gpio_set_dir(spiSettings.reset, GPIO_OUT);
  gpio_put(spiSettings.reset, false);
  sleep_us(1);
  gpio_put(spiSettings.reset, true);
  queue_init(&txMessages, sizeof(Message), 40);
  queue_init(&rxMessages, sizeof(Message), 40);
}

void SpiPort::initCommunications() {
  initInterrupt();
}

SpiPort::~SpiPort() {
  disableSide();
}

uint8_t SpiPort::crc_errors() {
  return 0;
}
uint8_t SpiPort::writeTo(uint8_t *data, size_t length) {
  if (data[0] >= 0x80) {
	Message message;
	if (queue_is_full(&txMessages)) {
	  return 0;
	}
	message.context.cmd = UPDATE_LED_BANK;
	message.context.size = length;
	for (uint8_t i = 1; i < length; ++i) {
	  message.data[i] = data[i];
	}
	message.data[0] = data[0] - 0x80;
	queue_add_blocking(&txMessages, &message);
  }
  return 0;
}

bool SpiPort::sendMessage(SpiPort::Message *data) {
  if (queue_is_full(&txMessages)) {
	return false;
  }
  queue_add_blocking(&txMessages, data);
  return true;
}

uint8_t SpiPort::readFrom(uint8_t *data, size_t length) {
  if (queue_is_empty(&rxMessages)) {
	data[0] = 0;
	return 6;
  }
  Message message;
  queue_remove_blocking(&rxMessages, &message);
  data[0] = 1;
  data[1] = message.data[0];
  data[2] = message.data[1];
  data[3] = message.data[2];
  data[4] = message.data[3];
  data[5] = message.data[4];
  if (millis() - lasTimeCommunication > 1000)
	return 0;
  return 6;
}

void SpiPort::irq() {
  irq_set_enabled(spiSettings.irq, false);
  irq_clear(spiSettings.irq);
  portUSB ? dma_channel_acknowledge_irq1(spiSettings.dmaIndexRx)
		  : dma_channel_acknowledge_irq0(spiSettings.dmaIndexRx);
  if (spiSettings.rxMessage.context.cmd==0) {
	//Something happened lest restart the communication
	if (Serial.available())
	  Serial.printf("Lost Connections with hand %i\n", portUSB);
	disableSide();
	gpio_put(spiSettings.reset, false);
	initInterrupt();
	gpio_put(spiSettings.reset, true);
	return;
  }
  lasTimeCommunication = millis();
  bool sideCom = spiSettings.rxMessage.context.bit_arr & 0b00000001;
  if (spiSettings.rxMessage.context.cmd!=1) {
	if (sideCom) {
	  queue_add_blocking(&portRight.rxMessages, &spiSettings.rxMessage);
	  if (!queue_is_empty(&portRight.txMessages)) {
		queue_remove_blocking(&portRight.txMessages, &spiSettings.txMessage);
	  }
	} else {
	  queue_add_blocking(&portLeft.rxMessages, &spiSettings.rxMessage);
	  if (!queue_is_empty(&portLeft.txMessages)) {
		queue_remove_blocking(&portLeft.txMessages, &spiSettings.txMessage);
	  }
	}
  }
  if (sideCom) {
	if (!queue_is_empty(&portRight.txMessages)) {
	  queue_remove_blocking(&portRight.txMessages, &spiSettings.txMessage);
	}
  } else {
	if (!queue_is_empty(&portLeft.txMessages)) {
	  queue_remove_blocking(&portLeft.txMessages, &spiSettings.txMessage);
	}
  }
  irq_set_enabled(spiSettings.irq, true);
  startDMA();
}

void SpiPort::disableSide() {
  spi_deinit(spiSettings.port);
  dma_channel_unclaim(spiSettings.dmaIndexTx);
  dma_channel_unclaim(spiSettings.dmaIndexRx);
}

}

#endif