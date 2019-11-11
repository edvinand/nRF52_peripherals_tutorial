# nRF52 Peripherals Tutorial
### Brief:
Several small tutorials/exercises that shows you how to:
- Create an Application Timer to toggle a GPIO pin
- Configure buttons to toggle a GPIO pin
- Generate a PWM pulse that is used to control an analog servo
- Send serial data to and from a terminal window
- Measure the die temperature
- Automate peripheral using the tasks and event system
-Configure a timer to toggle a gpio pin to interact autonomously with each other using tasks and events independent of the CPU

### Requirements
- nRF52 DK
- SDK v15.3.0
- Template project found in nRF5_SDK_15.3.0_59ac345\examples\peripheral\template_project

# Tasks
In all the tasks we'll be using the SDK drivers or libraries for the peripherals, i.e. nrf_drv_xxx.c, which can be found in nRF5_SDK_14.1.0_1dda907\components\drivers_nrf\ and nRF5_SDK_14.1.0_1dda907\components\libraries respectively.
Some of the drivers are found in SDK\modules\nrfx\drivers\src

### Warm-up
The template project includes all the peripheral libraries and drivers from the SDK, but we're only going to use a few, so to reduce the compile time and size of our project, we'll temporarily remove them from the project. 
1. Find the template_project from nRF5_SDK_15.3.0_59ac345\examples\peripheral\template_project
2. Create a copy of the template_project folder and rename it to nRF52_peripherals_tutorial
3. Open the template_pca10040.emProject Segger Embedded Studio project found in nRF52_peripherals_tutorial\pca10040\blank\ses
4. Search for micro-ecc in the sdk_config.h file, and make sure all the defines containing this string is defined to 0.
5. Remove the library (.a) file from the nRF_micro-ecc folder in the project explorer view on the left hand side. 
6. Now the project should compile.

### 1 - Blink an LED using a busy-wait loop
Goal: Blink an LED by keeping the CPU in a busy-wait loop.
1. Include the following headers in main.c
```C
#include "nrf_delay.h"
#include "nrf_gpio.h"
```

2. Use the nrf_gpio_cfg_output() function to configure one of the pins connected to one of the LEDs of the nRF52 DK as an output. Hint: See the back of the nRF52 DK for the pin assignments.
3. Use nrf_delay_ms() and nrf_gpio_pin_toggle() to blink an LED within the while-loop in main().

### 2 - Application Timer
Blinking an LED with a busy-wait loop is not very efficient, as you'll keep the CPU running without actually doing anything useful. A much better approach would be to set up a timer to toggle the LED at a given interval so that the CPU can do meaningful tasks or sleep in between the timer interrupts.</br>
The Application Timer library provides a user friendly way of using the Real Time Counter 1 (RTC1) peripheral to create multiple timer instances. The RTC uses the Low Frequency Clock (LFCLK). Most applications keep the LFCLK active at all times and when using one of the Nordic SoftDevices the LFCLK is always active. Therefore, there is normally very little extra power consumption associated with using the application timer. As the clock is 32.768 kHz and the RTC is 24 bit, the time/tick resolution is limited, but it takes a substantial amount of time before the counter wrap around from (0xFFFFFF to 0x000000). By using the 12 bit (1/X) prescaler the frequency of the RTC can be lowered if needed.</br>
The Application Timer library API is documented [here](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__timer.html)</br>
1. As mentioned in the introduction of this task, the application timer uses the RTX peripheral, which in turn uses the 32kHz LFCLK. Hence, we'll need to start the LFCLK for the application timer to function properly. Create a function called lfclk_init() where you add the following snippet.
```C
NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
NRF_CLOCK->TASKS_LFCLKSTART    = 1;

while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
{
    // Do nothing.
}
```
2. Include the app_timer.h file in your main.c file.
3. Next, you'll need to create a application timer instance using the APP_TIMER_DEF macro.
4. Create the function application_timer_init(), in which you initialize the application timer library, create and start the application timer. Hints: - You will need to use the functions app_timer_init(), app_timer_create() and app_timer_start() - You want to create a repeating timer, i.e. the mode of the applicaiton timer should be set to APP_TIMER_MODE_REPEATED. - The APP_TIMER_TICKS macro is very useful when setting the timeout interval. - Make sure to call the application_timer_init() function in main().
5. Call nrf_gpio_toogle() function to toggle one of the nRF52 DKs LEDs in the timeout handler that you specified when you initialized the application timer.
6. Compile and flash the example to your nRF52 DK and verify that the LED is blinking. Since there are a couple of steps to do possible errors, here is a suggestion to what the functions can look like. You can get inspiration from these if you are stuck:
```C
APP_TIMER_DEF(m_my_timer_id);
#define TIMEOUT_INTERVAL                  APP_TIMER_TICKS(1000)
```

```C
static void application_timer_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_my_timer_id, APP_TIMER_MODE_REPEATED, my_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_my_timer_id, TIMEOUT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
```

```C
static void my_timeout_handler(void * p_context)
{
    nrf_gpio_pin_toggle(17);
}
```

```C
int main(void)
{
    nrf_gpio_cfg_output(17);

    lfclk_init();
    application_timer_init();

    while (true)
    {
        // Do nothing.
    }
}
```

### 3 - Buttons - Button Handler Library
The button handler library uses GPIOTE Handler to detect that a button has been pushed. To handle debouncing, it will start a timer in GPIOTE event handler. The button will only be reported as pushed if the corresponding pin is still active when the timer expires. If there is a new GPIOTE event while the timer is running, the timer is restarted. </br>
The Button Handler Library API is documented [here](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__button.html)</br>

1. Normally we would need to add the Button Handler library, app_button.c, under nRF Libraries in our project, but this has already been added in the template project. However, we need to include the app_button.h header at the top of main.c.
2. Next, create a static void function called buttons_init(), where you initialize the Button Handler library using app_button_init().

Hints:
- You will need to create a app_button_cfg_t struct for each button you configure. Make sure to declare it as a *static*.
- It is possible to configure a separate event handler for each individual button, but in this exercise we will use one event handler for all the buttons.
- The button pin number as well as the active state of the buttons can be seen on the backside of the nRF52 DK.
- After initializing the Button Handler library with button configuration you will need to enable it. There should be an appropriate function int eh app_button API.

3. In the event handler that you set in the button configuration structure you will have to check which pin as well as which action that generated the event. Add code to the event handler so that one of the LEDs of the nRF52 DK is toggled when you push one of the buttons on the nRF52 DK. Hint:
- There are two button action types, APP_BUTTON_PUSH and APP_BUTTON_RELEASE.
- You can see which pins that are connected to the different buttons on the back of the nRF52 DK.
```C
void my_button_handler(uint8_t pin_no, uint8_t button_action)
{
    // Check which pin that generated the event as well as which type of button action that caused the event.
}
4. Compile and flash the project to your nRF52 DK and verify that the LED is toggling when you push the button.
```
### 4 - Servo, Controlling a servo using the PWM library
In this task we will use [Pulse-Width Modulation](https://learn.sparkfun.com/tutorials/pulse-width-modulation) to control an analog servo. The PWM library uses one of the nRF52s TIMER peripheral in addition to the PPI and GPIOTE peripherals. The app_pwm library is documented on [this]() Infocenter page.</br>
Connecting the Servo to your nRF52 DK:
The three wires coming from the SG90/SG92R Servo are:
- Brown: Ground - Should be connected to one of the pins marked GND on your nRF52 DK.
- Red: 5V Should be connected to the pin marked 5V on your nRF52 DK.
- Orange: PWM Control Signal - Should be connected to one of the unused GPIO pins of the nRF52 DK (for example P0.04, pin number 4). 
1. The first thing we have to do is to include the header to the PWM library, `app_pwm.h` and create a PWM instance with the [APP_PWM_INSTANCE](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__pwm.html&anchor=gaf01d3e06e17705a7453d91c70d40098f) macro that uses the TIMER2 peripheral.
2. The second thing we need to do is to create the function `pwm_init()` where we configure, initialize and enable the PWM peripheral. You configure the PWM library by creating an [app_pwm_config_t](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fstructapp__pwm__config__t.html) and pass it as a parameter to where the following parameters must be specified:
- *pins:* Array of two unsigned integers that indicate which physical pins will be used for the PWM output. In one-channel mode, the second element is ignored.
- *pin_polarity:* 2-element array of app_pwm_polarity_t that indicates the output signal polarity. In one-channel mode, the second element is ignored. 
- *num_of_channels:* Number of PWM channels (1 or 2). 
- *period_us:* Signal period (in microseconds).

Hints: 
- The polarity can be set to either `APP_PWM_POLARITY_ACTIVE_HIGH` or `APP_PWM_POLARITY_ACTIVE_LOW`. 
- We only need one channel.
- The second element of the pins array should be set to `APP_PWM_NOPIN`.
- The period of the PWM pulse should be 20ms.
3. The struct must be passed as an input to the [app_pwm_init](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__pwm.html&anchor=gae3b3e1d5404fd776bbf7bf22224b4b0d) functino which initializes the PWM library. After initializing the PWM library you have to enable the PWM instance by calling [app_pwm_enable](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__pwm.html&anchor=ga94f5d824afec86aff163f7cccedaa436).
Hints:
- We do not need to provide an event handler function, i.e. you can pass NULL instead of a function pointer. 
- Make sure that you add `pwm_init()`to the `main()`function before the while loop.
4. Now that we have initialized the PWM library you can set the duty cycle of the PWM signal to the servo using the [app_pwm_channel_duty_set](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__pwm.html&anchor=ga071ee86851d8c0845f297df5d23a240d) function. This will set the duty cycle of the PWM signal, i.e. the percentage of the total time the signal is high or low depending on the polarity that has been chosen. If we want to set the PWM signal to be high 50% of the time, then we call `app_pwm_channel_duty_set` with the following parameters:
```C
while (app_pwm_channel_duty_set(&PWM, 0, 50) == NRF_ERROR_BUSY);
```
Make the servo sweep from its maximum angle to its minimum angle. This can be done by calling `app_pwm_channel_duty_set()` twice with a delay between the two calls in the main while-loop, as shown below.
```C
    while (true)
    {
        while (app_pwm_channel_duty_set(&PWM, 0, 0) == NRF_ERROR_BUSY);
        nrf_delay_ms(1000);
        while (app_pwm_channel_duty_set(&PWM, 0, 0) == NRF_ERROR_BUSY);
        nrf_delay_ms(1000);
    }
```

The code snippet above sets the duty cycle to 0, you have to figure out the correct duty cycle values for the min and max angle.

5. Modify the button handler from Task 3 so that you can set the servo to its minimum and maximum angle by pressing the buttons on the nRF52 DK.

### 5 - UART.

