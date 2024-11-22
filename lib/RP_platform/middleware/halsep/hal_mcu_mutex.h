
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

#ifndef __HAL_MCU_MUTEX_H_
#define __HAL_MCU_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"
#include "hal_config.h"
#include HAL_MCU_LL_MUTEX_LINK

/* instance */
typedef struct hal_mcu_mutex hal_mcu_mutex_t;

extern void hal_mcu_mutex_init( hal_mcu_mutex_t ** __mutex );
extern void hal_mcu_mutex_destroy( hal_mcu_mutex_t * _mutex );
extern bool_t hal_mcu_mutex_trylock( hal_mcu_mutex_t * _mutex );
extern void hal_mcu_mutex_unlock( hal_mcu_mutex_t * _mutex );

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MCU_MUTEX_H_ */
