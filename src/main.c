#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// endereços base
#define RESETS_BASE (0x4000c000)
#define UART0_BASE (0x40034000)
#define IO_BANK0 0x40014000 
#define XIP_SSI_BASE (0x18000000)

// mapeamento dos registradores 
#define UART_FR *(uint32_t *) (UART0_BASE + 0x18) // Status
#define UART_DR *(uint32_t *) (UART0_BASE + 0x00) // Dados
#define RESETS_RESET *(uint32_t *) (RESETS_BASE + 0x00)
#define RESETS_RESET_DONE *(uint32_t *) (RESETS_BASE + 0x08) // pra zerar o bit

#define GPIO0_CTRL *(uint32_t *) (IO_BANK0 + 0x004) // Tx
#define GPIO1_CTRL *(uint32_t *) (IO_BANK0 + 0x00c) // Rx



#define CTRL_R0  *(uint32_t *) (XIP_SSI_BASE + 0x00) // control register 0, define o formato(clock, modo SPI)
#define SSI_ENR  *(uint32_t *) (XIP_SSI_BASE + 0x08) // ctrl reg 1, habilita/desabilita o SSI para cada leitura
#define DR0  *(uint32_t *) (XIP_SSI_BASE + 0x60) // data reg, envio e recebimento de dados da FLASH
#define SR  *(uint32_t *)  (XIP_SSI_BASE + 0X28)

// Valores escritos nos registradores, mascaras
#define TXFF_RST_BIT (1 << 5) // isola o bit 5(buffer/FIFO já cheio)
#define UART0_RST_BIT (1 << 22) 
#define FUNC_UART0 (1 << 0)
#define SSI_EN_BIT (1 << 0)
#define SR_TFNF (1 << 2) // transmit fifo not full, diz quando a FIFO de TX  *não* tá cheio
#define SR_RFNE (1 << 3) // Receive FIFO not empty, diz quando o dado chegou


void uart_putc(char data) // funcao de envio de bits
{
   // enquanto o bit 5 no flag register(UART_FR) for 1, espere
   while (UART_FR & TXFF_RST_BIT)
   {
    // a cpu espera 
   }

   UART_DR = data; // escreva o byte de dado diretamente no Data Register

}
   

// funcao de inicialização do SSI
void ssi_init(void)
{
   // 1. Desabilita o XIP
   SSI_ENR = 0; 

   // 2.  Configura o protocolo
   CTRL_R0 = (2 << 16) | 7;
 
   
   // 3. Habilita
   SSI_ENR |= SSI_EN_BIT; 

   
}

// func que faz a aquisição da FLASH
uint8_t ssi_read_byte(uint32_t address)
{
   // envio do comando Fast Read
   while ( !(SR & SR_TFNF) )
   {
      // espera TX
   }

   DR0 = 0x0B; // saiu, tem espaço
   
   while ( !(SR & SR_TFNF))
   {
      

   }

   DR0 = (address >> 16) & 0xFF; // envia os bits 23-16(byte 1)
   

   while ( !(SR & SR_TFNF))
   {
      
   }

   DR0 = (address >> 8) & 0xFF; // envia os bits 15-8(byte 2)
   
   while ( !(SR & SR_TFNF))
   {

   }
   DR0 = address & 0xFF; // envia os bits 7-0(byte 3)
   
   
   
   // 5. envia um "dummy byte"(byte ficticio)
   while ( !(SR & SR_TFNF))
   {
      // espera TX
   }

   DR0 = 0x15;

   // 6. Espera o dado de resposta chegar

   while ( !(SR & SR_RFNE))
   {
      // espera RX
   }   
   
   // 7. Lê o dado
   uint8_t dado = DR0; // A FIFO me fornece um byte
   
   return dado;
   
}





// inicializacao do uart
void uart_init(void)
{
   RESETS_RESET &= ~UART0_RST_BIT;

   while (RESETS_RESET_DONE & UART0_RST_BIT)
   {
      
   }

   GPIO0_CTRL = FUNC_UART0;
   GPIO1_CTRL = FUNC_UART0;
   

   // baud rate e control register
}





int main(void)
{

  uart_init;







}