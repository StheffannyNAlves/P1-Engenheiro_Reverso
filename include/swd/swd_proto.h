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

#ifdef __cplusplus
}
#endif

#endif /* SWD_PROTO_H */