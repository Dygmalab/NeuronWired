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
#include "hal\mcu\hal_mcu_mutex_ll.h"

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/mutex.h"

#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_RP20

struct hal_mcu_mutex
{
    mutex_t  rp_mtx;
};


static void _rp_mtx_init( hal_mcu_mutex_t * p_mutex )
{
    mutex_init( &p_mutex->rp_mtx );
}

void hal_ll_mcu_mutex_init( hal_mcu_mutex_t ** pp_mutex )
{
    /* Allocate the mutex instance */
    *pp_mutex = heap_alloc( sizeof(hal_mcu_mutex_t) );

    _rp_mtx_init( *pp_mutex );
}


void hal_ll_mcu_mutex_destroy( hal_mcu_mutex_t * p_mutex )
{
    ASSERT_DYGMA(false,"hal_ll_mcu_mutex_destroy not supported");
}

bool_t hal_ll_mcu_mutex_trylock( hal_mcu_mutex_t * p_mutex )
{
    return mutex_try_enter(&p_mutex->rp_mtx, NULL);
}

void hal_ll_mcu_mutex_unlock( hal_mcu_mutex_t * p_mutex )
{
    mutex_exit( &p_mutex->rp_mtx );
}

#endif /* HAL_CFG_MCU_SERIES */
