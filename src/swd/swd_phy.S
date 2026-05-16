.syntax unified
   .cpu cortex-m0plus
   .thumb

   .set DOLOS_PIN_SWCLK, 2
   .set DOLOS_PIN_SWDIO, 3

   .global swd_phy_init
   .type swd_phy_init, %function

   .global readbit
   .type readbit, %function
   .global writebit
   .type writebit, %function

   .global turnaround_host_to_target
   .type turnaround_host_to_target, %function

   .global turnaround_target_to_host
   .type turnaround_target_to_host, %function

   .equ DOLOS_SIO_BASE,         0xD0000000
   .equ DOLOS_SIO_GPIO_IN,      0x004   @ read current pin state
   .equ DOLOS_SIO_GPIO_OUT_SET, 0x014   @ force specific pins HIGH
   .equ DOLOS_SIO_GPIO_OUT_CLR, 0x018   @ force specific pins LOW
   .equ DOLOS_SIO_GPIO_OE_SET,  0x024   @ enable output
   .equ DOLOS_SIO_GPIO_OE_CLR,  0x028   @ disable output


swd_phy_init:
    PUSH {r4, r5, lr}

    LDR r1, =DOLOS_SIO_BASE
    MOVS r2, #(1 << DOLOS_PIN_SWCLK)

    MOVS r5, #0x24
    STR r2, [r1, r5]               @ SWCLK as output via GPIO_OE_SET

    MOVS r5, #0x18
    STR r2, [r1, r5]               @ Clear clock — SWCLK starts LOW

    POP {r4, r5, pc}
    .ltorg

writebit:
    PUSH {r4, r5, r6, lr}          @ save registers

    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWCLK)
    MOVS r3, #(1 << DOLOS_PIN_SWDIO)

    @ LOW half-cycle
    MOVS r6, #DOLOS_SIO_GPIO_OUT_CLR
    STR r2, [r1, r6]               @ SWCLK LOW

    LSLS r4, r0, #DOLOS_PIN_SWDIO  @ If R0=1, R4=0x08. If R0=0, R4=0x00
    SUBS r5, r3, r4                 @ If R4=0x08 -> R5=0x00 / If R4=0x00 -> R5=0x08

    MOVS r6, #DOLOS_SIO_GPIO_OUT_SET
    STR r4, [r1, r6]               @ Apply data

    MOVS r6, #DOLOS_SIO_GPIO_OUT_CLR
    STR r5, [r1, r6]               @ Clear data

    @ Cycle calculation: 1(LSLS) + 1(SUBS) + 3(STR) + 6(MOVS) = 11 cycles
    @ 62 - 11 = 51 NOPs required
    .rept 51
        NOP
    .endr

    @ HIGH half-cycle
    MOVS r6, #DOLOS_SIO_GPIO_OUT_SET
    STR r2, [r1, r6]               @ SWCLK HIGH

    @ 62 - 2 = 60 NOPs required for correct timing
    .rept 60
        NOP
    .endr

    POP {r4, r5, r6, pc}           @ Clear registers and jump back to caller
    .ltorg

readbit:
    PUSH {r4, r5, lr}

    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWCLK)
    MOVS r3, #(1 << DOLOS_PIN_SWDIO)

    @ LOW half-cycle
    MOVS r5, #DOLOS_SIO_GPIO_OE_CLR
    STR r3, [r1, r5]               @ SWDIO becomes input -> GPIO_OE_CLR with SWDIO mask

    @ 62 - 2(MOVS) - 1(STR) = 59 remaining cycles
    .rept 59
        NOP
    .endr

    @ HIGH half-cycle
    MOVS r5, #DOLOS_SIO_GPIO_OUT_SET
    STR r2, [r1, r5]               @ SWCLK HIGH

    MOVS r5, #DOLOS_SIO_GPIO_IN
    LDR r0, [r1, r5]               @ Read all pins

    LSRS r0, r0, #DOLOS_PIN_SWDIO  @ Shift SWDIO bit to position 0

    MOVS r4, #1
    ANDS r0, r0, r4                 @ Isolate bit

    MOVS r5, #DOLOS_SIO_GPIO_OUT_CLR
    STR r2, [r1, r5]               @ SWCLK LOW

    @ 62 - 4(MOVS) - 2(STR) - 1(LDR) - 1(LSRS) - 1(ANDS) = 53 NOPs required
    .rept 53
        NOP
    .endr

    POP {r4, r5, pc}
    .ltorg

@ Turnaround -> Transition period between one side stopping transmission and the other starting.
@ During this period, no one drives the wire. It remains at high impedance. This is necessary
@ to avoid electrical conflict, one side wanting HIGH and the other LOW.
turnaround_host_to_target:
    PUSH {r4, r5, lr}
    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWDIO)
    MOVS r4, #(1 << DOLOS_PIN_SWCLK)

    MOVS r5, #DOLOS_SIO_GPIO_OE_CLR
    STR r2, [r1, r5]               @ Release SWDIO — high impedance

    @ HIGH half-cycle
    MOVS r5, #DOLOS_SIO_GPIO_OUT_SET
    STR r4, [r1, r5]               @ Activate SWCLK

    @ 62 - 2(MOVS) - 1(STR) = 59 NOPs
    .rept 59
        NOP
    .endr

    @ LOW half-cycle
    MOVS r5, #DOLOS_SIO_GPIO_OUT_CLR
    STR r4, [r1, r5]               @ Take down SWCLK

    @ 62 - 2(MOVS) - 1(STR) = 59 NOPs
    .rept 59
        NOP
    .endr

    POP {r4, r5, pc}
    .ltorg

turnaround_target_to_host:
    PUSH {r4, r5, lr}
    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWDIO)
    MOVS r4, #(1 << DOLOS_PIN_SWCLK)

    @ HIGH half-cycle
    MOVS r5, #DOLOS_SIO_GPIO_OUT_SET
    STR r4, [r1, r5]               @ Activate SWCLK

    @ 62 - 2(MOVS) - 1(STR) = 59 NOPs
    .rept 59
        NOP
    .endr

    @ LOW half-cycle
    MOVS r5, #DOLOS_SIO_GPIO_OUT_CLR
    STR r4, [r1, r5]               @ Take down SWCLK

    @ 62 - 2(MOVS) - 1(STR) = 59 NOPs
    .rept 59
        NOP
    .endr

    MOVS r5, #DOLOS_SIO_GPIO_OE_SET
    STR r2, [r1, r5]               @ SWDIO assumes responsibility

    POP {r4, r5, pc}
    .ltorg