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
  return 0;
}
}

#endif

// EXAMPLEEEEE


// #include <stdio.h>
// #include <stdlib.h>
// #include "pico/stdlib.h"
// #include "pico/binary_info.h"
// #include "hardware/spi.h"
// #include "hardware/dma.h"

// #include "tusb.h"
// #include "hardware/irq.h"  // interrupts

// #ifdef PICO_DEFAULT_SPI
// #undef PICO_DEFAULT_SPI
// #define PICO_DEFAULT_SPI 0
// #endif

// #ifdef PICO_DEFAULT_SPI_SCK_PIN
// #undef PICO_DEFAULT_SPI_SCK_PIN
// #define PICO_DEFAULT_SPI_SCK_PIN 2          // 18 -> 2; see GPIO locations figure
// #endif

// #ifdef PICO_DEFAULT_SPI_TX_PIN
// #undef PICO_DEFAULT_SPI_TX_PIN
// #define PICO_DEFAULT_SPI_TX_PIN  3          // 19 -> 3; see GPIO locations figure  
// #endif

// #ifdef PICO_DEFAULT_SPI_RX_PIN
// #undef PICO_DEFAULT_SPI_RX_PIN
// #define PICO_DEFAULT_SPI_RX_PIN  0          // 16 -> 0; see GPIO locations figure  
// #endif

// #ifdef PICO_DEFAULT_SPI_CSN_PIN
// #undef PICO_DEFAULT_SPI_CSN_PIN
// #define PICO_DEFAULT_SPI_CSN_PIN  1          // 17 -> 1; see GPIO locations figure  
// #endif

// #define TEST_SIZE 1024

// int iStart_Rx_Dma = 0;
// uint dma_rx;

// // static uint16_t rxbuf_my = 0x23;
// static uint16_t rxbuf_my[4] = {0x23, 0x23, 0x23, 0x23};     // make sure this has same size as txbuf_my in the SPI master
// int rxbuf_my_INDEX = 0;
// volatile int dma_finished = 0;
// absolute_time_t t1, t2;

// void cs_pin_handler(){
//     // static int rxbuf_my_INDEX = 0;
//     t1 = get_absolute_time();
//     // printf("SPI CS asserted by master\n");
//     gpio_acknowledge_irq(PICO_DEFAULT_SPI_CSN_PIN, GPIO_IRQ_EDGE_FALL);
//      iStart_Rx_Dma = 1;
//     dma_start_channel_mask(1u << dma_rx);
//     // dma_channel_set_write_addr(dma_rx, &rxbuf_my[rxbuf_my_INDEX], true);
//     rxbuf_my_INDEX++;
// }

// void dma_irq_0_handler(){
//     // irq_set_enabled(DMA_IRQ_0, false);
//     irq_clear(DMA_IRQ_0);
//     // printf("DMA IRQ 0 happened, rxbuf_my_INDEX = %d\n", rxbuf_my_INDEX);
//     t2 = get_absolute_time();
//     printf("DMA IRQ 0 happened, val = 0x%x, rxbuf_my_INDEX = %d\n, time from CS assert to DMA data rx = %ld", rxbuf_my[rxbuf_my_INDEX-1], rxbuf_my_INDEX, (to_us_since_boot(t2) - to_us_since_boot(t1)) );
//     // for(int i = 0; i<4; i++){
//     //     printf("rxbuf_my[%d] = 0x%x\n", i, rxbuf_my[i]);
//     // }
//     // rxbuf_my = 0;
//     dma_channel_acknowledge_irq0(dma_rx);
//     // irq_set_enabled(DMA_IRQ_0, true);
//     // if(rxbuf_my_INDEX == 4){
//     //     dma_finished = 1;
//     // }
// }


// int main() {
//     // Enable UART so we can print status output
//     stdio_init_all();
// #if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
// #warning spi/spi_dma example requires a board with SPI pins
//     puts("Default SPI pins were not defined");
// #else

//     // remove this to skip having to wait to putty to start this program
//     while (!tud_cdc_connected()) {
//         tight_loop_contents();
//     }

//     printf("SPI slave DMA example\n");

//     // Enable SPI at 1 MHz and connect to GPIOs
//     spi_init(spi_default, 1000 * 1000);
//     gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
//     gpio_init(PICO_DEFAULT_SPI_CSN_PIN);        // check this again later
//     // manually check the state of SPI CSN pin in case of slave
//     gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_IN);
//     gpio_pull_up(PICO_DEFAULT_SPI_CSN_PIN);
//     gpio_set_irq_enabled_with_callback(PICO_DEFAULT_SPI_CSN_PIN, GPIO_IRQ_EDGE_FALL , true, &cs_pin_handler);
//     // gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI); // original did not generate CS low signal when MOSI/SCK were generated
//     gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
//     gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
//     // Make the SPI pins available to picotool
//     bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
//     // Make the CS pin available to picotool
//     bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

//     // Grab some unused dma channels
//     const uint dma_tx = dma_claim_unused_channel(true);
//     // const uint dma_rx = dma_claim_unused_channel(true);
//     dma_rx = dma_claim_unused_channel(true);

//     // Force loopback for testing (I don't have an SPI device handy)
//     // hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);     // manipulate SPI0 (default SPI) SSPCR1 register to set SPI to loopback mode
//     // enable normal SPI mode
//     hw_clear_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);
//     // hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_SSE_BITS);     // already set inside spi_init()
//     hw_set_bits(&spi_get_hw(spi_default)->cr0, SPI_SSPCR0_DSS_BITS);        // set 16 bits SPI data size
//     hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_MS_BITS);     // set as slave device
//     hw_set_bits(&spi_get_hw(spi_default)->dmacr, SPI_SSPDMACR_RXDMAE_BITS);


//     static uint8_t txbuf[TEST_SIZE];
//     static uint8_t rxbuf[TEST_SIZE];
//     for (uint i = 0; i < TEST_SIZE; ++i) {
//         txbuf[i] = rand();
//     }

//     // static uint16_t rxbuf_my = 0x23;

//     // We set the outbound DMA to transfer from a memory buffer to the SPI transmit FIFO paced by the SPI TX FIFO DREQ
//     // The default is for the read address to increment every element (in this case 1 byte - DMA_SIZE_8)
//     // and for the write address to remain unchanged.

//     printf("Configure RX DMA\n");
//     dma_channel_config c = dma_channel_get_default_config(dma_rx);
//     // channel_config_set_transfer_data_size(&c, DMA_SIZE_8);       
//     channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
//     channel_config_set_read_increment(&c, false);           // set this to false for slave; reading from peripheral
//     channel_config_set_write_increment(&c, false);           // set this to false in order to write to the same memory address everytime (for test only)
//     // channel_config_set_dreq(&c, spi_get_index(spi_default) ? DREQ_SPI1_TX : DREQ_SPI0_TX);
//     channel_config_set_dreq(&c, spi_get_index(spi_default) ? DREQ_SPI1_RX : DREQ_SPI0_RX);
//     dma_channel_configure(dma_rx, &c,
//                           // NULL, // write address
//                           rxbuf_my,
//                           &spi_get_hw(spi_default)->dr, // read address
//                           1, // element count (each element is of size transfer_data_size)
//                           false); // don't start yet
//     // c->ctrl = (c->ctrl | DMA_CH0_CTRL_TRIG_INCR_READ_BITS) : (c->ctrl & ~DMA_CH0_CTRL_TRIG_INCR_READ_BITS);

//     irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_0_handler); 
//     irq_set_enabled(DMA_IRQ_0, true);
//     dma_channel_set_irq0_enabled (dma_rx, true);

//    // for(int i = 0; i<3; i++){
//         printf("Entering first while() loop (waiting for CS pin)\n");
//         while(1){
//             __wfi(); // Wait for Interrupt
//             // if(iStart_Rx_Dma == 1){
//             //     // dma_start_channel_mask(1u << dma_rx);
//             //     break;
//             // }
//             // printf("%d\n", rxbuf_my_INDEX);
//             if(dma_finished == 1){
//             // if(rxbuf_my_INDEX == 4){
//                 break;
//             }

//         }
//         printf("Wait for RX complete...\n");
//         dma_channel_wait_for_finish_blocking(dma_rx);
//         // printf("Rx no. %d complete; rxbuf_my = 0x%x\n",i, rxbuf_my);
//         // printf("rxbuf_my = 0x%x\n", rxbuf_my);
//         // rxbuf_my = 0;
//    // }

//     printf("All good\n");
//     dma_channel_unclaim(dma_tx);
//     dma_channel_unclaim(dma_rx);

//     for(int i = 0; i<4; i++){
//         printf("rxbuf_my[%d] = 0x%x\n", i, rxbuf_my[i]);
//     }


//     printf("Entering dummy while() loop\n");
//     while(1){
//         __wfi(); // Wait for Interrupt
//     }

//     return 0;
// #endif
// }