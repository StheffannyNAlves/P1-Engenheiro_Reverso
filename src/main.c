// codigo a ser refatorado, o codigo vai rodar na SRAM invés da Flash.


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// endereços base
#define RESETS_BASE (0x4000c000)
#define UART0_BASE (0x40034000)
#define IO_BANK0 0x40014000 
#define XIP_SSI_BASE (0x18000000)

// mapeamento dos registradores  de reset
#define RESETS_RESET *(volatile uint32_t *) (RESETS_BASE + 0x00)
#define RESETS_RESET_DONE *(volatile uint32_t *) (RESETS_BASE + 0x08) // pra zerar o bit


// regs de UART
#define UART_FR *(volatile uint32_t *) (UART0_BASE + 0x18) // Status
#define UART_DR *(volatile uint32_t *) (UART0_BASE + 0x00) // Dados

// mascaras UART
#define UART0_RST_BIT (1 << 22) 
#define UART_RXFE_BIT (1 << 4) // isola o bit 4(Receive FIFO Empty)
#define TXFF_RST_BIT (1 << 5) // isola o bit 5(buffer/FIFO já cheio)








#define GPIO0_CTRL *(volatile uint32_t *) (IO_BANK0 + 0x004) // Tx
#define GPIO1_CTRL *(volatile uint32_t *) (IO_BANK0 + 0x00c) // Rx
#define FUNC_UART0 2



#define CTRL_R0  *(volatile uint32_t *) (XIP_SSI_BASE + 0x00) // control register 0, define o formato(clock, modo SPI)
#define SSI_ENR  *(volatile uint32_t *) (XIP_SSI_BASE + 0x08) // ctrl reg 1, habilita/desabilita o SSI para cada leitura
#define DR0  *(volatile uint32_t *) (XIP_SSI_BASE + 0x60) // data reg, envio e recebimento de dados da FLASH
#define SR  *(volatile uint32_t *)  (XIP_SSI_BASE + 0X28)

// Valores escritos nos registradores, mascaras
#define SSI_EN_BIT (1 << 0)
#define SR_TFNF (1 << 1) // transmit fifo not full, diz quando a FIFO de TX  *não* tá cheio(verificar)
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


void uart_getc(void)
{
   while (UART_FR & UART_RXFE_BIT) // 1. Enquanto a FIFO estiver vazia, espere
   {

   }
  
   // 2. Saiu tem dados, leia
   return (char)UART_DR;
}
   

// funcao de inicialização do SSI(correção critica)
void ssi_init(void)
{
   // 1. Desabilita o XIP
   SSI_ENR = 0; 


   // 2.  Configura o protocolo
   CTRL_R0 = (0 << 16) | 7;
 
   
   // 3. Habilita
   SSI_ENR |= SSI_EN_BIT; 

   
}






// inicializacao do uart
void uart_init(void)
{
   RESETS_RESET &= ~UART0_RST_BIT;

   while (!(RESETS_RESET_DONE & UART0_RST_BIT))
   {
      
   }

   GPIO0_CTRL = FUNC_UART0;
   GPIO1_CTRL = FUNC_UART0;
   

   // baud rate = clk do sistema//(16 * baud rate)
}








// func pra falar com o SPI
static uint8_t ssi_transfer_byte(uint8_t data_out)
{
   while (!(SR & SR_TFNF)) // espera ter espaço
   {
      /* code */
   }
   DR0 = data_out; 
   
   while (!(SR & SR_TFNF)) // espera o byte de resposta chegar
   {
      /* code */
   }

   return (uint8_t)DR0;
}






/**
 * Lê um byte da memória Flash usando o comando Fast Read.
 * Envia um byte e imediatamente lê um byte para manter as FIFOs TX e RX sincronizadas.
 * 
 * @param address O endereço de 24 bits para leitura
 * @return O byte lido da memória Flash
 */
uint8_t ssi_read_byte(uint32_t address)
{
   // 1. Envio do comando Fast Read
   ssi_transfer_byte(0x0B);
   
   // 2. Envia os bits 23-16(byte 1)
   ssi_transfer_byte((address >> 16) & 0xFF); 
   
   // 3. Envia os bits 15-8(byte 2)
   ssi_transfer_byte((address >> 8) & 0xFF); 
   
   
   // 4. Envia os bits 7-0(byte 3)
   ssi_transfer_byte(address & 0xFF); 

   // 5. envia um "dummy byte"(byte ficticio)
   ssi_transfer_byte(0x15);

   // 6. Envia um byte nulo(pra gerar clock)
   return ssi_transfer_byte(0x00);
   
}










// program principal
int main(void)
{

  // incompleto






}