
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
 
#ifndef __MUTEX_H_
#define __MUTEX_H_

#include "dl_middleware.h"
#include "halsep/hal_mcu_mutex.h"

typedef hal_mcu_mutex_t mutex_t;

static INLINE void mutex_init( mutex_t ** __mutex )
{
    hal_mcu_mutex_init( __mutex );
}

static INLINE void mutex_destroy( mutex_t * _mutex )
{
    hal_mcu_mutex_destroy( _mutex );
}

static INLINE bool_t mutex_trylock( mutex_t * _mutex )
{
    return hal_mcu_mutex_trylock( _mutex );
}

static INLINE void mutex_unlock( mutex_t * _mutex )
{
    hal_mcu_mutex_unlock( _mutex );
}


#endif /* __MUTEX_H_ */
