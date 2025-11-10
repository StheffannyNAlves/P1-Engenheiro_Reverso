# P1: Aquisição Forense Bare-Metal de FLASH (RP2040)

[![Status](https://img.shields.io/badge/Status-Em_Desenvolvimento-orange)]()
[![Plataforma](https://img.shields.io/badge/Plataforma-RP2040%20(Bare--Metal)-green)]()
[![Licença](https://img.shields.io/badge/Licença-MIT-blue)](LICENSE)

Ferramenta de aquisição forense de baixo nível para a memória FLASH do RP2040, desenvolvida sem o uso de abstrações do SDK.

---

## Objetivo

Na forense de hardware, a integridade da evidência começa no "metal". Abstrações de SDK (Software Development Kit) podem ser um véu que oculta malware de firmware ou bootloaders comprometidos.

Este projeto implementa um método de aquisição forense (dump) da memória FLASH de um dispositivo RP2040 contornando o SDK. O acesso é feito através de **Controle Direto via MMIO (Memory-Mapped I/O)**, interagindo diretamente com os registradores de hardware da SSI (Interface Serial Síncrona) e UART, conforme documentado no datasheet do processador.

O objetivo é garantir um dump bit-a-bit verdadeiro, estabelecendo o primeiro elo de uma cadeia de custódia confiável.

## Metodologia: Transparência e Controle DIreto via MMIO
Nessa seção explico por que contornar o uso de SDK(Integridade e Cadeia de Custódia), falo sobre o tamanho do firmware, pegada de memoria, contorno de proteções...



## Arquitetura

1. **Firmware Alvo (`src/main.c`):** Um pequeno firmware bare-metal  que é executado no RP2040. Ele comanda manualmente a FLASH externa via SSI, lê seu conteúdo e envia o *stream* de bytes brutos pela porta UART (exposta como USB Serial).
2. **Ferramenta Host (`tools/recv_dump.py`):** Um script Python  que roda no Host Linux do investigador. Ele ouve a porta serial, recebe o *stream* de bytes e o reconstrói em um arquivo binário (a imagem do firmware).
3. **Validação (`tools/verify_dump.py`):** Ao final da aquisição, o script host calcula o hash (SHA-256) da imagem recebida para validação de integridade.

### Fluxo de operação
Esse diagrama de fluxo mostra a interação entre o microcontrolador e o Host. O P1 é um sistema de duas partes: o Firmware de Aquisição(rodando no RP2040) e o Script de Análise(rodando no Host Linux). O firmware no Pico(main.c) atuará como "Alvo" e "Agente de Extração". O fluxo de dados depende de duas hierarquias Mestre-Escravo distintas:

1. **Camada USB (Host ↔ Alvo):** Comunicação entre o Host PC e o RP2040.
2. **Camada SPI (Alvo ↔ FLASH):** Comunicação entre o processador RP2040 e seu próprio chip de FLASH externa.

O objetivo do projeto é usar a Camada USB para comandar o RP2040 a executar uma extração forense de sua própria memória, utilizando a Camada SPI.

![Fluxo de Operação](diagrama.png)

#### Hierarquia SPI Mestre-Escravo

O núcleo deste projeto é o acesso **bare-metal** à FLASH externa.

- **Processador RP2040 (CPU):** atua como **MESTRE SPI**.  
- **Chip de FLASH Externa (W25Qxx):** atua como **ESCRAVO SPI**.

Normalmente, o SDK do Pico e o bootloader abstraem essa relação usando um modo chamado **XIP (eXecute-In-Place)**, que faz a FLASH parecer memória comum.  
Para uma aquisição forense, essa abstração não é aceitável, pois pode pular seções ou mascarar dados importantes.

Nosso fluxo de operação, portanto, ignora o XIP e implementa o protocolo Mestre-Escravo manualmente.



### Fluxo de Aquisição (Passo a Passo)

1. **Comando (Host PC):** O script Python envia um comando de `"iniciar dump"` para o RP2040 através do protocolo de dump forense.
2. **Recepção (RP2040):** O firmware `main.c`, rodando na RAM, recebe este comando.
3. **Acesso (RP2040 Mestre):** O firmware assume o controle direto do hardware SSI (controlador QSPI) do RP2040, escrevendo diretamente nos endereços de registradores mapeados em memória (MMIO).
4. **Comando (SPI):** O RP2040 (Mestre) envia comandos SPI brutos (ex.: `"Read Data" - 0x03`) para o chip de FLASH (Escravo), especificando o endereço de memória a ser lido.
5. **Resposta (SPI):** O chip de FLASH (Escravo) responde com os dados brutos pelo pino `MISO` (Master In, Slave Out).
6. **Retransmissão (RP2040):** O firmware intercepta esses dados brutos e os retransmite imediatamente para o Host PC através da Camada USB.
7. **Validação (Host PC):** O script Python recebe o fluxo de dados, reconstrói a imagem da FLASH e valida sua integridade.

---

Ao implementar diretamente o protocolo Mestre-Escravo do SPI, garantimos que o firmware está lendo a memória FLASH **byte por byte**, sem qualquer abstração, resultando em um **dump forense completo e confiável**.





## Uso (Exemplo)

```bash
# 1. Coloque o Pico em modo BOOTSEL e grave o firmware de aquisição
cp src/p1_dumper.uf2 /media/RPI-RP2

# 2. Inicie o script de recebimento no Host
python3 tools/recv_dump.py --port /dev/ttyACM0 --out firmware.bin --size 2097152

# 3. O script irá receber 2MB de dados e calcular o hash
> Recebendo 2097152 bytes de /dev/ttyACM0...
> ...
> Aquisição concluída.
> SHA256: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
> Salvo em: firmware.bin                                                                                                          
```

## 📋 Entregáveis do Projeto

[ ] Código-Fonte (src/main.c): Firmware de aquisição bare-metal com acesso direto MMIO.

[ ] Ferramenta Host (tools/): Scripts Python para receber, salvar e validar o dump.

[ ] Relatório Técnico (este README): Documentação da metodologia, cadeia de custódia e artefatos de boot encontrados.

## 🔐 Licença

Este projeto está sob a Licença MIT. Veja o arquivo LICENSE para mais detalhes.
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
