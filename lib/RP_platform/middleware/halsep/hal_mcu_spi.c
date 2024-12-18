
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

#include "hal_mcu_spi_ll.h"

result_t hal_mcu_spi_init( hal_mcu_spi_t ** pp_spi, const hal_mcu_spi_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_spi_init( pp_spi, p_conf );
    EXIT_IF_ERR( result, "hal_ll_mcu_spi_init failed" );

_EXIT:
    return result;
}

result_t hal_mcu_spi_reserve( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf, hal_mcu_spi_lock_t * p_lock )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_spi_reserve( p_spi, p_line_conf, p_lock );
    EXIT_IF_ERR( result, "hal_ll_mcu_spi_reserve failed" );

_EXIT:
    return result;
}

void hal_mcu_spi_release( hal_mcu_spi_t * p_spi, hal_mcu_spi_lock_t lock )
{
    hal_ll_mcu_spi_release( p_spi, lock );
}

result_t hal_mcu_spi_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_spi_data_transfer( p_spi, p_transfer_conf );
    EXIT_IF_ERR( result, "hal_ll_mcu_spi_data_transfer failed" );

_EXIT:
    return result;
}

bool_t hal_mcu_spi_is_slave( hal_mcu_spi_t * p_spi )
{
    return hal_ll_mcu_spi_is_slave( p_spi );
}

