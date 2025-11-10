# P1: Aquisição Forense Bare-Metal de FLASH (RP2040)

[![Status](https://img.shields.io/badge/Status-Em_Desenvolvimento-orange)]()
[![Plataforma](https://img.shields.io/badge/Plataforma-RP2040%20(Bare--Metal)-green)]()
[![Licença](https://img.shields.io/badge/Licença-MIT-blue)](LICENSE)

Ferramenta de aquisição forense de baixo nível para a memória FLASH do RP2040, desenvolvida sem o uso de abstrações do SDK.

A aquisição de evidências de dispositivos embarcados requer compreensão profunda da cadeia de inicialização e layout de memória. Bootloaders comprometidos podem ocultar malware de firmware que escapa à detecção tradicional. 

---

## Objetivo

Este projeto serve como a fundação introdutória para a especialização em Sistemas Embarcados e Segurança de Hardware.

O objetivo técnico específico desta missão é documentar a cadeia de inicialização do RP2040 e desenvolver um método de aquisição forense de memória FLASH sem usar abstrações do SDK.


## Problemas encontrados

### Problema 1
O RP2040 não possui FLASH interna. Ele executa o código diretamente de um chip de FLASH externa usando um modo chamado **XIP (eXecute-In-Place)**. Esse modo é controlado pelo periférico `XIP_SSI`.

O objetivo forense é **assumir o controle manual** desse periférico `XIP_SSI` para dumpar a FLASH. No entanto, o código que tenta fazer isso está, ele mesmo, rodando da FLASH e sendo servido pelo `XIP_SSI` no modo automático.

Tentar desabilitar o XIP (`SSI_ENR = 0`) para assumir o controle manual resulta em um **travamento imediato**, pois o processador "serra o galho em que está sentado".

### O Paradoxo do XIP: 
A tentativa ingênua (e com bugs) de ler a FLASH, rodando da própria FLASH, se parece com isto. Este código, extraído da branch feature/p1-ssi-driver, falha e trava o processador:

``` C

// codigo a ser refatorado, ele tenta ler a FLASH, rodando na própria(Erro imperdoável).


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


// ... (funções uart_putc, uart_getc, etc.) ...


// funcao de inicialização do SSI(correção critica)
void ssi_init(void)
{
   // 1. Desabilita o XIP
   // >>> A LINHA DA MORTE <<<
   // O processador está buscando esta instrução da FLASH (via XIP).
   // Ao executar esta linha, ele desliga o hardware que
   // buscaria a *próxima* instrução de código.
   SSI_ENR = 0; 


   // 2.  Configura o protocolo
   CTRL_R0 = (0 << 16) | 7;
 
   
   // 3. Habilita
   SSI_ENR |= SSI_EN_BIT; 

   
}

// ... (funções de inicialização da uart, ssi_transfer_byte, etc.) ...


// program principal
int main(void)
{
  // O `boot2` nos chama (rodando da FLASH)
  uart_init();
  ssi_init(); // O processador trava aqui.

  // ... O código nunca chega aqui ...
  while(1);
}

```

Este travamento é a justificação central para a arquitetura de "Loader + Payload" descrita na próxima seção.



## Arquitetura


Para resolver o Paradoxo do XIP, a arquitetura é dividida em dois estágios:

* **1. O Loader (na FLASH):**
    * Este é o `main()` que o `boot2` executa.
    * Seu único trabalho é copiar o "Payload Forense" (um array de bytes) para a SRAM (`0x20000000`).
    * Após a cópia, ele executa um salto em Assembly (`jump_to_sram`) para a SRAM, passando o controle.

* **2. O Payload (na SRAM):**
    * Este é o código de aquisição real (`ssi_init_manual`, `ssi_read_byte`).
    * Por rodar 100% da SRAM, ele é independente da FLASH e pode, com segurança, desligar o XIP (`SSI_ENR = 0`) para assumir o controle manual e realizar o dump.


## Metodologia: Transparência e Controle DIreto via MMIO

A escolha de uma abordagem "bare-metal" (sem SDK) não é somente um exercício acadêmico; é também um requisito forense.

* **Integridade e Cadeia de Custódia:** O SDK é uma camada de abstração que pode ser comprometida. Uma função como `flash_read()` pode ser interceptada por um malware de firmware para "mentir" e esconder dados. Nossa abordagem de MMIO fala diretamente com o silício (`XIP_SSI`), que não pode mentir.
* **Contorno de Proteções:** O SDK pode implementar proteções de software (ex: impedir a leitura do `boot2`). O acesso direto ao hardware ignora essas barreiras.
* **Pegada de Memória:** O SDK é massivo. Nosso Payload Forense precisa ser minúsculo para caber e rodar 100% da SRAM. O bare-metal é a única forma de atingir essa eficiência.


### Fluxo de Aquisição

1.  **(Pico) Boot:** O `boot2` chama o `main.c` (O Loader) na FLASH.
2.  **(Pico) Staging:** O Loader copia o Payload para a SRAM.
3.  **(Pico) Salto:** O Loader executa `jump_to_address(0x20000001)`. O código na FLASH morre.
4.  **(Pico) Execução na SRAM:** O Payload (agora na SRAM) assume. Ele inicializa a UART e o SSI (agora de forma segura) e envia um sinal de "PRONTO" (ex: `0xC0FFEE`) para o Host.
5.  **(Host) Início:** O `client.py` vê o sinal de "PRONTO" e responde com o comando "INICIAR DUMP".
6.  **(Pico/Host) O Loop:** O Payload lê um bloco, envia; o Host recebe, envia ACK.


### Protocolo de Transferência (Rascunho)

Para garantir que nenhum dado seja perdido na UART, um protocolo de *handshake* é usado:

1.  **Pico (Payload)** envia `0xC0FFEE` (Magic Start).
2.  **Host (Python)** recebe e envia `0xAC` (ACK/Ready).
3.  **Host** envia `0x47` ('G' - Go).
4.  **Pico** lê 256 bytes da FLASH e os envia pela UART.
5.  **Host** recebe os 256 bytes.
6.  **Host** envia `0x47` ('G' - Go) para o próximo bloco.
7.  ...repete 8192 vezes (para 2MB)...
8.  **Pico** envia `0xDEADBEEF` (Magic End).
9.  **Host** fecha o arquivo.



###  Diagrama de Cadeia de Custódia(rascunho)

`[Chip FLASH W25Q16]` -> `[Payload na SRAM]` -> `[UART/USB]` -> `[client.py]` -> `[firmware_dump.bin]` -> `[SHA-256]`

###  Validação de Integridade 

A integridade da imagem adquirida (`firmware_dump.bin`) é validada pelo `client.py`[cite: 16], que calcula e exibe um hash **SHA-256** do arquivo final. Isso prova que a *transferência de dados* foi bem-sucedida e que o arquivo no host corresponde ao que o Payload enviou.

### Artefatos de Boot (A Serem Investigados) 

A análise do `dump.bin` focará na identificação de:
* Assinaturas do Bootloader (Boot2).
* Localização da Tabela de Vetores de Interrupção.
* Strings de texto (debug, senhas, etc.) embutidas.
* Espaços vazios (`0xFF`) e áreas de dados.

##  deliverables

* **Código-Fonte (Loader):** `src/loader/`
* **Código-Fonte (Payload):** `src/payload/`
* **Sistema de Build:** `cmake/`, `CMakeLists.txt`, e os linker scripts (`.ld`)
* **Ferramenta Host:** `scripts/client.py`
* **Relatório Técnico:** `README.md` (este arquivo)





## Uso (Exemplo)


Este projeto usa CMake e um Toolchain ARM Bare-Metal (`arm-none-eabi-gcc`).

### 1. Construção (Build)

A partir da raiz do projeto:

```bash
# Criar o diretório de build
mkdir build
cd build

# Configurar o CMake (apontando para o toolchain)
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake ..

# Compilar o projeto (Loader + Payload)
make  
```                              




## 🔐 Licença

Este projeto está sob a Licença MIT. Veja o arquivo LICENSE para mais detalhes.
