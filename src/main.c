#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// endereços base
#define RESETS_BASE (0x4000c000)
#define UART0_BASE (0x40034000)
#define IO_BANK0 0x40014000 
#define SSIO_BASE (0x4003c000)

// mapeamento dos registradores 
#define UART_FR *(uint32_t *) (UART0_BASE + 0x18) // Status
#define UART_DR *(uint32_t *) (UART0_BASE + 0x00) // Dados
#define RESETS_RESET *(uint32_t *) (RESETS_BASE + 0x00)
#define RESETS_RESET_DONE *(uint32_t *) (RESETS_BASE + 0x08) // pra zerar o bit

// Configuração dos GPIOS
#define GPIO6_CTRL *(uint32_t *) (IO_BANK0 + 0x1C) 
#define GPIO7_CTRL *(uint32_t *) (IO_BANK0 + 0x20) 
#define GPIO8_CTRL *(uint32_t *) (IO_BANK0 + 0x24)
#define GPIO9_CTRL *(uint32_t *) (IO_BANK0 + 0x28) 
#define GPIO10_CTRL *(uint32_t *) (IO_BANK0 + 0x2C) 
#define GPIO11_CTRL *(uint32_t *) (IO_BANK0 + 0x30) 



#define SSI_CR0  *(uint32_t *) (SSIO_BASE + 0x00) // control register 0, define o formato(clock, modo SPI)
#define SSI_CR1  *(uint32_t *) (SSIO_BASE + 0x04) // ctrl reg 1, habilita/desabilita o SSI para cada leitura
#define SSI_DR0  *(uint32_t *) (SSIO_BASE + 0x08) // data reg, envio e recebimento de dados da FLASH


// Valores escritos nos registradores, mascaras
#define TXFF_RST_BIT (1 << 5) // isola o bit 5(buffer/FIFO já cheio)
#define UART0_RST_BIT (1 << 22) 
#define FUNC_UART0 (1 << 0)
#define FUNC_SSI 5
#define SSI0_RST_BIT (1<< 16) // mascara pra sair do reset
#define SSI_EN_BIT (1 << 0)




void uart_putc(char data) // funcao de envio de bits
{
   // enquanto o bit 5 no flag register(UART_FR) for 1, espere
   while (UART_FR & TXFF_RST_BIT)
   {
    // a cpu espera 
   }

   UART_DR = data; // escreva o byte de dado diretamente no Data Register

}
   

// func de inicialização
void ssi_init(void)
{
   // 1. sai do reset
   RESETS_RESET &= ~SSI0_RST_BIT;
   while (RESETS_RESET_DONE & SSI0_RST_BIT)
   {

   }   
   
   // 2.  Desabilita o SSI
   SSI_CR1 &= ~SSI_EN_BIT; 
   
   // 3. Configuração dos pinos
   GPIO6_CTRL = FUNC_SSI; // chip select(cs)
   GPIO7_CTRL = FUNC_SSI; // Serial Clock (SCLK)

   // quatros vias de dados, pra o chip ler quatros bits em um unico ciclo de clock.
   GPIO8_CTRL = FUNC_SSI; // Serial Data 0 (SD0)
   GPIO9_CTRL = FUNC_SSI; // Serial Data 1 (SD1)
   GPIO10_CTRL = FUNC_SSI; // Serial Data 2 (SD2)
   GPIO11_CTRL = FUNC_SSI; // Serial Data 3 (SD3)
   

   // 4. Protocolo
   SSI_CR0 = (2 << 16) | 7; // configuração da unidade de medida e o clock, (2 << 16) é o divisor de clock SCKDV (Clock/2)
   // 7 é o DFS (Data Frame Size) para 8 bits

   // 5. Modo mestre/escravo
   SSI_CR1 = 0; 
   
   // 6. Habilita
   SSI_CR1 |= SSI_EN_BIT; 

   
}

// func que faz a aquisição da FLASH
uint8_t ssi_read_byte(uint32_t address)
{

   SSI_DR0 = 0x0B;

   // envio dos bytes
   
}
