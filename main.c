/**
 * Copyright (c) 2009 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
* @brief Example template project.
* @defgroup nrf_templates_example Example Template
*
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_pwm.h"
#include "app_uart.h"

#define UART_TX_BUF_SIZE                256     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256     /**< UART RX buffer size. */

#define TIMEOUT_INTERVAL                APP_TIMER_TICKS(1000)
APP_TIMER_DEF(my_timer_id);
APP_PWM_INSTANCE(PWM, 2);

volatile app_pwm_duty_t pwm_duty_cycle = 0;


static void gpio_leds_init(void)
{
    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_cfg_output(LED_2);
    nrf_gpio_cfg_output(LED_3);
    //nrf_gpio_cfg_output(20);

    nrf_gpio_pin_set(LED_1);     // when the pin is high, the LED is off.
    nrf_gpio_pin_set(LED_2);     // when the pin is high, the LED is off.
    nrf_gpio_pin_set(LED_3);     // when the pin is high, the LED is off.
    //nrf_gpio_pin_set(20);     // when the pin is high, the LED is off.
}

static void toggle_gpio(uint32_t pin_no)
{
    nrf_gpio_pin_toggle(pin_no);
    nrf_delay_ms(1000);
}

static void lfclk_init(void)
{
    NRF_CLOCK->LFCLKSRC             = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED  = 0;
    NRF_CLOCK->TASKS_LFCLKSTART     = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing
    }
}

void my_timer_timeout_handler(void * p_context)
{
    nrf_gpio_pin_toggle(LED_1);
//    while (app_pwm_channel_duty_set(&PWM, 0, 5) == NRF_ERROR_BUSY)
//    {
//        // Do nothing
//    }
//    while (app_pwm_channel_duty_set(&PWM, 1, pwm_duty_cycle) == NRF_ERROR_BUSY)
//    {
//        // Do nothing
//    }
//    pwm_duty_cycle += 1;
//    if (pwm_duty_cycle > 10)
//    {
//        pwm_duty_cycle = 5;
//    }
}

static void application_timer_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&my_timer_id, APP_TIMER_MODE_REPEATED, my_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(my_timer_id, TIMEOUT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void my_button_handler(uint8_t pin_no, uint8_t button_action)
{
    //nrf_gpio_pin_toggle(18);
    if (button_action == APP_BUTTON_PUSH)
    {
        if (pin_no == BUTTON_1)
        {
            nrf_gpio_pin_clear(LED_2);
            while (app_pwm_channel_duty_set(&PWM, 0, 12) == NRF_ERROR_BUSY)
            {
                // Do nothing
            }
        }
        else if (pin_no == BUTTON_2)
        {
            nrf_gpio_pin_set(LED_2);
            while (app_pwm_channel_duty_set(&PWM, 0, 4) == NRF_ERROR_BUSY)
            {
                // Do nothing
            }
        }
    }
}

static void buttons_init(void)
{
    ret_code_t err_code;

    static app_button_cfg_t button_cfg[2] = {
        {BUTTON_1,APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,my_button_handler},
        {BUTTON_2,APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,my_button_handler}
    };
    
    err_code = app_button_init(button_cfg, 2, APP_TIMER_TICKS(50));
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

void pwm_callback_handler(uint32_t pwm_instance_id)
{
    // We don't need anything in this callback handler
}

static void pwm_init(void)
{
    ret_code_t err_code;

    app_pwm_config_t pwm_cfg = 
    {
        .num_of_channels = 1,
        .period_us = 20000L,
        .pins = {4, 20},
        .pin_polarity = {APP_PWM_POLARITY_ACTIVE_HIGH, APP_PWM_POLARITY_ACTIVE_LOW}
    };
    
    // Alternatively we could have used:
    //app_pwm_config_t pwm_cfg = APP_PWM_DEFAULT_CONFIG_1CH(20000L, 4);
    //pwm_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    err_code = app_pwm_init(&PWM, &pwm_cfg, pwm_callback_handler);
    //NRF_P0->PIN_CNF[4] = (NRF_P0->PIN_CNF[4] | 0x00000700);
    APP_ERROR_CHECK(err_code);

    app_pwm_enable(&PWM);
    while (app_pwm_channel_duty_set(&PWM, 0, 0) == NRF_ERROR_BUSY)
    {
        // Do nothing
    }
}

static void uart_print(uint8_t data_string[])
{
    for (uint32_t i=0; i< strlen((const char *)data_string); i++)
    {
        while (app_uart_put(data_string[i]) != NRF_SUCCESS);
    }
}

void uart_event_handler(app_uart_evt_t * p_event)
{
    static uint8_t data_array[32];
    static uint8_t index = 0;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            /* 
            The received data is stored in a receive buffer and can be retreived using app_uart_get.
            Data to be sent can be placed in a transmit buffer using app_uart_put.
            */
            app_uart_get(&data_array[index]);
            index++;

            if (data_array[index - 1] == '\n') 
            {
                uart_print(data_array);
                memset(data_array,0,sizeof(data_array));
                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

static void uart_init(void)
{
    ret_code_t err_code;
    const app_uart_comm_params_t uart_comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&uart_comm_params,
                      UART_RX_BUF_SIZE,
                      UART_TX_BUF_SIZE,
                      uart_event_handler,
                      APP_IRQ_PRIORITY_LOWEST,
                      err_code);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    lfclk_init();
    application_timer_init();
    buttons_init();
    gpio_leds_init();
    pwm_init();

    uart_init();
    
    while (true)
    {
        //toggle_gpio(17);  // Removed because we don't want to use nrf_delay_ms() in the final application. Replaced by app_timer.
    }
}
/** @} */
