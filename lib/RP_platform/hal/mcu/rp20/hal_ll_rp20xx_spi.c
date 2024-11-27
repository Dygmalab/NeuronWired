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
    /* Event handlers */
    void * p_instance;
    hal_mcu_spi_slave_buffers_set_done_handler_t buffers_set_done_handler;
    hal_mcu_spi_slave_transfer_done_handler_t transfer_done_handler;
} slave_t;

struct hal_mcu_spi
{
    hal_mcu_spi_role_t role;

    const periph_def_t * p_periph_def;

    bool_t reserved;
    bool_t busy;

    /* DMA */
    hal_mcu_dma_channel_t * p_dma_rx;
    hal_mcu_dma_channel_t * p_dma_tx;

    /* Transfer */
    uint8_t * p_data_out;
    uint8_t * p_data_in;
    size_t data_out_len;
    size_t data_in_len;

    uint8_t dummy_in;
    uint8_t dummy_out;

    /* Transfer */
    size_t data_in_progress_len;

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
    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI0, .pp_periph = &p_spi0, .p_pico_spi_inst = spi0,
            .dma_request_type_rx = HAL_MCU_DMA_REQUEST_TYPE_SPI0_RX, .dma_request_type_tx = HAL_MCU_DMA_REQUEST_TYPE_SPI0_TX, },

    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI1, .pp_periph = &p_spi1, .p_pico_spi_inst = spi1,
            .dma_request_type_rx = HAL_MCU_DMA_REQUEST_TYPE_SPI1_RX, .dma_request_type_tx = HAL_MCU_DMA_REQUEST_TYPE_SPI1_TX, },
};
#define get_periph_def( p_periph_def, id ) _get_def( p_periph_def, p_periph_def_array, periph_def_t, def, id )

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
static void _slave_dma_event_handler( hal_mcu_spi_t * p_spi );
result_t _slave_transfer(hal_mcu_spi_t * p_spi,  const hal_mcu_spi_transfer_conf_t * p_transfer_conf);


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

    p_spi->data_in_progress_len = 0;

    p_spi->dummy_in = 0x00;
    p_spi->dummy_out = DUMMY_OUT_VALUE;

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

static INLINE void _slave_buffers_set_done_handler( hal_mcu_spi_t * p_spi )
{
    if ( p_spi->slave.buffers_set_done_handler == NULL )
    {
            return;
    }

    p_spi->slave.buffers_set_done_handler( p_spi->slave.p_instance );
}

static void _dma_event_handler_t( void * p_instance, hal_mcu_dma_event_t event )
{
    hal_mcu_spi_t * p_spi = ( hal_mcu_spi_t *)p_instance;

    ASSERT_DYGMA( event == HAL_MCU_DMA_EVENT_TRANSFER_COMPLETE, "Unexpected DMA event detected" );

    switch( p_spi->role )
    {
        case HAL_MCU_SPI_ROLE_SLAVE:
        {
            p_spi->busy = false;
            _slave_dma_event_handler( p_spi );
        }
        break;

        default:
        {
            ASSERT_DYGMA( false, "Invalid SPI role detected." );
        }
        break;
    }
}

static result_t _dma_init( hal_mcu_spi_t * p_spi )
{
    result_t result = RESULT_ERR;
    hal_mcu_dma_channel_config_t config;

    /* Prepare the Rx DMA */
    config.packet_size = HAL_MCU_DMA_PACKET_SIZE_8;
    config.direction = HAL_MCU_DMA_DIRECTION_PERIPHERAL_TO_MEMORY;
    config.request_type = p_spi->p_periph_def->dma_request_type_rx;
    config.p_instance = p_spi;
    config.event_handler = _dma_event_handler_t;

    result = hal_mcu_dma_init( &p_spi->p_dma_rx, &config );
    EXIT_IF_ERR( result, "RX hal_mcu_dma_init failed" );

    /* Prepare the Tx DMA */
    config.packet_size = HAL_MCU_DMA_PACKET_SIZE_8;
    config.direction = HAL_MCU_DMA_DIRECTION_MEMORY_TO_PERIPHERAL;
    config.request_type = p_spi->p_periph_def->dma_request_type_tx;
    config.p_instance = p_spi;
    config.event_handler = _dma_event_handler_t;

    result = hal_mcu_dma_init( &p_spi->p_dma_tx, &config );
    EXIT_IF_ERR( result, "TX hal_mcu_dma_init failed" );

    /* enable the interrupt handlers initially */
    hal_mcu_dma_event_handler_enable( p_spi->p_dma_rx );
    hal_mcu_dma_event_handler_enable( p_spi->p_dma_tx );

_EXIT:
    return result;
}

static result_t _configure_dma( hal_mcu_spi_t* p_spi , const hal_mcu_spi_transfer_conf_t *p_conf)
{
    result_t result = RESULT_ERR;

    hal_mcu_dma_transfer_config_t dma_transfer_config_rx;
    hal_mcu_dma_transfer_config_t dma_transfer_config_tx;

    /* Get the number of bytes to be transfered in this step */
    p_spi->data_in_progress_len = ( p_spi->data_in_len > p_spi->data_out_len ) ? p_spi->data_in_len : p_spi->data_out_len;

    /* If the data in and data out are the same length, we can transfer them in one go */
    if (p_spi->data_in_len == p_spi->data_out_len)
    {
        p_spi->data_in_progress_len = p_spi->data_in_len;
    }

    /* Set the RX DMA transfer configuration */
    dma_transfer_config_rx.buffer_size = p_spi->data_in_progress_len;
    dma_transfer_config_rx.read_address = (void*)&spi_get_hw( p_spi->p_periph_def->p_pico_spi_inst )->dr;
    dma_transfer_config_rx.read_increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED;

    if( p_spi->data_in_len != 0 )
    {
            dma_transfer_config_rx.write_address = p_spi->p_data_in;
            dma_transfer_config_rx.write_increment_mode = HAL_MCU_DMA_INC_MODE_ENABLED;
    }
    else
    {
            dma_transfer_config_rx.write_address = &p_spi->dummy_in;
            dma_transfer_config_rx.write_increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED;
    }

    /* Set the TX DMA transfer configuration */
    dma_transfer_config_tx.buffer_size = p_spi->data_in_progress_len;
    dma_transfer_config_tx.write_address = (void *)&spi_get_hw( p_spi->p_periph_def->p_pico_spi_inst )->dr;
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

    result = hal_mcu_dma_prepare_channels_simultaneously( p_spi->p_dma_tx, &dma_transfer_config_tx, p_spi->p_dma_rx , &dma_transfer_config_rx );
    ASSERT_DYGMA( result == RESULT_OK, "hal_mcu_dma_start_channels_simultaneously failed" );
    return result;
}

/***********************************************/
/*              Slave processing              */
/***********************************************/

/* This callback will be triggered every time the CS pin is set low by the master. */
static void _cs_cb(hal_mcu_spi_t *p_spi , uint32_t events) 
{    
    if (events & GPIO_IRQ_EDGE_FALL) 
    {
        // The CS pin is set low by the master, the SPI transfer is starting.
        // we need to start the DMA transfer.
        //TODO this piece of code should be done with the PIO. 
        hal_mcu_dma_trigger_channels_simultaneously( p_spi->p_dma_tx, p_spi->p_dma_rx );
    }
    else if (events & GPIO_IRQ_EDGE_RISE) 
    {
        // The master sets the CS pin high, the SPI transfer is ending.
        // We need to finish the DMA transfer.

        hal_mcu_dma_abort(p_spi->p_dma_tx);
        hal_mcu_dma_abort(p_spi->p_dma_rx);
        p_spi->busy = false;
        _slave_dma_event_handler(p_spi); // Process remaining data.
    }
    
}

void _cs_cb_0 (uint gpio, uint32_t events) 
{
    if (p_spi0 == NULL)
    {
        ASSERT_DYGMA( false, "SPI0 is not initialized" );
    }
    _cs_cb(p_spi0, events);
}

void _cs_cb_1 (uint gpio, uint32_t events) 
{
    if (p_spi1 == NULL)
    {
        ASSERT_DYGMA( false, "SPI1 is not initialized" );
    }
    _cs_cb(p_spi1, events);
}

result_t _slave_init(hal_mcu_spi_t *p_spi, const hal_mcu_spi_conf_t *p_conf)
{

    #warning "TODO: Check DMA overflow."
    #warning "TODO: Check PIO script to use CS pin to trigger the DMA transfer."

    result_t result = RESULT_ERR;

    /* Initialize the Pico SPI peripheral */
    spi_init( p_spi->p_periph_def->p_pico_spi_inst, p_conf->line.freq );

    /*Set the SPI to slave mode.*/ 
    spi_set_slave( p_spi->p_periph_def->p_pico_spi_inst, true );

    /* Set the SPI PINs */
    gpio_set_function( p_conf->slave.pin_sck, GPIO_FUNC_SPI );
    gpio_set_function( p_conf->slave.pin_miso, GPIO_FUNC_SPI );
    gpio_set_function( p_conf->slave.pin_mosi, GPIO_FUNC_SPI );
    gpio_set_function( p_conf->slave.pin_cs, GPIO_FUNC_SPI );
    
    gpio_pull_up(p_conf->slave.pin_cs);

    if (p_spi->p_periph_def->def == HAL_MCU_SPI_PERIPH_DEF_SPI0)
    {
        gpio_set_irq_enabled_with_callback(p_conf->slave.pin_cs, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true, &_cs_cb_0);
    }
    else if (p_spi->p_periph_def->def == HAL_MCU_SPI_PERIPH_DEF_SPI1)
    {
        gpio_set_irq_enabled_with_callback(p_conf->slave.pin_cs, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true, &_cs_cb_1);
    }
    else
    {
        ASSERT_DYGMA( false, "Invalid SPI peripheral definition" );
    }
    
    /* Configure the SPI line */
    result = _spi_line_configure( p_spi, &p_conf->line );
    EXIT_IF_ERR( result, "_spi_line_configure failed" );

_EXIT:
    return result;
}

result_t _slave_transfer(hal_mcu_spi_t* p_spi,  const hal_mcu_spi_transfer_conf_t * p_transfer_conf)
{
    result_t result = RESULT_ERR;

    ASSERT_DYGMA( p_spi->role = HAL_MCU_SPI_ROLE_SLAVE, "Only SPI slave may initialize the SPI transfer." );

    if( p_spi->busy == true )
    {
        return RESULT_BUSY;
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

    result = _configure_dma( p_spi , p_transfer_conf);

    if( result != RESULT_OK )
    {
        p_spi->busy = false;
        return RESULT_ERR;
    }

    _slave_buffers_set_done_handler( p_spi );

    return result;
}

void _slave_transfer_done_handler(hal_mcu_spi_t * p_spi , hal_mcu_spi_transfer_result_t * _transfer_result)
{
    if ( p_spi->slave.transfer_done_handler == NULL )
    {
        return;
    }

    p_spi->slave.transfer_done_handler( p_spi->slave.p_instance, _transfer_result );
}

static void _slave_dma_event_handler(hal_mcu_spi_t *p_spi)
{
    /* Check the transfer is finished */
    if (p_spi->busy == false) 
    {
        hal_mcu_spi_transfer_result_t transfer_result;

        if (p_spi->data_in_len > 0) {
          p_spi->p_data_in += p_spi->data_in_progress_len;
          p_spi->data_in_len -= p_spi->data_in_progress_len;
        }

        if (p_spi->data_out_len > 0) {
          p_spi->p_data_out += p_spi->data_in_progress_len;
          p_spi->data_out_len -= p_spi->data_in_progress_len;
        }

        transfer_result.data_in_len = p_spi->data_in_len;
        transfer_result.data_out_len = p_spi->data_out_len;

        _slave_transfer_done_handler(p_spi, &transfer_result);

        p_spi->data_in_progress_len = 0;
    }
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
            result = _slave_transfer( p_spi, p_transfer_conf );
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


