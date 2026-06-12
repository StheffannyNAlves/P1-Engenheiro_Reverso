// Core 1
#ifndef TARGET_CTRL_H
#define TARGET_CTRL_H

#define PIN_RST 22u

void target_ctrl_init(void);
void target_reset_low(void);
void target_reset_high(void);

#endif