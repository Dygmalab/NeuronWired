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
#ifndef __HAL_CONFIG_H
#define __HAL_CONFIG_H

#include "hal_defines.h"
#include "config_app.h"

#ifdef HAL_CFG_MCU_SERIES
    #error "MCU series configuration needs to be left undefined. Proper MCU series will be determined automatically based on HAL_CFG_MCU configuration in your Makefile."
#endif /* HAL_CFG_MCU_SERIES */

#ifndef HAL_CFG_MCU
    #define HAL_CFG_MCU             HAL_MCU_UNKNOWN
    #define HAL_CFG_MCU_SERIES      HAL_MCU_SERIES_UNKNOWN

    #error "Invalid or unknown target MCU. Please, choose valid MCU in your Makefile file."
#endif /* HAL_CFG_MCU */

/*
 * Process the supported MCU series
 */
#include "hal/mcu/rp20/hal_ll_rp20xx.h"

#if !defined ( HAL_CFG_MCU_SERIES ) || ( HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_UNKNOWN )
    #error "Invalid target MCU chosen. Please, choose valid MCU in your Makefile."
#endif

#endif /* __HAL_CONFIG_H */

