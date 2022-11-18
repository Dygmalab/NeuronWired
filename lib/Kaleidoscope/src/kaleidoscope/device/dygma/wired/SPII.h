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
#include "hardware/spi.h"
#include "pico/multicore.h"


// SPI1 slave pins (SPI0 module at the RP2040)
#define SPI_PORT1 spi0
#define SPI_MOSI1  20   //SPI-1 slave IN, we are slave
#define SPI_MISO1  23   //SPI-1 slave OUT, we are slave
#define SPI_CLK1   18  //was 22   //SPI-1 clock IN, we are slave (must be changed in HW to 18)
#define SPI_CS1    21   //SPI-1 chip select IN, we are slave
// SPI2 slave pins (SPI1 module at the RP2040)
#define SPI_PORT2 spi1
#define SPI_MOSI2   8   //SPI-2 slave IN, we are slave
#define SPI_MISO2  11   //SPI-2 slave OUT, we are slave
#define SPI_CLK2   14  //was 10   //SPI-2 clock IN, we are slave  (must be changed in HW to 14)
#define SPI_CS2     9   //SPI-2 chip select IN, we are slave
#define TRANSFER_SIZE  256

namespace kaleidoscope {
namespace device {
namespace dygma {
namespace wired {


class SPII {
 public:

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
    uint16_t size;
  };

  union Message {
    Context context;
    uint8_t buf[256];
  };
  inline static Message tx_message;
  inline static Message rx_message;
  static void comunicationSPI() {
    sleep_ms(4000);
    // Enable UART so we can print
    Serial.printf("SPI slave example\n");

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(SPI_PORT2, 1000 * 1000);
    spi_set_format(SPI_PORT2, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_set_slave(SPI_PORT2, true);
    gpio_set_function(SPI_MOSI2, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO2, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CLK2, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CS2, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    for (size_t i = 0;; ++i) {
      // Write the output buffer to MISO, and at the same time read from MOSI.
      spi_write_read_blocking(SPI_PORT2, tx_message.buf, rx_message.buf, 256);
      if (rx_message.context.cmd == SPI_CMD_KEYS) {
        Serial.printf("We have new message woooo \n");
        Serial.printf("cmd=%d\n"
                      "bit_arr=%d\n"
                      "sync=%d\n"
                      "size=%d\n",
                      rx_message.context.cmd,
                      rx_message.context.bit_arr,
                      rx_message.context.sync,
                      rx_message.context.size);

        for (int j = sizeof(Context); j < sizeof(Context) + rx_message.context.size; ++j) {
          Serial.printf("%02X, ", rx_message.buf[j]);
        }
        Serial.printf("\n");
      }
    }
  }
  inline static bool init_ = false;

  explicit SPII(bool side) : side_(side), crc_errors_(0) {
    if (!init_) {
      multicore_launch_core1(comunicationSPI);
      init_ = true;
    }
    //dma_tx = dma_claim_unused_channel(true);
    //dma_rx = dma_claim_unused_channel(true);
    //init(12000 * 1000);
  }

  uint8_t writeTo(uint8_t *data, size_t length) { return 0; };
  uint8_t readFrom(uint8_t *data, size_t length) { return 0; };
  void disable() {};
  uint8_t crc_errors() {
    return crc_errors_;
  }

 private:
  bool side_;
  spi_inst_t *spi_;
  uint8_t crc_errors_;
  uint16_t clock_khz_;
  // Grab some unused dma channels
};

}
}
}
}
#endif
