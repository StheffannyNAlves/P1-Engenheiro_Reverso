#ifndef USB_TRANSPORT_H
#define USB_TRANSPORT_H // Guards

#define PACKET_MAGIC 0xAA
#define CMD_PING     0x01
#define EXPECTED_LEN 4

void transport_usb_init(void);
void transport_usb_task(void);
void transport_process_commands(void);

#endif // USB_TRANSPORT_H