
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
 
#ifndef __HAL_MCU_GPIO_H_
#define __HAL_MCU_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"
#include "hal_config.h"
#include HAL_MCU_LL_GPIO_LINK

typedef enum
{
    HAL_MCU_GPIO_DIR_INPUT = 1,
    HAL_MCU_GPIO_DIR_OUTPUT,
} hal_mcu_gpio_direction_t;

typedef enum
{
    HAL_MCU_GPIO_PULL_DISABLED = 1,
    HAL_MCU_GPIO_PULL_DOWN,
    HAL_MCU_GPIO_PULL_UP,
} hal_mcu_gpio_pull_t;

/*
 * The GPIO trigger is used to set the source for the GPIO interrupt. The IRQ register and enable should follow
 * in order to trigger the interrupt routine.
 *
 * Setting the GPIO trigger without the following IRQ register and enable, should work as the wake-up source.
 */
typedef enum
{
    HAL_MCU_GPIO_TRIGGER_NONE = 0,
    HAL_MCU_GPIO_TRIGGER_RISING,
    HAL_MCU_GPIO_TRIGGER_FALLING,
    HAL_MCU_GPIO_TRIGGER_BOTH,
} hal_mcu_gpio_trigger_t;

/* External handler definition */
typedef void( *hal_mcu_gpio_handler_t )( void * p_instance );

typedef struct
{
    hal_mcu_gpio_pull_t pull;

    /* Interrupt setup */
    hal_mcu_gpio_trigger_t trigger;

} hal_mcu_gpio_conf_input_t;

typedef struct
{
    bool_t init_val;
} hal_mcu_gpio_conf_output_t;

typedef union
{

    hal_mcu_gpio_conf_input_t input;
    hal_mcu_gpio_conf_output_t output;

} hal_mcu_gpio_conf_direction_t;

typedef struct
{
    hal_mcu_gpio_pin_t pin;
    hal_mcu_gpio_direction_t direction;

    hal_mcu_gpio_conf_direction_t dir_conf;

} hal_mcu_gpio_conf_t;

/* instance */

typedef struct hal_mcu_gpio hal_mcu_gpio_t;
/**
 * @brief Initializes a GPIO pin with the specified configuration.
 *
 * \n THIS FUNCTION HAS TO BE CALLED ONE TIME.\n
 * This function initializes a GPIO pin with the specified configuration. It validates the pin number,
 * initializes the pin, allocates memory for the GPIO instance, and configures the pin according to the provided configuration.
 *
 * @param pp_gpio Pointer to a pointer where the allocated GPIO structure will be stored.
 * @param p_conf Pointer to the GPIO configuration structure. This structure should be properly initialized with the desired pin configuration.
 *
 * @return result_t Returns RESULT_OK if the initialization is successful, otherwise returns an error code.
 *
 * @note The pin number must be valid and within the range defined by MAX_PIN_NUMBER.
 *       If the pin configuration fails at any step, the function will return an error.
 *
 * Example usage:
 * @code
 * hal_mcu_gpio_t * p_gpio;
 * hal_mcu_gpio_conf_t config;
 * config.pin = 5;
 * config.direction = HAL_MCU_GPIO_DIR_INPUT;
 * config.dir_conf.input.pull = HAL_MCU_GPIO_PULL_UP;
 * config.dir_conf.input.trigger = HAL_MCU_GPIO_TRIGGER_BOTH;
 *
 * result_t result = hal_ll_mcu_gpio_init(&my_gpio, &gpio_conf);
 *
 * @endcode
 */
extern result_t hal_mcu_gpio_init( hal_mcu_gpio_t ** pp_gpio, const hal_mcu_gpio_conf_t * p_conf );
/**
 * @brief Configures a GPIO pin with the specified configuration and returns the result.
 *
 * This function attempts to configure a GPIO pin using the specified configuration.
 * If the configuration fails, it returns an error result.
 *
 * @param pp_gpio Pointer to the GPIO structure to be configured.
 * @param p_conf Pointer to the GPIO configuration structure. This structure should be properly initialized with the desired pin configuration.
 *
 * @note This function should be called if we want to reconfigure a gpio after its initialization.
 */
extern result_t hal_mcu_gpio_config( hal_mcu_gpio_t * p_gpio, const hal_mcu_gpio_conf_t * p_conf );

extern result_t hal_mcu_gpio_out( hal_mcu_gpio_t * p_gpio, bool_t state );

/**
 * @brief Reads the state of a GPIO pin.
 *
 * This function reads the current state of a specified GPIO pin and stores the state in the provided variable.
 * The state is returned as a boolean value, where true indicates a high state and false indicates a low state.
 *
 * @param p_gpio Pointer to the GPIO structure. This structure should be properly initialized and configured.
 * @param p_state Pointer to a boolean variable where the state of the pin will be stored.
 *               true indicates the pin is high (1), and false indicates the pin is low (0).
 *
 * @return result_t Returns RESULT_OK if the read operation is successful.
 */
extern result_t hal_mcu_gpio_in( hal_mcu_gpio_t * p_gpio, bool_t * p_state );

/**
 *
 * @brief Registers an interrupt handler for a GPIO pin.
 *
 * This function registers an interrupt handler for a specified GPIO pin.
 * The GPIO pin must be configured as an input and must have a valid trigger condition.
 * The handler will be called when the specified trigger condition is met.
 *
 * @param p_gpio Pointer to the GPIO structure. This structure should be properly initialized and configured.
 * @param handler The function to be called when the interrupt is triggered. This function should match the signature defined by hal_mcu_gpio_handler_t.
 * @param p_instance Pointer to a user-defined instance or context to be passed to the handler function.
 *
 * @return result_t Returns RESULT_OK if the registration is successful, otherwise returns RESULT_ERR.
 *
 * Example usage:
 * @code
 * void callback( void * p_instance)
 * {
 *   hal_mcu_gpio_t * p_gpio = static_cast<hal_mcu_gpio_t *>(p_instance);
 *   bool_t state = false;
 *   hal_mcu_gpio_in(p_gpio, &state);
 *   DBG_PRINTF_TRACE("FALLING 0x%x %i \r\n",p_instance, state);
 * }
 * void setup()
 * {
 *   hal_mcu_gpio_t * p_gpio;
 *   hal_mcu_gpio_conf_t config;
 *   config.pin = 2; // Example pin number
 *   config.direction = HAL_MCU_GPIO_DIR_INPUT;
 *   config.dir_conf.input.pull = HAL_MCU_GPIO_PULL_UP;
 *   config.dir_conf.input.trigger = HAL_MCU_GPIO_TRIGGER_BOTH;
 *
 *   hal_mcu_gpio_init(&p_gpio,&config);
 *   hal_mcu_gpio_irq_register(p_gpio,callback,p_gpio);
 * }
 * @endcode
 */
extern result_t hal_mcu_gpio_irq_register( hal_mcu_gpio_t * p_gpio, hal_mcu_gpio_handler_t handler, void * p_instance );

/**
 * @brief Enables the interrupt for a specified GPIO pin.
 *
 * This function enables the interrupt for a specified GPIO pin. The GPIO pin
 * must be configured as an input and must have a valid trigger condition.
 *
 * @param p_gpio Pointer to the GPIO structure. This structure should be properly initialized and configured.
 *
 * @note The GPIO pin must be configured with a valid trigger condition (e.g., rising edge, falling edge).
 *       If the GPIO is not configured to trigger IRQ or is an output pin, the function will assert.
 *
 * Example usage:
 * @code
 * hal_mcu_gpio_t my_gpio;
 * my_gpio.dir = HAL_MCU_GPIO_DIR_INPUT;
 * my_gpio.dir_spec.input.trigger_def->trigger = HAL_MCU_GPIO_TRIGGER_RISING; // Example trigger condition
 * my_gpio.pin = 25; // Example pin number
 *
 * hal_ll_mcu_gpio_irq_enable(&my_gpio);
 * @endcode
 */
extern void hal_mcu_gpio_irq_enable( hal_mcu_gpio_t * p_gpio );

extern void hal_mcu_gpio_irq_disable( hal_mcu_gpio_t * p_gpio );

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MCU_GPIO_H_ */
