// Initial firmware
#include "bsp/board.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "target/target_ctrl.h"
#include "transport/usb_transport.h"
#include <stdbool.h>
#include <stdint.h>

// Initial States
typedef enum { BOOT, PLATAFORM_INIT, IDLE, ERROR, WATCHDOG_RESET } fsm_state;

fsm_state current_state = BOOT;

void fsm_run() {
    switch (current_state) {
    case BOOT:
        if (watchdog_caused_reboot()) {
            current_state = WATCHDOG_RESET;
        } else {
            current_state = PLATAFORM_INIT;
        }
        break;

    case WATCHDOG_RESET:
        // The event is logged, and normal initialization follows.
        transport_set_watchdog_flag();
        current_state = PLATAFORM_INIT;
        break;

    case PLATAFORM_INIT:
        board_init();
        transport_usb_init();
        if (!watchdog_caused_reboot()) { // Enables watchdog timer only during boot
                                         // normal
            watchdog_enable(8000, true);
        }
        target_ctrl_init();
        target_reset_high(); // Guaranteed target released.
        current_state = IDLE;
        break;

    case IDLE:
        transport_process_commands();
        break;
    case ERROR:
    default:
        target_reset_high();
        while (1) {
            transport_usb_task();
        }

        break;
    }
}

int main() {
    while (1) {
        fsm_run();
        transport_usb_task();
        /* if (current_state != ERROR){
          watchdog_update();
        }*/
    }

    return 0;
}