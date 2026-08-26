# Compatibilidade — conceito e implementação

Revisão de 11 ago. 2026. Este documento separa o que o instrumento **faz** do
que ele apenas **anuncia**. Uma função só é considerada ativa quando possui
rota de áudio/controle e controle acessível no painel.

| Campo | Conceito Antitotem | Estado no código | Leitura de performance |
| --- | --- | --- | --- |
| Energia própria | osciladores, clock e conexões como origem, sem MIDI | ativo | `ENERGIA`, cinco osciladores e clock interno; coerente |
| Scanner | 40106 → divisão/endereço → 4051 → CV | ativo como estudo | 16 passos, CV/AMP/FX/MUTE, loop, reset e métricas; não é alegação de reconstrução física |
| Matéria | resistor, diodo, capacitor, transistor e retorno mudam o sinal | ativo | seis portas de retorno, ganho limitado e saída protegida; coerente |
| Incerteza | gesto e consequência, não RNG opaco | parcialmente ativo | DERIVA, S&H e glitch são limitados/repetíveis; falta memória de topologia completa |
| Voz | formas contínuas, pulso e deriva | ativo | cinco OSC com forma, razão, mix e eixo X — OSC4 (4093 · 4020, sub/clock) e OSC5 (4046 · LM13600, heteródino/ring mod contra OSC A) prototipados no motor, testados, aprovados por escuta e integrados ao painel em 11 ago. 2026 |
| Filtro e contorno | filtro como corpo e ADSR como presença | ativo | LPF/BPF, CV, ATT/DEC/SUS/REL, linhas do ADSR alinhadas às do VCF; escuta humana ainda pendente |
| Espaço | não só estéreo: X/Y/Z como topologia | parcialmente ativo | X e mixer estéreo ativos; Y/Z ainda são proposta, não controle existente |
| Módulos de campo | LFO, ring, ruído, S&H, reverb, phaser, flanger | ativo | todos possuem DSP e controles no painel; LFO agora expõe seno/triângulo/pulso |
| Objeto 4 / modularidade | módulos pequenos conectáveis | parcialmente ativo | memória do mixer (4 de 8 slots do motor: `captureMixMemory`/`recallMixMemory`) exposta em 11 ago. 2026; `DualObjectEngine` (dois `SimpleSequencer` acoplados por rotas de feedback, testado) e `setExternalFeedbackAmount` (o fio entre eles) seguem só no motor — falta decidir com o autor como um segundo objeto sonoro inteiro cabe na superfície 1920×1080 antes de expor |
| Registro | take como parte da composição | ativo | REC WAV estéreo 24-bit, 1/3/5 min, metadados e estágios de faixa |
| Escuta e segurança | risco estético, não risco físico | ativo | teto técnico, teste de estresse e STOP silencioso; faltam estudos de escuta documentados |

## Prioridade de coerência

1. Não nomear como módulo histórico aquilo que é estudo autoral digital.
2. Antes de adicionar um CI, revelar e testar os controles dos módulos já
   ativos; o painel deve corresponder ao sinal que passa por ele.
3. Implementar Y/Z e patching como relações de rota, memória e proximidade —
   não como efeitos decorativos.
4. Validar cada avanço com dois critérios: teste técnico e estudo de escuta
   gravado em WAV.

## Camada de deriva e memória

`DERIVA` agora possui uma profundidade explícita e trabalha em segunda ordem:
a memória de frase desloca passos, efeitos, rotas e relações entre OSCs; a
intensidade desse deslocamento também se move de forma lenta e limitada entre
frases. Não há ruído de áudio como controle nem saltos de frequência sem
continuidade. A próxima extensão deve separar, no painel, as camadas de
deriva rítmica, tímbrica e topológica sem obrigá-las a compartilhar um único
ritmo.

## Próxima lacuna concreta

A direção do scanner já está materializada em `FWD`, `REV`, `ALT` e `MEM`.
`MEM` não é ruído opaco: escolhe endereços dentro do loop, sem repetir a etapa
anterior e a partir de estado interno reproduzível. A próxima lacuna é a
**memória de topologia por camada**; depois, três sequenciadores autônomos que
possam dialogar ou seguir independentes, com profundidades de deriva distintas.
