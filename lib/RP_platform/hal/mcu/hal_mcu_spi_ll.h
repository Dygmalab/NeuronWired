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
#ifndef __HAL_MCU_SPI_LL_H_
#define __HAL_MCU_SPI_LL_H_

#include "middleware/middleware.h"

#include "middleware\halsep\hal_mcu_spi.h"


extern result_t hal_ll_mcu_spi_init( hal_mcu_spi_t ** pp_spi, const hal_mcu_spi_conf_t * p_conf );
extern result_t hal_ll_mcu_spi_reserve( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf, hal_mcu_spi_lock_t * p_lock );
extern void hal_ll_mcu_spi_release( hal_mcu_spi_t * p_spi, hal_mcu_spi_lock_t lock );
extern result_t hal_ll_mcu_spi_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf );

extern bool_t hal_ll_mcu_spi_is_slave( hal_mcu_spi_t * p_spi );

#endif /* __HAL_MCU_SPI_LL_H_ */
