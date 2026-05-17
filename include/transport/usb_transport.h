#ifndef USB_TRANSPORT_H
#define USB_TRANSPORT_H // Guards

#include <stdint.h>

#define PACKET_MAGIC 0xAA
#define EXPECTED_LEN 4

void transport_usb_init(void);
void transport_usb_task(void);
void transport_send_event(const uint8_t *buffer, uint32_t tamanho);
void transport_set_watchdog_flag(void);

typedef enum {
    CMD_NONE = 0x00,
    CMD_PING = 0x01,
    CMD_HOLD = 0x02,
    CMD_RELEASE = 0x03,
    CMD_START_SESSION = 0x04,
    CMD_STATUS = 0x05,
} transport_command_t;

transport_command_t transport_process_command(void);

#endif // USB_TRANSPORT_H