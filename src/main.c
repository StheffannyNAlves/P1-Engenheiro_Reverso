// Initial firmware
#include "bsp/board.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "swd/swd_proto.h"
#include "target/target_ctrl.h"
#include "transport/usb_transport.h"
#include <stdbool.h>
#include <stdint.h>

// Initial States
typedef enum {
    BOOT,
    PLATFORM_INIT,
    TARGET_HOLD_RESET,
    ENTER_SWD,
    READ_IDCODE,
    IDLE,
    ERROR,
    WATCHDOG_RESET
} fsm_state;

fsm_state current_state = BOOT;

void fsm_run() {
    switch (current_state) {
    case BOOT:
        if (watchdog_caused_reboot()) {
            current_state = WATCHDOG_RESET;
        } else {
            current_state = PLATFORM_INIT;
        }
        break;

    case TARGET_HOLD_RESET:
        target_reset_low(); // Forces target into reset via RUN pin
        current_state = ENTER_SWD;
        break;

    case ENTER_SWD:
        // SWD protocol is activated while target is in reset, ensuring it enters debug mode
        // correctly.
        swd_enter_swd_mode();
        current_state = READ_IDCODE;
        break;

    case READ_IDCODE: {
        uint32_t idcode = swd_read_idcode();
        if (idcode != 0x0BC12477) { // Check if IDCODE matches expected value for RP2040
            current_state = ERROR;  // If IDCODE differs, something went wrong, and system enters
                                    // error state.
        } else {
            current_state = IDLE; // If IDCODE is correct, system is ready to process commands.
        }
        break;
    }

    case PLATFORM_INIT:
        board_init();
        transport_usb_init();
        if (!watchdog_caused_reboot()) { // Enables watchdog timer only during normal boot
            watchdog_enable(8000, true);
        }
        target_ctrl_init();
        target_reset_high(); // Guaranteed target released.
        current_state = IDLE;
        break;

    case IDLE: {
        transport_command_t cmd = transport_process_command();
        if (cmd == CMD_START_SESSION) {
            current_state =
                TARGET_HOLD_RESET; // Transition to hold reset state to start the session
        }
        break;
    }

    case WATCHDOG_RESET:
        // The event is logged, and normal initialization follows.
        transport_set_watchdog_flag();
        current_state = PLATFORM_INIT;
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