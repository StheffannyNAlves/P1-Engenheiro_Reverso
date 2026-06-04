#include "display/lcd.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Bus configuration
#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define LCD_ADDR 0x27

// Controle do PCF8574T
#define LCD_BACKLIGHT 0x08
#define ENABLE_BIT 0x04

// Comandos HD44780
#define LCD_CLEARDISPLAY 0x01
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET 0x20

#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_DISPLAYON 0x04
#define LCD_CURSOROFF 0x00
#define LCD_BLINKOFF 0x00
#define LCD_2LINE 0x08
#define LCD_5x8DOTS 0x00
#define LCD_4BITMODE 0x00

static void i2c_write_byte(uint8_t val) {
    i2c_write_timeout_per_char_us(I2C_PORT, LCD_ADDR, &val, 1, false, 5000);
}

static void lcd_toggle_enable(uint8_t val) {
    i2c_write_byte(val | ENABLE_BIT);
    sleep_us(1);
    i2c_write_byte(val & ~ENABLE_BIT);
    sleep_us(50);
}

static void lcd_send_nibble(uint8_t val, uint8_t mode) {
    uint8_t nibble = val & 0xF0;
    i2c_write_byte(nibble | mode | LCD_BACKLIGHT);
    lcd_toggle_enable(nibble | mode | LCD_BACKLIGHT);
}

static void lcd_send_byte(uint8_t val, uint8_t mode) {
    lcd_send_nibble(val & 0xF0, mode);
    lcd_send_nibble((val << 4) & 0xF0, mode);
}

// Public API

void lcd_bus_init(void) {
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, 0);
    sleep_ms(2);
}

// Matriz de bits para desenhar um triângulo de alerta (⚠️)
static const uint8_t custom_alert_icon[8] = {
    0b00100, //    *
    0b00100, //    *
    0b01010, //   * *
    0b01010, //   * *
    0b11011, //  ** **
    0b10001, //  *   *
    0b11111, //  *****
    0b00000  // Linha em branco
};

// Load the custom character into the index position (0 to 7) of the CGRAM
void lcd_create_char(uint8_t index, const uint8_t *charmap) {
    index &= 0x7;                                    // Ensure the index is between 0 and 7
    lcd_send_byte(0x40 | (index << 3), LCD_COMMAND); // Set address in CGRAM
    for (int i = 0; i < 8; i++) {
        lcd_send_byte(charmap[i], LCD_CHARACTER);
    }
}

void lcd_init(void) {
    sleep_ms(100);

    lcd_send_nibble(0x30, 0);
    sleep_ms(5);
    lcd_send_nibble(0x30, 0);
    sleep_ms(1);
    lcd_send_nibble(0x30, 0);
    lcd_send_nibble(0x20, 0); // enter 4-bit mode

    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE, 0);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF, 0);
    lcd_clear();
    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT, 0);
    lcd_create_char(0, custom_alert_icon);
}

void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, 0);
}

void lcd_char(char val) { lcd_send_byte(val, 1); }

void lcd_string(const char *s) {
    while (*s) {
        lcd_char(*s++);
    }
}

void lcd_set_state(const char *state_name) {
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "%-16s", state_name);
    lcd_set_cursor(0, 0);
    lcd_string(buffer);
}

void lcd_set_error(const char *error_code) {
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "%-16s", error_code);
    lcd_set_cursor(1, 0);
    lcd_string(buffer);
}