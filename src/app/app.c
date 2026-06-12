#include "app/app.h"
#include "hardware/gpio.h"
#include "platform/board_config.h"
#include "swd/swd_phy.h"
#include "swd/swd_proto.h"
#include "transport/usb_transport.h"

#include <stdio.h>
#include <string.h>

dolos_fault_t app_enter_swd(void) {

    gpio_init(2);
    gpio_init(3);

    gpio_set_dir(2, GPIO_OUT);
    gpio_set_dir(3, GPIO_OUT);
    gpio_put(2, 1); // SWCLK high
    gpio_put(3, 1); // SWDIO high

    swd_phy_init(); // Ensure physical layer is initialized before entering SWD mode
    // Enter SWD mode: line_reset + 0xE79E + line_reset + idle
    // Returns: error code or success
    swd_enter_swd_mode();

    return (dolos_fault_t){.code = ERR_NONE, .is_fatal = false};
}

dolos_fault_t app_read_idcode(uint32_t *idcode) {
    // Read and validate target IDCODE
    // Send result to host via USB
    // Returns: error code or success
    uint32_t read_idcode = swd_read_idcode();

    // Validate read IDCODE (S004 - Invalid IDCODE)
    if (read_idcode == 0xFFFFFFFF || read_idcode != 0x0BC12477) {
        transport_send_event((const uint8_t *)"S004\r\n", 6);
        return (dolos_fault_t){.code = S004, .is_fatal = true};
    }

    // Return value via pointer to caller
    *idcode = read_idcode;

    // Send result to host via USB in hexadecimal format
    char idcode_msg[32];
    snprintf(idcode_msg, sizeof(idcode_msg), "IDCODE:%08lX\r\n", read_idcode);
    transport_send_event((const uint8_t *)idcode_msg, strlen(idcode_msg));

    return (dolos_fault_t){.code = ERR_NONE, .is_fatal = false};
}