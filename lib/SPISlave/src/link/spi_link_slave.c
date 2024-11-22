
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

#include "middleware/utils/mutex.h"
#include "spi_link_def.h"
#include "spi_link_slave.h"

typedef enum
{
    SPILS_STATE_IDLE = 0,
    SPILS_STATE_LISTENING_START,
    SPILS_STATE_LISTENING,
    SPILS_STATE_DATA_RECEIVE_START,
    SPILS_STATE_DATA_RECEIVING,
    SPILS_STATE_DATA_SEND_START,
    SPILS_STATE_DATA_SENDING,
} spils_state_t;

typedef struct
{
    uint8_t * data;

    uint16_t size;
    uint16_t read_pos;
    uint16_t write_pos;

    uint16_t loadsize;
    uint16_t freesize;
} buffer_t;

struct spils
{
    spils_state_t state;

    /* HAL */
    hal_mcu_spi_t * p_spi;

    /* GPIO */
    bool_t pin_int_enabled;
//    hal_mcu_gpio_t * p_pin_int;  /* Interrupt signal */

    /* Buffers */
    buffer_t * p_buffer_in_cache;     /* The pointer for input buffer into which the data is currently being transmitted in */
    buffer_t * p_buffer_in;           /* The pointer for input buffer which can be read from superior layers */

    buffer_t * p_buffer_out_cache;    /* The pointer for output buffer from which the data is currently being transmitted out */
    buffer_t * p_buffer_out;          /* The pointer for output buffer which can be written from superior layers */

    /* Messages */
    uint8_t message_size_max;

    /* Mutexes */
    mutex_t * p_mutex_in;
    mutex_t * p_mutex_out;

    /* Flags */
    bool_t data_in_available;
    bool_t data_out_available;
    bool_t line_in_busy;
    bool_t line_in_is_saturated;

    /* Event handlers */
    void * p_instance;
    spils_event_handler_t event_handler;
};

/* Prototypes */
static result_t _int_signal_set( spils_t * p_spils );
static result_t _int_signal_reset( spils_t * p_spils );

static result_t _transfer_start( spils_t * p_spils, spils_state_t state );
static void _listening_start( spils_t * p_spils, spil_mess_type_t result_mess_type );
static void _data_receive_start( spils_t * p_spils, spil_mess_type_t result_mess_type );
static void _data_send_start( spils_t * p_spils );

/* Prototypes */
static result_t buffer_init( buffer_t ** pp_buffer, uint8_t buffer_size );

static result_t _spi_hal_init( spils_t * p_spils, const spils_conf_t * p_conf )
{
    result_t result = RESULT_ERR;
    hal_mcu_spi_conf_t spi_conf;

    /* Prepare the SPI HAL configuration */
    spi_conf.def = p_conf->spi.def;
    spi_conf.role = HAL_MCU_SPI_ROLE_SLAVE;

    /* SPI Line configuration */
    spi_conf.line = p_conf->spi.line;

    /* Pin configuration */
    spi_conf.slave.pin_miso = p_conf->spi.pin_miso;
    spi_conf.slave.pin_mosi = p_conf->spi.pin_mosi;
    spi_conf.slave.pin_sck = p_conf->spi.pin_sck;
    spi_conf.slave.pin_cs = p_conf->spi.pin_cs;

    result = hal_mcu_spi_init( &p_spils->p_spi, &spi_conf );
    EXIT_IF_ERR( result, "hal_mcu_spi_init failed" );

_EXIT:
    return result;
}

static result_t _gpio_init( spils_t * p_spils, const spils_conf_t * p_conf )
{
//    result_t result = RESULT_ERR;
//    hal_mcu_gpio_conf_t config;

    ASSERT_DYGMA( p_conf->pin_int_enable == false, "The SPI Link Slave INT pin is currently not supported" );
    p_spils->pin_int_enabled = false;

    return RESULT_OK;

//    /**************/
//    /* INT signal */
//    /**************/
//    if( p_conf->pin_int_enable == false )
//    {
//        p_spils->pin_int_enabled = false;
//        p_spils->p_pin_int = NULL;
//        return RESULT_OK;
//    }
//
//    /* Pin configuration */
//    config.pin = p_conf->pin_int;
//    config.direction = HAL_MCU_GPIO_DIR_OUTPUT;
//    config.dir_conf.output.init_val = false;       /* The INT signal is initially reset. */
//
//    result = hal_mcu_gpio_init( &p_spils->p_pin_int, &config );
//    EXIT_IF_ERR( result, "hal_mcu_gpio_init failed" );
//
//    p_spils->pin_int_enabled = true;
//
//_EXIT:
//    return result;
}

static result_t _buffers_init( spils_t * p_spils, const spils_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    ASSERT_DYGMA( p_conf->message_size_max <= UINT8_MAX - sizeof( spil_mess_header_t ), "FATAL: SPI link message_size_max exceeds the maximum possible value" );

    /* Compute the size of buffers */
    uint8_t buffer_size = p_conf->message_size_max + sizeof( spil_mess_header_t );

    result = buffer_init( &p_spils->p_buffer_in_cache, buffer_size );
    EXIT_IF_ERR( result, "buffer_init for buffer_in_cache failed" );

    result = buffer_init( &p_spils->p_buffer_in, buffer_size );
    EXIT_IF_ERR( result, "buffer_init for buffer_in failed" );

    result = buffer_init( &p_spils->p_buffer_out_cache, buffer_size );
    EXIT_IF_ERR( result, "buffer_init for buffer_out_cache failed" );

    result = buffer_init( &p_spils->p_buffer_out, buffer_size );
    EXIT_IF_ERR( result, "buffer_init for p_buffer_out failed" );

_EXIT:
    return result;
}

static result_t _init( spils_t * p_spils, const spils_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    /* SPI HAL */
    result = _spi_hal_init( p_spils, p_conf );
    EXIT_IF_ERR( result, "_spi_hal_init failed" );

    /* GPIO */
    result = _gpio_init( p_spils, p_conf );
    EXIT_IF_ERR( result, "_gpio_init failed" );

    /* Buffers */
    result = _buffers_init( p_spils, p_conf );
    EXIT_IF_ERR( result, "_buffers_init failed" );

    /* Messages */
    p_spils->message_size_max = p_conf->message_size_max;

    /* Initialize the Mutexes */
    mutex_init( &p_spils->p_mutex_in );
    mutex_init( &p_spils->p_mutex_out );

    /* Flags */
    p_spils->data_in_available = false;
    p_spils->data_out_available = false;
    p_spils->line_in_busy = false;
    p_spils->line_in_is_saturated = false;

    /* Event handlers */
    p_spils->p_instance = p_conf->p_instance;
    p_spils->event_handler = p_conf->event_handler;

    /* Initial state */
    p_spils->state = SPILS_STATE_IDLE;

_EXIT:
    return result;

}

result_t spils_init( spils_t ** pp_spils, const spils_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    /* Allocate the instance */
    *pp_spils = heap_alloc( sizeof(spils_t) );

    result = _init( *pp_spils, p_conf );
    EXIT_IF_ERR( result, "_spils_init failed" );

    /* Start the listening */
    _listening_start( *pp_spils, SPIL_MESS_TYPE_RESULT_READY );

_EXIT:
    return result;
}

static void _set_state( spils_t * p_spils, spils_state_t state )
{
    p_spils->state = state;

    switch( p_spils->state )
    {
        case SPILS_STATE_IDLE:
        case SPILS_STATE_LISTENING_START:
        case SPILS_STATE_DATA_RECEIVE_START:
        case SPILS_STATE_DATA_SEND_START:

            _int_signal_reset( p_spils );

            break;

        case SPILS_STATE_LISTENING:

            /* In the listening state, we have yet to decide about the INT signal whether output data is ready or not. */
            if( p_spils->data_out_available == true )
            {
                _int_signal_set( p_spils );
            }
            else
            {
                _int_signal_reset( p_spils );
            }

            break;

        case SPILS_STATE_DATA_RECEIVING:
        case SPILS_STATE_DATA_SENDING:

            /* Signalize that we are ready to receive data */
            _int_signal_set( p_spils );

            break;

//        default:
//            break;
    }
}

static void _event_handler( spils_t * p_spils, spils_event_type_t event_type )
{
    if( p_spils->event_handler == NULL )
    {
        return;
    }

    p_spils->event_handler( p_spils->p_instance, event_type );
}

/*************************/
/*        Mutexes        */
/*************************/

static INLINE bool_t _mutex_in_trylock( spils_t * p_spils )
{
    return mutex_trylock( p_spils->p_mutex_in );
}

static INLINE void _mutex_in_unlock( spils_t * p_spils )
{
    mutex_unlock( p_spils->p_mutex_in );
}

static INLINE bool_t _mutex_out_trylock( spils_t * p_spils )
{
    return mutex_trylock( p_spils->p_mutex_out );
}

static INLINE void _mutex_out_unlock( spils_t * p_spils )
{
    mutex_unlock( p_spils->p_mutex_out );
}

/*************************/
/*         GPIO          */
/*************************/

static result_t _int_signal_set( spils_t * p_spils )
{
    result_t result = RESULT_ERR;

    if( p_spils->pin_int_enabled == false )
    {
        return RESULT_OK;
    }

//    result = hal_mcu_gpio_out( p_spils->p_pin_int, true );
//    EXIT_IF_ERR( result, "hal_mcu_gpio_out failed" );

//_EXIT:
    return result;
}

static result_t _int_signal_reset( spils_t * p_spils )
{
    result_t result = RESULT_ERR;

    if( p_spils->pin_int_enabled == false )
    {
        return RESULT_OK;
    }

//    result = hal_mcu_gpio_out( p_spils->p_pin_int, false );
//    EXIT_IF_ERR( result, "hal_mcu_gpio_out failed" );

//_EXIT:
    return result;
}

/*************************/
/*        Buffers        */
/*************************/

static INLINE void buffer_clear( buffer_t * p_buffer )
{
    p_buffer->read_pos = 0;
    p_buffer->write_pos = 0;

    p_buffer->loadsize = 0;
    p_buffer->freesize = p_buffer->size;
}

static result_t buffer_init( buffer_t ** pp_buffer, uint8_t buffer_size )
{
    buffer_t * p_buffer;

    /* Allocate the instance */
    p_buffer = heap_alloc( sizeof( buffer_t ) );

    p_buffer->size = buffer_size;
    p_buffer->data = heap_alloc( p_buffer->size );

    buffer_clear( p_buffer );

    *pp_buffer = p_buffer;

    return RESULT_OK;
}

static void buffer_update_read_pos( buffer_t * p_buffer, int16_t len )
{
    if ( len == 0 )
    {
        return;
    }

    p_buffer->read_pos = ( uint16_t )( ( int16_t )p_buffer->read_pos + len );

    /* Update buffer sizes */
    p_buffer->loadsize = ( uint16_t )( ( int16_t )p_buffer->loadsize - len );
    p_buffer->freesize = ( uint16_t )( ( int16_t )p_buffer->freesize + len );
}

static void buffer_update_write_pos( buffer_t * p_buffer, int16_t len )
{
    if ( len == 0 )
    {
        return;
    }

    p_buffer->write_pos = ( uint16_t )( ( int16_t )p_buffer->write_pos + len );

    /* Update buffer sizes */
    p_buffer->loadsize = ( uint16_t )( ( int16_t )p_buffer->loadsize + len );
    p_buffer->freesize = ( uint16_t )( ( int16_t )p_buffer->freesize - len );
}

static INLINE uint8_t buffer_get_loadsize( buffer_t * p_buffer )
{
    return p_buffer->loadsize;
}

static void buffer_insert_data( buffer_t * _buffer, const uint8_t data[], uint16_t write_pos, uint16_t len )
{
    /*
     * NOTE: Assert could be here. However, there are currently other checks in the superior functions which ensure the buffer does not get overflown
     */

    /* Optimize for speed */
    if ( len == 1 )
    {
        _buffer->data[write_pos] = data[0];
    }
    else if ( len != 0 )
    {
        memcpy( _buffer->data + write_pos, data, len );
    }
}

static result_t buffer_add( buffer_t * p_buffer, const uint8_t data[], uint16_t len )
{
    /* Check the size */
    if ( len > p_buffer->freesize )
    {
        return RESULT_ERR;
    }
    else
    {
        buffer_insert_data( p_buffer, data, p_buffer->write_pos, len );
    }

    /* Update write position */
    buffer_update_write_pos( p_buffer, len );

    return RESULT_OK;
}

static result_t buffer_get_from_position( buffer_t * p_buffer, uint16_t from_position, uint8_t * data, uint16_t len )
{
    ASSERT_DYGMA( ( data != NULL ), "buffer_get target is NULL!" );

    /* Check the size */
    if ( len > p_buffer->loadsize )
    {
        return RESULT_ERR;
    }
    /* Optimizing for speed */
    else if ( len == 1 )
    {
        *data = p_buffer->data[ from_position ];
    }
    else if ( len != 0 )
    {
        memcpy( data, p_buffer->data + from_position, len );
    }

    return RESULT_OK;
}

static result_t buffer_get( buffer_t * p_buffer, uint8_t * data, uint16_t len )
{
    return buffer_get_from_position( p_buffer, p_buffer->read_pos, data, len );
}

static result_t buffer_get_and_discard( buffer_t * p_buffer, uint8_t * data, uint16_t len )
{
    result_t result = RESULT_ERR;

    result = buffer_get( p_buffer, data, len );
    EXIT_IF_ERR( result, "buffer_get failed" );

    buffer_update_read_pos( p_buffer, len );

_EXIT:
    return result;
}

static INLINE uint8_t * buffer_get_load_space_pointer( buffer_t * p_buffer, uint16_t offset )
{
    return &p_buffer->data[ p_buffer->read_pos + offset ];
}

static INLINE uint8_t * buffer_get_free_space_pointer( buffer_t * p_buffer )
{
    return &p_buffer->data[ p_buffer->write_pos ];
}

/* Returns the size either "from write_pos to read_pos" or "from write_pos to the end of data space"  */
static uint16_t buffer_get_free_space_line_size( const buffer_t * p_buffer )
{
    return p_buffer->size - p_buffer->write_pos;
}

static INLINE void _buffer_recycle( buffer_t * p_buffer )
{
    /*
     * This function is here to emphasize the need of using linear buffers for SPI DMA operation.
     * As the buffer_clear function sets the read_pos and write_pos to 0, the recycled buffers will
     * operate linearly as side effect.
     */

    buffer_clear( p_buffer );
}

static INLINE void _buffer_in_cache_swap( spils_t * p_spils )
{
    buffer_t * _temp_buffer;

    _temp_buffer = p_spils->p_buffer_in;
    p_spils->p_buffer_in = p_spils->p_buffer_in_cache;
    p_spils->p_buffer_in_cache = _temp_buffer;
}

static result_t _buffer_in_cache_data_move( spils_t * p_spils )
{
    result_t result;

    /* Try to lock the input data stream */
    if ( _mutex_in_trylock( p_spils ) == false )
    {
        return RESULT_BUSY;
    }

    /*
     * The swap will be done only if there is valid data in cache and the previous data have been processed
     */

    if( buffer_get_loadsize( p_spils->p_buffer_in_cache ) != 0 )
    {
        /* There is valid data in the cache, so let's try the swap */

        if( buffer_get_loadsize( p_spils->p_buffer_in ) != 0 )
        {
            /* There is still unprocessed data in the buffer_in. So the line is getting saturated */

            result = RESULT_BUSY;
            goto _EXIT;
        }

        _buffer_in_cache_swap( p_spils );

    }

    result = RESULT_OK;

_EXIT:
    /* Unlock the input stream */
    _mutex_in_unlock( p_spils );

    return result;
}

static INLINE void _buffer_out_cache_swap( spils_t * p_spils )
{
    buffer_t * p_temp_buffer;

    p_temp_buffer = p_spils->p_buffer_out;
    p_spils->p_buffer_out = p_spils->p_buffer_out_cache;
    p_spils->p_buffer_out_cache = p_temp_buffer;
}

static result_t _buffer_out_cache_data_move( spils_t * p_spils )
{
    /* Try to lock the output stream */
    if ( _mutex_out_trylock( p_spils ) == false )
    {
        return RESULT_BUSY;
    }

    /* The swap will be done only if there is valid data in source buffer and the cache buffer is empty */
    if( buffer_get_loadsize( p_spils->p_buffer_out ) != 0 && buffer_get_loadsize( p_spils->p_buffer_out_cache ) == 0 )
    {
        _buffer_out_cache_swap( p_spils );
    }

    /* Unlock the output stream */
    _mutex_out_unlock( p_spils );

    return RESULT_OK;
}

/*************************/
/*        Messages       */
/*************************/

static void _mess_compose_result( spils_t * p_spils, buffer_t * p_buffer, spil_mess_type_t transfer_result )
{
    spil_mess_result_t * p_mess_result;

    /* Recycle the buffer first */
    _buffer_recycle( p_buffer );

    /* Prepare the message */
    p_mess_result = (spil_mess_result_t *)buffer_get_free_space_pointer( p_buffer );

    p_mess_result->head.len = sizeof( spil_mess_result_t );
    p_mess_result->head.type = transfer_result;

    buffer_update_write_pos( p_buffer, p_mess_result->head.len );
}

static result_t _mess_compose_data( spils_t * p_spils, buffer_t * p_buffer, const uint8_t * p_data, uint8_t data_len )
{
    result_t result = RESULT_ERR;
    spil_mess_data_t * p_mess_data;

    /* Check the size of data */
    if( data_len > p_spils->message_size_max )
    {
        ASSERT_DYGMA( false, "SPI slave driver output data exceedes maximum message size" );
        return RESULT_ERR;
    }

    /* Recycle the buffer first */
    _buffer_recycle( p_buffer );

    /* Prepare the message */
    p_mess_data = (spil_mess_data_t *)buffer_get_free_space_pointer( p_buffer );

    p_mess_data->head.len = sizeof( spil_mess_data_t ) + data_len;
    p_mess_data->head.type = SPIL_MESS_TYPE_DATA;

    buffer_update_write_pos( p_buffer, sizeof( spil_mess_header_t ) );

    if( p_data != NULL && data_len != 0 )
    {
        result = buffer_add( p_buffer, p_data, data_len );
        ASSERT_DYGMA( result == RESULT_OK, "SPI slave driver output data exceedes available space" );
        EXIT_IF_ERR( result, "buffer_add failed" );
    }

    return RESULT_OK;

_EXIT:
    return result;
}

/*************************/
/*       Transfer        */
/*************************/

static INLINE void _transfer_data_in_prepare( spils_t * p_spils, hal_mcu_spi_transfer_conf_t * p_transfer_conf )
{
    uint16_t buffer_in_cache_loadsize = buffer_get_loadsize( p_spils->p_buffer_in_cache );

    /* If there is valid data unprocessed in the buffer_in_cache, we have to "disable" the input until the space is freed */

    if( buffer_in_cache_loadsize != 0 )
    {
        p_transfer_conf->p_data_in = NULL;
        p_transfer_conf->data_in_len = 0;
        p_spils->line_in_is_saturated = true;
    }
    else
    {
        p_transfer_conf->p_data_in = buffer_get_free_space_pointer( p_spils->p_buffer_in_cache );
        p_transfer_conf->data_in_len = buffer_get_free_space_line_size( p_spils->p_buffer_in_cache );
        p_spils->line_in_is_saturated = false;
    }
}

static INLINE void _transfer_data_out_prepare( spils_t * p_spils, hal_mcu_spi_transfer_conf_t * p_transfer_conf )
{
    uint16_t buffer_out_cache_loadsize = buffer_get_loadsize( p_spils->p_buffer_out_cache );

    /* If there is no valid output data, let's "disable" the output */

    if( buffer_out_cache_loadsize == 0 )
    {
        p_transfer_conf->p_data_out = NULL;
        p_transfer_conf->data_out_len = 0;
    }
    else
    {
        p_transfer_conf->p_data_out = buffer_get_load_space_pointer( p_spils->p_buffer_out_cache, 0 );
        p_transfer_conf->data_out_len = buffer_out_cache_loadsize;
    }
}

static INLINE void _transfer_receive_data_start( spils_t * p_spils )
{
    spil_mess_master_data_send_start_t * p_mess_master_data_send_start;
    spil_mess_type_t transfer_result = SPIL_MESS_TYPE_RESULT_ERR;

    /* Get the master_data_send_start message */
    p_mess_master_data_send_start = (spil_mess_master_data_send_start_t * )buffer_get_load_space_pointer( p_spils->p_buffer_in_cache, 0 );

    /* Check that all message has been received and it is consistent */
    if( p_mess_master_data_send_start->head.len != sizeof(spil_mess_master_data_send_start_t) ||
      ( p_mess_master_data_send_start->head.len > buffer_get_loadsize(p_spils->p_buffer_in_cache ) ))
    {
        /*
         * The data is not complete. We will ignore this message with ERR result code.
         */
        transfer_result = SPIL_MESS_TYPE_RESULT_ERR;
        goto _EXIT;
    }

    transfer_result = SPIL_MESS_TYPE_RESULT_OK;

_EXIT:
    /* Prepare input cache for new data receive */
    _buffer_recycle( p_spils->p_buffer_in_cache );

    if( transfer_result == SPIL_MESS_TYPE_RESULT_OK )
    {
        _data_receive_start( p_spils, transfer_result );
    }
    else
    {
        _listening_start(p_spils, transfer_result );
    }
}

static INLINE void _transfer_send_data_start( spils_t * p_spils )
{
    result_t result;
    spil_mess_master_data_recv_start_t * p_mess_master_data_recv_start;
    spil_mess_type_t transfer_result = SPIL_MESS_TYPE_RESULT_ERR;

    /* Get the master_data_recv_start message */
    p_mess_master_data_recv_start = (spil_mess_master_data_recv_start_t * )buffer_get_load_space_pointer( p_spils->p_buffer_in_cache, 0 );

    /* Check that all message has been received and it is consistent */
    if( p_mess_master_data_recv_start->head.len != sizeof(spil_mess_master_data_recv_start_t) ||
      ( p_mess_master_data_recv_start->head.len > buffer_get_loadsize(p_spils->p_buffer_in_cache) ))
    {
        /*
         * The data is not complete. We will ignore this message with ERR result code.
         */
        transfer_result = SPIL_MESS_TYPE_RESULT_ERR;
        goto _EXIT;
    }

    /* Move the prepared data into the output cache */
    result = _buffer_out_cache_data_move( p_spils );
    if( result != RESULT_OK )
    {
        /* The output line is busy */

        transfer_result = SPIL_MESS_TYPE_RESULT_BUSY;
    }
    else
    {
        p_spils->data_out_available = false;
        transfer_result = SPIL_MESS_TYPE_RESULT_OK;
    }

    /* If there is no data to be sent, prepare "dummy" message with length size 0 */
    if( buffer_get_loadsize( p_spils->p_buffer_out_cache ) == 0 )
    {
        _mess_compose_data( p_spils, p_spils->p_buffer_out_cache, NULL, 0 );
    }

_EXIT:
    /* Prepare input cache for new data receive */
    _buffer_recycle( p_spils->p_buffer_in_cache );

    if( transfer_result == SPIL_MESS_TYPE_RESULT_OK )
    {
        _data_send_start( p_spils );
    }
    else
    {
        _listening_start(p_spils, transfer_result );
    }
}

static INLINE void _transfer_receive_data( spils_t * p_spils )
{
    result_t result;
    spil_mess_data_t * p_mess_data;
    spil_mess_type_t transfer_result = SPIL_MESS_TYPE_RESULT_ERR;

    /* Check the machine is in the correct state */
    if( p_spils->state != SPILS_STATE_DATA_RECEIVING )
    {
        /*
         * We are in unexpected state. We will ignore this message with ERR result code.
         */
        transfer_result = SPIL_MESS_TYPE_RESULT_ERR;
        goto _EXIT;
    }

    /* Get the data message */
    p_mess_data = (spil_mess_data_t * )buffer_get_load_space_pointer( p_spils->p_buffer_in_cache, 0 );

    /* Check that all message has been received */
    if( p_mess_data->head.len > buffer_get_loadsize(p_spils->p_buffer_in_cache) )
    {
        /*
         * The data is not complete. We will ignore this message with ERR result code.
         */
        transfer_result = SPIL_MESS_TYPE_RESULT_ERR;
        goto _EXIT;
    }

    /* Set the true message space within the buffer */
    int16_t temp_write_pos_shift = -(int16_t)buffer_get_loadsize( p_spils->p_buffer_in_cache );
    buffer_update_write_pos( p_spils->p_buffer_in_cache, temp_write_pos_shift);
    buffer_update_write_pos( p_spils->p_buffer_in_cache, p_mess_data->head.len );

    /* Skip the link header */
    buffer_update_read_pos( p_spils->p_buffer_in_cache, sizeof(spil_mess_header_t) );

    /* Move the input cache data into the process space */
    result = _buffer_in_cache_data_move( p_spils );
    if( result != RESULT_OK )
    {
        /* The input line got saturated */

        transfer_result = SPIL_MESS_TYPE_RESULT_OK_BUSY;
        p_spils->line_in_busy = true;
    }
    else
    {
        p_spils->data_in_available = true;
        transfer_result = SPIL_MESS_TYPE_RESULT_OK;
    }

_EXIT:

    /* Check if the cache needs to be preserved */
    if( p_spils->line_in_busy == false )
    {
        _buffer_recycle( p_spils->p_buffer_in_cache );
    }

    /* Initiate new listening session */
    _listening_start( p_spils, transfer_result );

    /* Notify the new data available */
    if( transfer_result == SPIL_MESS_TYPE_RESULT_OK || transfer_result == SPIL_MESS_TYPE_RESULT_OK_BUSY )
    {
        _event_handler( p_spils, SPILS_EVENT_TYPE_DATA_IN_READY );
    }

    UNUSED(result);
}

static INLINE void _transfer_data_in_process( spils_t * p_spils, hal_mcu_spi_transfer_result_t * _transfer_result )
{
    spil_mess_header_t * p_message_in_header;

    if( p_spils->line_in_busy == true || p_spils->line_in_is_saturated )
    {
        /* The input data line is still saturated. */

        _listening_start( p_spils, SPIL_MESS_TYPE_RESULT_BUSY );

        return;
    }
    else if( _transfer_result->data_in_len < sizeof(spil_mess_header_t) )
    {
        /*
         * The data did not come or it is broken. Initiate the new listening process with the ERR response.
         * The currently set cache will be reused;
         */

        _listening_start( p_spils, SPIL_MESS_TYPE_RESULT_ERR );

        return;
    }

    /* Update the incoming buffer space */
    buffer_update_write_pos( p_spils->p_buffer_in_cache, _transfer_result->data_in_len );

    /* Get the message header */
    p_message_in_header = (spil_mess_header_t * )buffer_get_load_space_pointer( p_spils->p_buffer_in_cache, 0 );

    switch( p_message_in_header->type )
    {
        case SPIL_MESS_TYPE_MASTER_DATA_SEND_START:

            _transfer_receive_data_start( p_spils );

            break;

        case SPIL_MESS_TYPE_MASTER_DATA_RECV_START:

            _transfer_send_data_start( p_spils );

            break;

        case SPIL_MESS_TYPE_DATA:

            _transfer_receive_data( p_spils );

            break;

        default:

            _buffer_recycle( p_spils->p_buffer_in_cache );

            if( p_spils->state == SPILS_STATE_DATA_RECEIVING )
            {
                /* Renew the receive process */
                _data_receive_start( p_spils, SPIL_MESS_TYPE_RESULT_OK );  /* NOTE: SPIL_MESS_TYPE_RESULT_OK is the only possibility now.
                                                                                     If that changes, some kind of "result_mess_backup"
                                                                                     should be stored in p_spils */
            }
            else if( p_spils->data_out_available == true )
            {
                _listening_start( p_spils, SPIL_MESS_TYPE_RESULT_DATA_READY );
            }
            else
            {
                _listening_start( p_spils, SPIL_MESS_TYPE_RESULT_READY );
            }
            break;
    }

}

static INLINE void _transfer_data_out_process( spils_t * p_spils, hal_mcu_spi_transfer_result_t * p_transfer_result )
{
    /* We assume the data has been clocked out by the SPI master. So we clear the Tx buffer for the next use */

    _buffer_recycle( p_spils->p_buffer_out_cache );

    if( p_spils->state == SPILS_STATE_DATA_SENDING )
    {
        _event_handler( p_spils, SPILS_EVENT_TYPE_DATA_OUT_SENT );
    }
}

static void _spi_slave_buffers_set_done_handler( spils_t * p_spils )
{
    switch( p_spils->state )
    {
        case SPILS_STATE_LISTENING_START:

            _set_state( p_spils, SPILS_STATE_LISTENING );

            break;

        case SPILS_STATE_DATA_RECEIVE_START:

            _set_state( p_spils, SPILS_STATE_DATA_RECEIVING );

            break;

        case SPILS_STATE_DATA_SEND_START:

            _set_state( p_spils, SPILS_STATE_DATA_SENDING );

            break;

        default:
            break;
    }
}

static void _spi_slave_transfer_done_handler( spils_t * p_spils, hal_mcu_spi_transfer_result_t * p_transfer_result )
{
    /* Reset the INT signal - the SPI interface is not active now */
    _int_signal_reset( p_spils );

    /* Finish the data_out first to possibly have prepared output spaces depending on input data process needs */
    _transfer_data_out_process( p_spils, p_transfer_result );

    /* Finish the data_out first to possibly have prepared output spaces depending on input data process needs */
    _transfer_data_in_process( p_spils, p_transfer_result );
}


static result_t _transfer_start( spils_t * p_spils, spils_state_t state )
{
    result_t result = RESULT_ERR;
    hal_mcu_spi_transfer_conf_t transfer_conf;
    spils_state_t state_backup;

    /* SPI Transfer configuration */
    _transfer_data_in_prepare( p_spils, &transfer_conf );
    _transfer_data_out_prepare( p_spils, &transfer_conf );

    transfer_conf.slave_handlers.p_instance = p_spils;
    transfer_conf.slave_handlers.buffers_set_done_handler = ( hal_mcu_spi_slave_buffers_set_done_handler_t )_spi_slave_buffers_set_done_handler;
    transfer_conf.slave_handlers.transfer_done_handler = ( hal_mcu_spi_slave_transfer_done_handler_t )_spi_slave_transfer_done_handler;

    /* Set state. NOTE the handlers might be called before we leave the function. So we set the state before we start the transition
     *                 and in the ERR case, we recall the state from the backup */
    state_backup = p_spils->state;
    _set_state( p_spils, state );

    /* Start the spi transfer */
    result =  hal_mcu_spi_data_transfer( p_spils->p_spi, &transfer_conf );
    if( result != RESULT_OK )
    {
        _set_state( p_spils, state_backup );
        EXIT_IF_ERR( result, "hal_mcu_spi_data_transfer failed" );
    }

_EXIT:
    return result;
}

/*
 * NOTE: The result_mess_type is actually the result of the last operation.
 */
static void _listening_start( spils_t * p_spils, spil_mess_type_t result_mess_type )
{
    result_t result = RESULT_ERR;

    /* Prepare the response message */
    _mess_compose_result( p_spils, p_spils->p_buffer_out_cache, result_mess_type );

    /* Start the transfer */
    result = _transfer_start( p_spils, SPILS_STATE_LISTENING_START );
    ASSERT_DYGMA( result == RESULT_OK, "The start of the SPI slave listening failed. This should not happen." );

    UNUSED( result );
}

/*
 * NOTE: The result_mess_type is actually the result of the last operation.
 */
static void _data_receive_start( spils_t * p_spils, spil_mess_type_t result_mess_type )
{
    result_t result = RESULT_ERR;

    /* Prepare the response message */
    _mess_compose_result( p_spils, p_spils->p_buffer_out_cache, result_mess_type );

    /* Start the transfer */
    result = _transfer_start( p_spils, SPILS_STATE_DATA_RECEIVE_START );
    ASSERT_DYGMA( result == RESULT_OK, "The start of the SPI slave listening failed. This should not happen." );

    UNUSED( result );
}

static void _data_send_start( spils_t * p_spils )
{
    result_t result = RESULT_ERR;

    /* Start the transfer */
    result = _transfer_start( p_spils, SPILS_STATE_DATA_SEND_START );
    ASSERT_DYGMA( result == RESULT_OK, "The start of the SPI slave listening failed. This should not happen." );

    UNUSED( result );
}

/*************************/
/*          API          */
/*************************/

bool_t spils_data_read_available( spils_t * p_spils )
{
    return p_spils->data_in_available;
}

result_t spils_data_read( spils_t * p_spils, uint8_t * p_data, uint16_t * p_data_size )
{
    result_t result = RESULT_ERR;

    if( p_spils->data_in_available == false )
    {
        *p_data_size = 0;
        return RESULT_OK;
    }

    /* Try to lock the input data stream */
    if ( _mutex_in_trylock( p_spils ) == false )
    {
        return RESULT_BUSY;
    }

    /* Get the data. */
    *p_data_size = buffer_get_loadsize( p_spils->p_buffer_in );
    result = buffer_get_and_discard( p_spils->p_buffer_in, p_data, *p_data_size );
    EXIT_IF_ERR( result, "buffer_get_and_discard failed." );

    /* Recycle the buffer_in */
    _buffer_recycle( p_spils->p_buffer_in );
    p_spils->data_in_available = false;

    /* Possibly move the cache */
    if( p_spils->line_in_busy == true )
    {
        _buffer_in_cache_swap( p_spils );
        p_spils->data_in_available = true;
        p_spils->line_in_busy = false;
    }

_EXIT:
    /* Unlock the input stream */
    _mutex_in_unlock( p_spils );

    return result;
}

result_t spils_data_send( spils_t * p_spils, const uint8_t * p_data, uint16_t data_size )
{
    result_t result = RESULT_ERR;

    if( p_spils->data_out_available == true )
    {
        return RESULT_BUSY;
    }

    /* Try to lock the output data stream */
    if ( _mutex_out_trylock( p_spils ) == false )
    {
        return RESULT_BUSY;
    }

    /* Compose the data message */
    result = _mess_compose_data( p_spils, p_spils->p_buffer_out, p_data, data_size );
    EXIT_IF_ERR( result, "_mess_compose_data failed" );
    p_spils->data_out_available = true;

    /* WARNING: POSSIBLE HAZARD HERE
     * Setting the INT signal here may result in undesired and wrong signaling in case the SPI interrupt comes
     * before the INT signal is actually set. After the interrupt, the INT signal should be reset until the next
     * _spi_slave_buffers_set_done_handler. The SPI master will then be mistaken and might try a transfer which
     * will be ignored due to the SPI slave being configured still.
     *
     * Mutex does not solve this situation nor temporarily disabling the SPI interrupt.
     *
     * This situation is the choice whether we will be asynchronously signaling the new available data or not.
     * We will have to watch the SPI slave behavior and see how critical this issue could be.
     *
     * SUGGESTED SOLUTION: Forcefully finishing the current transfer process (Is it possible? What happens if the data
     *                     is actually in the middle of the transition? ) and invoking the transfer_done handler to
     *                     set a new transfer and the INT signal accordingly.
     *                     This way there would be still some ignored messages during the SPI slave transfer reconfiguration,
     *                     but the INT signal would be handled correctly.
     */

    if ( p_spils->state == SPILS_STATE_LISTENING )
    {
        _int_signal_set( p_spils );
    }

_EXIT:
    /* Unlock the output stream */
    _mutex_out_unlock( p_spils );

    return result;
}
