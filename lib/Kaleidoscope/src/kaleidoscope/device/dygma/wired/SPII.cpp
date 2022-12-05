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

SPII communication_left(0);
SPII communication_right(1);

static void spiInit();

static void initSide(spi_side &side);

static void disableSide(spi_side &side);

static void startDMA(spi_side &side);

static void updateHand(spi_side &side);

static void updateLeds(spi_side &side);

static void irqHandler(uint8_t irqNum, spi_side &side);

static void dma_irq_1_handler();

static void dma_irq_0_handler();

void startDMA(spi_side &side) {
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

void updateHand(spi_side &side) {
  bool sideCom = side.rx_message.context.bit_arr & 0b00000001;
  uint8_t (&key_data)[5] = sideCom ? right_keys : left_keys;
  bool &new_key = sideCom ? new_key_right : new_key_left;
  key_data[0] = side.rx_message.buf[sizeof(Context) + 0];
  key_data[1] = side.rx_message.buf[sizeof(Context) + 1];
  key_data[2] = side.rx_message.buf[sizeof(Context) + 2];
  key_data[3] = side.rx_message.buf[sizeof(Context) + 3];
  key_data[4] = side.rx_message.buf[sizeof(Context) + 4];
  new_key = true;
}

void updateLeds(spi_side &side) {
  bool sideCom = side.rx_message.context.bit_arr & 0b00000001;
  auto tx_messages = sideCom ? &tx_messages_right : &tx_messages_left;

  if (!tx_messages->empty()) {
    memcpy(side.tx_message.buf, tx_messages->front().buf, sizeof(Message));
    tx_messages->pop();
  } else {
    side.tx_message.context.cmd = 0;
  }
}

void __no_inline_not_in_flash_func(irqHandler)(uint8_t irqNum,
                                               spi_side &side) {
  irq_set_enabled(irqNum, false);
  irq_clear(irqNum);
  side.side ? dma_channel_acknowledge_irq1(side.dma_rx) : dma_channel_acknowledge_irq0(side.dma_rx);
  bool sideCom = side.rx_message.context.bit_arr & 0b00000001;
  uint32_t &lastTime = sideCom ? lastTime_right : lastTime_left;
  lastTime = millis();
  if (side.rx_message.context.sync == 0) {
    //Something happened lest restart the communication
    Serial.printf("Lost Connections with hand %i\n", side.side);
    disableSide(side);
    gpio_put(side.side ? SIDE_nRESET_1 : SIDE_nRESET_2, false);
    initSide(side);
    gpio_put(side.side ? SIDE_nRESET_1 : SIDE_nRESET_2, true);
    return;
  }
  if (side.rx_message.context.cmd == SPI_CMD_KEYS) {
    updateHand(side);
  }
  updateLeds(side);
  irq_set_enabled(irqNum, true);
  startDMA(side);
}

void __no_inline_not_in_flash_func(dma_irq_1_handler)() {
  irqHandler(DMA_IRQ_1, spi_right);
}

void __no_inline_not_in_flash_func(dma_irq_0_handler)() {
  irqHandler(DMA_IRQ_0, spi_left);
}

void disableSide(spi_side &side) {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_deinit(side.port);
  dma_channel_unclaim(side.dma_tx);
  dma_channel_unclaim(side.dma_rx);
}

void initSide(spi_side &side) {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
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
  irq_set_exclusive_handler(side.side ? DMA_IRQ_1 : DMA_IRQ_0, side.side ? dma_irq_1_handler : dma_irq_0_handler);
//  irq_set_enabled(side.side ? DMA_IRQ_1 : DMA_IRQ_0, true);
  side.side ? dma_channel_set_irq1_enabled(side.dma_rx, true) : dma_channel_set_irq0_enabled(side.dma_rx, true);
  startDMA(side);
}

static void spiInit() {
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

void SPII::initSPI() {

}

SPII::SPII(bool side) : side_(side) {
  if (!initSPII_) {
//    multicore_launch_core1(SPIComunications::init);
    spiInit();
    initSPII_ = true;
  }
}

SPII::~SPII() {
}

uint8_t SPII::crc_errors() {
  return 0;
}
uint8_t SPII::writeTo(uint8_t *data, size_t length) {
  auto tx_messages = side_ ? &tx_messages_right : &tx_messages_left;
  auto lastTime = side_ ? lastTime_right : lastTime_left;
  if (data[0] >= 0x80) {
    //The device is not online
    if (millis() - lastTime > 1000) {
      *tx_messages = {};
      return 0;
    }
    Message m;
    m.context.cmd = 1;
    m.context.size = length;
    for (uint8_t i = 1; i < length; ++i) {
      m.buf[sizeof(Context) + i] = data[i];
    }
    m.buf[sizeof(Context)] = data[0] - 0x80;
    tx_messages->push(m);
  }
  return 0;
}
uint8_t SPII::readFrom(uint8_t *data, size_t length) {
  auto key_data = side_ ? right_keys : left_keys;
  auto new_key = side_ ? new_key_right : new_key_left;
  auto lastTime = side_ ? lastTime_right : lastTime_left;
  data[0] = new_key;
  data[1] = key_data[0];
  data[2] = key_data[1];
  data[3] = key_data[2];
  data[4] = key_data[3];
  data[5] = key_data[4];
  if (millis() - lastTime > 1000) return 0;
  return 6;
}
}

#endif