#ifndef APP_H
#define APP_H
#include "platform/platform_init.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

dolos_fault_t app_enter_swd(void);

dolos_fault_t app_read_idcode(uint32_t *idcode);
}
#endif // APP_H