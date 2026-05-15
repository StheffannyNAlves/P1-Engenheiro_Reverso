#include "platform/platform_init.h"
#include "platform/board_config.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

static void _configure_run_pin(void)
{
    gpio_init(PIN_TARGET_RUN);
    gpio_set_dir(PIN_TARGET_RUN, GPIO_OUT);
    gpio_put(PIN_TARGET_RUN, TARGET_IN_RESET);
}

static void _configure_swd_pins_passive(void) // Passive state means "the pin exists but the probe is not driving the bus yet."
{    gpio_init(PIN_SWCLK);                    // Passive during initial testing.
    gpio_set_dir(PIN_SWCLK, GPIO_OUT);
    gpio_put(PIN_SWCLK, 0);

    gpio_init(PIN_SWDIO);
    gpio_set_dir(PIN_SWDIO, GPIO_IN);
    gpio_pull_up(PIN_SWDIO);
}

static void _configure_led(void)
{
    gpio_init(PIN_PROBE_LED);
    gpio_set_dir(PIN_PROBE_LED, GPIO_OUT);
    gpio_put(PIN_PROBE_LED, LED_OFF);
}

static void _configure_watchdog(void){
    watchdog_enable(1000, true);

}


bool platform_init(void)
{

    stdio_init_all();

    _configure_led();
    _configure_run_pin();
    // _configure_swd_pins_passive();

    sleep_ms(10);

    return true;
}

void platform_led_on(void)
{
    gpio_put(PIN_PROBE_LED, 1);
}

void platform_led_off(void)
{
    gpio_put(PIN_PROBE_LED, 0);
}

void platform_led_blink(uint8_t n, uint32_t period_ms)
{
    for (uint8_t i = 0; i < n; i++) {
        gpio_put(PIN_PROBE_LED, 1);
        sleep_ms(period_ms / 2);
        gpio_put(PIN_PROBE_LED, 0);
        sleep_ms(period_ms / 2);
    }
}