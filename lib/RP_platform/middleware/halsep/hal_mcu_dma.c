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

result_t hal_mcu_dma_init( hal_mcu_dma_channel_t ** pp_channel , const hal_mcu_dma_channel_config_t *p_config )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_dma_channel_init( pp_channel , p_config );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_init failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_start( hal_mcu_dma_channel_t* p_channel, const hal_mcu_dma_transfer_config_t *p_transfer_config )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_transfer_start( p_channel, p_transfer_config );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_start failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_is_complete( hal_mcu_dma_channel_t* p_channel )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_is_complete( p_channel );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_is_complete failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_stop( hal_mcu_dma_channel_t* p_channel )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_stop(p_channel);
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_stop failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_prepare_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, const hal_mcu_dma_transfer_config_t * p_transfer_config_1,
                                                       hal_mcu_dma_channel_t * p_channel_2, const hal_mcu_dma_transfer_config_t * p_transfer_config_2 )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_prepare_channels_simultaneously( p_channel_1, p_transfer_config_1, p_channel_2, p_transfer_config_2 );
    EXIT_IF_ERR( result, "hal_mcu_dma_prepare_channels_simultaneously failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_trigger_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, hal_mcu_dma_channel_t * p_channel_2 )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_trigger_channels_simultaneously( p_channel_1, p_channel_2 );
    EXIT_IF_ERR( result, "hal_mcu_dma_trigger_channels_simultaneously failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_start_channels_simultaneously( hal_mcu_dma_channel_t* p_channel_1, const hal_mcu_dma_transfer_config_t *p_transfer_config_1,
                                                    hal_mcu_dma_channel_t* p_channel_2, const hal_mcu_dma_transfer_config_t *p_transfer_config_2 )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_start_channels_simultaneously( p_channel_1, p_transfer_config_1, p_channel_2, p_transfer_config_2 );
    EXIT_IF_ERR( result, "hal_mcu_dma_start_channels_simultaneously failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_event_handler_enable( const hal_mcu_dma_channel_t * p_channel )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_event_handler_enable( p_channel );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_event_handler_unmute failed" );

    _EXIT:
    return result;
}

result_t hal_mcu_dma_event_handler_disable( const hal_mcu_dma_channel_t * p_channel )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_event_handler_disable( p_channel );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_event_handler_mute failed" );

    _EXIT:
    return result;
}

uint32_t hal_mcu_dma_get_transfer_count( hal_mcu_dma_channel_t *p_channel )
{

    uint32_t result = hal_ll_mcu_dma_get_transfer_count( p_channel );

    return result;
}

result_t hal_mcu_dma_abort( hal_mcu_dma_channel_t * p_channel )
{
    result_t result = RESULT_ERR;

    result =  hal_ll_mcu_dma_abort( p_channel );
    EXIT_IF_ERR( result, "hal_ll_mcu_dma_abort failed" );

    _EXIT:
    return result;
}