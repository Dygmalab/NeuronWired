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

#include "SPIComms.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

namespace kaleidoscope::device::dygma::wired {

SPIComms::SPIComms(Side side) : side_(side) {}

void SPIComms::disable() {
  spi_side &side = side_ ? spi_left : spi_right;
  disableSide(side);
}

SPIComms::~SPIComms() { disable(); }

uint8_t SPIComms::crc_errors() { return 0; }
uint8_t SPIComms::writeTo(uint8_t *data, size_t length) {
  spi_side &side = side_ ? spi_left : spi_right;
//  if (data[0] >= 0x80) {
//    Message m;
//    m.context.cmd = 1;
//    m.context.size = length;
//    for (uint8_t i = 0; i < length; ++i) {
//      m.buf[sizeof(Context) + i] = data[i];
//    }
//    side.tx_messages.push(m);
//  }
  return 0;
}
uint8_t SPIComms::readFrom(uint8_t *data, size_t length) {
//  spi_side &side = side_ ? spi_left : spi_right;
//  data[0] = side.new_key;
//  data[1] = side.last_keys[0];
//  data[2] = side.last_keys[1];
//  data[3] = side.last_keys[2];
//  data[4] = side.last_keys[3];
//  data[5] = side.last_keys[4];
  return 6;
}

void SPIComms::updateHand(spi_side &side) {
//  side.last_keys[0] = side.rx_message.buf[sizeof(Context) + 0];
//  side.last_keys[1] = side.rx_message.buf[sizeof(Context) + 1];
//  side.last_keys[2] = side.rx_message.buf[sizeof(Context) + 2];
//  side.last_keys[3] = side.rx_message.buf[sizeof(Context) + 3];
//  side.last_keys[4] = side.rx_message.buf[sizeof(Context) + 4];
//  side.new_key = true;
}

void SPIComms::updateLeds(spi_side &side) {
//  if (!side.tx_messages.empty()) {
//    side.tx_message = side.tx_messages.front();
//    side.tx_messages.pop();
//  } else {
//    side.tx_message.context.cmd = 0;
//  }
}

void __no_inline_not_in_flash_func(SPIComms::irqHandler)(spi_side &side) {
  irq_set_enabled(side.dma_irq, false);
  irq_clear(side.dma_irq);
  side.side ? dma_channel_acknowledge_irq1(side.dma_rx) : dma_channel_acknowledge_irq0(side.dma_rx);
  if (side.rx_message.context.sync == 0) {
    //Something happened lest restart the communication
    Serial.printf("Disconnected %i", side.side);
    disableSide(side);
    gpio_put(side.side ? SIDE_nRESET_1 : SIDE_nRESET_2, false);
    initSide(side);
    gpio_put(side.side ? SIDE_nRESET_1 : SIDE_nRESET_2, true);
    return;
  }
  bool sideCom = side.rx_message.context.bit_arr & 0b00000001;
  if (side.rx_message.context.cmd == SPI_CMD_KEYS) {
    Serial.printf("Key recibed %i", side.side);
    //updateHand(side);
  }
//  updateLeds(side);
  irq_set_enabled(side.dma_irq, true);
  startDMA(side);
}

void SPIComms::irqHandlerLeft() {
  Serial.printf("DMA Left");
  irq_set_enabled(spi_left.dma_irq, false);

//  irqHandler(spi_left);
}

void SPIComms::irqHandlerRight() {
  Serial.printf("DMA Right");
  irq_set_enabled(spi_right.dma_irq, false);
//  irqHandler(spi_right);
}

void SPIComms::startDMA(spi_side &side) {
  channel_config_set_transfer_data_size(&side.config_tx, DMA_SIZE_8);
  channel_config_set_dreq(&side.config_tx, spi_get_dreq(side.port, true));

  channel_config_set_transfer_data_size(&side.config_rx, DMA_SIZE_8);
  channel_config_set_dreq(&side.config_rx, spi_get_dreq(side.port, false));
  channel_config_set_read_increment(&side.config_rx, false);
  channel_config_set_write_increment(&side.config_rx, true);

  dma_channel_configure(side.dma_tx, &side.config_tx,
                        &spi_get_hw(side.port)->dr, // write address
                        side.tx_message.buf, // read address
                        32, // element count (each element is of size transfer_data_size)
                        false); // don't start yet
  dma_channel_configure(side.dma_rx, &side.config_rx,
                        side.rx_message.buf, // write address
                        &spi_get_hw(side.port)->dr, // read address
                        32, // element count (each element is of size transfer_data_size)
                        false); // don't start yet
  dma_start_channel_mask((1u << side.dma_tx) | (1u << side.dma_rx));
}

void SPIComms::disableSide(spi_side &side) {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_deinit(side.port);
  dma_channel_unclaim(side.dma_tx);
  dma_channel_unclaim(side.dma_rx);
}

void SPIComms::initSide(spi_side &side) {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  Serial.printf("Init %i", side.side);
  spi_init(side.port, side.speed);
  spi_set_format(side.port, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
  spi_set_slave(side.port, true);
  gpio_set_function(side.mosi, GPIO_FUNC_SPI);
  gpio_set_function(side.miso, GPIO_FUNC_SPI);
  gpio_set_function(side.clock, GPIO_FUNC_SPI);
  gpio_set_function(side.cs, GPIO_FUNC_SPI);

  side.dma_tx = dma_claim_unused_channel(true);
  side.dma_rx = dma_claim_unused_channel(true);
  side.config_tx = dma_channel_get_default_config(side.dma_tx);
  side.config_rx = dma_channel_get_default_config(side.dma_rx);
  irq_set_exclusive_handler(side.side ? DMA_IRQ_1 : DMA_IRQ_0, side.side ? irqHandlerRight : irqHandlerLeft);
  irq_set_enabled(side.side ? DMA_IRQ_1 : DMA_IRQ_0, true);
  side.side ? dma_channel_set_irq1_enabled(side.dma_rx, true) : dma_channel_set_irq0_enabled(side.dma_rx, true);
  startDMA(side);
}

void SPIComms::initSPI() {
  gpio_init(SIDE_nRESET_1);  // SWe 20220719: new Neuron2 reset or no reset
  gpio_set_dir(SIDE_nRESET_1, GPIO_OUT);
  gpio_init(SIDE_nRESET_2);  // SWe 20220719: new Neuron2 reset or no reset
  gpio_set_dir(SIDE_nRESET_2, GPIO_OUT);
  gpio_put(SIDE_nRESET_1, false);
  gpio_put(SIDE_nRESET_2, false);
  sleep_ms(1);
  gpio_put(SIDE_nRESET_1, true);
  gpio_put(SIDE_nRESET_2, true);
  sleep_ms(100);
  initSide(spi_right);
  initSide(spi_left);
}

}

#endif