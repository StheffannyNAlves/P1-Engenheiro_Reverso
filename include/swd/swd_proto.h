#ifndef SWD_PROTO_H
#define SWD_PROTO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Executes the full SWD entry sequence.
 * * Flow: line_reset -> 0xE79E -> line_reset -> 2 idle cycles.
 * Leaves the bus ready for the first SWD transaction.
 */
void swd_enter_swd_mode(void);

/**
 * @brief Executes a blind TARGETSEL write for SWD multidrop.
 * * @param target_id The 32-bit ID of the target to select (e.g., 0x01021009).
 */
void swd_write_targetsel(uint32_t target_id);

/**
 * @brief Reads a 32-bit value from a specific SWD register.
 * * @param apndp Access Port (1) or Debug Port (0).
 * @param a2    Address bit 2.
 * @param a3    Address bit 3.
 * @return uint32_t The 32-bit data read from the target.
 */
uint32_t swd_read(uint8_t apndp, uint8_t a2, uint8_t a3);

/**
 * @brief Writes a 32-bit value to a specific SWD register.
 * * @param apndp         Access Port (1) or Debug Port (0).
 * @param a2            Address bit 2.
 * @param a3            Address bit 3.
 * @param data_to_write The 32-bit data to transmit.
 * @return uint8_t      The ACK response from the target (001b = OK).
 */
uint8_t swd_write(uint8_t apndp, uint8_t a2, uint8_t a3, uint32_t data_to_write);

/**
 * @brief Reads the target identification register (IDCODE).
 * * @return uint32_t 32-bit IDCODE of the connected chip, or 0xFFFFFFFF on error.
 */
uint32_t swd_read_idcode(void);

void swd_wake_dormant(void);

#ifdef __cplusplus
}
#endif

#endif /* SWD_PROTO_H */