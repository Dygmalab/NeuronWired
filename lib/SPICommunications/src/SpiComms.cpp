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
#include "SpiComms.h"
#include "hardware/spi.h"
#include "hardware/dma.h"


SpiComms spi_0(0);
SpiComms spi_1(1);

void __no_inline_not_in_flash_func(dma_irq_1_handler)() {
  spi_1.irq();
}

void __no_inline_not_in_flash_func(dma_irq_0_handler)() {
  spi_0.irq();
}

void SpiComms::startDMA() {
  channel_config_set_transfer_data_size(&spiSettings.channelConfigTx, DMA_SIZE_8);
  channel_config_set_dreq(&spiSettings.channelConfigTx, spi_get_dreq(spiSettings.port, true));

  channel_config_set_transfer_data_size(&spiSettings.channelConfigRx, DMA_SIZE_8);
  channel_config_set_dreq(&spiSettings.channelConfigRx, spi_get_dreq(spiSettings.port, false));
  channel_config_set_read_increment(&spiSettings.channelConfigRx, false);
  channel_config_set_write_increment(&spiSettings.channelConfigRx, true);

  dma_channel_configure(spiSettings.dmaIndexTx, &spiSettings.channelConfigTx,
                        &spi_get_hw(spiSettings.port)->dr,  // write address
                        spiSettings.txMessage.buf,          // read address
                        MAX_TRANSFER_SIZE,                  // element count (each element is of size transfer_data_size)
                        false);                             // don't start yet
  dma_channel_configure(spiSettings.dmaIndexRx, &spiSettings.channelConfigRx,
                        spiSettings.rxMessage.buf,          // write address
                        &spi_get_hw(spiSettings.port)->dr,  // read address
                        MAX_TRANSFER_SIZE,                  // element count (each element is of size transfer_data_size)
                        false);                             // don't start yet
  dma_start_channel_mask((1u << spiSettings.dmaIndexTx) | (1u << spiSettings.dmaIndexRx));
}

void SpiComms::initInterrupt() {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_init(spiSettings.port, spiSettings.speed);
  spi_set_format(spiSettings.port, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
  spi_set_slave(spiSettings.port, true);
  gpio_set_function(spiSettings.mosi, GPIO_FUNC_SPI);
  gpio_set_function(spiSettings.miso, GPIO_FUNC_SPI);
  gpio_set_function(spiSettings.clock, GPIO_FUNC_SPI);
  gpio_set_function(spiSettings.cs, GPIO_FUNC_SPI);

  spiSettings.dmaIndexTx      = dma_claim_unused_channel(true);
  spiSettings.dmaIndexRx      = dma_claim_unused_channel(true);
  spiSettings.channelConfigTx = dma_channel_get_default_config(spiSettings.dmaIndexTx);
  spiSettings.channelConfigRx = dma_channel_get_default_config(spiSettings.dmaIndexRx);
  irq_set_exclusive_handler(spiSettings.irq, portUSB ? dma_irq_1_handler : dma_irq_0_handler);
  irq_set_enabled(spiSettings.irq, true);
  portUSB ? dma_channel_set_irq1_enabled(spiSettings.dmaIndexRx, true) : dma_channel_set_irq0_enabled(spiSettings.dmaIndexRx, true);
  startDMA();
}

SpiComms::SpiComms(bool side)
  : portUSB(side) {
  if (side) {
    spiSettings = {SPI_PORT_0, SPI_MOSI_0, SPI_MISO_0, SPI_CLK_0, SPI_CS_0, SPI_SPEED, SIDE_nRESET_1, DMA_IRQ_1};
  } else {
    spiSettings = {SPI_PORT_1, SPI_MOSI_1, SPI_MISO_1, SPI_CLK_1, SPI_CS_1, SPI_SPEED, SIDE_nRESET_2, DMA_IRQ_0};
  }
  gpio_init(spiSettings.reset);
  gpio_set_dir(spiSettings.reset, GPIO_OUT);
  gpio_put(spiSettings.reset, false);
  sleep_us(1);
  gpio_put(spiSettings.reset, true);
  queue_init(&txMessages, sizeof(Packet), 40);
  queue_init(&rxMessages, sizeof(Packet), 40);
}

void SpiComms::initCommunications() {
  initInterrupt();
}

SpiComms::~SpiComms() {
  disableSide();
}

bool SpiComms::sendPacket(Packet *data) {
  if (queue_is_full(&txMessages)) {
    return false;
  }
  queue_add_blocking(&txMessages, data);
  return true;
}

void SpiComms::irq() {
  irq_set_enabled(spiSettings.irq, false);
  irq_clear(spiSettings.irq);
  portUSB ? dma_channel_acknowledge_irq1(spiSettings.dmaIndexRx)
          : dma_channel_acknowledge_irq0(spiSettings.dmaIndexRx);

  if (spiSettings.rxMessage.context.command == IS_DEAD) {
    //Something happened lest restart the communication
    if (Serial.available())
      Serial.printf("Lost Connections with hand %i\n", portUSB);
    disableSide();
    gpio_put(spiSettings.reset, false);
    initInterrupt();
    gpio_put(spiSettings.reset, true);
    return;
  }

  SpiComms &spi             = spiSettings.rxMessage.context.device == KEYSCANNER_DEFY_RIGHT ? spi_1 : spi_0;
  spi.sideCommunications    = spiSettings.rxMessage.context.device;
  spi.lastTimeCommunication = millis();
  if (spiSettings.rxMessage.context.command != IS_ALIVE) {
    queue_add_blocking(&spi.rxMessages, &spiSettings.rxMessage);
  }

  if (!queue_is_empty(&spi.txMessages)) {
    queue_remove_blocking(&spi.txMessages, &spiSettings.txMessage);
  } else {
    spiSettings.txMessage.context.command = Side_communications_protocol::IS_ALIVE;
  }
  spiSettings.txMessage.context.has_more_packets = !queue_is_empty(&spi.txMessages);
  irq_set_enabled(spiSettings.irq, true);
  startDMA();
}

void SpiComms::disableSide() {
  spi_deinit(spiSettings.port);
  dma_channel_unclaim(spiSettings.dmaIndexTx);
  dma_channel_unclaim(spiSettings.dmaIndexRx);
}
void SpiComms::run() {

  const bool now_active = millis() - lastTimeCommunication <= timeout;

  //If it was active and there and now it no longer active then notify the chanel
  if (active && !now_active) {
    active = now_active;
    active_callback_(active);
  }

  //If it was not active and now is active then notify the chanel that now is active
  if (!active && now_active) {
    active = now_active;
    active_callback_(active);
  }

  if (queue_is_empty(&rxMessages)) {
    return;
  }

  Packet packet;
  queue_remove_blocking(&rxMessages, &packet);
  callbacks_.call(packet.context.command, packet);
}


#endif