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
#include "SPII.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

namespace kaleidoscope::device::dygma::wired {

SPII port_left(0);
SPII port_right(1);

//void updateHand(spi_side &spi_settings_) {
//  bool sideCom = spi_settings_.rx_message.context.bit_arr & 0b00000001;
//  uint8_t (&key_data)[5] = sideCom ? right_keys : left_keys;
//  bool &new_key = sideCom ? new_key_right : new_key_left;
//  key_data[0] = spi_settings_.rx_message.buf[sizeof(Context) + 0];
//  key_data[1] = spi_settings_.rx_message.buf[sizeof(Context) + 1];
//  key_data[2] = spi_settings_.rx_message.buf[sizeof(Context) + 2];
//  key_data[3] = spi_settings_.rx_message.buf[sizeof(Context) + 3];
//  key_data[4] = spi_settings_.rx_message.buf[sizeof(Context) + 4];
//  new_key = true;
//}
//
//void updateLeds(spi_side &spi_settings_) {
//  bool sideCom = spi_settings_.rx_message.context.bit_arr & 0b00000001;
//  auto tx_messages = sideCom ? &tx_messages_right : &tx_messages_left;
//
//  if (!tx_messages->empty()) {
//    memcpy(spi_settings_.tx_message.buf, tx_messages->front().buf, sizeof(Message));
//    tx_messages->pop();
//  } else {
//    spi_settings_.tx_message.context.cmd = 0;
//  }
//}
//
//void __no_inline_not_in_flash_func(irqHandler)(uint8_t irqNum,
//                                               spi_side &spi_settings_) {

//}

void __no_inline_not_in_flash_func(dma_irq_1_handler)() {
  port_right.irq();
}

void __no_inline_not_in_flash_func(dma_irq_0_handler)() {
  port_left.irq();
}

void SPII::startDMA() {
  channel_config_set_transfer_data_size(&spi_settings_.config_tx, DMA_SIZE_8);
  channel_config_set_dreq(&spi_settings_.config_tx, spi_get_dreq(spi_settings_.port, true));

  channel_config_set_transfer_data_size(&spi_settings_.config_rx, DMA_SIZE_8);
  channel_config_set_dreq(&spi_settings_.config_rx, spi_get_dreq(spi_settings_.port, false));
  channel_config_set_read_increment(&spi_settings_.config_rx, false);
  channel_config_set_write_increment(&spi_settings_.config_rx, true);

  dma_channel_configure(spi_settings_.dma_tx, &spi_settings_.config_tx,
                        &spi_get_hw(spi_settings_.port)->dr, // write address
                        spi_settings_.tx_message.buf, // read address
                        32, // element count (each element is of size transfer_data_size)
                        false); // don't start yet
  dma_channel_configure(spi_settings_.dma_rx, &spi_settings_.config_rx,
                        spi_settings_.rx_message.buf, // write address
                        &spi_get_hw(spi_settings_.port)->dr, // read address
                        32, // element count (each element is of size transfer_data_size)
                        false); // don't start yet
  dma_start_channel_mask((1u << spi_settings_.dma_tx) | (1u << spi_settings_.dma_rx));
}

void SPII::initInterrupt() {
  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_init(spi_settings_.port, spi_settings_.speed);
  spi_set_format(spi_settings_.port, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
  spi_set_slave(spi_settings_.port, true);
  gpio_set_function(spi_settings_.mosi, GPIO_FUNC_SPI);
  gpio_set_function(spi_settings_.miso, GPIO_FUNC_SPI);
  gpio_set_function(spi_settings_.clock, GPIO_FUNC_SPI);
  gpio_set_function(spi_settings_.cs, GPIO_FUNC_SPI);

  spi_settings_.dma_tx = dma_claim_unused_channel(true);
  spi_settings_.dma_rx = dma_claim_unused_channel(true);
  spi_settings_.config_tx = dma_channel_get_default_config(spi_settings_.dma_tx);
  spi_settings_.config_rx = dma_channel_get_default_config(spi_settings_.dma_rx);
  irq_set_exclusive_handler(spi_settings_.irq, port_ ? dma_irq_1_handler : dma_irq_0_handler);
  irq_set_enabled(spi_settings_.irq, true);
  port_ ? dma_channel_set_irq1_enabled(spi_settings_.dma_rx, true) : dma_channel_set_irq0_enabled(spi_settings_.dma_rx,
                                                                                                  true);
  startDMA();
}

SPII::SPII(bool side) : port_(side) {
  if (side) {
    spi_settings_ = {SPI_PORT1, SPI_MOSI1, SPI_MISO1, SPI_CLK1, SPI_CS1, SPI_SPEED, SIDE_nRESET_1, DMA_IRQ_1};
  } else {
    spi_settings_ = {SPI_PORT2, SPI_MOSI2, SPI_MISO2, SPI_CLK2, SPI_CS2, SPI_SPEED, SIDE_nRESET_2, DMA_IRQ_0};
  }
  gpio_init(spi_settings_.reset);
  gpio_set_dir(spi_settings_.reset, GPIO_OUT);
  gpio_put(spi_settings_.reset, false);
  sleep_us(1);
  gpio_put(spi_settings_.reset, true);
  queue_init(&tx_messages, sizeof(Message), 40);
  queue_init(&rx_messages, sizeof(Message), 40);
}

void SPII::initCommunications() {
  initInterrupt();
}

SPII::~SPII() {
  disableSide();
}

uint8_t SPII::crc_errors() {
  return 0;
}
uint8_t SPII::writeTo(uint8_t *data, size_t length) {
  if (data[0] >= 0x80) {
    //The device is not online
//    if (millis() - last_time_communication_ > 1000) {
//      return 0;
//    }
    Message message;
    if (queue_is_full(&tx_messages)) {
      queue_remove_blocking(&tx_messages,&message);
    }
    message.context.cmd = 1;
    message.context.size = length;
    for (uint8_t i = 1; i < length; ++i) {
      message.buf[sizeof(Context) + i] = data[i];
    }
    message.buf[sizeof(Context)] = data[0] - 0x80;
    queue_add_blocking(&tx_messages, &message);
  }
  return 0;
}
uint8_t SPII::readFrom(uint8_t *data, size_t length) {
  if (queue_is_empty(&rx_messages)) {
    data[0] = 0;
    return 6;
  }
  Message message;
  queue_remove_blocking(&rx_messages, &message);
  data[0] = 1;
  data[1] = message.buf[sizeof(Context) + 0];
  data[2] = message.buf[sizeof(Context) + 1];
  data[3] = message.buf[sizeof(Context) + 2];
  data[4] = message.buf[sizeof(Context) + 3];
  data[5] = message.buf[sizeof(Context) + 4];
  if (millis() - last_time_communication_ > 1000) return 0;
  return 6;
}

void SPII::irq() {
  irq_set_enabled(spi_settings_.irq, false);
  irq_clear(spi_settings_.irq);
  port_ ? dma_channel_acknowledge_irq1(spi_settings_.dma_rx)
        : dma_channel_acknowledge_irq0(spi_settings_.dma_rx);
  if (spi_settings_.rx_message.context.sync == 0) {
    //Something happened lest restart the communication
    if (Serial.available())
      Serial.printf("Lost Connections with hand %i\n", port_);
    disableSide();
    gpio_put(spi_settings_.reset, false);
    initInterrupt();
    gpio_put(spi_settings_.reset, true);
    return;
  }
  last_time_communication_ = millis();
  bool sideCom = spi_settings_.rx_message.context.bit_arr & 0b00000001;
  if (spi_settings_.rx_message.context.cmd != 0) {
    if (sideCom) {
      queue_add_blocking(&port_right.rx_messages, &spi_settings_.rx_message);
      if (!queue_is_empty(&port_right.tx_messages)) {
        queue_remove_blocking(&port_right.tx_messages, &spi_settings_.tx_message);
      }
    } else {
      queue_add_blocking(&port_left.rx_messages, &spi_settings_.rx_message);
      if (!queue_is_empty(&port_left.tx_messages)) {
        queue_remove_blocking(&port_left.tx_messages, &spi_settings_.tx_message);
      }
    }
  }
  if (sideCom) {
    if (!queue_is_empty(&port_right.tx_messages)) {
      queue_remove_blocking(&port_right.tx_messages, &spi_settings_.tx_message);
    }
  } else {
    if (!queue_is_empty(&port_left.tx_messages)) {
      queue_remove_blocking(&port_left.tx_messages, &spi_settings_.tx_message);
    }
  }
  irq_set_enabled(spi_settings_.irq, true);
  startDMA();
}

void SPII::disableSide() {
  spi_deinit(spi_settings_.port);
  dma_channel_unclaim(spi_settings_.dma_tx);
  dma_channel_unclaim(spi_settings_.dma_rx);
}
}

#endif