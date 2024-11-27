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
#ifndef _HAL_MCU_DMA_H
#define _HAL_MCU_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"
#include "hal_config.h"
#include HAL_MCU_LL_DMA_LINK

// DMA transfer direction
typedef enum {
    HAL_MCU_DMA_DIRECTION_MEMORY_TO_MEMORY = 1,
    HAL_MCU_DMA_DIRECTION_MEMORY_TO_PERIPHERAL,
    HAL_MCU_DMA_DIRECTION_PERIPHERAL_TO_MEMORY,
    HAL_MCU_DMA_DIRECTION_PERIPHERAL_TO_PERIPHERAL
} hal_mcu_dma_direction_t;


typedef enum {
    HAL_MCU_DMA_PACKET_SIZE_8 = 1,
    HAL_MCU_DMA_PACKET_SIZE_16,
    HAL_MCU_DMA_PACKET_SIZE_32
} hal_mcu_dma_packet_size_t;

typedef enum
{
    HAL_MCU_DMA_INC_MODE_DISABLED = 1,
    HAL_MCU_DMA_INC_MODE_ENABLED,
} hal_mcu_dma_increment_mode_t;

/* Events */
typedef enum
{
    HAL_MCU_DMA_EVENT_TRANSFER_COMPLETE = 1,
//    HAL_MCU_DMA_EVENT_HALF_TRANSFER,
    HAL_MCU_DMA_EVENT_TRANSFER_ERROR,
} hal_mcu_dma_event_t;

typedef void (*hal_mcu_dma_event_handler_t)( void * p_instance, hal_mcu_dma_event_t event );

typedef struct hal_mcu_dma_channel   hal_mcu_dma_channel_t;

// DMA configuration structure
typedef struct {

    // Size of the data to be transferred.
    hal_mcu_dma_packet_size_t packet_size;

    // Direction of the transfer.
    hal_mcu_dma_direction_t direction;

    // Request type signal for the DMA channel.
    hal_mcu_dma_request_type_t request_type;

    // DMA callback function.
    hal_mcu_dma_event_handler_t event_handler;

    void * p_instance;

} hal_mcu_dma_channel_config_t;

typedef struct {

    // Periferial or memory address for reading.
    void *read_address;
    hal_mcu_dma_increment_mode_t read_increment_mode;

    // Periferial or memory address for writing.
    void *write_address;
    hal_mcu_dma_increment_mode_t write_increment_mode;

    // Number of elements (bytes) to transfer.
    uint32_t buffer_size;

} hal_mcu_dma_transfer_config_t;

/**
 * @brief Initializes the DMA.
 *
 * @return result_t Returns RESULT_OK if the initialization is successful, otherwise returns RESULT_ERR.
 */
result_t hal_mcu_dma_init( hal_mcu_dma_channel_t ** pp_channel , const hal_mcu_dma_channel_config_t * p_config );

/**
 * @brief Starts the DMA transfer on the specified channel.
 *
 * @param p_channel Pointer to the DMA channel structure.
 *
 * @return result_t Returns RESULT_OK if the start operation is successful, otherwise returns RESULT_ERR.
 */
result_t hal_mcu_dma_start( hal_mcu_dma_channel_t* p_channel, const hal_mcu_dma_transfer_config_t *p_transfer_config );

/**
 * @brief Checks if the DMA transfer is complete on the specified channel.
 *
 * @param p_channel Pointer to the DMA channel structure.
 *
 * @return result_t Returns RESULT_OK if the DMA transfer is complete, otherwise returns RESULT_ERR.
 */
result_t hal_mcu_dma_is_complete( hal_mcu_dma_channel_t *p_channel );

/**
 * @brief Stops the DMA transfer on the specified channel.
 *
 * @param p_channel Pointer to the DMA channel structure.
 *
 * @return result_t Returns RESULT_OK if the stop operation is successful, otherwise returns RESULT_ERR.
 */
result_t hal_mcu_dma_stop( hal_mcu_dma_channel_t *p_channel );

/**
 * @brief Starts the DMA transfer on two channels simultaneously.
 *
 * @param p_channel_1 Pointer to the first DMA channel structure.
 * @param p_channel_2 Pointer to the second DMA channel structure.
 *
 * @return result_t Returns RESULT_OK if the start operation is successful, otherwise returns RESULT_ERR.
 */

result_t hal_mcu_dma_prepare_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, const hal_mcu_dma_transfer_config_t * p_transfer_config_1,
                                                       hal_mcu_dma_channel_t * p_channel_2, const hal_mcu_dma_transfer_config_t * p_transfer_config_2 );

result_t hal_mcu_dma_trigger_channels_simultaneously( hal_mcu_dma_channel_t * p_channel_1, hal_mcu_dma_channel_t * p_channel_2 );

result_t hal_mcu_dma_start_channels_simultaneously( hal_mcu_dma_channel_t* p_channel_1, const hal_mcu_dma_transfer_config_t *p_transfer_config_1,
                                                    hal_mcu_dma_channel_t* p_channel_2, const hal_mcu_dma_transfer_config_t *p_transfer_config_2 );
/**
 * @brief Enables the DMA event handler on the specified channel.
 *
 * @param p_channel Pointer to the DMA channel structure.
 *
 * @return result_t Returns RESULT_OK if the enable operation is successful, otherwise returns RESULT_ERR.
 */
result_t hal_mcu_dma_event_handler_enable( const hal_mcu_dma_channel_t * p_channel );

result_t hal_mcu_dma_event_handler_disable( const hal_mcu_dma_channel_t * p_channel );

uint32_t hal_mcu_dma_get_transfer_count( hal_mcu_dma_channel_t *p_channel );

result_t hal_mcu_dma_abort( hal_mcu_dma_channel_t * p_channel );

#ifdef __cplusplus
}
#endif

#endif //_HAL_MCU_DMA_H
