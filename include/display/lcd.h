#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_COMMAND   0x00
#define LCD_CHARACTER 0x01

void lcd_bus_init(void);
void lcd_init(void);
void lcd_clear(void);
void lcd_set_state(const char *state_name);
void lcd_set_error(const char *error_code);

#endif // LCD_H
