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

#include "SPII.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

namespace kaleidoscope::device::dygma::wired {

SPII::SPII(bool side) {
  side_ = side == 0 ? spi_left_ : spi_right_;
  initSide();
}

SPII::~SPII() {
  dma_channel_unclaim(dma_tx);
  dma_channel_unclaim(dma_rx);
}

void SPII::initSide() {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  //Serial.printf("init Serial");
  spi_init(side_.port, clock_khz_);
  spi_set_format(side_.port, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
  spi_set_slave(side_.port, true);
  gpio_set_function(side_.mosi, GPIO_FUNC_SPI);
  gpio_set_function(side_.miso, GPIO_FUNC_SPI);
  gpio_set_function(side_.clock, GPIO_FUNC_SPI);
  gpio_set_function(side_.cs, GPIO_FUNC_SPI);

  dma_tx = dma_claim_unused_channel(true);
  dma_rx = dma_claim_unused_channel(true);
  config_tx = dma_channel_get_default_config(dma_tx);
  channel_config_set_transfer_data_size(&config_tx, DMA_SIZE_8);
  channel_config_set_dreq(&config_tx, spi_get_dreq(side_.port, true));
  dma_channel_configure(dma_tx, &config_tx,
                        &spi_get_hw(side_.port)->dr, // write address
                        tx_message.buf, // read address
                        sizeof (Message), // element count (each element is of size transfer_data_size)
                        false); // don't start yet

  // We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
  // We configure the read address to remain unchanged for each element, but the write
  // address to increment (so data is written throughout the buffer)
  config_rx = dma_channel_get_default_config(dma_rx);
  channel_config_set_transfer_data_size(&config_rx, DMA_SIZE_8);
  channel_config_set_dreq(&config_rx, spi_get_dreq(side_.port, false));
  channel_config_set_read_increment(&config_rx, false);
  channel_config_set_write_increment(&config_rx, true);
  dma_channel_configure(dma_rx, &config_rx,
                        rx_message.buf, // write address
                        &spi_get_hw(side_.port)->dr, // read address
                        sizeof(Message), // element count (each element is of size transfer_data_size)
                        false); // don't start yet

  dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
}

uint8_t SPII::crc_errors() {
  return crc_errors_;
}
uint8_t SPII::writeTo(uint8_t *data, size_t length) { return 0; }
uint8_t SPII::readFrom(uint8_t *data, size_t length) {
  uint8_t rtn = 0;
  if(dma_channel_is_busy(dma_tx)||dma_channel_is_busy(dma_rx)) return 0;
  if(rx_message.context.cmd == SPI_CMD_KEYS){
    Serial.printf("It's me the side %i\n",side_.side);
    Serial.printf("cmd=%d\n"
                  "bit_arr=%d\n"
                  "sync=%d\n"
                  "size=%d\n",
                  rx_message.context.cmd,
                  rx_message.context.bit_arr,
                  rx_message.context.sync,
                  rx_message.context.size);
    data[0] = rx_message.buf[0];
    data[1] = rx_message.buf[4];
    data[2] = rx_message.buf[5];
    data[3] = rx_message.buf[6];
    data[4] = rx_message.buf[7];
    data[5] = rx_message.buf[8];
    rtn = 6;
  }
  dma_channel_configure(dma_tx, &config_tx,
                        &spi_get_hw(side_.port)->dr, // write address
                        tx_message.buf, // read address
                        sizeof(Message), // element count (each element is of size transfer_data_size)
                        false); // don't start yet
  dma_channel_configure(dma_rx, &config_rx,
                        rx_message.buf, // write address
                        &spi_get_hw(side_.port)->dr, // read address
                        sizeof (Message), // element count (each element is of size transfer_data_size)
                        false); // don't start yet
  dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
  return rtn;
}
}

#endif