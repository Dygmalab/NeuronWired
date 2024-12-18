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
#ifndef __HAL_LL_RP20XX_H
#define __HAL_LL_RP20XX_H

#include "hal/hal_config.h"

#if !defined( HAL_CFG_MCU_SERIES )

    #define HAL_CFG_MCU_SERIES      HAL_MCU_SERIES_RP20

    #if HAL_CFG_MCU == HAL_MCU_RP2040
        #define HAL_MCU_LL_SPEC_LINK "hal/mcu/rp20/rp2040/hal_ll_rp2040_private.h"
    #else
        #undef HAL_CFG_MCU_SERIES
    #endif

#endif /* HAL_CFG_MCU_SERIES */

#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_RP20

//    #define HAL_MCU_LL_COMP_LINK "hal/mcu/rp20/hal_ll_rp20xx_comp.h"
    #define HAL_MCU_LL_DMA_LINK "hal/mcu/rp20/hal_ll_rp20xx_dma.h"
//    #define HAL_MCU_LL_ECB_LINK "hal/mcu/rp20/hal_ll_rp20xx_ecb.h"
//    #define HAL_MCU_LL_FLASH_LINK "hal/mcu/rp20/hal_ll_rp20xx_flash.h"
    #define HAL_MCU_LL_GPIO_LINK "hal/mcu/rp20/hal_ll_rp20xx_gpio.h"
//    #define HAL_MCU_LL_I2C_LINK "hal/mcu/rp20/hal_ll_rp20xx_i2c.h"
//    #define HAL_MCU_LL_MCU_LINK "hal/mcu/rp20/hal_ll_rp20xx_mcu.h"
    #define HAL_MCU_LL_MUTEX_LINK "hal/mcu/rp20/hal_ll_rp20xx_mutex.h"
    #define HAL_MCU_LL_PWR_LINK "hal/mcu/rp20/hal_ll_rp20xx_pwr.h"
//    #define HAL_MCU_LL_RNG_LINK "hal/mcu/rp20/hal_ll_rp20xx_rng.h"
    #define HAL_MCU_LL_SPI_LINK "hal/mcu/rp20/hal_ll_rp20xx_spi.h"
    #define HAL_MCU_LL_SYSTIM_LINK "hal/mcu/rp20/hal_ll_rp20xx_systim.h"
//    #define HAL_MCU_LL_UART_LINK "hal/mcu/rp20/hal_ll_rp20xx_uart.h"
    #define HAL_MCU_LL_WDT_LINK "hal/mcu/rp20/hal_ll_rp20xx_wdt.h"

#endif /* HAL_CFG_MCU_SERIES */

#endif /* __HAL_LL_RP20XX_H */
