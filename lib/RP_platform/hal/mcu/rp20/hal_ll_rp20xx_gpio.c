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
#include "pico/stdio.h"
#include "hardware/gpio.h"

#include "hal_mcu_gpio_ll.h"


#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_RP20

#define MAX_PIN_NUMBER 30

/* Trigger definitions */
typedef struct
{
    hal_mcu_gpio_trigger_t trigger;
    uint32_t rp20xx_edge;
} trigger_def_t;

static const trigger_def_t p_trigger_def_array[] =
        {
                { .trigger = HAL_MCU_GPIO_TRIGGER_NONE    , .rp20xx_edge = 0},
                { .trigger = HAL_MCU_GPIO_TRIGGER_RISING  , .rp20xx_edge = GPIO_IRQ_EDGE_RISE },
                { .trigger = HAL_MCU_GPIO_TRIGGER_FALLING , .rp20xx_edge = GPIO_IRQ_EDGE_FALL},
                { .trigger = HAL_MCU_GPIO_TRIGGER_BOTH    , .rp20xx_edge = (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL) },
        };
#define get_trigger_def( def, id ) _get_def( def, p_trigger_def_array, trigger_def_t, trigger, id )

typedef struct
{
    const trigger_def_t * p_trigger_def;

    hal_mcu_gpio_handler_t handler;
    void * p_instance;
} gpio_dir_spec_input_t;

typedef union
{
    //gpio_dir_spec_output_t output;
    gpio_dir_spec_input_t input;
} gpio_dir_spec_t;

struct hal_mcu_gpio
{
    hal_mcu_gpio_pin_t pin;
    hal_mcu_gpio_direction_t dir;

    gpio_dir_spec_t dir_spec;
};


static hal_mcu_gpio_t * p_gpio_list[MAX_PIN_NUMBER];

/*
 * Prototypes
 */
static result_t _pin_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_t * p_conf );
static inline void _gpio_pull_config (  hal_mcu_gpio_t const * p_gpio , hal_mcu_gpio_pull_t  p_pull);

/*
 * Execute interrupt callback function.
 */
static void _rp_gpio_evt_handler( uint pin, uint32_t events )
{
    hal_mcu_gpio_t * p_gpio = p_gpio_list[ pin ];

    if( p_gpio->dir_spec.input.handler != NULL )
    {
        p_gpio->dir_spec.input.handler( p_gpio->dir_spec.input.p_instance );
    }
}

result_t hal_ll_mcu_gpio_init( hal_mcu_gpio_t ** pp_gpio, const hal_mcu_gpio_conf_t * p_conf )
{
    result_t result = RESULT_OK;

    ASSERT_DYGMA( p_conf->pin < MAX_PIN_NUMBER, "Invalid pin number." );

    /* First, initialize the pin  */
    gpio_init(p_conf->pin);
    EXIT_IF_ERR( result, "_gpiote_init failed" );

    /* Allocate the gpio instance */
    p_gpio_list[ p_conf->pin ] = heap_alloc( sizeof(hal_mcu_gpio_t) );

    /* configure pin */
    result = _pin_config( p_gpio_list[ p_conf->pin ], p_conf );
    EXIT_IF_ERR( result, "pin config failed" );

    *pp_gpio = p_gpio_list[ p_conf->pin ];

    _EXIT:
    return result;
}

static result_t _pin_output_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_output_t * p_dir_conf )
{
    result_t result = RESULT_OK;

    gpio_set_dir(p_gpio->pin, GPIO_OUT);

    gpio_put(p_gpio->pin, p_dir_conf->init_val);

    return result;
}


static result_t _pin_input_config( hal_mcu_gpio_t  * p_gpio, const hal_mcu_gpio_conf_input_t * p_dir_conf )
{
    gpio_dir_spec_input_t * p_spec_input = &p_gpio->dir_spec.input;

    result_t result = RESULT_OK;

    gpio_set_dir(p_gpio->pin, GPIO_IN);

   /* Configure pull gpio */

    _gpio_pull_config(p_gpio , p_dir_conf->pull);

    /* Configure interrupts */

    get_trigger_def( p_spec_input->p_trigger_def, p_dir_conf->trigger );

    ASSERT_DYGMA( p_spec_input->p_trigger_def != NULL, "invalid trigger config" );

    if (p_spec_input->p_trigger_def->trigger == HAL_MCU_GPIO_TRIGGER_NONE)
    {
        gpio_set_irq_enabled(p_gpio->pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    }
    else
    {
        gpio_set_irq_enabled_with_callback(p_gpio->pin, p_spec_input->p_trigger_def->rp20xx_edge, true, _rp_gpio_evt_handler);
    }

    return result;
}

static result_t _pin_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_t * p_conf )
{

    result_t result = RESULT_ERR;

    p_gpio->pin = p_conf->pin;
    p_gpio->dir = p_conf->direction;

    switch( p_gpio->dir )
    {
        case HAL_MCU_GPIO_DIR_INPUT:

            result = _pin_input_config( p_gpio, &p_conf->dir_conf.input );
            EXIT_IF_ERR( result, "_pin_input_config failed" );

            break;

        case HAL_MCU_GPIO_DIR_OUTPUT:

            result = _pin_output_config( p_gpio, &p_conf->dir_conf.output );
            EXIT_IF_ERR( result, "_pin_out_config failed" );

            break;

        default:

            ASSERT_DYGMA( false, "invalid direction config" );

            break;
    }

    _EXIT:
    return result;

}

result_t hal_ll_mcu_gpio_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    result = _pin_config( p_gpio, p_conf );
    EXIT_IF_ERR( result, "pin config failed" );

    _EXIT:
    return result;
}

result_t hal_ll_mcu_gpio_out( hal_mcu_gpio_t * p_gpio, bool_t state )
{

    gpio_set_dir(p_gpio->pin, p_gpio->dir);

    gpio_put(p_gpio->pin, state);

    return RESULT_OK;
}

result_t hal_ll_mcu_gpio_in( hal_mcu_gpio_t * p_gpio, bool_t * p_state )
{

    *p_state = ( gpio_get( p_gpio->pin ) == 0 ) ? false : true;

    return RESULT_OK;
}

result_t hal_ll_mcu_gpio_irq_register( hal_mcu_gpio_t * p_gpio, hal_mcu_gpio_handler_t handler, void * p_instance )
{
    result_t result = RESULT_ERR;

    switch( p_gpio->dir )
    {
        case HAL_MCU_GPIO_DIR_INPUT:

            if( p_gpio->dir_spec.input.p_trigger_def->trigger != HAL_MCU_GPIO_TRIGGER_NONE )
            {
                /* Save the trigger handler */
                p_gpio->dir_spec.input.handler = handler;
                p_gpio->dir_spec.input.p_instance = p_instance;

                result = RESULT_OK;
            }
            else
            {
                ASSERT_DYGMA( false, "The GPIO is not configured to trigger IRQ." );
            }

            break;

        case HAL_MCU_GPIO_DIR_OUTPUT:

            ASSERT_DYGMA( false, "There is currently no IRQ for the output pins" );

            break;
    }

    return result;
}

void hal_ll_mcu_gpio_irq_enable( hal_mcu_gpio_t * p_gpio )
{
    gpio_dir_spec_input_t const * p_spec_input = &p_gpio->dir_spec.input;

    switch( p_gpio->dir )
    {
        case HAL_MCU_GPIO_DIR_INPUT:

            if( p_gpio->dir_spec.input.p_trigger_def->trigger != HAL_MCU_GPIO_TRIGGER_NONE )
            {
                gpio_set_irq_enabled(p_gpio->pin, p_spec_input->p_trigger_def->rp20xx_edge, true);
            }
            else
            {
                ASSERT_DYGMA( false, "The GPIO is not configured to trigger IRQ." );
            }

            break;

        case HAL_MCU_GPIO_DIR_OUTPUT:

            ASSERT_DYGMA( false, "There is currently no IRQ for the output pins" );

            break;
    }
}

void hal_ll_mcu_gpio_irq_disable( hal_mcu_gpio_t * p_gpio )
{
    switch( p_gpio->dir )
    {
        case HAL_MCU_GPIO_DIR_INPUT:

            if( p_gpio->dir_spec.input.p_trigger_def->trigger != HAL_MCU_GPIO_TRIGGER_NONE )
            {
                gpio_set_irq_enabled(p_gpio->pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
            }
            else
            {
                ASSERT_DYGMA( false, "The GPIO is not configured to trigger IRQ." );
            }

            break;

        case HAL_MCU_GPIO_DIR_OUTPUT:

            ASSERT_DYGMA( false, "There is currently no IRQ for the output pins" );

            break;
    }
}

static inline void _gpio_pull_config (  hal_mcu_gpio_t const * p_gpio , const hal_mcu_gpio_pull_t  p_pull)
{

    switch ( p_pull ) {

        case HAL_MCU_GPIO_PULL_DISABLED:
        {
            gpio_disable_pulls(p_gpio->pin);
        }
        break;

        case HAL_MCU_GPIO_PULL_DOWN:
        {
            gpio_pull_down(p_gpio->pin);
        }
        break;

        case HAL_MCU_GPIO_PULL_UP:
        {
            gpio_pull_up(p_gpio->pin);
        }
        break;

        default:
        {
            ASSERT_DYGMA(false,"Wrong pull configuration");
        }
        break;
    }
}

#endif /* HAL_CFG_MCU_SERIES */
