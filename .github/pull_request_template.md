
# Descrição da Modificação

[Explique tecnicamente o que foi alterado e qual problema isso resolve na arquitetura atual.]

## Checklist Arquitetural (Obrigatório)

Se o seu PR violar qualquer uma das regras de domínio abaixo, ele será rejeitado.

- [ ] **Isolamento do Core 1:** O código modificado no Core 1 introduz alguma interrupção, I/O bloqueante ou delegação ao compilador? (Se sim, justifique exaustivamente).
- [ ] **Timing SWD:** As alterações no Assembly puro afetam a contagem de ciclos (NOPs) do `writebit` ou `readbit`?
- [ ] **Acesso GPIO:** Os acessos físicos aos pinos continuam sendo feitos exclusivamente via bloco SIO (Single-cycle IO)?
- [ ] **Safe-Read:** A política de não-modificação da flash alvo (Safe-Read) foi rigorosamente mantida?

## Evidência de Validação

[Insira logs de execução, capturas de analisador lógico (ex: Hantek) ou saída estruturada do host Python que comprovem a estabilidade da alteração.]
