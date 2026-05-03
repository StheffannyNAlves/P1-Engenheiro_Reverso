#ifndef USB_TRANSPORT_H
#define USB_TRANSPORT_H // Guards

#define PACKET_MAGIC 0xAA
#define CMD_PING 0x01
#define CMD_HOLD 0x02
#define CMD_RELEASE 0x03
#define EXPECTED_LEN 4

void transport_usb_init(void);
void transport_usb_task(void);
void transport_process_commands(void);

#endif // USB_TRANSPORT_H