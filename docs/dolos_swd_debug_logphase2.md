# Projeto Dolos — Log de Depuração SWD

**Fase 2 — Validação do IDCODE** **Período:** 10/06/2025

---

## Contexto

Sonda forense bare-metal baseada em RP2040 tentando se comunicar via SWD com outro RP2040 alvo. O sintoma inicial era ACK=7 (0b111) constante e IDCODE=0xFFFFFFFF em todas as tentativas.

---

## Problemas Identificados e Corrigidos

### 1\. Conexão física nos pinos errados

**Sintoma:** Zero atividade no SWCLK, IDCODE=0xFFFFFFFF.  
**Causa:** O cabo JST SH estava conectado nos pinos UART da sonda em vez dos pinos SWD corretos (GPIO2/GPIO3).  
**Correção:** Cabo movido para GPIO2 (SWCLK) e GPIO3 (SWDIO) da sonda, conectado ao conector JST SH de debug do alvo.  
**Justificativa:** O conector JST SH do Pico H expõe os pinos dedicados de debug do RP2040, é por ali que o DAP escuta SWD, não pelos GPIOs genéricos.

---

### 2\. Definições de pinos conflitantes entre arquivos

**Sintoma:** RUN do alvo nunca subia, alvo preso em reset.  
**Causa:** `board_config.h` definia `PIN_TARGET_RUN = 22` enquanto `target_ctrl.h` definia `PIN_RUN = 14`. O pino fisicamente conectado ao RUN do alvo era o GPIO14, mas `platform_init` configurava o GPIO22 — que não estava conectado a nada.  
**Correção:** Unificação das definições com `PIN_TARGET_RUN = 14` em `board_config.h`. Remoção de `PIN_RUN` do `target_ctrl.h`.  
**Justificativa:** Dois arquivos definindo o mesmo conceito com valores diferentes é um bug silencioso, o código compila normalmente mas o hardware errado é controlado.

---

### 3\. `target_reset_low` e `target_reset_high` controlavam o pino errado

**Sintoma:** RUN do alvo permanecia em 0V mesmo após correção das definições.  
**Causa:** As funções `target_reset_low` e `target_reset_high` manipulavam `PIN_RST` (GPIO22) e `PIN_PROBE_LED`, nunca tocando no `PIN_RUN` (GPIO14) que controla o RUN físico do alvo.  
**Correção:**

void target\_reset\_low(void) {

    GPIO\_OUT\_CLR \= (1 \<\< PIN\_RUN);

    GPIO\_OUT\_CLR \= (1 \<\< PIN\_PROBE\_LED);

}

void target\_reset\_high(void) {

    GPIO\_OUT\_SET \= (1 \<\< PIN\_RUN);

    GPIO\_OUT\_SET \= (1 \<\< PIN\_PROBE\_LED);

}

**Justificativa:** O pino RUN do RP2040 controla o estado de reset do chip inteiro. Com RUN em LOW, o DAP não inicializa e não responde nenhuma transação SWD.

---

### 4\. Instrução STR faltando no `swd_phy_init`

**Sintoma:** SWDIO não iniciava em HIGH corretamente.  
**Causa:** O assembly carregava o offset `0x14` (GPIO\_OUT\_SET) em `r5` mas nunca executava o `STR` correspondente. O `POP` logo após destruía o valor carregado.  
**Correção:**

MOVS r2, \#(1 \<\< DOLOS\_PIN\_SWDIO)

MOVS r5, \#0x14

STR r2, \[r1, r5\]    @ GPIO\_OUT\_SET — SWDIO HIGH  ← instrução que faltava

POP {r4, r5, pc}

**Justificativa:** Sem o STR, o SWDIO nunca era posto em HIGH pelo assembly. O estado idle do SWD exige SWDIO em HIGH — linha em LOW indica erro ou ausência de dispositivo.

---

### 5\. Ordem errada nos turnarounds

**Sintoma:** ACK=7 constante mesmo com hardware correto.  
**Causa:** O `turnaround_host_to_target` liberava o SWDIO (OE\_CLR) antes de pulsar o clock, e havia um OE\_CLR duplicado no final. O `turnaround_target_to_host` original fazia o OE\_SET após o clock em vez de antes.  
**Correção — `turnaround_host_to_target`:**

1\. SWCLK HIGH

2\. SWCLK LOW

3\. OE\_CLR (libera SWDIO após o clock descer)

**Correção — `turnaround_target_to_host`:**

1\. OE\_SET (assume SWDIO antes do clock)

2\. SWCLK HIGH

3\. SWCLK LOW

**Justificativa:** No protocolo SWD, o clock é o árbitro da transferência de controle da linha. Quem libera, libera após o clock. Quem assume, assume antes do clock. Isso garante que nunca há dois drivers simultâneos na linha e que o alvo tem referência temporal exata de quando a sonda parou de transmitir.

---

### 6\. Sequência de reset na FSM invertida

**Sintoma:** Alvo saía do reset antes da sequência SWD ser enviada.  
**Causa:** O `target_reset_high()` estava sendo chamado no `ENTER_SWD` antes do `app_enter_swd()`. O alvo já estava rodando quando a sonda tentava entrar no modo SWD.  
**Correção:** `target_reset_high()` movido para após o `app_enter_swd()`:

TARGET\_HOLD\_RESET → target\_reset\_low()

ENTER\_SWD → app\_enter\_swd() → target\_reset\_high() → sleep\_ms(2)

READ\_IDCODE → swd\_read\_idcode()

**Justificativa:** A sequência foi alterada para manter o alvo em reset durante a entrada no modo SWD.
A expectativa é que o DAP esteja disponível imediatamente após a liberação do RUN, reduzindo possíveis condições de corrida entre a inicialização do alvo e a tentativa de comunicação da sonda.
Resultado definitivo ainda pendente de validação através da leitura bem-sucedida do IDCODE.

---

### 7\. Ausência do delay após `target_reset_low`

**Causa:** Sem delay entre derrubar o RUN e iniciar a sequência SWD, o alvo não tinha tempo de estabilizar.  
**Correção:** `sleep_ms(10)` adicionado no estado `TARGET_HOLD_RESET` após `target_reset_low()`.  
**Justificativa:** Foi adicionado um atraso de segurança após a ativação do reset para garantir a estabilização do alvo antes do início das transações SWD.
A necessidade exata desse atraso ainda será confirmada experimentalmente.

---

## Adições — 10/06

### A. `swd_write` — Escrita genérica no DP/AP

**Motivação:** O `swd_read` já existia, mas sem `swd_write` não era possível enviar o `TARGETSEL` nem qualquer escrita de registrador.

**Fluxo:**

send\_request → turnaround\_host\_to\_target → lê ACK (3 bits)

→ se ACK OK: turnaround\_target\_to\_host → write\_data (32 bits) → writebit(paridade)

→ retorna ACK

**Diferença chave em relação ao `swd_read`:** Após o ACK, no read o alvo envia dados. No write, a sonda envia dados. Por isso o `turnaround_target_to_host` acontece antes dos dados no write, e após os dados no read.

---

### B. `swd_write_targetsel` — Seleção de alvo no multidrop

**Motivação:** O RP2040 implementa SWD v2 multidrop. Sem o `TARGETSEL`, o DAP permanece dormente e não responde nenhuma transação — incluindo leitura do IDCODE.

**TARGETID do RP2040:** `0x01002927`  
**Endereço do registrador TARGETSEL:** `0x0C` → A\[3:2\] \= `11` → `a2=1, a3=1`  
**Header resultante:** `0x99`

**Peculiaridade:** O `TARGETSEL` não retorna ACK. O alvo mantém a linha em alta impedância durante os 3 ciclos de ACK propositalmente — é parte do protocolo multidrop para evitar colisões quando múltiplos dispositivos estão no barramento. A sonda fornece o clock, lê o "vazio" e descarta.

**Fluxo:**

send\_request(0, 0, 1, 1\) → turnaround\_host\_to\_target

→ 3x readbit() descartado → turnaround\_target\_to\_host

→ write\_data(0x01002927) → writebit(paridade)

---

### C. `swd_wake_dormant` — Saída do estado dormente

**Motivação:** Após reset, o DAP do RP2040 pode inicializar em dormant state, um estado de isolamento total onde o dispositivo ignora completamente o barramento, incluindo line\_reset e JTAG-to-SWD. A persistência do ACK=7 após a implementação do TARGETSEL levantou a hipótese de que o DAP possa estar inicializando em dormant state.

**Por que existe o dormant state:** Em sistemas multidrop, múltiplos dispositivos compartilham o barramento. O dormant state garante isolamento total até que o dispositivo receba explicitamente a sequência de ativação correta, impossível de ocorrer acidentalmente.

**Sequência completa:**

1. **8 bits LOW** — aborta transições pendentes  
2. **128 bits da Selection Alert Sequence** — magic word definida pelo ARM, escolhida por propriedades de autocorrelação que a tornam inconfundível mesmo em transmissão parcial  
3. **4 bits LOW** — break de protocolo  
4. **4 bits do activation code** — `0x1` LSB first (`1, 0, 0, 0`) para selecionar SWD

**Selection Alert Sequence:**

static const uint8\_t alert\_seq\[16\] \= {

    0x92, 0xf3, 0x09, 0x62, 0x95, 0x2d, 0x85, 0x86,

    0xe9, 0xaf, 0xdd, 0xe3, 0xa2, 0x0e, 0xbc, 0x19

};

---

## Sequência de inicialização SWD completa — estado atual

swd\_wake\_dormant()       ← acorda o DAP do dormant state

line\_reset()             ← 50+ bits HIGH, reseta o estado do protocolo

0xE79E (16 bits)         ← seleciona SWD sobre JTAG

line\_reset()             ← confirma seleção

2x idle                  ← ciclos de guarda

swd\_write\_targetsel()    ← seleciona Core 0 do RP2040 no multidrop

2x idle                  ← ciclos de guarda

swd\_read\_idcode()        ← lê e valida 0x0BC12477

---

## Estado atual

### Confirmado

- Conexões SWD revisadas e corrigidas
- Controle do pino RUN validado
- Sequência de turnaround revisada
- FSM de inicialização corrigida
- Implementação de TARGETSEL concluída
- Implementação de dormant wake concluída

### Ainda não confirmado

- Saída correta do estado dormant
- Seleção efetiva do alvo via TARGETSEL
- Resposta válida do DAP
- Leitura válida do IDCODE

### Sintoma atual

- ACK = 0b111 (7)
- IDCODE = 0xFFFFFFFF

### Próximo passo

Validar o comportamento após a execução da sequência de dormant wake e verificar se ocorre transição para ACK válido durante a leitura do IDCODE.
