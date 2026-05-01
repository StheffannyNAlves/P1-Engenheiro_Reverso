#ifndef TARGET_CTRL_H
#define TARGET_CTRL_H

#define PIN_RUN 14

void target_ctrl_init(void);
void target_reset_low(void);
void target_reset_high(void);

#endif