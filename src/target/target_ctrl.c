// Core 0
// Responsible for providing visual feedback on the target's status.
#include "target/target_ctrl.h"
#include "hardware/gpio.h"
#include "platform/board_config.h"

#define SIO_BASED 0xD0000000u
#define GPIO_OUT_SET *(volatile uint32_t *)(SIO_BASED + 0x014)
#define GPIO_OE_SET *(volatile uint32_t *)(SIO_BASED + 0x024)
#define GPIO_OUT_CLR *(volatile uint32_t *)(SIO_BASED + 0x018)

void target_ctrl_init(void) {
    gpio_init(PIN_RUN);
    gpio_init(PIN_RST);
    gpio_init(PIN_PROBE_LED);

    GPIO_OE_SET = (1 << PIN_RUN | 1 << PIN_PROBE_LED | 1 << PIN_RST);
    GPIO_OUT_CLR = (1 << PIN_RUN);
    GPIO_OUT_SET = (1 << PIN_RST);
    GPIO_OUT_CLR = (1 << PIN_PROBE_LED);
}

void target_reset_low(void) {
    GPIO_OUT_SET = (1 << PIN_RST);
    GPIO_OUT_CLR = (1 << PIN_PROBE_LED);
}

void target_reset_high(void) {
    GPIO_OUT_CLR = (1 << PIN_RST);
    GPIO_OUT_SET = (1 << PIN_PROBE_LED);
}