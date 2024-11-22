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
#ifndef _HAL_MCU_DMA_LL_H
#define _HAL_MCU_DMA_LL_H

#include "dl_middleware.h"
#include "hal_mcu_dma.h"

// Initialize the DMA subsystem
extern result_t hal_ll_mcu_dma_channel_init( hal_mcu_dma_channel_t ** pp_channel ,  const hal_mcu_dma_channel_config_t *p_config);

// Start a DMA transfer
extern result_t hal_ll_mcu_dma_transfer_start(hal_mcu_dma_channel_t* p_channel, const hal_mcu_dma_transfer_config_t *p_transfer_config );

// Start two DMA transfers simultaneously
extern result_t hal_ll_mcu_dma_prepare_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, const hal_mcu_dma_transfer_config_t * p_transfer_config_1,
                                                       hal_mcu_dma_channel_t * p_channel_2, const hal_mcu_dma_transfer_config_t * p_transfer_config_2 );

extern result_t hal_ll_mcu_dma_trigger_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, hal_mcu_dma_channel_t * p_channel_2 );

extern result_t hal_ll_mcu_dma_start_channels_simultaneously (hal_mcu_dma_channel_t* p_channel_1, const hal_mcu_dma_transfer_config_t *p_transfer_config_1,
                                                                                                                hal_mcu_dma_channel_t* p_channel_2, const hal_mcu_dma_transfer_config_t *p_transfer_config_2);

// Check if the DMA transfer is complete
extern result_t hal_ll_mcu_dma_is_complete(hal_mcu_dma_channel_t* p_channel);

// Stop a DMA transfer
extern result_t hal_ll_mcu_dma_stop(hal_mcu_dma_channel_t* p_channel);

extern result_t hal_ll_mcu_dma_event_handler_disable(const hal_mcu_dma_channel_t * p_channel);

extern result_t hal_ll_mcu_dma_event_handler_enable( const hal_mcu_dma_channel_t * p_channel);

extern uint32_t hal_ll_mcu_dma_get_transfer_count(hal_mcu_dma_channel_t *p_channel);


//extern result_t hal_ll_mcu_dma_restart_channel(hal_mcu_dma_channel_t *p_channel,   hal_mcu_dma_transfer_config_t  *config);

#endif //_HAL_MCU_DMA_LL_H
