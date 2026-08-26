# Pesquisa: acentuação como sistema multidimensional

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-ACC-001` nesse arquivo. Origem: terceiro brief colado
ao vivo pelo autor (19 ago. 2026), mesma fonte dos dois anteriores ("mais
um diálogo com o chatgpt"), desta vez sobre acentuação. Irmão direto de
`CRI-MEL-001` e `CRI-SEQ-001` - acento é o eixo em que os dois já se
encontram (o brief do CRI-SEQ-001 já dizia "acento do step → articulação",
prevendo esta conexão antes deste documento existir).

## 1. Por que este documento existe

O ANTITOTEM já tem um recurso chamado ACENTO (a UI ainda usa o nome
histórico MÉTRICA internamente em alguns pontos - ver `TAREFAS.md`), mas
ele é hoje **uma única dimensão**: um multiplicador de ganho, binário por
posição métrica (`currentStep % metricBeats == 0`), sem variação
independente de timbre, articulação, altura ou estrutura de frase. O brief
do autor argumenta que isso é exatamente o "botão ACCENT" genérico que
vale a pena questionar - acento como **sistema multidimensional**, não
como aumento de velocity. Antes de mexer em código, o funil de criação
pede verificar o que já existe no projeto (I2 - "separar referência
primária, interpretação, lembrança e especulação").

## 2. O que já existe no projeto RASGO sobre este assunto

Varredura (`grep -rn accent`) em ANTITOTEM, RASGO_SYNTH e AQUORBIUM antes
de escrever qualquer coisa nova.

### 2.1 ANTITOTEM - estado atual do próprio ACENTO

`src/core/SimpleSequencer.cpp`, `renderSample()`:

```cpp
constexpr float weakAccentAtUnit1 = 0.65f, weakAccentAtUnit16 = 0.35f;
const auto unitPhase = (static_cast<float>(metricUnit) - 1.0f) / 15.0f;
const auto metricAccent = currentStep % metricBeats == 0
    ? 1.0f
    : weakAccentAtUnit1 - unitPhase * (weakAccentAtUnit1 - weakAccentAtUnit16);
const auto voiceGain = 0.42f * contour * playGate * levels[currentStep] * metricAccent;
```

Isto é **puramente família 1 (dinâmico)** do brief (seção 3.1) e **puramente
família 2 (métrico)** como fonte - um único float multiplicando ganho,
ligado só à posição no compasso. Nenhuma das outras famílias (agógico,
tímbrico, articulatório, de altura, estrutural) participa hoje. Vale notar
que EXCITAÇÃO (mesmo projeto, ver `CRI-MEL-001`) já tem, sem ter sido
nomeado assim na hora, um pedaço de família 5 (articulatório): o dip de
`excitationArticulation` num salto grande já é, na terminologia deste
brief, um "acento articulatório" (uma pequena rearticulação antes do
salto) - implementado antes deste documento existir, por um caminho
diferente (legato/conexão, item #13 do `CRI-MEL-001`), mas é o mesmo
conceito.

### 2.2 RASGO_SYNTH - prior art de código, já implementado

- `rasgo-synth-core/src/dsp/AcidBasslineVoice.hpp` - acento estilo TB-303:
  binário (`bool accent`), mas genuinamente **multi-parâmetro** - o mesmo
  flag aumenta ganho (`0.6 + 0.4 * accentAmount_`) E aprofunda o filtro
  (`envDepthHz_ * filterEnv_ * accentAmount_`). É um precedente real,
  simples, de família 1 (dinâmico) + família 4 (tímbrico) combinadas a
  partir de UMA causa, exatamente o que o brief chama de "acento
  resultante de múltiplas famílias".
- `rasgo-synth-core/src/engine/RhythmGenerator.hpp` - `longAccent`: marca
  o início de cada grupo "longo" (3) num ritmo aksak (aditivo, tipo
  3+3+2). Implementação real de **família 7 (estrutural)** via
  agrupamento - o mesmo conceito do item 19 do brief ("accent groups").
- `rasgo-synth-core/src/engine/DrumArchetypes.hpp` - backbeat (acento só
  em 2 e 4), acento pesado do surdo no tempo 2 (samba), acentos
  sincopados de caixa/clap fora do tempo. Estes são padrões fixos
  compostos à mão, não generativos - prior art de **família 2 (métrico)**
  e do item 10 do brief ("syncopation accent"), mas sem nenhum mecanismo
  probabilístico ou paramétrico por trás.
- `rasgo-synth-core/src/engine/PieceRenderer.hpp` - o acento do
  `AcidBasslineVoice` é sorteado por passo (`unit(rng) < 0.25`, condicional
  a já haver nota no passo) - implementação real, ainda que simples, de
  **item 2 do brief (accent probability)**.

### 2.3 AQUORBIUM - o achado mais forte desta pesquisa

`src/core/GeometricSequencer.h`/`.cpp`: `Step::accent` já é **contínuo
(0-1), não binário**:

```cpp
result.accent = gate ? std::clamp(
    (orbitStep == 0 ? 0.92f : 0.58f) + cohesion * 0.18f + (mutated ? 0.08f : 0.0f),
    0.0f, 1.0f) : 0.0f;
```

- posição inicial da órbita geométrica pesa mais (0.92 vs. 0.58) -
  família 2 (métrico/estrutural) via geometria, não grade fixa;
- coerência do sistema (`cohesion`) empurra o valor pra cima -
  aproximação real do que o brief chama "accent by phrase position"
  (item 16), só que a partir de um estado do sistema, não de uma frase
  contada;
- mutação do passo soma um pouco - accent como sinal de "isto é um
  evento incomum", família 7 (estrutural).

Mas o achado mais importante está em `src/core/BiomaEngine.cpp`: este
único valor contínuo `geometricAccent` (via `AutonomousEnergy.h`) é
propagado como **entrada de várias famílias simultaneamente** - não é só
ganho:

```cpp
result.hardness = std::clamp(0.08f + accent * 0.52f + desire.danger * 0.20f + ..., 0.0f, 1.0f);   // timbre/físico
result.particleDensity = std::clamp(0.10f + ... + accent * 0.18f, 0.0f, 1.0f);                       // timbre/granular
result.irregularity = std::clamp(0.08f + ... + accent * 0.08f, 0.0f, 1.0f);                          // articulação
// e, em BiomaEngine.cpp:681-723, o mesmo accent também escala a excitação
// de três motores físicos diferentes (resonator/granular/string)
```

Ou seja: o AQUORBIUM **já tem, em produção, uma versão embrionária exata
do "Accent Field"** que o brief do autor propõe como conceito novo na
seção final - uma única camada contínua de energia da qual vários
parâmetros (dureza, densidade granular, irregularidade, intensidade de
excitação) herdam graus diferentes de ênfase, em vez de um botão ACCENT
isolado. Isto não estava nomeado como "Accent Field" no código-fonte, mas
é a mesma arquitetura, escrita antes deste documento.

### 2.4 Instrumentos irmãos sem material equivalente

TRIOIO, NAVALHA2_JUCE, NAVALHA2_PD, RASGO_MODULAR - varredura
rápida sem ocorrências de peso sobre acentuação além do já listado.

## 3. O brief do autor (fonte secundária/interpretativa, não primária)

Colado ao vivo (19 ago. 2026), mesma origem dos dois anteriores. Resumo
das partes estruturais (o texto completo do autor tem também 25 técnicas
numeradas de aplicação, preservadas na íntegra na mensagem original, não
reproduzidas aqui ponto a ponto por brevidade - ver histórico da
conversa):

- **Sete famílias de acento**: dinâmico, métrico, agógico (duração/
  antecipação/prolongamento), tímbrico, articulatório, de altura
  (registro/salto/ápice/desvio de pitch), estrutural (importância na
  frase, independente da grade).
- **Dados por passo propostos**: `accent_strength` (contínuo 0-1, não
  binário), `accent_probability`, `accent_density` (média numa janela),
  `accent_pattern` (máscara própria, comprimento independente do padrão
  de notas), **hierarquia de acentos** (níveis 0-3: neutro/secundário/
  forte/estrutural, não só ligado/desligado), `accent_contour` (curva ao
  longo da frase), `accent_rotation`, `accent_drift` (desloca lentamente
  entre passos ao longo de vários ciclos), `accent_displacement`
  (antecipação/atraso do acento esperado), acento por sincopação,
  antecipação, atraso, por intervalo (salto grande → mais chance de
  acento), por registro (extremos → mais peso), por repetição, por
  posição na frase, **herança** (um acento forte contamina o(s) passo(s)
  seguinte(s)), **supressão** (período refratário depois de um acento
  forte), grupos assimétricos (3+3+2, 2+2+3), gramática de acento
  (probabilidade condicional ao acento anterior), mutação, morphing entre
  dois padrões de acento, entropia (previsibilidade global), tensão
  (intensidade + proximidade + antecipação + síncope, separada de
  entropia), cross-accent (uma voz acentua em relação a outra).
- **Estrutura de dado por passo proposta**: `accent_strength`,
  `accent_probability`, `accent_type`, `accent_timing`, `accent_duration`,
  `accent_timbre`, `accent_articulation`, `accent_source` (de onde veio:
  `METRIC | PHRASE | INTERVAL | RANDOM | GROOVE | EXTERNAL | POLYRHYTHM |
  STRUCTURAL`) - permitindo **combinar causas** (ex.: passo metricamente
  fraco, mas ápice melódico E acento polirrítmico coincidentes).
- **Conceito final, o mais ambicioso**: "Accent Field" - uma camada
  contínua de energia/acento sobre o tempo, da qual eventos herdam graus
  diferentes de ênfase, fazendo acentuação, groove, dinâmica e fraseado
  "conversarem" em vez de serem parâmetros isolados. **Já existe uma
  versão real disso em produção no AQUORBIUM** (ver 2.3) - o autor propôs
  o conceito sem saber que um instrumento irmão já implementa uma forma
  dele.

## 4. Referências acadêmicas reais (fonte primária, verificadas por conhecimento próprio - não busca ao vivo nesta rodada)

- **Lerdahl, F. & Jackendoff, R. (1983). *A Generative Theory of Tonal
  Music*.** MIT Press. A raiz acadêmica real da taxonomia do brief: GTTM
  já distingue **acento métrico** (posição na hierarquia regular de
  pulsos), **acento estrutural** (importância de um evento na estrutura
  da peça, independente da grade) e **acento fenomenal** - um "guarda-
  chuva" para qualquer evento de superfície que produza ênfase (mudança
  súbita de dinâmica, duração, registro, harmonia, timbre, ataque). As
  "sete famílias" do brief (dinâmico/agógico/tímbrico/articulatório/de
  altura) são, na prática, uma subdivisão explícita do que GTTM já chamava
  genericamente de acento fenomenal - um movimento comum e legítimo em
  pesquisa de performance (nomear as partes de uma categoria "catch-all").
- **Acento agógico** - termo clássico e padrão de teoria musical (ênfase
  por duração/prolongamento; a tradição costuma atribuir a formalização a
  Hugo Riemann, séc. XIX), não um termo inventado pelo brief - confirma
  que a família 3 do brief tem base real e antiga, não é neologismo.
- **Drake, C. & Palmer, C. (1993). "Accent structures in music
  performance."** *Music Perception*, 10(3). Estudo empírico real
  distinguindo acento melódico (contorno/altura), acento harmônico e
  acento métrico como estruturas separadas que o performer combina e
  expressa via duração e intensidade - precedente acadêmico direto para
  a ideia do brief de "múltiplas famílias coincidindo num mesmo evento"
  (accent_source combinando METRIC + INTERVAL + STRUCTURAL, seção 3).
- **Huron, D. (2006). *Sweet Anticipation: Music and the Psychology of
  Expectation*.** MIT Press. Síncope como violação deliberada de
  expectativa métrica (teoria ITPRA); base real para os itens 10-12 do
  brief (syncopation accent, anticipation, delayed accent) e para o
  conceito de "tension" (item 24) como algo distinto de entropia -
  tensão vem de expectativa violada, entropia vem de imprevisibilidade
  estatística; são eixos diferentes mesmo quando correlacionados.
- **Já citadas no projeto, reaproveitadas aqui sem repetir a citação
  completa** (ver `CRI-SEQ-001`/`PESQUISA_SEQUENCER_GENERATIVO.md`):
  Toussaint (2005, ritmos euclidianos) e London (2004, métrica não-
  isócrona) - relevantes para "accent groups" (item 19, grupos 3+3+2,
  já real em `RhythmGenerator.hpp::longAccent`, ver 2.2); Shannon (1948,
  entropia) - relevante para "accent entropy" (item 23), já implementado
  como técnica (não como acento especificamente) em `CoherenceCollapse.hpp`.

**Nota de proveniência**: as quatro referências novas desta seção (GTTM,
agógico, Drake & Palmer, Huron) foram citadas por conhecimento próprio do
modelo (Claude), não verificadas via busca web nesta sessão - mesma
ressalva já registrada em `PESQUISA_MELODIA_GENERATIVA.md`, seção 4. Se
citações exatas (edição/página/DOI) importarem para uso formal, precisam
de verificação externa antes.

## 5. Estado atual vs. o brief

| Conceito do brief | Estado no RASGO |
|---|---|
| Família 1 - dinâmico | **Feito** (ANTITOTEM, `metricAccent` multiplica ganho) e **feito de outra forma** (RASGO_SYNTH `AcidBasslineVoice`, binário) |
| Família 2 - métrico | **Feito** (ANTITOTEM, posição no compasso) e **feito de forma fixa/não generativa** (DrumArchetypes, backbeat/samba) |
| Família 3 - agógico | **Não feito** em nenhum instrumento RASGO hoje |
| Família 4 - tímbrico | **Feito** parcialmente (AcidBasslineVoice, filtro; AQUORBIUM `hardness`/`particleDensity`) |
| Família 5 - articulatório | **Feito** por outro caminho (EXCITAÇÃO `excitationArticulation`, dip proporcional ao salto - não nomeado como "acento" na hora) |
| Família 6 - de altura/registro | **Não feito** como acento; existe correlato não-relacionado (item #10 do `CRI-MEL-001`, timbre por registro, ainda não implementado) |
| Família 7 - estrutural | **Feito** parcialmente (AQUORBIUM `mutated`, `orbitStep==0`; RASGO_SYNTH `longAccent` por grupo aksak) |
| Accent strength contínuo (não binário) | **Feito** (AQUORBIUM `Step::accent`, 0-1 real) e **feito no ANTITOTEM** (19 ago. 2026) - tempo forte continua exatamente 1.0 (sem regressão no pico já afinado ao vivo), mas o tempo fraco agora soma até +0.12 do `instability` (Noise Field, já compartilhado) em cima da resposta por `metricUnit`, deixando de ser função só da posição/denominador - mesmo princípio do `Step::accent` (base por posição + contribuição contínua de estado real do sistema) |
| Accent probability | **Feito** (RASGO_SYNTH `PieceRenderer.hpp`, acid accent 25%) |
| Accent groups (3+3+2 etc.) | **Feito** (RASGO_SYNTH `RhythmGenerator.hpp::longAccent`) |
| Accent Field (camada única alimentando vários parâmetros) | **Feito, sem o nome** (AQUORBIUM `geometricAccent` → hardness/density/irregularity/excitação, ver 2.3) - **não feito** no ANTITOTEM |
| Hierarquia de níveis (0-3) | **Não feito** em nenhum instrumento (tudo hoje é contínuo OU binário, nunca em níveis nomeados) |
| Density, pattern, rotation, drift, displacement, inheritance, suppression, grammar, mutation, morphing, entropy, tension, cross-accent | **Não feito** em nenhum instrumento RASGO hoje |

## 6. Próximos passos sugeridos (não decididos - aguardando o autor)

Mesma lição dos dois documentos irmãos: o valor não está em implementar
os 25 itens, está em escolher poucos que já se pagam com pouco código,
priorizando o que **já tem precedente real** noutro instrumento RASGO
(menor risco, caminho já testado) sobre o que seria inteiramente novo:

1. **Accent strength contínuo no ANTITOTEM** - trocar o `metricAccent`
   binário por algo mais próximo do `Step::accent` do AQUORBIUM (0-1 real,
   combinando posição + algum sinal de estado do sistema). Caminho mais
   barato: já existe o precedente de código pronto pra adaptar.
2. ~~Acento como fonte de EXCITAÇÃO~~ - **feito** (19 ago. 2026).
   `SimpleSequencer::isMetricAccentStep()` novo (read-only, mesmo teste
   que já dava o pico do `metricAccent`), lido em `DualObjectEngine::
   render()` com detecção de borda (dispara uma vez por tempo forte, não
   a cada sample que ele permanece verdadeiro) - soma +0.03 direto em
   `excitationActivity`, competindo/somando com o estímulo de energia
   bruta pelo MESMO comparador de limiar, não um segundo mecanismo
   paralelo. Realiza a ponte que o autor já tinha traçado no brief do
   `CRI-SEQ-001` ("acento do step → articulação").
3. **Accent Field explícito** - **feito, os dois destinos** (19 ago.
   2026). Articulação: `metricAccent` soma em `filterCv` (o mesmo CV que
   `contour` já modula) - tempo forte abre um pouco mais o filtro, tempo
   fraco fecha um pouco. Timbre: autor perguntou "podemos agir no item 1
   também (sugira algo)" - sugerido reaproveitar `setOscillatorShape`,
   mesmo mecanismo do #10 da EXCITAÇÃO; autor confirmou "sim".
   Implementado com cuidado pra não brigar com o knob FORMA-A do próprio
   usuário: `SimpleSequencer::setOscillatorShape(0, v)` agora guarda `v`
   em `oscillatorShapeBaseA` (a base do usuário) em vez de aplicar direto
   - `renderSample()` soma `(metricAccent-0.65)*0.25` em cima dessa base
   a cada sample, antes de chamar `voice.setOscillatorShape(0, ...)` de
   verdade. Faixa bem mais contida que a da EXCITAÇÃO (±0.25, não o
   0-3 inteiro) - um tempero no timbre já afinado de PRINCIPAL/CLONE,
   não uma reescrita.
4. ~~Herança de acento~~ - **feito** (19 ago. 2026). `accentTail` novo:
   ao pousar num tempo forte, vira 0.3; a cada passo seguinte, cai pela
   metade em vez de zerar - soma no ramo do tempo fraco (nunca no forte,
   que já é 1.0 fixo), lendo como uma cauda dinâmica real de 2-3 passos
   depois de cada tempo forte, não um segundo acento.
5. ~~Rotação de acento~~ - **feito** (19 ago. 2026). `accentRotation`
   novo: desloca QUAL passo conta como forte, `metricBeats` (o tamanho
   do ciclo) nunca muda - velocidade escalada por `instability` (no
   repouso típico, ~0.2, gira uma vez a cada 20-30s, estimativa). Também
   a primeira sobreposição real com `PESQUISA_DERIVA_GENERATIVA.md`
   ("deriva métrica" - "o centro perceptivo dos acentos muda
   lentamente... sem necessariamente mudar o tamanho do ciclo", quase
   palavra por palavra o que foi implementado aqui) - mesmo mecanismo
   serve aos dois documentos.
6. O restante (hierarquia de níveis, displacement, supressão -
   pressupõe acento probabilístico, que ACENTO não é hoje -, gramática,
   morphing, entropia/tensão como eixos nomeados, cross-accent) fica
   registrado, sem prioridade definida - mesmo tratamento dado ao
   excedente do `CRI-SEQ-001`.

## 7. Calibração pendente (19 ago. 2026)

Ver a nota geral em `PESQUISA_MELODIA_GENERATIVA.md`, seção 7. Todas em
`SimpleSequencer.cpp`, salvo indicação contrária:

- Contribuição do `instability` no tempo fraco: +0.12 - único valor
  desta pesquisa nunca testado ISOLADAMENTE (a rodada de "não percebi as
  diferenças" testou timbre/articulação, não este termo específico,
  dependente de o Noise Field ter realmente se movido durante o teste).
- Timbre (Accent Field, item 3): 0.9 - **já confirmado** ("agora está
  bem perceptivel").
- Articulação (Accent Field, item 3): 0.3 - **já confirmado**, mesma
  mensagem.
- Herança de acento: cauda inicial 0.3, decaimento pela metade a cada
  passo.
- Rotação de acento: `instability * 0.000004` de velocidade (~20-30s por
  rotação no repouso típico de `instability`, ~0.2).
- Step fatigue (também listado em `PESQUISA_SEQUENCER_GENERATIVO.md`):
  +0.03 por disparo, teto 0.3.

**Item ANTIGO que já foi recalibrado nesta sessão, mas antes da
convergência com Noise Field/rotação/herança**: o dip original de
MÉTRICA (`weakAccentAtUnit1`/`weakAccentAtUnit16`, 0.65/0.35) foi
calibrado quando o ACENTO ainda era só essa dupla de valores fixos por
`metricUnit`. Hoje esse dip serve de BASE pra `instability`+`accentTail`
+`accentFatigue` somarem em cima - vale reconsiderar se 0.65/0.35 ainda
é a base certa agora que tem tanta coisa se somando por cima, ou se
ficou "empilhado" demais.

## 8. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, tabela da
seção 5, entrada `CRI-ACC-001`.
