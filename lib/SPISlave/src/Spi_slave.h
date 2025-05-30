/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2020  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Juan Hauara @JuanHauara
 */

/*
    This class implements EasyDMA for accessing RAM without CPU involvement.

    Some chips like the NRF52833 require the chip select signal to goes high
    at the end of the transmission for the SPI to work correctly.
    On this particular chip the SPI will not work if the chip select signal
    is left low permanently.

    The default options:
    nrf_spis_mode_t spi_mode = NRF_SPIS_MODE_0,
    nrf_gpio_pin_drive_t pin_miso_strength = NRF_GPIO_PIN_S0S1,
    nrf_gpio_pin_pull_t pin_csn_pullup = NRF_GPIO_PIN_NOPULL);

    Official documentation:
    https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.1.0/hardware_driver_spi_slave.html
    https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.1.0/group__nrfx__spis.html

    Default configuration of the SPI slave instance:
    {
        .miso_pin = NRFX_SPIS_PIN_NOT_USED,
        .mosi_pin = NRFX_SPIS_PIN_NOT_USED,
        .sck_pin = NRFX_SPIS_PIN_NOT_USED,
        .csn_pin = NRFX_SPIS_PIN_NOT_USED,
        .mode = NRF_SPIS_MODE_0,
        .bit_order = NRF_SPIS_BIT_ORDER_MSB_FIRST,
        .csn_pullup = NRFX_SPIS_DEFAULT_CSN_PULLUP,   // NRFX_SPIS_DEFAULT_CSN_PULLUP is -> NRF_GPIO_PIN_NOPULL
        .miso_drive = NRFX_SPIS_DEFAULT_MISO_DRIVE,   // NRFX_SPIS_DEFAULT_MISO_DRIVE is -> NRF_GPIO_PIN_S0S1
        .def = NRFX_SPIS_DEFAULT_DEF,
        .orc = NRFX_SPIS_DEFAULT_ORC,
        .irq_priority = NRFX_SPIS_DEFAULT_CONFIG_IRQ_PRIORITY,
    }

    NRF_SPIS_MODE_0: SCK active high, sample on leading edge of clock.  -> CPOL = 0 / CPHA = 0
    NRF_SPIS_MODE_1: SCK active high, sample on trailing edge of clock. -> CPOL = 0 / CPHA = 1
    NRF_SPIS_MODE_2: SCK active low, sample on leading edge of clock.   -> CPOL = 1 / CPHA = 0
    NRF_SPIS_MODE_3: SCK active low, sample on trailing edge of clock.  -> CPOL = 1 / CPHA = 1
*/

#ifndef __SPI_SLAVE_H__
#define __SPI_SLAVE_H__

#include <Communications_protocol.h>
#include "Fifo_buffer.h"
#include "link/spi_link_slave.h"

#define SPI_SLAVE_DEBUG                 0
#define SPI_DEBUG_PRINT_RX_PACKET       0
#define NUM_BYTES_OF_RX_PACKET_TO_PRINT 8

#define COMPILE_SPI0_SUPPORT            1
#define COMPILE_SPI1_SUPPORT            1
#define COMPILE_SPI2_SUPPORT            0

#define SPI_SLAVE_PACKET_SIZE           sizeof(Communications_protocol::Packet)

class Spi_slave {
   public:
    Spi_slave(uint8_t _spi_port,
              uint32_t _miso_pin,
              uint32_t _mosi_pin,
              uint32_t _sck_pin,
              uint32_t _cs_pin
            );

    void init(void);
    //void deinit(void);
    void run(void);

    bool_t is_connected(void);

    Fifo_buffer *rx_fifo;
    Fifo_buffer *tx_fifo;

   private:
    uint8_t spi_port;

    uint32_t miso_pin;
    uint32_t mosi_pin;
    uint32_t sck_pin;
    uint32_t cs_pin;

    uint8_t spi_mode;                // NRF_SPIS_MODE_0, NRF_SPIS_MODE_1, ..

    spils_t * p_spils;

    /* Flags */
    bool_t is_connected_ = false;

    bool_t spils_data_in_received = false;
    bool_t spils_data_out_sending = false;

    /* Buffers */
    Fifo_buffer spi_rx_fifo = Fifo_buffer(SPI_SLAVE_PACKET_SIZE);
    Fifo_buffer spi_tx_fifo = Fifo_buffer(SPI_SLAVE_PACKET_SIZE);

    static void spils_event_handler( void * p_instance, spils_event_type_t event_type );

    void packet_in_process( Communications_protocol::Packet * p_spi_packet );

    /*
    * This function will act when the event SPILS_EVENT_TYPE_DATA_IN_READY is received.
    * It will read the data from the SPI slave and put it in the rx_fifo.
    * The data is read in packets of SPI_SLAVE_PACKET_SIZE.
    */
    void data_in_process(void);
    void data_out_process(void);
};


#endif  // __SPI_SLAVE_H__
