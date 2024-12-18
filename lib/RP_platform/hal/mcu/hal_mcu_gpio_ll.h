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
#ifndef __HAL_MCU_GPIO_LL_H
#define __HAL_MCU_GPIO_LL_H

#include "dl_middleware.h"

#include "hal_mcu_gpio.h"


extern result_t hal_ll_mcu_gpio_init( hal_mcu_gpio_t ** pp_gpio, const hal_mcu_gpio_conf_t * p_conf );
extern result_t hal_ll_mcu_gpio_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_t * p_conf );

extern result_t hal_ll_mcu_gpio_out( hal_mcu_gpio_t * p_gpio, bool_t state );
extern result_t hal_ll_mcu_gpio_in( hal_mcu_gpio_t * p_gpio, bool_t * p_state );

extern result_t hal_ll_mcu_gpio_irq_register( hal_mcu_gpio_t * p_gpio, hal_mcu_gpio_handler_t handler, void * p_instance );
extern void hal_ll_mcu_gpio_irq_enable( hal_mcu_gpio_t * p_gpio );
extern void hal_ll_mcu_gpio_irq_disable( hal_mcu_gpio_t * p_gpio );

#endif //__HAL_MCU_GPIO_LL_H
