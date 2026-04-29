#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa plataforma: Pico SDK, GPIOs, alvo em reset.
 * @return true se bem-sucedida, false se falhar
 */
typedef enum {
  ERR_NONE = 0,
  CLOCK_FAIL,
  RUN_FAIL,
  LED_FAIL,
} dolos_fault_code_t;

typedef struct {
  dolos_fault_code_t code;
  bool is_fatal;
} dolos_fault_t;

/** @brief Acende o LED onboard. */
void platform_led_on(void);

/** @brief Apaga o LED onboard. */
void platform_led_off(void);

/** @brief Pisca o LED n vezes (sinalização de estado/erro).
 *  @param n número de piscadas
 *  @param period_ms período em ms
 */
void platform_led_blink(uint8_t n, uint32_t period_ms);

#endif /* PLATFORM_INIT_H */