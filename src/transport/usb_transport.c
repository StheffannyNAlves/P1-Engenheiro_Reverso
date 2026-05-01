#include "transport/usb_transport.h"
#include "tusb.h"

void transport_usb_init(void) { tusb_init(); }

void transport_usb_task(void) { tud_task(); }

void transport_process_commands(
    void) { // It only executes when the probe is ready to "listen," that is, in
            // the IDLE state
  if (tud_cdc_available() >= EXPECTED_LEN) {
    uint8_t buffer[EXPECTED_LEN];
    uint32_t count = tud_cdc_read(buffer, EXPECTED_LEN);

    if (count == EXPECTED_LEN && buffer[0] == PACKET_MAGIC) {
      uint8_t checksum = buffer[0] ^ buffer[1] ^ buffer[2];

      if (checksum == buffer[3]) {
        if (buffer[1] == CMD_PING) {
          tud_cdc_write_str("C000\r\n"); // Communication channel ready
        } else {
          tud_cdc_write_str("C003\r\n"); // Command not supported
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
  }
}
