# Projeto Dolos — SWD Forensic Extractor

![Status](https://img.shields.io/badge/Status-Sprint_1_(Foundation)-blue)
![Platform](https://img.shields.io/badge/Platform-RP2040-c51a4a?logo=raspberry-pi)
![Language](https://img.shields.io/badge/Language-C_(bare--metal_híbrido)-00599C?logo=c)
![Toolchain](https://img.shields.io/badge/Toolchain-arm--none--eabi--gcc_13.2-orange)
![License](https://img.shields.io/badge/License-MIT-green)

> Ferramenta de aquisição bare-metal que extrai e valida o conteúdo da memória Flash de um RP2040 alvo via protocolo SWD, usando outro RP2040 como sonda.

---

## O que é este projeto

O **Dolos** transforma um Raspberry Pi Pico (Sonda) em um extrator forense capaz de adquirir **2 MB de firmware em menos de 60 segundos**, com validação criptográfica SHA-256 e registro estruturado da sessão, sem depender de nenhuma ferramenta de debug convencional (CMSIS-DAP, OpenOCD, picotool).

O diferencial não está apenas em implementar SWD sem biblioteca pronta. Está em tratar a extração como uma **"operação forense"**: integridade verificável, política explícita de não-modificação do alvo e evidência auditável do processo.

### Origem

Este projeto é continuação direta de uma fase de pesquisa onde o RP2040 foi controlado sem nenhuma abstração de SDK, boot manual, clock tree manual, GPIO e UART via MMIO direto. Essa fase estabeleceu a base técnica para as decisões de arquitetura aqui tomadas.

→ **[Ver Fase 0: uart-baremetal-rp2040](https://github.com/StheffannyNAlves/uart-baremetal-rp2040)**

---

## Status

### Fase 1 — Concluída

Fundação da sonda validada: ambiente de build, firmware modular, USB CDC funcional e controle do alvo.

- [x] Ambiente CMake + TinyUSB configurado
- [x] Firmware inicial modular com FSM mínima (ST_BOOT → ST_IDLE)
- [x] Controle do pino RUN do alvo via GPIO
- [x] USB CDC enumerado e validado com echo PC ↔ Sonda
- [x] Comandos básicos de controle recebidos e respondidos via USB

### Fase 2 — Em andamento

Implementação da camada física SWD em assembly ARM via SIO e início da camada de protocolo.

- [x] `swd_phy_init` — inicialização de GPIO_OE e estado inicial dos pinos
- [x] `writebit` — transmissão determinística de 1 bit via SIO, timing calibrado por NOPs
- [x] `readbit` — leitura de 1 bit com chaveamento de direção via GPIO_OE
- [x] `turnaround_host_to_target` e `turnaround_target_to_host`
- [x] `line_reset` — 50 ciclos HIGH via loop assembly
- [x] `swd_enter_swd_mode` — sequência completa: line_reset → 0xE79E → line_reset → idle cycles
- [ ] Validação no analisador lógico
- [ ] Leitura do IDCODE
- [ ] Navegação DAP

---

## Arquitetura

O projeto adota uma arquitetura híbrida onde cada camada tem uma justificativa técnica explícita.

```mermaid
flowchart TD
    %% Estilização e formas semânticas para arquitetura multicore

    subgraph SONDA["Sonda RP2040 (Projeto Dolos)"]
        direction LR

        subgraph CORE1["⚡ CORE 1 (Domínio Crítico / Tempo Real)"]
            direction TB
            PHY{{"SWD PHY<br>(Assembly ARM)"}}
            PROT["Protocolo SWD / DAP<br>(C Bare-metal)"]

            PHY ==>|Timing determinístico| PROT
        end

        subgraph CORE0["CORE 0 (Domínio de I/O e Logs)"]
            direction TB
            BUF[/"Buffer Circular<br>de Logs em RAM"\]
            USB{{"TinyUSB CDC<br>(Pico SDK)"}}

            BUF -.->|Consumo Assíncrono| USB
        end

        SRAM[("SRAM Compartilhada<br>(FIFO HW)")]

        %% Conexões internas de sincronização
        PROT ===|Push de Ponteiros| SRAM
        SRAM ===|Pull de Ponteiros| BUF
    end

    ALVO["Alvo (RP2040)<br>Flash ROM / QSPI"]
    HOST["Host (PC - Python)<br>FSM Espelhada / PDF"]

    %% Conexões externas
    PROT ===|Barramento SWD| ALVO
    USB <==>|Transporte USB CDC| HOST

    %% Definição de Classes (Paleta Tech/Hardware)
    classDef sonda fill:#1E293B,stroke:#475569,stroke-width:2px,color:#F8FAFC,rx:10,ry:10
    classDef core1 fill:#450A0A,stroke:#EF4444,stroke-width:2px,color:#FECACA,stroke-dasharray: 5 5
    classDef core0 fill:#082F49,stroke:#0EA5E9,stroke-width:2px,color:#BAE6FD,stroke-dasharray: 5 5
    classDef nodeCrit fill:#7F1D1D,stroke:#FCA5A5,stroke-width:1px,color:#FFF
    classDef nodeIO fill:#0284C7,stroke:#7DD3FC,stroke-width:1px,color:#FFF
    classDef mem fill:#4B5563,stroke:#D1D5DB,stroke-width:2px,color:#FFF
    classDef ext fill:#111827,stroke:#10B981,stroke-width:2px,color:#FFF

    class SONDA sonda
    class CORE1 core1
    class CORE0 core0
    class PHY,PROT nodeCrit
    class BUF,USB nodeIO
    class SRAM mem
    class ALVO,HOST ext
```

### Decisões de design

**Por que bit-banging em Assembly e não PIO ou C?**
  O pino `SWDCLK` opera de forma simétrica com atrasos calculados ciclo a ciclo via `NOPs`. Implementar em C delegaria o timing ao compilador, gerando instabilidade. O PIO automatizaria o processo, mas a implementação em Assembly ARM puro garante o controle cirúrgico e o determinismo necessários para auditoria forense.
**Por que isolamento multicore (Core 1 vs Core 0)?**
  Rotinas de exibição ou stacks de comunicação (como TinyUSB) possuem latências imprevisíveis de milissegundos. No Core 1, o barramento roda com **zero interrupções** e acesso exclusivo aos pinos via bloco **SIO (Single-cycle IO)** conectado ao barramento IOPORT do Cortex-M0+, garantindo escritas atômicas e livres de jitter de crossbar.
**Por que o pino RUN é controlado sob Reset?**
  Ao segurar a linha `RUN` do alvo em LOW antes da inicialização, a sonda impede o processador alvo de executar código local malicioso ou de reconfigurar os registradores do subsistema QSPI, assumindo o controle do silício a partir de um estado virgem e previsível.

---

## Robustez e Tratamento de Erros (LUT)

O gerenciamento de falhas do sistema é centralizado em uma **Look-Up Table (LUT) estática** armazenada em Flash. Toda transição de erro na FSM passa obrigatoriamente pela função unificada `error_policy(code)`.

As falhas são categorizadas por severidade, disparando reações determinísticas imediatas:
-`ACTION_RETRY`: Reexecução de operações (ex: colisões de `ACK_WAIT`).
-`ACTION_ABORT`: Encerramento seguro da sessão preservando os dados coletados até o momento.
-`ACTION_FATAL_HALT`: Paralisação imediata da sonda com liberação do pino `RUN` do alvo em HIGH (mecanismo de *Safety Net* contra travamentos).

---

## Política de integridade: Safe-Read

A sonda opera sob um modelo de **Safe-Read forçado por software**. Após o handshake inicial e a configuração estrita do subsistema QSPI/XIP do alvo, o firmware bloqueia qualquer envio de payloads de escrita. Qualquer tentativa de desvio ou comando não autorizado aborta a sessão imediatamente (`ST_ABORT`), invalidando o token de sessão criptográfico gerado no boot.

---

## Hardware

### Componentes

- 2× Raspberry Pi Pico
- 1× Resistor 330 Ω
- 1× Resistor 1 kΩ
- Analisador lógico (ex: Hantek 6022BL com DSView)
- Protoboard e jumpers

### Ligação Sonda → Alvo

| Sinal | GPIO Sonda | Componente | Pino Alvo | Função |
| :------ | :---------- | :----------- | :---------- | :------- |
| SWCLK | `GP2` | Cabo JST SH 1.00mm | `SWCLK` | Clock SWD —> sempre saída |
| SWDIO | `GP3` | Cabo JST SH 1.00mm + Resistor **330 Ω** série | `SWDIO` | Dados bidirecionais |
| RESET | `GP22` | Resistor **1 kΩ** série | `RUN` (pino 30) | Kill switch |
| GND | `GND` | Cabo JST SH 1.00mm | `GND` | Referência comum —> obrigatório |

---

## Como compilar

### Requisitos

| Ferramenta | Versão |
| ------------ | -------- |
| `arm-none-eabi-gcc` | 13.2 |
| CMake | ≥ 3.13 |
| [Pico SDK](https://github.com/raspberrypi/pico-sdk) | em `PICO_SDK_PATH` |

### Build

```bash
git clone https://github.com/StheffannyNAlves/swd-forensic-extractor.git
cd swd-forensic-extractor/firmware

mkdir build && cd build
cmake ..
make -j$(nproc)
```

Para flashar: segure BOOTSEL no Pico sonda, conecte o USB e copie `dolos.uf2` para o drive que aparecer.

---

## Referências

- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf) — §2.3.1 (SIO), §4.7 (Watchdog)
- [ARM ADIv5 Architecture Specification](https://developer.arm.com/documentation/ihi0031/latest) — protocolo SWD
- [ARMv6-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0419/latest) — DHCSR, Cortex-M0+
- [TinyUSB](https://github.com/hathach/tinyusb) — stack USB CDC

---

*Desenvolvido por [Stheffanny N. Alves](https://github.com/StheffannyNAlves)*
