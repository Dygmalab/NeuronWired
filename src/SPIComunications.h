#ifndef NEURONWIRED_SRC_SPICOMUNICATIONS_H_
#define NEURONWIRED_SRC_SPICOMUNICATIONS_H_

#include <Arduino.h>
#include <hardware/dma.h>
#include "hardware/spi.h"
#include "kaleidoscope/device/dygma/wired/Hand.h"
#include "kaleidoscope/device/dygma/Wired.h"

namespace SPIComunications {

#define SPI_PORT1 spi0
#define SPI_SPEED 1000 * 1000
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
  bool side;
  int dma_tx;
  int dma_rx;
  dma_channel_config config_tx;
  dma_channel_config config_rx;
  Message tx_message;
  Message rx_message;
};

static spi_side spi_right{SPI_PORT1, SPI_MOSI1, SPI_MISO1, SPI_CLK1, SPI_CS1, SPI_SPEED, 1};
static spi_side spi_left{SPI_PORT2, SPI_MOSI2, SPI_MISO2, SPI_CLK2, SPI_CS2, SPI_SPEED, 0};

static void startDMA(spi_side &side) {
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
  dma_start_channel_mask((1u << spi_right.dma_tx) | (1u << spi_right.dma_rx));
}

static void updateHand(kaleidoscope::device::dygma::wired::Hand &hand, spi_side &side) {
  //mutex_enter_blocking(&hand.mutex);
  hand.key_data_.rows[0] = side.rx_message.buf[sizeof(Context) + 0];
  hand.key_data_.rows[1] = side.rx_message.buf[sizeof(Context) + 1];
  hand.key_data_.rows[2] = side.rx_message.buf[sizeof(Context) + 2];
  hand.key_data_.rows[3] = side.rx_message.buf[sizeof(Context) + 3];
  hand.key_data_.rows[4] = side.rx_message.buf[sizeof(Context) + 4];
  hand.new_key = true;
  //mutex_exit(&hand.mutex);
}

static void dma_irq_1_handler() {
  irq_set_enabled(DMA_IRQ_1, false);
  irq_clear(DMA_IRQ_1);
  dma_channel_acknowledge_irq1(spi_right.dma_rx);
  if (spi_right.rx_message.context.cmd == SPI_CMD_KEYS) {
    updateHand(kaleidoscope::device::dygma::WiredHands::leftHand ,spi_right);
  }
  irq_set_enabled(DMA_IRQ_1, true);
  startDMA(spi_right);
}

static void initSide(spi_side &side) {
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
  irq_set_exclusive_handler(DMA_IRQ_1, dma_irq_1_handler);
  irq_set_enabled(DMA_IRQ_1, true);
  dma_channel_set_irq1_enabled(side.dma_rx, true);
  startDMA(side);
}

static void init() {
  gpio_set_function(22, GPIO_FUNC_SIO);  // SWe 20220719: new Neuron2 reset or no reset
  gpio_set_function(10, GPIO_FUNC_SIO);  // SWe 20220719: new Neuron2 reset or no reset
  gpio_put(22, true);
  gpio_put(10, true);
  sleep_ms(100);
  initSide(spi_right);
}

//static void updateLed(kaleidoscope::device::dygma::wired::Hand &hand, spi_side &side) {
//  mutex_enter_blocking(&hand.mutex);
//  for (int i = 0; i < 11; ++i) {
//    if (hand.bankUpdated[i]) {
//      side.tx_message.buf[sizeof(Context)] = i;
//      for (int j = 0; j < 24; j++) {
//        side.tx_message.buf[sizeof(Context) + j + 1] = hand.led_data_spi.bytes[i][j];
//      }
//      hand.bankUpdated[i] = 0;
//      side.tx_message.context.cmd = SPI_CMD_LED_SET_BANK_TO;
//      mutex_exit(&hand.mutex);
//      return;
//    }
//  }
//  mutex_exit(&hand.mutex);
//  side.tx_message.context.cmd = 0;
//}

};

#endif //NEURONWIRED_SRC_SPICOMUNICATIONS_H_
