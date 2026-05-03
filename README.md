# Projeto Dolos — SWD Forensic Extractor

![Status](https://img.shields.io/badge/Status-Sprint_1_(Foundation)-blue)
![Platform](https://img.shields.io/badge/Platform-RP2040-c51a4a?logo=raspberry-pi)
![Language](https://img.shields.io/badge/Language-C_(bare--metal_híbrido)-00599C?logo=c)
![Toolchain](https://img.shields.io/badge/Toolchain-arm--none--eabi--gcc_13.2-orange)
![License](https://img.shields.io/badge/License-MIT-green)

> Ferramenta de aquisição forense bare-metal que extrai e valida o conteúdo da memória Flash de um RP2040 alvo via protocolo SWD, usando outro RP2040 como sonda.

---

## O que é este projeto

O **Dolos** transforma um Raspberry Pi Pico (Sonda) em um extrator forense capaz de adquirir **2 MB de firmware em menos de 60 segundos**, com validação criptográfica SHA-256 e registro estruturado da sessão, sem depender de nenhuma ferramenta de debug convencional (CMSIS-DAP, OpenOCD, picotool).

O diferencial não está apenas em implementar SWD sem biblioteca pronta. Está em tratar a extração como uma **"operação forense"**: integridade verificável, política explícita de não-modificação do alvo e evidência auditável do processo.

### Origem

Este projeto é continuação direta de uma fase de pesquisa onde o RP2040 foi controlado sem nenhuma abstração de SDK, boot manual, clock tree manual, GPIO e UART via MMIO direto. Essa fase estabeleceu a base técnica para as decisões de arquitetura aqui tomadas.

→ **[Ver Fase 0: uart-baremetal-rp2040](https://github.com/StheffannyNAlves/uart-baremetal-rp2040)**

---

## Estado atual — Sprint 1

O ambiente de build está configurado e a arquitetura do firmware está definida. O trabalho em andamento é a fundação da sonda: controle do pino RUN do alvo, USB CDC funcional e as primitivas físicas do protocolo SWD.

- [x] Ambiente CMake + TinyUSB configurado
- [x ] Kill switch: controle do pino RUN via registradores SIO diretos
- [x] USB CDC echo: validação do pipeline PC ↔ Sonda
- [ ] SWD PHY: `WriteBit`, `ReadBit`, `Turnaround`, `LineReset` — a validar no analisador lógico

---

## Arquitetura

O projeto adota uma arquitetura híbrida onde cada camada tem uma justificativa técnica explícita.

```text
┌─────────────────────────────────────────────────────────┐
│                     SONDA (RP2040)                      │
│                                                         │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │  SWD PHY    │  │  SWD Proto   │  │      DAP      │  │
│  │  (Custom C) │→ │  (Custom C)  │→ │  (Custom C)   │  │
│  │  bit-bang   │  │  pacotes/ACK │  │  reg. acesso  │  │
│  └─────────────┘  └──────────────┘  └───────────────┘  │
│           registradores SIO diretos (sem SDK)           │
│                                             │           │
│  ┌──────────────────────────────────────────▼────────┐  │
│  │              FSM de Aquisição                     │  │
│  │  BOOT → INIT → RESET → SWD → HALT → QSPI →       │  │
│  │  EXTRACT → FINALIZE → DONE                        │  │
│  └───────────────────────────┬───────────────────────┘  │
│                              │                          │
│  ┌───────────────────────────▼───────────────────────┐  │
│  │           TinyUSB CDC  (Pico SDK)                 │  │
│  └───────────────────────────┬───────────────────────┘  │
└──────────────────────────────│──────────────────────────┘
                               │ USB
┌──────────────────────────────▼──────────────────────────┐
│                   HOST (PC — Python)                    │
│         receiver.py → dump.bin → SHA-256 → PDF          │
└─────────────────────────────────────────────────────────┘
```

### Decisões de design

**Por que bit-banging em C e não PIO?**
O RP2040 tem PIO que poderia gerar o clock SWD com precisão de ciclo. A escolha pelo bit-banging foi deliberada: o objetivo é controlar e auditar explicitamente cada bit do protocolo. PIO delegaria esse controle para microinstruções, correto para produção, opaco para fins forenses.

**Por que registradores SIO diretos e não `gpio_put()`?**
`gpio_put()` faz leitura-modificação-escrita internamente. Uma interrupção do TinyUSB entre a leitura e a escrita corrompe o estado do pino silenciosamente. Os registradores `GPIO_OUT_SET`/`CLR` e `GPIO_OE_SET`/`CLR` do SIO são atômicos,uma escrita altera apenas os bits da máscara.

**Por que TinyUSB (SDK) é aceitável para transporte mas não para SWD?**
O critério é: *o código afeta o timing do protocolo?* TinyUSB opera no transporte após a extração, variação de milissegundos não quebra nada. Nos pinos SWD o timing é medido em microssegundos. 

**Por que controlar o pino RUN do alvo?**
Conexão sob reset garante que a sonda estabelece a sessão SWD antes de o alvo executar qualquer código, impedindo que o firmware do alvo reconfigure periféricos (incluindo QSPI) antes do acesso à flash estar preparado.

---

## Política de integridade — Safe-Read

A ferramenta opera em modo **Safe-Read forçado por software**: após a inicialização do alvo, qualquer tentativa de escrita na flash resulta em `ST_ABORT`. O sistema é projetado para **observar sem modificar**.

As únicas escritas no alvo são as necessárias para inicializar a interface QSPI, e ocorrem antes da ativação do Safe-Read. O dump é validado por SHA-256 e comparado com referência obtida via picotool oficial.

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
| SWCLK | `GP2` | Fio direto | `SWCLK` | Clock SWD — sempre saída |
| SWDIO | `GP3` | Resistor **330 Ω** série | `SWDIO` | Dados bidirecionais |
| RESET | `GP22` | Resistor **1 kΩ** série | `RUN` (pino 30) | Kill switch |
| GND | `GND` | Fio direto | `GND` | Referência comum — obrigatório |

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
