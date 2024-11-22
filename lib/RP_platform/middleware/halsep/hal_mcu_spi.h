
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
 
#ifndef __HAL_MCU_SPI_H_
#define __HAL_MCU_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"
#include "hal_config.h"
#include HAL_MCU_LL_SPI_LINK

typedef enum
{
    HAL_MCU_SPI_ROLE_SLAVE = 1,
} hal_mcu_spi_role_t;

typedef enum
{
    HAL_MCU_SPI_CPHA_LEAD = 1,  /* Sample on the first CLK edge */
    HAL_MCU_SPI_CPHA_TRAIL,     /* Sample on the second CLK edge */
} hal_mcu_spi_cpha_t;   /* Clock phase */

typedef enum
{
    HAL_MCU_SPI_CPOL_ACTIVE_HIGH = 1,   /* The CLK is pulled low when the SPI is not active */
    HAL_MCU_SPI_CPOL_ACTIVE_LOW,        /* The CLK is pulled high when the SPI is not active */
} hal_mcu_spi_cpol_t;   /* Clock polarity */

typedef enum
{
    HAL_MCU_SPI_BIT_ORDER_LSB_FIRST = 1,
    HAL_MCU_SPI_BIT_ORDER_MSB_FIRST,
} hal_mcu_spi_bit_order_t;

/********************************************/
/*          SPI Line configuration          */
/********************************************/

typedef struct
{
    hal_mcu_spi_frequency_t freq;

    hal_mcu_spi_cpha_t cpha;            /* Clock phase */
    hal_mcu_spi_cpol_t cpol;            /* Clock polarity */
    hal_mcu_spi_bit_order_t bit_order;
} hal_mcu_spi_line_conf_t;

typedef struct
{
    /* Pin configuration */
    hal_mcu_gpio_pin_t pin_miso;
    hal_mcu_gpio_pin_t pin_mosi;
    hal_mcu_gpio_pin_t pin_sck;
    hal_mcu_gpio_pin_t pin_cs;
} hal_mcu_spi_slave_conf_t;

typedef struct
{
    /* SPI peripheral definition */
    hal_mcu_spi_periph_def_t def;
    hal_mcu_spi_role_t role;

    /* SPI Line configuration */
    hal_mcu_spi_line_conf_t line;

    hal_mcu_spi_slave_conf_t slave;

} hal_mcu_spi_conf_t;

typedef struct
{
    size_t data_out_len;
    size_t data_in_len;
} hal_mcu_spi_transfer_result_t;

typedef void( *hal_mcu_spi_slave_buffers_set_done_handler_t )( void *  p_instance );
typedef void( *hal_mcu_spi_slave_transfer_done_handler_t )( void * p_instance, hal_mcu_spi_transfer_result_t * p_result );

typedef struct
{
    /* Event handlers */
    void * p_instance;
    hal_mcu_spi_slave_buffers_set_done_handler_t buffers_set_done_handler;
    hal_mcu_spi_slave_transfer_done_handler_t transfer_done_handler;
} hal_mcu_spi_transfer_conf_slave_handlers_t;

typedef struct
{
    uint8_t * p_data_out;
    uint8_t * p_data_in;
    size_t data_out_len;
    size_t data_in_len;

    /* Event handlers */
    hal_mcu_spi_transfer_conf_slave_handlers_t slave_handlers;
} hal_mcu_spi_transfer_conf_t;

/* Lock */
typedef uint32_t hal_mcu_spi_lock_t;

/* instance */
typedef struct hal_mcu_spi hal_mcu_spi_t;

extern result_t hal_mcu_spi_init( hal_mcu_spi_t ** pp_spi, const hal_mcu_spi_conf_t * p_conf );
extern result_t hal_mcu_spi_reserve( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf, hal_mcu_spi_lock_t * p_lock );
extern void hal_mcu_spi_release( hal_mcu_spi_t * p_spi, hal_mcu_spi_lock_t lock );
extern result_t hal_mcu_spi_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf );

extern bool_t hal_mcu_spi_is_slave( hal_mcu_spi_t * p_spi );

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MCU_SPI_H_ */
