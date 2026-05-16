---
name: Relato de Bug (Dolos)
about: Falhas na FSM, extração corrompida ou travamentos físicos.
title: '[BUG] '
labels: bug
assignees: ''
---

## Contexto da Falha

Descreva de forma direta o comportamento anômalo observado durante a extração bare-metal.

## Dados do Módulo de Política de Erros (LUT)

Para que este bug seja analisado, forneça os dados exatos reportados pelo sistema:

- **Estado da FSM no momento da falha:** [ex: ST_HALT_CORE, ST_PREPARE_QSPI]
- **Código de Erro Emitido:** [ex: F003, M004, S101]
- **Ação Tomada pela LUT:** [ex: ACTION_FATAL_HALT, ACTION_ABORT]

## Log Estruturado

Cole o trecho relevante do log gerado pelo Buffer Circular no Core 0 (Python Host). Se houve travamento antes do acionamento do watchdog, especifique.

```json
[Cole o JSON do log estruturado aqui]
```

## Ambiente Físico

- **Frequência de operação do SWCLK testada:** [ex: 1 MHz, 500 kHz]
- **Meio físico:** [ex: Protoboard 1660 pontos, jumpers de 20cm]
- **Alimentação/USB:** [ex: Porta USB 3.0 direta na placa-mãe]
