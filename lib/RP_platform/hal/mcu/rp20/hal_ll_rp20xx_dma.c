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
#include "hal\mcu\hal_mcu_dma_ll.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/irq.h"


/*
 * We are currently using the DMA_IRQ_0. If, for any reason, we needed to use the other interrupt source DMA_IRQ_1,
 * we will have to make abstraction for all the IRQ0 calls.
 */

#define HAL_DMA_IRQ         DMA_IRQ_0
#define HAL_DMA_IRQ_INDEX   0

typedef struct
{
    hal_mcu_dma_packet_size_t size;
    enum dma_channel_transfer_size rp20xx_size;
} packet_size_def_t;

static const packet_size_def_t p_packet_size_def_array[] =
{
    { .size = HAL_MCU_DMA_PACKET_SIZE_8   , .rp20xx_size = DMA_SIZE_8  },
    { .size = HAL_MCU_DMA_PACKET_SIZE_16  , .rp20xx_size = DMA_SIZE_16 },
    { .size = HAL_MCU_DMA_PACKET_SIZE_32  , .rp20xx_size = DMA_SIZE_32 },
};
#define get_size_def( def, id ) _get_def( def, p_packet_size_def_array, packet_size_def_t, size, id )

/* DMA request types definitions */
#define HAL_MCU_DMA_REQUEST_TYPE_TIMER0     0x3B
#define HAL_MCU_DMA_REQUEST_TYPE_TIMER1     0x3C
#define HAL_MCU_DMA_REQUEST_TYPE_TIMER2     0x3D
#define HAL_MCU_DMA_REQUEST_TYPE_TIMER3     0x3E
#define HAL_MCU_DMA_REQUEST_TYPE_FORCE      0x3F

typedef struct
{
    hal_mcu_dma_request_type_t request_type;
    uint rp20xx_dreq;
} dreq_t;

static const dreq_t p_dreq_array[] =
{
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_SPI0_TX  , .rp20xx_dreq = DREQ_SPI0_TX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_SPI0_RX  , .rp20xx_dreq = DREQ_SPI0_RX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_SPI1_TX  , .rp20xx_dreq = DREQ_SPI1_TX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_SPI1_RX  , .rp20xx_dreq = DREQ_SPI1_RX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_I2C0_TX  , .rp20xx_dreq = DREQ_I2C0_TX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_I2C0_RX  , .rp20xx_dreq = DREQ_I2C0_RX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_I2C1_TX  , .rp20xx_dreq = DREQ_I2C1_TX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_I2C1_RX  , .rp20xx_dreq = DREQ_I2C1_RX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_UART0_TX , .rp20xx_dreq = DREQ_UART0_TX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_UART0_RX , .rp20xx_dreq = DREQ_UART0_RX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_UART1_TX , .rp20xx_dreq = DREQ_UART1_TX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_UART1_RX , .rp20xx_dreq = DREQ_UART1_RX },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_ADC      , .rp20xx_dreq = DREQ_ADC },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_PIO0_TX      , .rp20xx_dreq = DREQ_PIO0_TX0 },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_PIO0_RX      , .rp20xx_dreq = DREQ_PIO0_RX0 },

    /* Internal DREQ types */
//    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_TIMER0   , .rp20xx_dreq = DREQ_DMA_TIMER0 },
//    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_TIMER1   , .rp20xx_dreq = DREQ_DMA_TIMER1 },
//    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_TIMER2   , .rp20xx_dreq = DREQ_DMA_TIMER2 },
//    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_TIMER3   , .rp20xx_dreq = DREQ_DMA_TIMER3 },
    { .request_type= HAL_MCU_DMA_REQUEST_TYPE_FORCE    , .rp20xx_dreq = DREQ_FORCE },
};
#define get_dreq( def, id ) _get_def( def, p_dreq_array, dreq_t, request_type, id )

/* Increment mode definition */
typedef struct
{
    hal_mcu_dma_increment_mode_t increment_mode;
    bool_t rp20xx_increment_mode;
} increment_mode_def_t;

static const increment_mode_def_t p_increment_mode_def_array[] =
{
    { .increment_mode = HAL_MCU_DMA_INC_MODE_DISABLED  , .rp20xx_increment_mode = false },
    { .increment_mode = HAL_MCU_DMA_INC_MODE_ENABLED   , .rp20xx_increment_mode = true },
};
#define get_increment_mode_def( def, id ) _get_def( def, p_increment_mode_def_array, increment_mode_def_t, increment_mode, id )

struct hal_mcu_dma_channel {

    uint8_t channel_id;

    //DMA configuration
    packet_size_def_t const *p_packet_size_def;

    hal_mcu_dma_direction_t direction;

    dreq_t const *p_dreq ;

    //sdk DMA channel config
    dma_channel_config sdk_dma_config;

    // Transfer information
    // Number of elements (bytes) to transfer.
    uint32_t transfer_buffer_size;

    // DMA callback function.
    void * p_instance;
    hal_mcu_dma_event_handler_t event_handler;
};

typedef struct {

    bool is_initialized;

    //  Channel List
    hal_mcu_dma_channel_t *p_channel_list[NUM_DMA_CHANNELS];
    uint8_t channel_count;

} hal_mcu_dma_t ;

static hal_mcu_dma_t hal_mcu_dma =
{
    .is_initialized = false,
};

/******************** Low-Level hal internal functions ********************/
static void _enable_interrupts( const hal_mcu_dma_channel_t * p_channel );

static void _channel_handler( hal_mcu_dma_channel_t * p_channel )
{
    // Check if the RX channel has completed the transfer
    if ( dma_hw->ints0 & ( 1u << ( p_channel->channel_id ) ) )
    {
        dma_hw->ints0 = 1u << p_channel->channel_id; // Clear the interrupt flag
    }

    if ( p_channel->event_handler == NULL )
    {
        return;
    }

    p_channel->event_handler( p_channel->p_instance, HAL_MCU_DMA_EVENT_TRANSFER_COMPLETE );
}

static void _dma_handler( hal_mcu_dma_t * p_hal_mcu_dma )
{
    uint8_t channel_id;
    bool result = false;

    for ( channel_id = 0; channel_id < NUM_DMA_CHANNELS; ++channel_id )
    {
        result = dma_irqn_get_channel_status( HAL_DMA_IRQ_INDEX, channel_id );
        if ( result )
        {
            _channel_handler( p_hal_mcu_dma->p_channel_list[channel_id] );
        }
    }
}

static void _sdk_dma_handler( void )
{
    _dma_handler( &hal_mcu_dma );
}

// Disable DMA interrupts
static void _disable_interrupts( const hal_mcu_dma_channel_t * p_channel )
{
    dma_channel_set_irq0_enabled( p_channel->channel_id, false );
}

// Enable DMA interrupts
static void _enable_interrupts( const hal_mcu_dma_channel_t * p_channel )
{
    dma_channel_set_irq0_enabled( p_channel->channel_id, true );
}

static result_t _dma_init( hal_mcu_dma_t * p_hal_mcu_dma )
{

    if ( p_hal_mcu_dma->is_initialized == true )
    {
        return RESULT_OK;
    }

    // Initialize the channel list.
    p_hal_mcu_dma->channel_count = 0;

    memset( p_hal_mcu_dma->p_channel_list, 0x00, sizeof( p_hal_mcu_dma->p_channel_list ) );

    // Register the DMA interrupt handler
    irq_set_exclusive_handler( HAL_DMA_IRQ, _sdk_dma_handler );
    irq_set_enabled( HAL_DMA_IRQ, true );

    p_hal_mcu_dma->is_initialized = true;

    return RESULT_OK;
}

static result_t _init_channel( hal_mcu_dma_channel_t * p_channel, const hal_mcu_dma_channel_config_t * p_config )
{
    // Set the DMA channel configuration
    p_channel->sdk_dma_config = dma_channel_get_default_config( p_channel->channel_id );

    // Set the size of the data to be transferred to p_config->packet_size
    get_size_def( p_channel->p_packet_size_def, p_config->packet_size );
    ASSERT_DYGMA( p_channel->p_packet_size_def != NULL, "Invalid packet size" );

    channel_config_set_transfer_data_size( &p_channel->sdk_dma_config, p_channel->p_packet_size_def->rp20xx_size );

    // Set the direction of the transfer
    p_channel->direction = p_config->direction;

    // Set the DREQ type for the DMA channel
    if( p_channel->direction == HAL_MCU_DMA_DIRECTION_MEMORY_TO_MEMORY )
    {
        /* The case of the memory to memory transfer. The request on the PICO MCU level will be permanent and thus
         * the transfer will be done in each system clock tick. */
        get_dreq( p_channel->p_dreq, HAL_MCU_DMA_REQUEST_TYPE_FORCE );
    }
    else
    {
        /* The case when a peripheral is involved. The request will depend on the peripheral status. */
        get_dreq( p_channel->p_dreq, p_config->request_type );
    }

    ASSERT_DYGMA( p_channel->p_dreq != NULL, "Invalid DREQ type" );
    channel_config_set_dreq( &p_channel->sdk_dma_config, p_channel->p_dreq->rp20xx_dreq );

    // Initialize the transfer buffer size to 0
    p_channel->transfer_buffer_size = 0;

    // DMA callback function.
    p_channel->event_handler = p_config->event_handler;
    p_channel->p_instance = p_config->p_instance;

    return RESULT_OK;
}

static hal_mcu_dma_channel_t * _channel_claim( void )
{
    hal_mcu_dma_channel_t * p_channel;

    if ( hal_mcu_dma.channel_count >= NUM_DMA_CHANNELS )
    {
        ASSERT_DYGMA( false, "No DMA channels are available" );
        return NULL;
    }

    // Allocate channel instance
    p_channel = heap_alloc( sizeof(hal_mcu_dma_channel_t) );

    // Save the channel id, and claim it.
    p_channel->channel_id = hal_mcu_dma.channel_count;

    dma_channel_claim( p_channel->channel_id );

    // Save the channel instance in the list.
    hal_mcu_dma.p_channel_list[p_channel->channel_id] = p_channel;

    hal_mcu_dma.channel_count++;

    return p_channel;
}

static result_t _channel_transfer_increment_set( hal_mcu_dma_channel_t * p_channel, const hal_mcu_dma_transfer_config_t * p_transfer_config )
{
    const increment_mode_def_t * p_incr_def_read;
    const increment_mode_def_t * p_incr_def_write;

    /* Get the increment modes */
    get_increment_mode_def( p_incr_def_read, p_transfer_config->read_increment_mode );
    get_increment_mode_def( p_incr_def_write, p_transfer_config->write_increment_mode );

    ASSERT_DYGMA( p_incr_def_read != NULL, "DMA invalid Read increment mode" );
    ASSERT_DYGMA( p_incr_def_write != NULL, "DMA invalid Write increment mode" );

    /* Prepare the increment mode */
    switch( p_channel->direction )
    {
        case HAL_MCU_DMA_DIRECTION_MEMORY_TO_MEMORY:

            // Configure DMA channel to transfer data from memory to memory
            channel_config_set_read_increment( &p_channel->sdk_dma_config, p_incr_def_read->rp20xx_increment_mode );
            channel_config_set_write_increment( &p_channel->sdk_dma_config, p_incr_def_write->rp20xx_increment_mode );

            break;

        case HAL_MCU_DMA_DIRECTION_MEMORY_TO_PERIPHERAL:

            ASSERT_DYGMA( p_incr_def_write->increment_mode == HAL_MCU_DMA_INC_MODE_DISABLED, "DMA Invalid peripheral write mode" );

            channel_config_set_read_increment( &p_channel->sdk_dma_config, p_incr_def_read->rp20xx_increment_mode );
            channel_config_set_write_increment( &p_channel->sdk_dma_config, false );

            break;

        case HAL_MCU_DMA_DIRECTION_PERIPHERAL_TO_MEMORY:

            ASSERT_DYGMA( p_incr_def_read->increment_mode == HAL_MCU_DMA_INC_MODE_DISABLED, "DMA Invalid peripheral read mode" );

            channel_config_set_read_increment( &p_channel->sdk_dma_config, false );
            channel_config_set_write_increment( &p_channel->sdk_dma_config, p_incr_def_write->rp20xx_increment_mode );

            break;

        case HAL_MCU_DMA_DIRECTION_PERIPHERAL_TO_PERIPHERAL:

            ASSERT_DYGMA( p_incr_def_write->increment_mode == HAL_MCU_DMA_INC_MODE_DISABLED, "DMA Invalid peripheral write mode" );
            ASSERT_DYGMA( p_incr_def_read->increment_mode == HAL_MCU_DMA_INC_MODE_DISABLED, "DMA Invalid peripheral read mode" );

            channel_config_set_read_increment( &p_channel->sdk_dma_config, false );
            channel_config_set_write_increment( &p_channel->sdk_dma_config, false );

            break;

        default:
            ASSERT_DYGMA( false, "Invalid DMA direction" );
            break;
    }

    return RESULT_OK;
}

static result_t _channel_transfer_prepare( hal_mcu_dma_channel_t * p_channel, const hal_mcu_dma_transfer_config_t * p_transfer_config )
{
    result_t result = RESULT_ERR;

    // Save the size of transfer buffer for later use
    p_channel->transfer_buffer_size = p_transfer_config->buffer_size;

    result = _channel_transfer_increment_set( p_channel, p_transfer_config );
    EXIT_IF_ERR( result, "_channel_transfer_increment_set failed" );

    dma_channel_configure(
            p_channel->channel_id,
            &p_channel->sdk_dma_config,
            p_transfer_config->write_address,            // Write address: Memory buffer
            p_transfer_config->read_address,             // Read address: SPI data register
            p_transfer_config->buffer_size,                 // Number of elements (bytes)
            false
    );

_EXIT:
    return result;
}

/******************** Low-Level hal functions ********************/

result_t hal_ll_mcu_dma_channel_init( hal_mcu_dma_channel_t ** pp_channel ,  const hal_mcu_dma_channel_config_t * p_config )
{
    result_t result = RESULT_ERR;

    hal_mcu_dma_channel_t * p_channel;

    result = _dma_init( &hal_mcu_dma );
    EXIT_IF_ERR( result, "_dma_init failed" );

    p_channel = _channel_claim( );

    if ( p_channel == NULL )
    {
        return RESULT_ERR;
    }

    // Initialize the DMA channel.
    result = _init_channel( p_channel, p_config );
    EXIT_IF_ERR( result, "_init_channel failed" );

    *pp_channel = p_channel;

_EXIT:
    return result;
}

result_t hal_ll_mcu_dma_transfer_start( hal_mcu_dma_channel_t * p_channel, const hal_mcu_dma_transfer_config_t * p_transfer_config )
{
    result_t result = RESULT_ERR;

    result = _channel_transfer_prepare( p_channel, p_transfer_config );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_transfer_start failed" );

    /* Start the dma channel */
    dma_channel_start( p_channel->channel_id );

_EXIT:
    return result;
}

result_t hal_ll_mcu_dma_prepare_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, const hal_mcu_dma_transfer_config_t * p_transfer_config_1,
                                                       hal_mcu_dma_channel_t * p_channel_2, const hal_mcu_dma_transfer_config_t * p_transfer_config_2 )
{
    result_t result = RESULT_ERR;

    if ( p_channel_1 == NULL || p_channel_2 == NULL )
    {
        ASSERT_DYGMA( false, "DMA channel not initialized" );
        return RESULT_ERR;
    }

    /* Prepare the first dma channel */
    result = _channel_transfer_prepare( p_channel_1, p_transfer_config_1 );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_transfer_start 1 failed" );

    /* Prepare the second dma channel */
    result = _channel_transfer_prepare( p_channel_2, p_transfer_config_2 );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_transfer_start 2 failed" );

_EXIT:
    return result;
}

result_t hal_ll_mcu_dma_trigger_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, hal_mcu_dma_channel_t * p_channel_2 )
{
    dma_start_channel_mask( ( 1u << p_channel_1->channel_id ) | ( 1u << p_channel_2->channel_id ) );
    return RESULT_OK;
}

result_t hal_ll_mcu_dma_start_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, const hal_mcu_dma_transfer_config_t * p_transfer_config_1,
                                                       hal_mcu_dma_channel_t * p_channel_2, const hal_mcu_dma_transfer_config_t * p_transfer_config_2 )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_dma_prepare_channels_simultaneously( p_channel_1, p_transfer_config_1, p_channel_2, p_transfer_config_2 );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_transfer_prepare_simultaneously failed" );

    /* Start the dma channels simultaneously */
    result = hal_ll_mcu_dma_trigger_channels_simultaneously( p_channel_1, p_channel_2 );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_transfer_trigger_simultaneously failed" );

_EXIT:
    return result;
}

result_t hal_ll_mcu_dma_is_complete( hal_mcu_dma_channel_t * p_channel )
{
    if ( p_channel == NULL )
    {
        ASSERT_DYGMA( false, "DMA not initialized" );
        return RESULT_ERR;
    }
    if ( dma_channel_is_busy( p_channel->channel_id ) )
    {
        return RESULT_BUSY;
    }
    else
    {
        return RESULT_OK;
    }
}

result_t hal_ll_mcu_dma_stop( hal_mcu_dma_channel_t * p_channel )
{
    if ( p_channel == NULL )
    {
        ASSERT_DYGMA( false, "DMA not initialized" );
        return RESULT_ERR;
    }

    dma_channel_abort( p_channel->channel_id );
    return RESULT_OK;
}

result_t hal_ll_mcu_dma_event_handler_disable( const hal_mcu_dma_channel_t * p_channel )
{
    _disable_interrupts( p_channel );
    return RESULT_OK;
}

result_t hal_ll_mcu_dma_event_handler_enable( const hal_mcu_dma_channel_t * p_channel )
{
    _enable_interrupts( p_channel );
    return RESULT_OK;
}

uint32_t hal_ll_mcu_dma_get_transfer_count(hal_mcu_dma_channel_t *p_channel)
{
    dma_channel_hw_t * p_dma_channel_hw = dma_channel_hw_addr(p_channel->channel_id);

    return p_channel->transfer_buffer_size - p_dma_channel_hw->transfer_count;
}
