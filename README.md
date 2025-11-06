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
Esse diagrama de fluxo mostra a interação entre o microcontrolador e o Host. O P1 é um sistema de duas partes: o Firmware de Aquisição(rodando no RP2040) e o Script de Análise(rodando no Host Linux). O firmware no Pico(main.c) atuará como "Alvo" e "Agente de Extração". O fluxo de dados depende de duas hierarquias Mestre-Escravo distintas.
[!Fluxo de Operação](.../diagramap1.svg)
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
