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

// Categoria B — Boot
#define ERR_NONE 0x0000
#define B001 0x0101 // Falha na configuração de clocks
#define B002 0x0102 // Falha na inicialização de GPIO
#define B003 0x0103 // Falha na inicialização do timer
#define B004 0x0104 // Falha na alocação de buffers

// Categoria S — SWD
#define S000 0x0200 // Modo SWD ativado
#define S001 0x0201 // Line reset falhou
#define S002 0x0202 // Falha na sequência JTAG-to-SWD
#define S003 0x0203 // Alvo não respondeu
#define S004 0x0204 // IDCODE inválido
#define S101 0x0211 // ACK WAIT persistente
#define S102 0x0212 // ACK FAULT
#define S103 0x0213 // ACK inválido
#define S104 0x0214 // Erro de paridade
#define S105 0x0215 // Timeout de transação
#define S106 0x0216 // Falha de turnaround
#define S107 0x0217 // Protocolo fora de sincronismo

// Categoria F — Fatal
#define F001 0x0901 // Assert interno
#define F002 0x0902 // Estado inválido da FSM
#define F003 0x0903 // Watchdog acionado
#define F004 0x0904 // Corrupção de memória detectada
#define F005 0x0905 // Falha irrecuperável

// Categoria U — USB
#define U000 0x0300 // USB inicializado
#define U001 0x0301 // Controlador USB não inicializou
#define U002 0x0302 // Enumeração falhou
#define U003 0x0303 // Timeout de conexão com host
#define U004 0x0304 // Endpoint não configurado
#define U005 0x0305 // Erro de transmissão
#define U006 0x0306 // Erro de recepção

// Categoria T — Controle do Alvo
#define T000 0x0400 // Alvo em reset (RUN LOW)
#define T001 0x0401 // Alvo liberado (RUN HIGH)
#define T002 0x0402 // Halt da CPU executado
#define T003 0x0403 // Halt falhou — CPU continua rodando
#define T004 0x0404 // Reset pelo watchdog do alvo

// Categoria Q — QSPI/XIP
#define Q000 0x0500 // QSPI/XIP inicializado
#define Q001 0x0501 // Falha ao configurar registradores XIP
#define Q002 0x0502 // Flash externa não respondeu
#define Q003 0x0503 // Timeout na inicialização QSPI

// Categoria M — Memória
#define M000 0x0600 // Leitura iniciada
#define M001 0x0601 // Endereço inválido
#define M002 0x0602 // Falha de leitura
#define M003 0x0603 // Leitura fora da região permitida
#define M004 0x0604 // Overflow de buffer (FATAL)
#define M005 0x0605 // Tamanho de bloco inválido

// Categoria H — Hash/Integridade
#define H000 0x0700 // Cálculo SHA-256 iniciado
#define H001 0x0701 // Cálculo SHA-256 falhou
#define H002 0x0702 // Hash inconsistente (FATAL)
#define H003 0x0703 // Dados corrompidos (FATAL)
#define H004 0x0704 // Checksum intermediário divergente

// Categoria C — Comunicação com Host
#define C000 0x0800 // Canal de comunicação pronto
#define C001 0x0801 // Comando inválido
#define C002 0x0802 // Argumento inválido
#define C003 0x0803 // Comando não suportado
#define C004 0x0804 // Checksum de pacote inválido
#define C005 0x0805 // Timeout de comando

// Categoria I — Estados Operacionais
#define I000 0x0A00 // Idle
#define I001 0x0A01 // Inicializando
#define I002 0x0A02 // Aguardando host
#define I003 0x0A03 // Conectando ao alvo
#define I004 0x0A04 // Lendo IDCODE
#define I005 0x0A05 // Extraindo firmware
#define I006 0x0A06 // Calculando hash
#define I007 0x0A07 // Enviando dados
#define I008 0x0A08 // Concluído

/** @brief Acende o LED onboard. */
void platform_led_on(void);

/** @brief Apaga o LED onboard. */
void platform_led_off(void);

/** @brief Blinks the LED n times (status/error signaling).
 *  @param n number of blinks
 *  @param period_ms period in ms
 */
void platform_led_blink(uint8_t n, uint32_t period_ms);

#endif /* PLATFORM_INIT_H */