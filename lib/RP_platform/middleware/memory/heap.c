
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

#include "heap.h"
#include "config_app.h"

#ifndef HEAP_SIZE
    #error "The size of heap is not defined. Do it in your config_app.h."
#endif /* HEAP_SIZE */

static uint8_t _pool[ HEAP_SIZE ];
static uint8_t * _pool_pointer = _pool;

void * heap_alloc( size_t size )
{
    uint8_t * result = NULL;
    
    /* Check the heap size */
    ASSERT_DYGMA( ( _pool_pointer - _pool + size ) <= HEAP_SIZE, "failed - heap size exceeded" );

    result = _pool_pointer;
    _pool_pointer += alignment_ceil( size, MCU_ALIGNMENT_SIZE );

    return result;
}

void heap_clear( void )
{
    memset( _pool, 0x00, sizeof( _pool ) );
    _pool_pointer = _pool;
}
