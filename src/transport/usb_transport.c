#include "transport/usb_transport.h"
#include "target/target_ctrl.h"
#include "tusb.h"

void transport_usb_init(void) { tusb_init(); }

void transport_usb_task(void) { tud_task(); }

void transport_send_event(const uint8_t *buffer, uint32_t tamanho) {
    tud_cdc_write(buffer, tamanho);
    tud_cdc_write_flush();
    transport_usb_task();
}

static bool watchdog_reset_flag = false;

void transport_set_watchdog_flag(void) { watchdog_reset_flag = true; }
void transport_process_commands(void) { // It only executes when the probe is ready to "listen,"
                                        // that is, in the IDLE state
    if (tud_cdc_available() >= EXPECTED_LEN) {
        uint8_t buffer[EXPECTED_LEN];
        uint32_t count = tud_cdc_read(buffer, EXPECTED_LEN);

        if (count == EXPECTED_LEN && buffer[0] == PACKET_MAGIC) {
            uint8_t checksum = buffer[0] ^ buffer[1] ^ buffer[2];

            if (checksum == buffer[3]) {
                switch (buffer[1]) {
                case CMD_PING:
                    tud_cdc_write_str("C000\r\n"); // Communication channel ready
                    if (watchdog_reset_flag) {
                        transport_send_event((const uint8_t *)"F003\r\n", 6);
                        watchdog_reset_flag = false;
                    }

                    break;

                case CMD_HOLD:
                    target_reset_low();
                    tud_cdc_write_str("C000\r\n");
                    break;

                case CMD_RELEASE:
                    target_reset_high();
                    tud_cdc_write_str("C000\r\n");
                    break;

                default:
                    tud_cdc_write_str("C003\r\n"); // Command not supported
                    break;
                }
            }

            else {
                tud_cdc_write_str("C004\r\n"); // Invalid packet checksum
                tud_cdc_read_flush();
            }
        } else {
            tud_cdc_write_str("C001\r\n"); // Invalid command
            tud_cdc_read_flush();
        }

        tud_cdc_write_flush(); // Ensures the response is sent to the host correctly
        transport_usb_task();
    }
}
