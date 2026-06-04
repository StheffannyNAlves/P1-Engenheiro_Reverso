#define SWD_PROTO_C
#include "swd/swd_proto.h"
#include "swd/swd_phy.h"
#include <stdint.h>
#include <stdio.h>
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

static uint8_t calculate_parity(uint32_t apndp, uint8_t rnw, uint8_t a2, uint8_t a3) {
    uint8_t count = (apndp & 1) + (rnw & 1) + (a2 & 1) + (a3 & 1);
    return (count % 2) != 0 ? 1 : 0; // Return 1 for odd parity, 0 for even parity
}

static void send_request(uint8_t apndp, uint8_t rnw, uint8_t a2, uint8_t a3) {
    uint8_t request_packet = 0;

    uint8_t parity = calculate_parity(apndp, rnw, a2, a3);

    // Build the byte by packing each bit into its exact position (LSB-first)
    request_packet |= (1 << 0);      // bit 0: Start (sempre 1)
    request_packet |= (apndp << 1);  // bit 1: APnDP (0 para DP, 1 para AP)
    request_packet |= (rnw << 2);    // bit 2: RnW (0 para Escrita, 1 para Leitura)
    request_packet |= (a2 << 3);     // bit 3: A[2]
    request_packet |= (a3 << 4);     // bit 4: A[3]
    request_packet |= (parity << 5); // bit 5: Parity calculada sobre os bits 1-4
    request_packet |= (0 << 6);      // bit 6: Stop (sempre 0)
    request_packet |= (1 << 7);      // bit 7: Park (sempre 1)

    for (int i = 0; i < 8; i++) {
        uint8_t bit = (request_packet >> i) & 1;
        writebit(bit);
    }
}

static uint32_t read_data(void) {
    uint32_t data = 0;
    for (int i = 0; i < 32; i++) {
        data |= (readbit() << i);
    }
    return data;
}

static uint8_t calculate_data_parity(uint32_t data) {
    uint8_t parity = 0;
    for (int i = 0; i < 32; i++) {
        parity ^= (data >> i) & 1;
    }
    return parity;
}

// 1. writebit por writebit, envia 0xA5 (8 bits, LSB first)
// 2. turnaround_host_to_target()
// 3. lê 3 bits de ACK com readbit()
// 4. se ACK == OK (0b001):
//        lê 32 bits de data com readbit()
//        lê 1 bit de paridade com readbit()
//        turnaround_target_to_host()
// 5. retorna o valor de 32 bits lido
uint32_t swd_read(uint8_t apndp, uint8_t addr, uint8_t rnw) {
    send_request(apndp, rnw, (addr >> 2) & 1, (addr >> 3) & 1); // RnW = 1 for read

    // Turnaround: Host to Target
    turnaround_host_to_target();

    // Read ACK
    uint8_t ack = 0;
    for (int i = 0; i < 3; i++) {
        ack |= (readbit() << i);
    }

    if (ack == 0b001) { // ACK OK
        uint32_t data = read_data();
        uint8_t parity = readbit();

        // Turnaround: Target to Host
        turnaround_target_to_host();

        if (parity == calculate_data_parity(data)) {
            printf("C000\n");
            printf("IDCODE: 0x%08X\n", (unsigned int)data);
            return data; // Return the read data
        } else {
            printf("S003\n");  // Parity error
            return 0xFFFFFFFF; // Indicate error with a special value
        }
    } else {
        turnaround_target_to_host(); // Ensure we return to host mode even on ACK error
        printf("S004\n");            // ACK error
        return 0xFFFFFFFF;           // Indicate error with a special value
    }
}

uint32_t swd_read_idcode(void) {
    return swd_read(0, 0, 1); // APnDP = 0 (DP), addr = 0 (IDCODE), RnW = 1 (read)
}