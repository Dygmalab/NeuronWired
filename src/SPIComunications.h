#ifndef NEURONWIRED_SRC_SPICOMUNICATIONS_H_
#define NEURONWIRED_SRC_SPICOMUNICATIONS_H_

#include <Arduino.h>
#include <hardware/dma.h>
#include "hardware/spi.h"
#include "kaleidoscope/device/dygma/wired/Hand.h"
#include "kaleidoscope/device/dygma/Wired.h"

namespace SPIComunications {

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
  uint8_t counter;
  int dma_tx;
  int dma_rx;
  dma_channel_config config_tx;
  dma_channel_config config_rx;
  Message tx_message;
  Message rx_message;
};

static spi_side spi_right{SPI_PORT1, SPI_MOSI1, SPI_MISO1, SPI_CLK1, SPI_CS1, SPI_SPEED, 1};
static spi_side spi_left{SPI_PORT2, SPI_MOSI2, SPI_MISO2, SPI_CLK2, SPI_CS2, SPI_SPEED, 0};

static void init();

static void initSide(spi_side &side);

static void disableSide(spi_side &side);

static void startDMA(spi_side &side);

static void updateHand(kaleidoscope::device::dygma::wired::Hand &hand, spi_side &side);

static void updateLeds(kaleidoscope::device::dygma::wired::Hand &hand, spi_side &side);

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

void updateHand(kaleidoscope::device::dygma::wired::Hand &hand, spi_side &side) {
  //mutex_enter_blocking(&hand.mutex);
  hand.key_data_.rows[0] = side.rx_message.buf[sizeof(Context) + 0];
  hand.key_data_.rows[1] = side.rx_message.buf[sizeof(Context) + 1];
  hand.key_data_.rows[2] = side.rx_message.buf[sizeof(Context) + 2];
  hand.key_data_.rows[3] = side.rx_message.buf[sizeof(Context) + 3];
  hand.key_data_.rows[4] = side.rx_message.buf[sizeof(Context) + 4];
  hand.new_key = true;
  //mutex_exit(&hand.mutex);
}

void updateLeds(kaleidoscope::device::dygma::wired::Hand &hand, spi_side &side) {
  side.tx_message.context.cmd = 0;
  if (hand.new_leds) {
    side.tx_message.context.cmd = 1;
    side.tx_message.buf[sizeof(Context) + 0] = side.counter;
    for (uint8_t i = 0; i < 24; ++i) {
      side.tx_message.buf[sizeof(Context) + 1 + i] = hand.led_data.bytes[side.counter][i];
    }
    if (++side.counter == 11) {
      side.counter = 0;
      hand.new_leds = false;
    }
  }
}

void __no_inline_not_in_flash_func(irqHandler)(uint8_t irqNum,
                                               spi_side &side) {
  irq_set_enabled(irqNum, false);
  irq_clear(irqNum);
  side.side ? dma_channel_acknowledge_irq1(side.dma_rx) : dma_channel_acknowledge_irq0(side.dma_rx);
  if (side.rx_message.context.sync == 0) {
    //Something happened lest restart the communication
    Serial.printf("Shit\n");
    disableSide(side);
    gpio_put(side.side ? SIDE_nRESET_1 : SIDE_nRESET_2, false);
    initSide(side);
    gpio_put(side.side ? SIDE_nRESET_1 : SIDE_nRESET_2, true);
    return;
  }
  bool sideCom = side.rx_message.context.bit_arr & 0b00000001;
  if (side.rx_message.context.cmd == SPI_CMD_KEYS) {
    updateHand(sideCom ? kaleidoscope::device::dygma::WiredHands::rightHand
                       : kaleidoscope::device::dygma::WiredHands::leftHand,
               side);
  }
  updateLeds(sideCom ? kaleidoscope::device::dygma::WiredHands::rightHand
                     : kaleidoscope::device::dygma::WiredHands::leftHand, side);
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
  irq_set_enabled(side.side ? DMA_IRQ_1 : DMA_IRQ_0, true);
  side.side ? dma_channel_set_irq1_enabled(side.dma_rx, true) : dma_channel_set_irq0_enabled(side.dma_rx, true);
  startDMA(side);
}

void init() {
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
};

#endif //NEURONWIRED_SRC_SPICOMUNICATIONS_H_
