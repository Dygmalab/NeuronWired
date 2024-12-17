/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
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
 */
#include "hal_mcu_dma_ll.h"
#include "hal_mcu_spi_ll.h"
#include "hardware/gpio.h"
#include "hardware/resets.h"
#include "hardware/spi.h"

//#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_RP20

#define DUMMY_OUT_VALUE     0xFF;

/* Peripheral definitions */
typedef struct
{
    hal_mcu_spi_periph_def_t def;

    hal_mcu_spi_t ** pp_periph;

    /* Pico SDK SPI */
    spi_inst_t * p_pico_spi_inst;
    uint32_t pico_reset_bits;

    hal_mcu_dma_request_type_t dma_request_type_rx;
    hal_mcu_dma_request_type_t dma_request_type_tx;

} periph_def_t;

typedef struct
{
    hal_mcu_spi_cpha_t cpha;
    spi_cpha_t pico_cpha;
} cpha_def_t;

typedef struct
{
    hal_mcu_spi_cpol_t cpol;
    spi_cpol_t pico_cpol;
} cpol_def_t;

typedef struct
{
    hal_mcu_spi_bit_order_t bit_order;
    spi_order_t pico_order;
} bit_order_def_t;

typedef struct
{
    hal_mcu_spi_t * p_spi;
    hal_mcu_dma_channel_t * p_dma;
} spi_dma_t;

typedef struct
{
    /* Chip Select */
    hal_mcu_gpio_pin_t pin_cs;

    /* SPI Line configuration */
    hal_mcu_spi_line_conf_t line;

    /* Event handlers */
    void * p_instance;
    hal_mcu_spi_slave_buffers_set_done_handler_t buffers_set_done_handler;
    hal_mcu_spi_slave_transfer_done_handler_t transfer_done_handler;

    /* Flags */
    bool_t dummy_in_used;
    bool_t resend_request;
} slave_t;

struct hal_mcu_spi
{
    hal_mcu_spi_role_t role;

    const periph_def_t * p_periph_def;

    bool_t reserved;
    bool_t busy;

    /* DMA */
    spi_dma_t dma_rx;
    spi_dma_t dma_tx;

    /* Transfer */
    uint8_t * p_data_out;
    uint8_t * p_data_in;
    size_t data_out_len;
    size_t data_in_len;

    uint8_t dummy_in;
    uint8_t dummy_out;

    /* SDK */
    spi_hw_t * p_spi_hw;

    /* Lock */
    hal_mcu_spi_lock_t lock;

    /* Role specific */
    union{
        slave_t slave;
    };
};

/* SPI peripherals */
static hal_mcu_spi_t * p_spi0 = NULL;
static hal_mcu_spi_t * p_spi1 = NULL;

/* SPI peripheral definitions */
static const periph_def_t p_periph_def_array[] =
{
    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI0, .pp_periph = &p_spi0, .p_pico_spi_inst = spi0, .pico_reset_bits = RESETS_RESET_SPI0_BITS,
            .dma_request_type_rx = HAL_MCU_DMA_REQUEST_TYPE_SPI0_RX, .dma_request_type_tx = HAL_MCU_DMA_REQUEST_TYPE_SPI0_TX, },

    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI1, .pp_periph = &p_spi1, .p_pico_spi_inst = spi1, .pico_reset_bits = RESETS_RESET_SPI1_BITS,
            .dma_request_type_rx = HAL_MCU_DMA_REQUEST_TYPE_SPI1_RX, .dma_request_type_tx = HAL_MCU_DMA_REQUEST_TYPE_SPI1_TX, },
};
#define get_periph_def( p_periph_def, id ) _get_def( p_periph_def, p_periph_def_array, periph_def_t, def, id )
#define get_slave_cs_periph_def( p_periph_def, pin_cs_id ) _get_def( p_periph_def, p_periph_def_array, periph_def_t, pp_periph[0]->slave.pin_cs, pin_cs_id )

/* SPI Clock Phase definitions */
static const cpha_def_t p_cpha_def_array[] =
{
    { .cpha = HAL_MCU_SPI_CPHA_LEAD, .pico_cpha = SPI_CPHA_0},
    { .cpha = HAL_MCU_SPI_CPHA_TRAIL, .pico_cpha = SPI_CPHA_1},
};
#define get_cpha_def( p_cpha_def, id ) _get_def( p_cpha_def, p_cpha_def_array, cpha_def_t, cpha, id )

/* SPI Clock Polarity definitions */
static const cpol_def_t p_cpol_def_array[] =
{
    { .cpol = HAL_MCU_SPI_CPOL_ACTIVE_HIGH, .pico_cpol = SPI_CPOL_0},
    { .cpol = HAL_MCU_SPI_CPOL_ACTIVE_LOW, .pico_cpol = SPI_CPOL_1},
};
#define get_cpol_def( p_cpol_def, id ) _get_def( p_cpol_def, p_cpol_def_array, cpol_def_t, cpol, id )

/* SPI Bit Order definitions */
static const bit_order_def_t p_bit_order_def_array[] =
{
    { .bit_order = HAL_MCU_SPI_BIT_ORDER_LSB_FIRST, .pico_order = SPI_LSB_FIRST},
    { .bit_order = HAL_MCU_SPI_BIT_ORDER_MSB_FIRST, .pico_order = SPI_MSB_FIRST},
};
#define get_bit_order_def( p_bit_order_def, id ) _get_def( p_bit_order_def, p_bit_order_def_array, bit_order_def_t, bit_order, id )


/* Prototypes */
static result_t _dma_init( hal_mcu_spi_t * p_spi );
static result_t _slave_init(hal_mcu_spi_t *p_spi, const hal_mcu_spi_conf_t *p_conf);
static result_t _slave_transfer( hal_mcu_spi_t* p_spi );
static INLINE result_t _slave_spi_enable( hal_mcu_spi_t * p_spi );
static INLINE void _slave_spi_disable( hal_mcu_spi_t * p_spi );

static result_t _spi_init( hal_mcu_spi_t * p_spi, const hal_mcu_spi_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    /* Initialize SPI */
    p_spi->role = p_conf->role;
    p_spi->reserved = false;
    p_spi->busy = false;

    /* Initialize the DMA - must be done before the Role-specific configuration */
    result = _dma_init( p_spi );
    EXIT_IF_ERR( result, "_dma_init failed" );

    /* Transfer */
    p_spi->p_data_out = NULL;
    p_spi->p_data_in = NULL;
    p_spi->data_out_len = 0;
    p_spi->data_in_len = 0;

    p_spi->dummy_in = 0x00;
    p_spi->dummy_out = DUMMY_OUT_VALUE;

    /* SDK */
    p_spi->p_spi_hw = spi_get_hw( p_spi->p_periph_def->p_pico_spi_inst );

    /* Lock */
    p_spi->lock = 0;

    /* Role-specific configuration */
    switch( p_spi->role )
    {
        case HAL_MCU_SPI_ROLE_SLAVE:
        {
            result = _slave_init( p_spi, p_conf );
            EXIT_IF_ERR( result, "_slave_init failed" );
        }
        break;

        default:
        {
            ASSERT_DYGMA( false, "Invalid SPI role selection." );
        }
        break;
    }

_EXIT:
    return result;
}

static result_t _spi_line_configure( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf )
{
    const cpha_def_t * p_cpha_def;
    const cpol_def_t * p_cpol_def;
    const bit_order_def_t * p_bit_order_def;

    /* Get the line definitions */
    get_cpha_def( p_cpha_def, p_line_conf->cpha );
    get_cpol_def( p_cpol_def, p_line_conf->cpol );
    get_bit_order_def( p_bit_order_def, p_line_conf->bit_order );

    ASSERT_DYGMA( p_cpha_def != NULL, "The SPI CPHA line configuration is not valid" );
    ASSERT_DYGMA( p_cpol_def != NULL, "The SPI CPOL line configuration is not valid" );
    ASSERT_DYGMA( p_bit_order_def != NULL, "The SPI Bit Order line configuration is not valid" );

    /* Set the CLK baudrate */
    spi_set_baudrate( p_spi->p_periph_def->p_pico_spi_inst, p_line_conf->freq );

    /* Set the SPI format */
    spi_set_format( p_spi->p_periph_def->p_pico_spi_inst, 8, p_cpol_def->pico_cpol, p_cpha_def->pico_cpha, p_bit_order_def->pico_order );

    return RESULT_OK;
}

/***********************************************/
/*                     DMA                     */
/***********************************************/

//static void _dma_event_handler_t( void * p_instance, hal_mcu_dma_event_t event )
//{
//    spi_dma_t * p_spi_dma = ( spi_dma_t *)p_instance;
//    hal_mcu_spi_t * p_spi = p_spi_dma->p_spi;
//
//    /*
//     * We currently do nothing when the DMA event comes. The crucial point for is is the chip_deselect event.
//     * The template for possible future DMA event handling is prepared here.
//     */
//
//    if( p_spi_dma == &p_spi->dma_rx )
//    {
//        // _dma_rx_finished_process( p_spi );
//    }
//    else if( p_spi_dma == &p_spi->dma_tx )
//    {
//        // _dma_tx_finished_process( p_spi );
//    }
//}

static result_t _dma_init( hal_mcu_spi_t * p_spi )
{
    result_t result = RESULT_ERR;
    hal_mcu_dma_channel_config_t config;

    /* Prepare the Rx DMA */
    p_spi->dma_rx.p_spi = p_spi;

    config.packet_size = HAL_MCU_DMA_PACKET_SIZE_8;
    config.direction = HAL_MCU_DMA_DIRECTION_PERIPHERAL_TO_MEMORY;
    config.request_type = p_spi->p_periph_def->dma_request_type_rx;
    config.p_instance = &p_spi->dma_rx;
    config.event_handler = NULL;        //_dma_event_handler_t;

    result = hal_mcu_dma_init( &p_spi->dma_rx.p_dma, &config );
    EXIT_IF_ERR( result, "RX hal_mcu_dma_init failed" );

    /* Prepare the Tx DMA */
    p_spi->dma_tx.p_spi = p_spi;

    config.packet_size = HAL_MCU_DMA_PACKET_SIZE_8;
    config.direction = HAL_MCU_DMA_DIRECTION_MEMORY_TO_PERIPHERAL;
    config.request_type = p_spi->p_periph_def->dma_request_type_tx;
    config.p_instance = &p_spi->dma_tx;
    config.event_handler = NULL;        //_dma_event_handler_t;

    result = hal_mcu_dma_init( &p_spi->dma_tx.p_dma, &config );
    EXIT_IF_ERR( result, "TX hal_mcu_dma_init failed" );

    /* enable the interrupt handlers initially */
    hal_mcu_dma_event_handler_enable( p_spi->dma_rx.p_dma );
    hal_mcu_dma_event_handler_enable( p_spi->dma_tx.p_dma );

_EXIT:
    return result;
}

/***********************************************/
/*              Slave processing              */
/***********************************************/

static INLINE void _slave_spi_reset( hal_mcu_spi_t * p_spi )
{
    reset_block( p_spi->p_periph_def->pico_reset_bits );
    unreset_block( p_spi->p_periph_def->pico_reset_bits );
}

static INLINE bool_t _slave_spi_is_running( hal_mcu_spi_t * p_spi )
{
    return ( ~resets_hw->reset_done & p_spi->p_periph_def->pico_reset_bits ) ? false : true;
}

static INLINE bool_t _slave_spi_running_wait_blocking( hal_mcu_spi_t * p_spi )
{

    while ( _slave_spi_is_running == false )
    {
        tight_loop_contents();
    }
}

static INLINE void _slave_spi_dma_enable( hal_mcu_spi_t * p_spi )
{
    hw_set_bits( &p_spi->p_spi_hw->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS );
}

static INLINE void _slave_spi_dma_disable( hal_mcu_spi_t * p_spi )
{
    hw_clear_bits( &p_spi->p_spi_hw->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS );
}

static INLINE result_t _slave_spi_enable( hal_mcu_spi_t * p_spi )
{
    result_t result = RESULT_ERR;

    if( _slave_spi_is_running( p_spi ) == false )
    {
        return RESULT_BUSY;
    }

    /* Configure the SPI line */
    result = _spi_line_configure( p_spi, &p_spi->slave.line );
    EXIT_IF_ERR( result, "_spi_line_configure failed" );

    /* Set the SPI to slave mode. */
    spi_set_slave( p_spi->p_periph_def->p_pico_spi_inst, true );

    /* Enable the DMA */
    _slave_spi_dma_enable( p_spi );

    /* Check the current state of the CS pin*/
#warning "This is still not perfect. There is still possibility of enabling the SPI Slave when the Master already started transmission."
    if( gpio_get( p_spi->slave.pin_cs ) == false )
    {
        /* Do not start the SPI if the Chip select is active */
        _slave_spi_disable( p_spi );

        return RESULT_BUSY;
    }

    /* Finally enable the SPI */
    hw_set_bits( &p_spi->p_spi_hw->cr1, SPI_SSPCR1_SSE_BITS );

_EXIT:
    return result;
}

static INLINE void _slave_spi_disable( hal_mcu_spi_t * p_spi )
{
    /* Disable the SPI */
    hw_clear_bits( &p_spi->p_spi_hw->cr1, SPI_SSPCR1_SSE_BITS );

    /* Disable the DMA */
    _slave_spi_dma_disable( p_spi );

    /* We completely de-initialize the SPI to wipe out all data which might be still sitting in the SPI internal buffers */
    _slave_spi_reset ( p_spi );
}

static INLINE hal_mcu_spi_t * _slave_cs_instance_get( uint gpio )
{
    const periph_def_t * p_periph_def;
    hal_mcu_spi_t * p_spi = NULL;

    /* Get the peripheral definition */
    get_slave_cs_periph_def( p_periph_def, gpio );
    ASSERT_DYGMA( p_periph_def != NULL, "SPI Slave - Invalid Chip Select gpio detected" );

    /* Get the SPI instance from the peripheral definition */
    p_spi = *p_periph_def->pp_periph;

    return p_spi;
}

static INLINE void _slave_buffers_set_done_handler( hal_mcu_spi_t * p_spi )
{
    if ( p_spi->slave.buffers_set_done_handler == NULL )
    {
            return;
    }

    p_spi->slave.buffers_set_done_handler( p_spi->slave.p_instance );
}

static INLINE void _slave_transfer_done_handler(hal_mcu_spi_t * p_spi , hal_mcu_spi_transfer_result_t * _transfer_result)
{
    if ( p_spi->slave.transfer_done_handler == NULL )
    {
        return;
    }

    p_spi->slave.transfer_done_handler( p_spi->slave.p_instance, _transfer_result );
}

static INLINE bool_t _slave_transfer_result_get( hal_mcu_spi_t * p_spi, hal_mcu_spi_transfer_result_t * p_transfer_result )
{
    bool_t result;

    /* Get the final count of data that has been transferred during the transfer session */
    p_transfer_result->data_in_len = hal_ll_mcu_dma_get_transfer_count( p_spi->dma_rx.p_dma );
    p_transfer_result->data_out_len = hal_ll_mcu_dma_get_transfer_count( p_spi->dma_tx.p_dma );

    /* NOTE: The DMA Tx number is lowered when the data is transferred to the SPI Tx buffer. However, it might not be sent out to the SPI
     *       master yet. Hence the data_out_len is never going to be 100% sure.
     *
     *       We can only check the TFE (Transmit FIFO Empty) flag of the SSPSR register to approve all data has been sent. However, if the
     *       flag is not asserted, we cannot determine how much data is still waiting in the Tx FIFO buffer. */

    /*
     * Because we are making sure that there is at least one data_in byte transferred, if this value is 0, there was no data truly transferred
     * and thus we claim the transfer was invalid.
     *
     * NOTE: We cannot use the data_out_len for this because we cannot say whether the data has really departed to Master. Read the NOTE above.
     *
     * We chose this approach to determine the situations when we receive the CS signal, but no data has been transferred. If the transfer result
     * of data is of 0 length, the transfer was invalid. ( For example the master has pulled the CS pin but there was not a single byte processed.
     * This can happen when the Master is restarted. )
     */
    result = ( p_transfer_result->data_in_len != 0 ) ? true : false;

    /*
     * If the dummy_in_used flag is set, the requested data in length was 0 and we used the non-zero value for the transfer validity detection.
     * Let's nullify the transfer result here.
     */
    if( p_spi->slave.dummy_in_used == true )
    {
        p_transfer_result->data_in_len = 0;
    }

    return result;
}

/* This callback will be triggered every time the CS pin is set low by the master. */
static INLINE void _slave_cs_selected_process( hal_mcu_spi_t *p_spi )
{
    /* Nothing to do now */
    ASSERT_DYGMA( false, "Unexpected SPI Slave Chip select behavior." )
}

static INLINE void _slave_cs_deselected_process( hal_mcu_spi_t *p_spi )
{
    result_t result = RESULT_ERR;
    bool_t transfer_is_valid;
    hal_mcu_spi_transfer_result_t transfer_result;

    // The master sets the CS pin high, the SPI transfer is ending.
    // We need to finish the DMA transfer.

    /* Check if the transfer was running first */
    if( p_spi->busy == false )
    {
        return;
    }

    /* Disable the SPI peripheral */
    _slave_spi_disable( p_spi );

    /* Get the Transfer result */
    transfer_is_valid = _slave_transfer_result_get( p_spi, &transfer_result );

    /* Stop (and clear) the DMA channels */
    hal_mcu_dma_stop( p_spi->dma_tx.p_dma );
    hal_mcu_dma_stop( p_spi->dma_rx.p_dma );

    /* The transfer is done */
    p_spi->slave.dummy_in_used = false;
    p_spi->busy = false;

    /* Check whether there was a valid transfer actually. */
    if( transfer_is_valid == false || p_spi->slave.resend_request == true )
    {
        /* Release the resend request flag */
        p_spi->slave.resend_request = false;

        /* Transfer is invalid, no data was processed - restart the transfer here and wait for the next Chip select */
        result = _slave_transfer( p_spi );
        ASSERT_DYGMA( result == RESULT_OK, "Unexpected failure of the SPI slave transfer start" );
    }
    else
    {
        /* Transfer is valid - Let the superior layers know that the transfer is finished */
        _slave_transfer_done_handler( p_spi, &transfer_result );
    }

    UNUSED( result );
}

static void _slave_cs_irq_handler( uint gpio, uint32_t event_mask )
{
    /* Get the SPI instance from the peripheral definition */
    hal_mcu_spi_t * p_spi = _slave_cs_instance_get( gpio );

    if ( event_mask & GPIO_IRQ_EDGE_FALL )
    {
        _slave_cs_selected_process( p_spi );
    }
    else if ( event_mask & GPIO_IRQ_EDGE_RISE )
    {
        _slave_cs_deselected_process( p_spi );
    }
}

static result_t _slave_init(hal_mcu_spi_t *p_spi, const hal_mcu_spi_conf_t *p_conf)
{

    /* Save the SPI line configuration fo the later use */
    p_spi->slave.line = p_conf->line;

    /* Set the SPI PINs */
    p_spi->slave.pin_cs = p_conf->slave.pin_cs;

    gpio_set_function( p_conf->slave.pin_sck, GPIO_FUNC_SPI );
    gpio_set_function( p_conf->slave.pin_miso, GPIO_FUNC_SPI );
    gpio_set_function( p_conf->slave.pin_mosi, GPIO_FUNC_SPI );
    gpio_set_function( p_conf->slave.pin_cs, GPIO_FUNC_SPI );
    
    gpio_pull_up(p_conf->slave.pin_cs);

    /* Enable the CS gpio interrupt. We are interested only in the rising edge which is signalling the transfer has been finished */
    gpio_set_irq_enabled_with_callback(p_conf->slave.pin_cs, GPIO_IRQ_EDGE_RISE , true, _slave_cs_irq_handler );

    /* Disable the slave DMA handlers because they are not used in Slave mode */
    hal_mcu_dma_event_handler_disable( p_spi->dma_rx.p_dma );
    hal_mcu_dma_event_handler_disable( p_spi->dma_tx.p_dma );
    
    /* Flags */
    p_spi->slave.dummy_in_used = false;
    p_spi->slave.resend_request = false;

    /* Perform initial SPI reset to set it into default state */
    _slave_spi_reset ( p_spi );

    /* Test */

    return RESULT_OK;
}

static result_t _slave_transfer( hal_mcu_spi_t* p_spi )
{
    result_t result = RESULT_ERR;

    ASSERT_DYGMA( p_spi->busy == false, "The SPI slave must not be busy before starting the new SPI transfer." );

    hal_mcu_dma_transfer_config_t dma_transfer_config_rx;
    hal_mcu_dma_transfer_config_t dma_transfer_config_tx;

    /* Set the RX DMA transfer configuration */
    dma_transfer_config_rx.read_address = (void*)&p_spi->p_spi_hw->dr;
    dma_transfer_config_rx.read_increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED;

    if( p_spi->data_in_len != 0 )
    {
        dma_transfer_config_rx.buffer_size = p_spi->data_in_len;
        dma_transfer_config_rx.write_address = p_spi->p_data_in;
        dma_transfer_config_rx.write_increment_mode = HAL_MCU_DMA_INC_MODE_ENABLED;

        p_spi->slave.dummy_in_used = false;
    }
    else
    {
        /*
         * Here we are making sure that there will always be non-zero length of data input. This is important for for indicating
         * valid data transfer. If the transfer result of data in is of 0 length, the transfer was invalid. ( For example the
         * master has pulled the CS pin but there was not a single byte processed. This can happen when the Master is restarted. )
         */
        dma_transfer_config_rx.buffer_size = sizeof( p_spi->dummy_in );
        dma_transfer_config_rx.write_address = &p_spi->dummy_in;
        dma_transfer_config_rx.write_increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED;

        p_spi->slave.dummy_in_used = true;
    }

    /* Set the TX DMA transfer configuration */
    dma_transfer_config_tx.buffer_size = p_spi->data_out_len;
    dma_transfer_config_tx.write_address = (void *)&p_spi->p_spi_hw->dr;
    dma_transfer_config_tx.write_increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED;

    if( p_spi->data_out_len != 0 )
    {
            dma_transfer_config_tx.read_address = p_spi->p_data_out;
            dma_transfer_config_tx.read_increment_mode = HAL_MCU_DMA_INC_MODE_ENABLED;
    }
    else
    {
            p_spi->dummy_out = DUMMY_OUT_VALUE;
            dma_transfer_config_tx.read_address = &p_spi->dummy_out;
            dma_transfer_config_tx.read_increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED;
    }

    /* Prepare both DMA channels simultaneously */
    p_spi->busy = true;

    result = hal_ll_mcu_dma_start_channels_simultaneously( p_spi->dma_tx.p_dma, &dma_transfer_config_tx, p_spi->dma_rx.p_dma , &dma_transfer_config_rx );
    ASSERT_DYGMA( result == RESULT_OK, "hal_mcu_dma_start_channels_simultaneously failed" );
    EXIT_IF_NOK( result );

    /* Enable the SPI */
    result = _slave_spi_enable( p_spi );
    EXIT_IF_ERR( result, "_slave_spi_enable failed" );
    EXIT_IF_NOK( result );

_EXIT:

    if( result != RESULT_OK )
    {
        /* Stop (and clear) the DMA channels */
        hal_mcu_dma_stop( p_spi->dma_tx.p_dma );
        hal_mcu_dma_stop( p_spi->dma_rx.p_dma );

        if( result == RESULT_BUSY )
        {
            /*
             * Set the resend request flag to set the transfer again on the next Chip de-select event.
             */
            p_spi->slave.resend_request = true;

            /* Set OK result because the transfer is prepared and will start later */
            result = RESULT_OK;
        }
        else
        {
            p_spi->busy = false;
        }
    }

    return result;
}

static result_t _slave_transfer_start(hal_mcu_spi_t* p_spi,  const hal_mcu_spi_transfer_conf_t * p_transfer_conf)
{
    result_t result = RESULT_ERR;

    ASSERT_DYGMA( p_spi->role = HAL_MCU_SPI_ROLE_SLAVE, "Only SPI slave may initialize the SPI transfer." );

    if( p_spi->busy == true )
    {
        return RESULT_BUSY;
    }

    if( p_transfer_conf->data_in_len == 0 && p_transfer_conf->data_out_len == 0 )
    {
        ASSERT_DYGMA( false, "SPI Slave zero-length transfer is currently invalid." );
        return RESULT_ERR;
    }

    /* Prepare the transfer event handlers */
    p_spi->slave.p_instance = p_transfer_conf->slave_handlers.p_instance;
    p_spi->slave.buffers_set_done_handler = p_transfer_conf->slave_handlers.buffers_set_done_handler;
    p_spi->slave.transfer_done_handler = p_transfer_conf->slave_handlers.transfer_done_handler;

    /* Prepare the buffers*/
    p_spi->p_data_out = p_transfer_conf->p_data_out;
    p_spi->p_data_in = p_transfer_conf->p_data_in;
    p_spi->data_in_len = p_transfer_conf->data_in_len;
    p_spi->data_out_len = p_transfer_conf->data_out_len;

    result = _slave_transfer( p_spi );
    EXIT_IF_ERR( result, "_slave_transfer failed" );

    _slave_buffers_set_done_handler( p_spi );

_EXIT:
    return result;
}

/***********************************************/
/*                     API                     */
/***********************************************/

result_t hal_ll_mcu_spi_init( hal_mcu_spi_t ** pp_spi, const hal_mcu_spi_conf_t * p_conf )
{
    hal_mcu_spi_t * p_spi;
    const periph_def_t * p_periph_def;
    result_t result = RESULT_ERR;

    /* Get the peripheral definition */
    get_periph_def( p_periph_def, p_conf->def );
    ASSERT_DYGMA( p_periph_def != NULL, "invalid periph definition" );

    /* Check the init request validity */
    ASSERT_DYGMA( *p_periph_def->pp_periph == NULL, "Chosen SPI peripheral has already been initialized" );

    /* Allocate the peripheral */
    *p_periph_def->pp_periph = heap_alloc( sizeof(hal_mcu_spi_t) );
    p_spi = *p_periph_def->pp_periph;

    p_spi->p_periph_def = p_periph_def;

    /* initialize spi */
    result = _spi_init( p_spi, p_conf );
    EXIT_IF_ERR( result, "SPI init failed" );

    *pp_spi = p_spi;

_EXIT:
    return result;

}

bool_t hal_ll_mcu_spi_is_slave( hal_mcu_spi_t * p_spi )
{
    return ( p_spi->role == HAL_MCU_SPI_ROLE_SLAVE ) ? true : false;
}

result_t hal_ll_mcu_spi_reserve( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf, hal_mcu_spi_lock_t * p_lock )
{
    result_t result = RESULT_ERR;

    if( p_spi->reserved == true )
    {
        return RESULT_BUSY;
    }

    /* Try to configure the SPI line */
    result = _spi_line_configure( p_spi, p_line_conf );
    EXIT_IF_ERR( result, "_spi_line_configure failed" );

    /* Reserve and lock the SPI peripheral */
    p_spi->reserved = true;

    p_spi->lock++;
    *p_lock = p_spi->lock;

_EXIT:
    return result;
}

void hal_ll_mcu_spi_release( hal_mcu_spi_t * p_spi, hal_mcu_spi_lock_t lock )
{
    /* Try to unlock the SPI peripheral */
    if( lock != p_spi->lock )
    {
        ASSERT_DYGMA( false, "Detected an attempt to unlock the SPI with a wrong lock ID" );
        return;
    }

    /* Release the reservation */
    p_spi->reserved = false;
}

result_t hal_ll_mcu_spi_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf )
{
    result_t result = RESULT_ERR;

    switch( p_spi->role )
    {
        case HAL_MCU_SPI_ROLE_SLAVE:
        {
            result = _slave_transfer_start( p_spi, p_transfer_conf );
            EXIT_IF_ERR( result, "_slave_data_transfer failed" );
        }
        break;

        default:
        {
            ASSERT_DYGMA( false, "Invalid SPI role selection." );
        }
        break;
    }

_EXIT:
    return result;
}

//#endif /* HAL_CFG_MCU_SERIES */


