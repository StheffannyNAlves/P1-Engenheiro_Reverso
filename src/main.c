// em desenvolvimento. esse daqui prova que a comunicação uart funciona


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// endereços base
#define RESETS_BASE (0x4000c000)
#define UART0_BASE (0x40034000)
#define IO_BANK0 0x40014000 
// define XIP_SSI_BASE (0x18000000)

// mapeamento dos registradores  de reset
#define RESETS_RESET *(volatile uint32_t *) (RESETS_BASE + 0x00)
#define RESETS_RESET_DONE *(volatile uint32_t *) (RESETS_BASE + 0x08) // pra zerar o bit

// regs de UART
#define UART_FR *(volatile uint32_t *) (UART0_BASE + 0x18) // Status
#define UART_DR *(volatile uint32_t *) (UART0_BASE + 0x00) // Dados
#define UART0_CR *(volatile uint32_t *) (UART0_BASE + 0x030) // Control Register
#define UART0_IBRD *(volatile uint32_t *) (UART0_BASE + 0x024) // Integer Baud Rate Register
#define UART0_FBRD *(volatile uint32_t *) (UART0_BASE + 0x028) //Fractional Baud Rate Register
#define UART0LCR_H *(volatile uint32_t *) (UART0_BASE + 0x02c) // Line Control Register



// mascaras 
#define UART0_RST_BIT (1 << 22) 
#define UART_RXFE_BIT (1 << 4) // isola o bit 4(Receive FIFO Empty)
#define TXFF_BIT (1 << 5) // isola o bit 5(buffer/FIFO já cheio)
#define IO_BANK0_RST_BIT (1 << 5)
#define PADS_RST_BANK0 ( 1 << 8) // controla a eletrica do pino



#define GPIO0_CTRL *(volatile uint32_t *) (IO_BANK0 + 0x004) // Tx
#define GPIO1_CTRL *(volatile uint32_t *) (IO_BANK0 + 0x00c) // Rx
#define FUNC_UART0 2



// define CTRL_R0  *(volatile uint32_t *) (XIP_SSI_BASE + 0x00) // control register 0, define o formato(clock, modo SPI)
// define SSI_ENR  *(volatile uint32_t *) (XIP_SSI_BASE + 0x08) // ctrl reg 1, habilita/desabilita o SSI para cada leitura
// define DR0  *(volatile uint32_t *) (XIP_SSI_BASE + 0x60) // data reg, envio e recebimento de dados da FLASH
// define SR  *(volatile uint32_t *)  (XIP_SSI_BASE + 0X28)

// Valores escritos nos registradores, mascaras
// define SSI_EN_BIT (1 << 0)
//define SR_TFNF (1 << 1) // transmit fifo not full, diz quando a FIFO de TX  *não* tá cheio(verificar)
// define SR_RFNE (1 << 3) // Receive FIFO not empty, diz quando o dado chegou


   







// inicializacao do uart
void uart_init(void)
{
   RESETS_RESET &= ~(UART0_RST_BIT | IO_BANK0_RST_BIT | PADS_RST_BANK0); // tira todos do reset //

   uint32_t rst_mascara = (UART0_RST_BIT | IO_BANK0_RST_BIT);
   while (!(RESETS_RESET_DONE & rst_mascara))
   {
      
   }

   GPIO0_CTRL = FUNC_UART0;
   GPIO1_CTRL = FUNC_UART0;
   

   // baud rate = clk do sistema//(16 * baud rate)
   const uint32_t clk_peri = 125000000;
   const uint32_t baud_rate = 115200;

   // calculo com arredendomento
   const uint32_t div_v64 = ((clk_peri * 4) + (baud_rate / 2)) / baud_rate;
   UART0_CR = 0;

   //A ORDEM DE ESCRITA DESSES REGS IMPORTA
   // pontoo fixo
   UART0_IBRD = div_v64 >> 6; 
   UART0_FBRD = div_v64 & 0x3F;


   // 8 bits de dados, FIFO habilitada
   UART0LCR_H = (1 << 4) | (3 << 5);




   // ativa UARTEN(0)- 1, TXE(8)- 256, RXE(9)- 512
   UART0_CR = (1 << 0) | (1 << 8) | (1 << 9); // total= 769


}





void uart_putc(char data) // funcao de envio de bits
{
   // enquanto o bit 5 no flag register(UART_FR) for 1, espere
   while (UART_FR & TXFF_BIT)
   {
    // a cpu espera 
   }

   UART_DR = data; // escreva o byte de dado diretamente no Data Register

}









// program principal
int main(void)
{

  uart_init();
   
  while (1)
  {
   uart_putc('L');

   for (volatile int i = 0; i < 100000; i++ );
  }
  

}