
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

#ifndef __SPI_LINK_SLAVE_H_
#define __SPI_LINK_SLAVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"
#include "hal_mcu_gpio.h"
#include "hal_mcu_spi.h"

typedef enum
{
    SPILS_EVENT_TYPE_DATA_IN_READY = 1,
    SPILS_EVENT_TYPE_DATA_OUT_SENT,
} spils_event_type_t;

typedef void( *spils_event_handler_t )( void * p_instance, spils_event_type_t event_type );

typedef struct
{
    /* SPI peripheral definition */
    hal_mcu_spi_periph_def_t def;

    /* Pin configuration */
    hal_mcu_gpio_pin_t pin_miso;
    hal_mcu_gpio_pin_t pin_mosi;
    hal_mcu_gpio_pin_t pin_sck;
    hal_mcu_gpio_pin_t pin_cs;

    hal_mcu_spi_line_conf_t line;
} spils_spi_hal_conf_t;

typedef struct
{
    /* HAL */
    spils_spi_hal_conf_t spi;


    /* GPIO */
    bool_t pin_int_enable;
//    hal_mcu_gpio_pin_t pin_int;     /* INT output pin. Will be used only if the pin_int_enable is true */

    /* Messages */
    uint8_t message_size_max;

    /* Event handlers */
    void * p_instance;
    spils_event_handler_t event_handler;

} spils_conf_t;

typedef struct spils spils_t;

extern result_t spils_init( spils_t ** pp_spils, const spils_conf_t * p_conf );
extern bool_t spils_data_read_available( spils_t * p_spils );
extern result_t spils_data_read( spils_t * p_spils, uint8_t * p_data, uint16_t * p_data_size );
extern result_t spils_data_send( spils_t * p_spils, const uint8_t * p_data, uint16_t data_size );

#ifdef __cplusplus
}
#endif

#endif /* __SPI_LINK_SLAVE_H_ */
