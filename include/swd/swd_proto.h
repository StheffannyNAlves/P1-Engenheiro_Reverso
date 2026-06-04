#ifndef SWD_PROTO_H
#define SWD_PROTO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * swd_enter_swd_mode()
 *
 * Executes the full SWD entry sequence:
 * line_reset → 0xE79E → line_reset → 2 idle cycles
 * Leaves the bus ready for the first SWD transaction.
 */
void swd_enter_swd_mode(void);

/**
 *  @brief Initializes and forces the target interface to enter SWD mode.
 * Executes the line resets and transmits the SWD entry sequence (0xE79E)
 * followed by another line reset and 2 idle cycles. After this function
 * completes, the bus will be ready for the first SWD transaction.
 */
uint32_t swd_read(uint8_t apndp, uint8_t addr, uint8_t rnw);

/**
 * @brief Reads the target identification register (IDCODE).
 * @return uint32_t 32-bit IDCODE of the connected chip, or 0xFFFFFFFF on error.
 */
uint32_t swd_read_idcode(void);

#ifdef __cplusplus
}
#endif

#endif /* SWD_PROTO_H */