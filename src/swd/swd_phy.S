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

   .global line_reset
   .type line_reset, %function

   .equ DOLOS_SIO_BASE,         0xD0000000
   .equ DOLOS_SIO_GPIO_IN,      0x004   @ read current pin state
   .equ DOLOS_SIO_GPIO_OUT_SET, 0x014   @ force specific pins HIGH
   .equ DOLOS_SIO_GPIO_OUT_CLR, 0x018   @ force specific pins LOW
   .equ DOLOS_SIO_GPIO_OE_SET,  0x024   @ enable output
   .equ DOLOS_SIO_GPIO_OE_CLR,  0x028   @ disable output


swd_phy_init:
    PUSH {r4, r5, lr}

    LDR r1, =DOLOS_SIO_BASE
    MOVS r2, #(1 << DOLOS_PIN_SWCLK) | (1 << DOLOS_PIN_SWDIO)

    MOVS r5, #0x24
    STR r2, [r1, r5]               @ SWCLK as output via GPIO_OE_SET

    @ SWCLK in LOW
    MOVS r2, #(1 << DOLOS_PIN_SWCLK)
    MOVS r5, #0x18
    STR r2, [r1, r5]               @ Clear clock — SWCLK starts LOW

    @  SWDIO in HIGH
    MOVS r2, #(1 << DOLOS_PIN_SWDIO)
    MOVS r5, #0x14      @ DOLOS_SIO_GPIO_OUT_SET
    STR r2, [r1, r5]    @ GPIO_OUT_SET — SWDIO HIGH

    POP {r4, r5, pc}
    .ltorg

writebit:
    PUSH {r4, r5, r6, r7, lr}      @ Salva registradores (r7 incluído)

    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWCLK)
    MOVS r3, #(1 << DOLOS_PIN_SWDIO)

    @ LOW half-cycle
    MOVS r6, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r2, [r1, r6]               @ SWCLK LOW

    LSLS r4, r0, #DOLOS_PIN_SWDIO
    SUBS r5, r3, r4

    MOVS r6, #0x14                 @ DOLOS_SIO_GPIO_OUT_SET
    STR r4, [r1, r6]               @ Aplica os Dados

    MOVS r6, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r5, [r1, r6]               @ Limpa a máscara

    @ Atraso Gordo Implacável: ~600 ciclos (4.8us)
    MOVS r7, #200
1:  SUBS r7, r7, #1
    BNE 1b

    @ HIGH half-cycle
    MOVS r6, #0x14                 @ DOLOS_SIO_GPIO_OUT_SET
    STR r2, [r1, r6]               @ SWCLK HIGH

    @ Atraso Gordo Implacável: ~600 ciclos (4.8us)
    MOVS r7, #200
2:  SUBS r7, r7, #1
    BNE 2b

    MOVS r6, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r2, [r1, r6]               @ Garante que o clock termine em LOW

    POP {r4, r5, r6, r7, pc}
    .ltorg

readbit:
    PUSH {r4, r5, r7, lr}

    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWCLK)
    MOVS r3, #(1 << DOLOS_PIN_SWDIO)

    @ LOW half-cycle
    MOVS r5, #0x28                 @ DOLOS_SIO_GPIO_OE_CLR
    STR r3, [r1, r5]               @ SWDIO vira input

    MOVS r5, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r2, [r1, r5]               @ SWCLK LOW

    MOVS r7, #200
1:  SUBS r7, r7, #1
    BNE 1b

    @ HIGH half-cycle
    MOVS r5, #0x14                 @ DOLOS_SIO_GPIO_OUT_SET
    STR r2, [r1, r5]               @ SWCLK HIGH

    @ Atraso de estabilização de leitura
    MOVS r7, #100
2:  SUBS r7, r7, #1
    BNE 2b

    MOVS r5, #0x004                @ DOLOS_SIO_GPIO_IN
    LDR r0, [r1, r5]               @ Lê todos os pinos

    LSRS r0, r0, #DOLOS_PIN_SWDIO
    MOVS r4, #1
    ANDS r0, r0, r4                @ Isola o bit

    MOVS r5, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r2, [r1, r5]               @ SWCLK LOW

    MOVS r7, #200
3:  SUBS r7, r7, #1
    BNE 3b

    POP {r4, r5, r7, pc}
    .ltorg

turnaround_host_to_target:
    PUSH {r4, r5, r7, lr}
    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWDIO)
    MOVS r4, #(1 << DOLOS_PIN_SWCLK)

    @ HIGH half-cycle
    MOVS r5, #0x14                 @ DOLOS_SIO_GPIO_OUT_SET
    STR r4, [r1, r5]               @ Levanta SWCLK

    MOVS r7, #200
1:  SUBS r7, r7, #1
    BNE 1b

    @ LOW half-cycle
    MOVS r5, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r4, [r1, r5]               @ Derruba SWCLK

    MOVS r7, #200
2:  SUBS r7, r7, #1
    BNE 2b

    MOVS r5, #0x28                 @ DOLOS_SIO_GPIO_OE_CLR
    STR r2, [r1, r5]               @ Libera a linha

    POP {r4, r5, r7, pc}
    .ltorg

turnaround_target_to_host:
    PUSH {r4, r5, r7, lr}
    LDR r1, =0xD0000000
    MOVS r2, #(1 << DOLOS_PIN_SWDIO)
    MOVS r4, #(1 << DOLOS_PIN_SWCLK)

    MOVS r5, #0x24                 @ DOLOS_SIO_GPIO_OE_SET
    STR r2, [r1, r5]               @ Sonda assume

    @ HIGH half-cycle
    MOVS r5, #0x14                 @ DOLOS_SIO_GPIO_OUT_SET
    STR r4, [r1, r5]               @ Levanta SWCLK

    MOVS r7, #200
1:  SUBS r7, r7, #1
    BNE 1b

    @ LOW half-cycle
    MOVS r5, #0x18                 @ DOLOS_SIO_GPIO_OUT_CLR
    STR r4, [r1, r5]               @ Derruba SWCLK

    MOVS r7, #200
2:  SUBS r7, r7, #1
    BNE 2b

    POP {r4, r5, r7, pc}
    .ltorg


line_reset:
    PUSH {r4, lr}

    MOVS r4, #50

line_reset_loop:
    MOVS r0, #1
    BL writebit @ Executes the clock cycle by sending the HIGH bit through the SIO

    SUBS r4, r4, #1
    BNE line_reset_loop

    POP {r4, pc}
    .ltorg