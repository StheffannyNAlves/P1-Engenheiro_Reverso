   .syntax unified
   .cpu cortex-m0plus
   .thumb

    #include "platform/board_config.h"

   .global writebit
   .type writebit, %function

   .equ SIO_BASE,     0xD0000000
   .equ SIO_OUT_SET,  0x014
   .equ SIO_OUT_CLR,  0x018

writebit:
    PUSH {r4, r5, lr} // save registers

    LDR r1, =SIO_BASE

    MOVS r2, #(1 << PIN_SWCLK)
    MOVS r3, #(1 << PIN_SWDIO)

    // LOW half-cycle
    STR r2, [r1, #SIO_OUT_CLR] // Clear clock mask in register

    LSLS r4, r0, #PIN_SWDIO // If R0 was 1, R4 becomes 0x08. If was 0, R4 becomes 0x00
    SUBS r5, r3, r4 // If R4=0x08 -> R5 = 0x08 - 0x08 = 0x00
                    // If R4=0x00 -> R5 = 0x08 - 0x00 = 0x08

    STR r4, [r1, #SIO_OUT_SET] // Apply data
    STR r5, [r1, #SIO_OUT_CLR] // Clear data

    // Wait for the rest of LOW half-cycle time (62 cycles total)
    // Cycle calculation: 1(LSLS) + 1(SUBS) + 3(STR) = 5 cycles. Only instructions within the LOW half-cycle are counted
    // 62 - 5 = 57 NOPs required
    .rept 57
        NOP
    .endr


    // HIGH half-cycle
    STR r2, [r1, #SIO_OUT_SET]

    // Wait for HIGH half-cycle to finish and return
    // We spend 1 cycle on clock SET. The POP command spends 3 cycles to return
    // 62 - 1 = 61 NOPs required for correct timing
    .rept 61
        NOP
    .endr

    // Clear registers and jump back to caller
    POP {r4, r5, pc}