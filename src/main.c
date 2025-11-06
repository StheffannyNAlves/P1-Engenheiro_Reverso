#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// endereços base
#define RESETS_BASE*(uint32_t *)  (0x4000c000)
#define UART0_BASE *(uint32_t *) (0x40034000)
#define IO_BANK0 *(uint32_t *) (0x40014000 + 0x0CC)

// mapeamento dos registradores
#define UART_FR *(uint32_t *) (UART0_BASE + 0x18) // Status
#define UART_DR *(uint32_t *) (UART0_BASE + 0x00) // Dados
#define RESETS_RESET_DONE*(uint32_t *) (RESETS_BASE + 0x00) // pra zerar o bit
#define GPIO0_CTRL *(uint32_t *) (IO_BANK0 + 0) // Configuração do pino Tx

// Valores escritos nos registradores
#define TXFF_BIT (1 << 5) // mascara pra isolar o bit 5(buffer/FIFO já cheio)
#define UART0_RST_BIT (1 << 22) // mascara para isolar o bit 22
#define FUNC_UART0 (1)






void uart_putc(char data) // funcao de envio de bits
{
   // enquanto o bit 5 no flag register(UART_FR) for 1, espere
   while (UART_FR & TXFF_BIT)
   {
    // a cpu espera 
   }

   UART_DR = data; // escreva o byte de dado diretamente no Data Register

}
   


void ssi_init(void)
{
   
}
