# Fluxo de sinal — Antitotem / Objeto Sonoro

Mapa completo, ponta a ponta, de como o áudio e o controle circulam dentro
do motor (`src/core/`). Escrito em 18 ago. 2026 depois de dois bugs reais
encontrados na cadeia de NOISE (ver `TAREFAS.md`) - existia documentação
parcial (`DESIGN.md`, "Mapa do painel Eurorack digital") mas nada cobrindo
do clock até a saída final em um único lugar.

Três partes, deliberadamente separadas (author, live, 18 ago. 2026: "eu
diferenciaria três coisas no documento"):

1. **Fluxo de áudio** — por onde o som efetivamente passa, em ordem, sem
   se preocupar ainda com bifurcação/convergência.
2. **Fluxo de controle/modulação** — CV, gates, triggers, envelopes: o que
   decide como esse som se comporta, não o som em si.
3. **Topologia de roteamento** — onde esses caminhos se bifurcam,
   convergem, realimentam ou mudam de destino. É a parte que separa "o
   sinal passa por aqui" de "desligar isto aqui desliga aquilo ali" - a
   distinção que os bugs do NOISE (ver `TAREFAS.md`) mostraram não ser
   óbvia só de olhar o fluxo de áudio sozinho.

## Parte 1 — Fluxo de áudio

Por objeto (`SimpleSequencer::renderSample()`), a ordem em que o som é
processado:

```text
5 OSC (CmosVoice::tickStereo)
   │  cada oscilador: FREQ (ratio) × EIXO X (pan) × EIXO Y (proximidade/
   │  filtro por voz) × EIXO Z (órbita) × MIX (nível) - somados em estéreo
   │  dentro do próprio CmosVoice
   ▼
voz (voiceLeft/voiceRight) × ADSR (voiceGain)
   │
   + ruído bruto × NOISE MIX × gain do canal NOISE do MIXER
   ▼
[ RING ] (RingModulator, portadora = LFO)
   ▼
[ 4069 · VCF ] → [ MaterialFilter · MAT ]
   ▼
[ PHASER ] → [ FLANGER ] → [ REVERB ] → [ RESONATOR ]   (ESPAÇO)
   │  cada estágio ganhado pelo FX send do step atual (effectSends[step])
   ▼
MIXER (MutableMixer, 4 canais: FILTRO/RING/NOISE/ESPAÇO)
   │  cada canal: ON/M/S, GAIN (0-1.5), PAN, RET
   ▼
mixedLeft/mixedRight
   ▼
SignalLeveler (nivelamento automático, não é um limiter agressivo)
   ▼
LinkedOverloadProtector (proteção vinculada L/R - teto técnico 0,851)
   ▼
OutputStage (aplica o gain "master" do PRÓPRIO objeto -
  SimpleSequencer::setMasterGain, distinto do MASTER geral - ver Parte 3)
   ▼
saída estéreo deste objeto (PRINCIPAL ou CLONE)
```

Entre dois objetos (`DualObjectEngine::render()`), depois de cada um gerar
sua própria saída acima:

```text
PRINCIPAL (saída própria) × gain/mute (MIXER OBJETOS) ─┐
CLONE (saída própria) × gain/mute (MIXER OBJETOS) ─────┼─► soma × 0.5
                                                         │
                                                         ▼
                                                  OutputStage final
                                                  (MASTER geral)
                                                         │
                                                         ▼
                                                  saída da placa
```

Este quadro é intencionalmente linear - onde cada trecho se bifurca,
converge ou realimenta está na Parte 3, não aqui.

## Parte 2 — Fluxo de controle / modulação

```text
CLOCK (40106) ──rate──► SCANNER (4040 + 4051, PERCURSO: FWD/REV/ALT/MEM)
                              │
                              ▼
                     avança currentStep (0-15)
                              │
              ┌───────────────┼────────────────┬───────────────┐
              ▼               ▼                ▼               ▼
       CV[step]          AMP[step]         FX[step]        MUTE[step]
    (tensão 0-1,      (escala voiceGain   (escala o SEND   (pula o disparo
     define o          junto com ADSR)     pro ESPAÇO -     do ADSR nesse
     fundamental                           PHASER/FLANGER/  step; scanner
     via CmosVoice)                        REVERB/          continua)
                                            RESONATOR)
```

```text
início de um step ativo (não mutado)
       │
       ▼
ADSR dispara (ContourEnvelope) ──contour──┬──► voiceGain (amplitude da voz -
                                            │    isso já É o VCA do instrumento,
                                            │    ver TAREFAS.md 18 ago. 2026)
                                            └──► filterCv += contour × 0.12
                                                 (ADSR também "encosta" no
                                                  CUTOFF do VCF, não só na
                                                  amplitude)

LFO (LfoSource, RATE + FORMA) ──lfoSample──► portadora do RING modulator
       │
       ├─ SEN/TRI/PUL: formas fixas de LFO
       └─ CAOS/VAGA: substituem a forma por ChaosField/WanderSource,
          com seus próprios DRIVE/DAMPING/DEPTH (coluna CAOS do painel)

ENERGIA ──► CmosVoice::energy: desloca o fundamental, a "supply" interna,
            outputGain e o drift relativo entre os 3 primeiros osciladores
            (não é um modelo elétrico real - é uma interpretação musical,
            ver DESIGN.md)

DERIVA (nível do app, fora do DSP - Main.cpp, não SimpleSequencer)
       │  a cada N steps, se armada (verde) e no play:
       └─► muta CV/AMP/FX de cada step lentamente, de forma determinística
           (gerador xorshift próprio, reproduzível - não é ruído de áudio)
           - também muta rotas (ver Parte 3, PORTAS DE FEEDBACK/CONEXÃO
           ENTRE OBJETOS), por isso DERIVA aparece nas duas partes.
```

## Parte 3 — Topologia de roteamento

Onde os caminhos da Parte 1 se bifurcam, convergem, realimentam ou mudam
de destino - a parte que decide se "desligar um canal" realmente
silencia algo, ou só uma parcela dele.

### 3.1 — MIXER: 4 pontos de escuta em série, não 4 fontes paralelas

RING alimenta FILTRO, que alimenta ESPAÇO, sempre, processando de
qualquer jeito. Cada canal ON/gain do MIXER controla só a contribuição
própria (soma mono + diferencial estéreo) daquele ponto à mixagem final -
não interrompe o que segue adiante:

- RING off + FILTRO on → ainda se ouve a modulação em anel (o filtro
  processa o sinal já modulado).
- FILTRO off + ESPAÇO on → ainda se ouve o sinal filtrado, via reverb/
  phaser/flanger/resonator.
- NOISE é a única exceção genuína: é uma injeção paralela na voz, não um
  elo da série, e desde 18 ago. 2026 seu canal no MIXER é um interruptor
  mestre real (ON/gain zeram totalmente a injeção antes do RING também,
  não só a soma mono do próprio canal - ver `TAREFAS.md`).

Deliberado, não uma falha - mesmo princípio de um envio pré-fader numa
mesa real; permite combinações como "só a reverberação do filtro, sem o
filtro seco por cima" (ver `DESIGN.md`, seção "Fluxo de sinal pós-voz",
para a discussão completa).

### 3.2 — PORTAS DE FEEDBACK internas (`CmosVoice::feedbackSample()`)

Convergência dentro de um único objeto: até 6 rotas combináveis (DIRETO,
RETIFICADO, CAPACITIVO, PULSO, TRANSISTOR, REFLUX - bits independentes,
não mutuamente exclusivos) somam a própria saída da amostra anterior de
volta na fase do OSC A da amostra seguinte, escalada por FB GAIN. Cada
rota transforma o retorno de um jeito diferente (direto, retificado,
memória capacitiva, pulso de limiar, ganho assimétrico tipo transistor,
ou uma mistura com a própria memória capacitiva) - ligar mais de uma ao
mesmo tempo tira a média das ativas, não soma tudo.

### 3.3 — RET do MIXER (`mixReflux`)

Segunda rota de retorno, distinta da anterior: cada canal do MIXER tem
seu próprio slider RET (0-0.72); a soma desses retornos vira `mixReflux`,
que entra como segundo parâmetro de `voice.tickStereo()` na amostra
seguinte - um retorno pós-processamento (depois de RING/FILTRO/ESPAÇO),
enquanto as PORTAS DE FEEDBACK (3.2) são um retorno pré-processamento (só
a voz crua). As duas convergem no mesmo lugar (`feedback` dentro de
`CmosVoice::tickStereo`), mas entram em pontos diferentes da cadeia.

### 3.4 — CONEXÃO ENTRE OBJETOS (`DualObjectEngine::render()`)

Bifurcação/convergência entre os dois objetos inteiros (PRINCIPAL e
CLONE), com um sample de atraso (não é instantâneo):

```text
                 ┌─────────────────────────────────────────┐
                 │                                          │
   AUX 1 ────────┼──► auxiliaryToFirst ──┐                  │
   (osc. auxiliar│                       ▼                  │
   alimentado    │              ┌─────────────────┐         │
   pelo nível de │   fromFifth ─┤  PRINCIPAL       │         │
   CLONE)        │  (via ROTA   │  (SimpleSequencer│         │
                 │   CLONE→     │   "first")       │──lastFirst──┐
                 │   PRINCIPAL: │  renderSample(    │         │  │
                 │   DIRETO/    │   ..., fromFifth  │         │  │
                 │   DIODO/CAP/ │   + auxA)          │         │  │
                 │   PULSO)     └─────────────────┘         │  │
                 │                                          │  │
   AUX 2 ────────┼──► auxiliaryToFifth ─┐                   │  │
   (osc. auxiliar│                      ▼                   │  │
   alimentado    │              ┌─────────────────┐         │  │
   pelo nível de │  fromFirst ──┤  CLONE           │         │  │
   PRINCIPAL)    │ (via ROTA    │  (SimpleSequencer│──lastFifth─┤
                 │  PRINCIPAL→  │   "fifth")       │         │  │
                 │  CLONE)      │  renderSample(    │         │  │
                 │              │   ..., fromFirst  │         │  │
                 │              │   + auxB)          │         │  │
                 │              └─────────────────┘         │  │
                 └────────────────◄───────────────────◄─────┘  │
                                  (lastFirst/lastFifth realimentam o
                                   próximo sample)                │
                                                                   │
   (segue para o MIXER OBJETOS e soma final - ver Parte 1)  ◄──────┘
```

Cada rota (PRINCIPAL→CLONE e CLONE→PRINCIPAL) é combinável entre DIRETO/
DIODO/CAP/PULSO, igual às PORTAS DE FEEDBACK internas (3.2), mas operando
entre objetos em vez de dentro de um só.

**Nota importante:** `lastFirst`/`lastFifth` (o que cada objeto manda pro
outro) **não** é afetado pelo M(ute)/gain do "MIXER OBJETOS" (Parte 1) -
mutar a saída audível de um objeto não silencia sua influência sobre o
outro via CONEXÃO ENTRE OBJETOS/rotas; são preocupações deliberadamente
separadas (ver comentário em `DualObjectEngine.h`). Outro caso do mesmo
princípio da seção 3.1: um controle "desliga" uma coisa específica, não
necessariamente tudo que parece relacionado a ele.

### 3.5 — DERIVA muta rotas, não só valores

DERIVA (Parte 2) não se limita a mudar CV/AMP/FX dos steps - também pode
mutar as PORTAS DE FEEDBACK (3.2) e as rotas de CONEXÃO ENTRE OBJETOS
(3.4) ao longo do tempo, proporcional a PROFUNDIDADE. Ou seja, a própria
topologia (não só os valores que correm por ela) pode mudar sozinha
enquanto DERIVA está armada.

---

`DESIGN.md` ("Mapa do painel Eurorack digital" e "Fluxo de sinal
pós-voz") mantém a leitura conceitual/histórica de cada seção; este
documento é a referência técnica de onde cada sinal realmente entra, sai
e se cruza - útil sobretudo antes de mexer em qualquer coisa que pareça
"não fazer diferença no som", como aconteceu com NOISE MIX (ver
`TAREFAS.md`, 18 ago. 2026).
