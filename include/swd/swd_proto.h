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
 *  @brief Inicializa e força a interface do alvo a entrar no modo SWD
 * Executa os reset's de linha e transmite a sequência de entrada no modo SWD (0xE79E) seguida por
 * mais um reset de linha e 2 ciclos ociosos. Após a execução desta função, o barramento estará
 * pronto para a primeira transação SWD.
 */
uint32_t swd_read(uint8_t apndp, uint8_t addr, uint8_t rnw);

/**
 * @brief Lê o registrador de identificação (IDCODE) do alvo.
 * @return uint32_t IDCODE de 32 bits do chip conectado, ou 0xFFFFFFFF em caso de erro.
 */
uint32_t swd_read_idcode(void);

#ifdef __cplusplus
}
#endif

#endif /* SWD_PROTO_H */