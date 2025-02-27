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

    Official documentation:
    https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.1.0/hardware_driver_spi_slave.html
    https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.1.0/group__nrfx__spis.html

    NRF_SPIS_MODE_0: SCK active high, sample on leading edge of clock.  -> CPOL = 0 / CPHA = 0
    NRF_SPIS_MODE_1: SCK active high, sample on trailing edge of clock. -> CPOL = 0 / CPHA = 1
    NRF_SPIS_MODE_2: SCK active low, sample on leading edge of clock.   -> CPOL = 1 / CPHA = 0
    NRF_SPIS_MODE_3: SCK active low, sample on trailing edge of clock.  -> CPOL = 1 / CPHA = 1
*/

#include "Spi_slave.h"
#include "link/spi_link_slave.h"
#include "CRC_wrapper.h"
#include "common.h"

#define SPILS_MESSAGE_SIZE_MAX          (SPI_SLAVE_PACKET_SIZE * 4)
#define SPILS_DISCONNECT_TIMEOUT_MS     1000

typedef struct
{
    uint8_t spi_port;
    hal_mcu_spi_periph_def_t periph_def;
} spi_port_def_t;

static const spi_port_def_t p_spi_port_def_array[] =
{
    { .spi_port = 0, .periph_def = HAL_MCU_SPI_PERIPH_DEF_SPI0 },
    { .spi_port = 1, .periph_def = HAL_MCU_SPI_PERIPH_DEF_SPI1 },
};
#define get_spi_port_def( def, id ) _get_def( def, p_spi_port_def_array, spi_port_def_t, spi_port, id )

void Spi_slave::spils_event_handler(void *p_instance, spils_event_type_t event_type) 
{
    Spi_slave * p_slave = ( Spi_slave* )p_instance;

    switch( event_type )
    {
        case SPILS_EVENT_TYPE_DISCONNECTED:

            p_slave->is_connected_ = false;

            break;

        case SPILS_EVENT_TYPE_CONNECTED:

            p_slave->is_connected_ = true;

            break;

        case SPILS_EVENT_TYPE_DATA_IN_READY:

            p_slave->spils_data_in_received = true;

            break;

        case SPILS_EVENT_TYPE_DATA_OUT_SENT:

            p_slave->spils_data_out_sending = false;

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled SPI Link Slave event" );

            break;
    }


}

Spi_slave::Spi_slave(uint8_t _spi_port, uint32_t _miso_pin, uint32_t _mosi_pin, uint32_t _sck_pin, uint32_t _cs_pin)
  : spi_port(_spi_port),
    miso_pin(_miso_pin),
    mosi_pin(_mosi_pin),
    sck_pin(_sck_pin),
    cs_pin(_cs_pin) {

  rx_fifo = &spi_rx_fifo;
  tx_fifo = &spi_tx_fifo;
};

void Spi_slave::init( void )
{
    result_t result = RESULT_ERR;

    spils_conf_t config;
    const spi_port_def_t * p_spi_port_def;

    /* Get the SPI port definition */
    get_spi_port_def( p_spi_port_def, spi_port );
    ASSERT_DYGMA(p_spi_port_def != NULL, "Invalid SPI port selection");

    /* SPI HAL */
    config.spi.def = p_spi_port_def->periph_def;

    config.spi.pin_miso = (hal_mcu_gpio_pin_t)miso_pin;
    config.spi.pin_mosi = (hal_mcu_gpio_pin_t)mosi_pin;
    config.spi.pin_sck  = (hal_mcu_gpio_pin_t)sck_pin;
    config.spi.pin_cs   = (hal_mcu_gpio_pin_t)cs_pin;

    config.spi.line.freq = SPI_SPEED;
    config.spi.line.cpha = HAL_MCU_SPI_CPHA_TRAIL; /* Clock phase */
    config.spi.line.cpol = HAL_MCU_SPI_CPOL_ACTIVE_HIGH; /* Clock polarity */
    config.spi.line.bit_order = HAL_MCU_SPI_BIT_ORDER_MSB_FIRST;

    /* GPIO */
    config.pin_int_enable = false;
    //config.pin_int = ;

    /* Cache */
    config.message_size_max = SPILS_MESSAGE_SIZE_MAX;

    /* Connection */
    config.disconnect_timeout_ms = SPILS_DISCONNECT_TIMEOUT_MS;

    /* Event handlers */
    config.p_instance = this;
    config.event_handler = spils_event_handler;

    result = spils_init( &p_spils, &config );
    ASSERT_DYGMA( result == RESULT_OK, "spils_init failed" );
    EXIT_IF_ERR( result, "spils_init failed" );

_EXIT:
    return;
}

void Spi_slave::run( void )
{
    spils_poll( p_spils );

    data_in_process( );
    data_out_process( );
}

bool_t Spi_slave::is_connected(void)
{
    return is_connected_;
}

void Spi_slave::packet_in_process( Communications_protocol::Packet * p_spi_packet )
{
    uint8_t spi_packet_crc;

    /* Parse the packet */
    spi_packet_crc = p_spi_packet->header.crc;
    p_spi_packet->header.crc = 0;
    if ( crc8( p_spi_packet->buf, sizeof(Communications_protocol::Header) + p_spi_packet->header.size ) == spi_packet_crc )
    {
        spi_rx_fifo.put( p_spi_packet );  // Put the new spi_packet in the Rx FIFO.
    }
}

void Spi_slave::data_in_process( void )
{
    result_t result = RESULT_ERR;

    Communications_protocol::Packet * p_spi_packet_in;

    uint8_t p_data[SPILS_MESSAGE_SIZE_MAX];
    uint16_t data_pos = 0;
    uint16_t data_in_len;

    //This will be true if the event handler (spils_event_handler) has been called with SPILS_EVENT_TYPE_DATA_IN_READY event.
    if( spils_data_in_received == false )
    {
        return;
    }

    memset( p_data, 0x00, sizeof( p_data ) );

    result = spils_data_read( p_spils, p_data, &data_in_len );
    ASSERT_DYGMA( (data_in_len % sizeof(Communications_protocol::Packet) ) == 0, "Invalid size of the SPI slave packet received" );
    EXIT_IF_NOK( result );

    spils_data_in_received = spils_data_read_available( p_spils );

    while( data_in_len >= sizeof(Communications_protocol::Packet) )
    {
        p_spi_packet_in = ( Communications_protocol::Packet *)&p_data[data_pos];
        packet_in_process( p_spi_packet_in );

        data_pos += sizeof(Communications_protocol::Packet);
        data_in_len -= sizeof(Communications_protocol::Packet);
    }

_EXIT:
    return;
}

void Spi_slave::data_out_process( void )
{
    result_t result = RESULT_ERR;
    Communications_protocol::Packet spi_packet;
    size_t data_out_len;

    /* Check if the send process is still running or the TX fifo is empty */
    if( spils_data_out_sending == true || spi_tx_fifo.is_empty( ) == true )
    {
        return;
    }

    /* Get packet from the Tx fifo */
    data_out_len = spi_tx_fifo.get( &spi_packet );
    ASSERT_DYGMA( data_out_len == sizeof( spi_packet ), "Failure: Empty Packet received from FIFO" );

    spi_packet.header.has_more_packets = ( spi_tx_fifo.is_empty() == true ) ? false : true;
    spi_packet.header.crc = 0;
    spi_packet.header.crc = crc8( spi_packet.buf, sizeof(Communications_protocol::Header) + spi_packet.header.size );

    /* This is for the possible hazard handling. The receive end callback might theoretically come before the end of the function */
    spils_data_out_sending = true;
    result = spils_data_send( p_spils, spi_packet.buf, sizeof( spi_packet.buf ) );
    ASSERT_DYGMA( result == RESULT_OK, "Failure: spils_data_send failed" );
    EXIT_IF_NOK( result );

_EXIT:
    if( result != RESULT_OK )
    {
        spils_data_out_sending = false;
    }

    return;

    UNUSED( data_out_len );
}
