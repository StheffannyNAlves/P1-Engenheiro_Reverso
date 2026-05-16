#define SWD_PROTO_C
#include "swd/swd_proto.h"
#include "swd/swd_phy.h"
#include <stdint.h>
#define JTAG_TO_SWD_SEQ 0xE79E

void swd_enter_swd_mode(void) {
    line_reset();

    uint16_t sequence = JTAG_TO_SWD_SEQ;
    for (int i = 0; i < 16; i++) {
        uint8_t bit = (sequence >> i) & 1;

        writebit(bit);
    }

    line_reset();

    writebit(0); // Idle cycle 1
    writebit(0); // Idle cycle 2
}