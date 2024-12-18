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
#ifndef _HAL_LL_RP20XX_DMA_H
#define _HAL_LL_RP20XX_DMA_H

// DMA Data Request (DREQ) signals
typedef enum {
    HAL_MCU_DMA_REQUEST_TYPE_SPI0_TX = 1,
    HAL_MCU_DMA_REQUEST_TYPE_SPI0_RX ,
    HAL_MCU_DMA_REQUEST_TYPE_SPI1_TX ,
    HAL_MCU_DMA_REQUEST_TYPE_SPI1_RX ,
    HAL_MCU_DMA_REQUEST_TYPE_I2C0_TX,
    HAL_MCU_DMA_REQUEST_TYPE_I2C0_RX ,
    HAL_MCU_DMA_REQUEST_TYPE_I2C1_TX ,
    HAL_MCU_DMA_REQUEST_TYPE_I2C1_RX ,
    HAL_MCU_DMA_REQUEST_TYPE_UART0_TX ,
    HAL_MCU_DMA_REQUEST_TYPE_UART0_RX ,
    HAL_MCU_DMA_REQUEST_TYPE_UART1_TX ,
    HAL_MCU_DMA_REQUEST_TYPE_UART1_RX ,
    HAL_MCU_DMA_REQUEST_TYPE_ADC,
    HAL_MCU_DMA_REQUEST_TYPE_PIO0_TX,
    HAL_MCU_DMA_REQUEST_TYPE_PIO0_RX,
} hal_mcu_dma_request_type_t;


#endif //_HAL_LL_RP20XX_DMA_H
