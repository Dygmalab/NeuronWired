
#include "hal_mcu_gpio_ll.h"

result_t hal_mcu_gpio_init( hal_mcu_gpio_t ** pp_gpio, const hal_mcu_gpio_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_gpio_init( pp_gpio, p_conf );
    EXIT_IF_ERR( result, "hal_ll_mcu_gpio_init failed" );

_EXIT:
    return result;
}

result_t hal_mcu_gpio_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_gpio_config( p_gpio, p_conf );
    EXIT_IF_ERR( result, "hal_ll_mcu_gpio_config failed" );

_EXIT:
    return result;
}

result_t hal_mcu_gpio_out( hal_mcu_gpio_t * p_gpio, bool_t state )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_gpio_out( p_gpio, state );
    EXIT_IF_ERR( result, "hal_ll_mcu_gpio_out failed" );

_EXIT:
    return result;
}

result_t hal_mcu_gpio_in( hal_mcu_gpio_t * p_gpio, bool_t * p_state )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_gpio_in( p_gpio, p_state );
    EXIT_IF_ERR( result, "hal_ll_mcu_gpio_in failed" );

_EXIT:
    return result;
}

result_t hal_mcu_gpio_irq_register( hal_mcu_gpio_t * p_gpio, hal_mcu_gpio_handler_t handler, void * p_instance )
{
    result_t result = RESULT_ERR;

    result = hal_ll_mcu_gpio_irq_register( p_gpio, handler, p_instance );
    EXIT_IF_ERR( result, "hal_ll_mcu_gpio_irq_register failed" );

_EXIT:
    return result;
}

void hal_mcu_gpio_irq_enable( hal_mcu_gpio_t * p_gpio )
{
    hal_ll_mcu_gpio_irq_enable( p_gpio );
}

void hal_mcu_gpio_irq_disable( hal_mcu_gpio_t * p_gpio )
{
    hal_ll_mcu_gpio_irq_disable( p_gpio );
}

