/**

* Physical Layer SWD

* C interface for low-level routines implemented in
* ARM Cortex-M0+ assembly (swd_phy.S).

* DOMAIN: Exclusive Core 1.

* All functions in this layer access GPIO via SIO (0xD0000000),

* with a guaranteed latency of 1 cycle (RP2040 Datasheet §2.3.1).

* NEVER call these functions from Core 0.

*/

#ifndef SWD_PHY_H
#define SWD_PHY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* swd_phy_init()
* Configures GPIO_OE and initial state of SWD pins:
* SWCLK = LOW, SWDIO = HIGH (idle, ADIv5 §B4.3.1)

* Must be called ONCE at the start of Core 1, before any

* SWD operation. Not thread-safe — Core 1 only.

*/
void swd_phy_init(void);

/**
 * swd_write_bit(uint32_t bit)
 *
 * Transmits a single bit over the SWD bus.
 *
 * @param bit  Bit value (0 or 1). Bits > 1 will be treated as 1
 *             due to implicit AND mask in the branchless path.
 *
 * Timing: SWDIO stable before SWCLK rising edge.
 * Target samples SWDIO on rising edge (ADIv5 §B4.3.2).
 *
 * Target frequency: 1 MHz @ 125 MHz sysclk.
 * Calibrate NOP_HALF_HIGH / NOP_HALF_LOW in swd_phy.S with oscilloscope.
 */
void write_bit(uint32_t bit);

/**
 * swd_write_byte(uint32_t byte)
 *
 * Transmits 8 bits, LSB first (ADIv5 §B4.3.4).
 * Equivalent to 8 sequential calls to swd_write_bit.
 *
 * @param byte  Byte to transmit (only bits [7:0] are used).
 */
void write_byte(uint32_t byte);

// Turnaround control
/**
 * swd_turnaround_host_to_target()
 *
 * Releases SWDIO (high impedance) and generates 1 turnaround cycle.
 * Call BEFORE reading bits from target (e.g.: ACK phase, DATA phase in read).
 * (ADIv5 §B4.3.3 — default turnaround = 1 cycle)
 */
void swd_turnaround_host_to_target(void);

/**
 * swd_turnaround_target_to_host()
 *
 * Generates 1 turnaround cycle and re-enables SWDIO as output.
 * Call AFTER the last bit read from target, before writing again.
 */
void swd_turnaround_target_to_host(void);

#ifdef __cplusplus
}
#endif

#endif /* SWD_PHY_H */