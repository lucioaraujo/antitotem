# Pesquisa: ruído como sistema multidimensional

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-NOI-001` nesse arquivo. Origem: quarto brief colado ao
vivo pelo autor (19 ago. 2026), mesma fonte de `CRI-MEL-001`/`CRI-SEQ-001`/
`CRI-ACC-001` ("outro dialogo com o chatgpt - a parte que precisamos também
aprimorar no antitotem, o noise"). Quarto irmão da mesma família de
documentos.

## 1. Por que este documento existe

O ANTITOTEM já tem um canal NOISE real (ver 2.1) - mas ele vive
inteiramente na primeira das três "territórios" que o brief separa: ruído
como **matéria sonora** (uma camada de áudio misturada, com cor
espectral). O brief argumenta que ruído pode também ser **modulador
tímbrico** (uma fonte de controle, não de áudio) e **acontecimento
estrutural** (um campo lento que dispara e colore eventos formais) - e
que o salto conceitual real é tratar ruído como **princípio de
organização do comportamento**, não como um tipo de sinal. Antes de
escrever qualquer coisa nova, o funil pede o mesmo levantamento já feito
nos três documentos irmãos (I2 - "separar referência primária,
interpretação, lembrança e especulação").

## 2. O que já existe no projeto RASGO sobre este assunto

### 2.1 ANTITOTEM - estado atual do próprio NOISE

`src/core/NoiseFields.h` (`NoisePalette`) já cobre boa parte da família 1
do brief (matéria sonora) de verdade: seis cores reais -
`white/pink/brown/blue/violet/bit` - pink e brown via integração real de
white noise (não só nomes), blue via diferenciação (derivada), violet via
`blue - pink`, e `bit` como um degrau digital (LFSR-like,
`(state & 0x10000) != 0`) - já cobre o "digital noise" do item 1 do brief
também. Roteado em `SimpleSequencer.cpp` como um canal dedicado do
`MutableMixer` (índice 2), passando por FILTER/RING como qualquer outra
fonte - ou seja, já pode ser "filtered noise" e "resonant noise" (dois
outros subitens do item 1) simplesmente por já estar na cadeia certa.

O que falta, comparado ao brief inteiro:
- **Envelope próprio** (item 2): `noiseMix` é um nível fixo, sem
  attack/sustain/release independentes do envelope da nota - não existe
  hoje um "sopro no ataque" ou "cauda de ruído" diferente do resto do som.
- **Modulador tímbrico** (item 3): NOISE hoje só vira ÁUDIO (mixado); não
  modula cutoff/pitch/waveshape/pan de outro estágio. É o mesmo padrão já
  visto em `ModulationSources.h`/`ChaosSources.h` só que essas classes
  modulam outros parâmetros e NÃO passam pelo mixer de áudio - as duas
  metades já existem separadamente no projeto, só nunca uma alimentando
  ativamente a outra a partir do MESMO NOISE.
- **Sincronizado ao sequenciador/passo** (itens 6-9): `noiseMix` é
  contínuo, sem `noise_probability`/`noise_density`/máscara por passo.
- **Campo lento/"clima"** (itens 26-30): não existe hoje um NOISE FIELD
  lento e global no ANTITOTEM - o mais próximo é `ChaosSources.h`'s
  Hénon/Lorenz (contínuo, mas não nomeado como "instabilidade global" nem
  ligado a limiares/eventos).

### 2.2 RASGO_SYNTH - a fonte mais rica de prior art já encontrada nas quatro pesquisas

Um verdadeiro "departamento de ruído" já existe em
`rasgo-synth-core/src/dsp/`, cada arquivo citando sua própria fonte real:

- **`ColoredNoise.hpp`** - white/pink/red(brownian)/blue, citando
  explicitamente o Sapèl da Frap Tools como motivação (hardware real com
  4 cores dedicadas) e o algoritmo "economy" de Paul Kellet pra pink
  noise (técnica real e conhecida, não aproximação inventada).
- **`GrainyNoise.hpp`** - ruído contínuo passado por um comparador com
  histerese, virando um trem irregular de pulsos discretos - porta
  direta da seção "Grainy Noise" do *Noise Cornucopia* de Ray Wilson
  (MFOS), um projeto DIY real. Isto **é literalmente o item 16 do brief
  ("noise clouds") e o item 15 ("noise bursts") ao mesmo tempo** - a
  "graininess" (0=esparso/poppy, 1=denso/crepitante) é exatamente
  `cloud_density`/`particle_rate` do brief, só com outro nome.
- **`BreathExciter.hpp`** - a resposta mais direta e já pronta pro item 2
  do brief inteiro: ruído branco passado por passa-baixas (cor de
  turbulência) MULTIPLICADO por um envelope attack/release PRÓPRIO
  (`envelope_`, independente de qualquer envelope de amplitude da nota),
  citando a distinção real "Blow vs. Strike" do exciter da Mutable
  Instruments Elements. É quase literalmente o exemplo que o próprio
  autor deu no brief ("numa voz tipo flauta: oscillator + breath noise")
  - já implementado, só não portado pro ANTITOTEM.
- **`Rungler.hpp`** - o Rungler de Rob Hordijk (núcleo do circuito
  Benjolin): dois osciladores realimentando um registrador de
  deslocamento de 8 bits; baixa realimentação = padrão repetitivo,
  realimentação alta = "lurch" imprevisível entre travamento rítmico e
  ruído puro. Prior art real e citada (cena DIY Nonlinearcircuits/clones
  de Benjolin) pro item 5 do brief (`noise_smoothness`/correlação -
  "quanto maior a correlação temporal, mais o ruído se parece com gesto")
  e pro item 20 (noise feedback, com limitação contra runaway - o próprio
  Rungler já satura por natureza do circuito).
- **`BuchlaRandomSource.hpp`** - LFSR de 8 bits modelado no Random
  Voltage Source do Buchla Music Easel (citando o redesenho de esquema de
  Scott Stites, 2004), com **múltiplas saídas sendo combinações
  ponderadas diferentes do MESMO estado** - "leituras" correlacionadas de
  uma única fonte, não geradores independentes. É um precedente real e
  muito preciso pra ideia central do brief (item 29, "Noise Field": um
  campo único do qual vários comportamentos herdam grau).
- **`SpectralGhost.hpp`** - FFT real com magnitude preservada e fase
  substituída por uma deriva lenta e independente ("phase randomization"
  espectral, citando o modo espectral do Mutable Instruments Clouds/
  Beads) - implementação real do item 11 do brief (spectral noise via
  `noise_center_frequency`/`noise_bandwidth`, aqui expresso como blur
  espectral em vez de banda estreita, mas a mesma família de técnica).
- **`NoiseGateGenerator.hpp`** - porta a seção "Random Gates" do mesmo
  *Noise Cornucopia*: um comparador watching ruído filtrado cruza zero em
  tempos irregulares, clocka um contador binário (CD4024 real), e uma
  das saídas divididas vira um gerador de trigger. Prior art real pro
  item 26 do brief (NOISE FIELD → THRESHOLD DETECTOR → EVENT) - já
  implementado como *fonte de disparo*, não só de textura.
- **`AY8910Chip.hpp`** - gerador de ruído real do chip AY-3-8910 (LFSR de
  17 bits, mesma estrutura documentada do hardware real) - outro
  precedente de "digital noise" (item 1) já implementado.
- **`OchdBank.hpp`** - inspirado no ochd da Instruo (citado, colaboração
  com Ben "DivKid" Wilson): oito saídas derivadas de uma base
  compartilhada, cada uma com sua razão + wobble lento próprio - não é
  ruído no sentido estrito, mas é a mesma arquitetura de "um campo
  compartilhado, várias leituras levemente diferentes dele" que o brief
  propõe pro NOISE FIELD.
- **`engine/MaterialAgency.hpp`** - já tem uma família nomeada `Rupture`
  (ao lado de Core/Texture/Instrument/Voice/Polyphony/Rhythm) e um estado
  `MaterialFocus::Fragmented` descrito como "event-shaped rather than
  section-shaped" - precedente real, ainda que mais estreito, do
  vocabulário de "acontecimentos" do item 30 do brief (burst/rupture/
  fragmentation/etc. como uma classe própria de evento formal, não uma
  textura).
- **`engine/CoherenceCollapse.hpp`** (já citado em `CRI-SEQ-001`, não
  relido nesta rodada) - detector de "ruptura por coerência excessiva"
  via entropia de Shannon real: é o precedente mais próximo que o projeto
  já tem da arquitetura NOISE FIELD → THRESHOLD → EVENT (itens 26-29 do
  brief), só que a partir de entropia, não de um `NoiseField` nomeado
  como tal.

### 2.3 AQUORBIUM

- `src/core/BiomaGranularStriker.h` - vocabulário real de partícula/grão
  já existente (`particleCapacity`, `grainDurationMs`, `particleDensity`,
  2-12 partículas por strike) - outro precedente real (junto com
  `GrainyNoise.hpp`) pro item 16 do brief (noise clouds).
- `src/core/BiomaEngine.cpp`/`AutonomousEnergy.h` - já citado em
  `CRI-ACC-001`: um único `geometricAccent` contínuo alimenta
  `particleDensity`, `hardness`, `irregularity` e excitação de três
  motores físicos ao mesmo tempo. Não é ruído no sentido estrito
  (deriva de acento/geometria, não de uma fonte aleatória), mas é o
  mesmo princípio arquitetural que o brief propõe pro NOISE FIELD - "um
  campo alimentando timbre, ritmo e eventos ao mesmo tempo" já está em
  produção no AQUORBIUM, só a partir de outra fonte.

## 3. O brief do autor (fonte secundária/interpretativa, não primária)

Colado ao vivo (19 ago. 2026), mesma origem dos três anteriores. Estrutura
das 30 técnicas descritas (texto completo preservado na mensagem
original, resumo aqui por brevidade):

- **Três territórios**: ruído como matéria sonora (item 1: white/pink/
  brown/blue/band-limited/filtered/resonant/impulse/digital/sample-
  derived); ruído como modulador tímbrico (item 3: cutoff, resonance,
  amplitude, pitch, waveshape, FM amount, pulse width, posição de
  wavetable, distorção, pan, largura estéreo, delay time, posição de
  grão, tilt espectral); ruído como acontecimento estrutural (item 30).
- **Envelope próprio do ruído** (item 2): `noise_attack/sustain/release/
  amount`, independentes do envelope da nota (sopro no ataque, fricção na
  sustentação, ruído concentrado na saída).
- **Velocidade** (item 4): fast noise (textura/aspereza/FM caótica) vs.
  slow noise (deriva/instabilidade orgânica, "quase um organismo
  interno").
- **`noise_smoothness`/correlação** (item 5): de aleatório puro a suave/
  gestual - quanto maior a correlação temporal, mais lê como gesto.
- **Sincronizado ao passo** (itens 6-9): `noise_probability`,
  `noise_density` (separando *event density* de *spectral density*),
  padrão próprio de ruído por passo.
- **Acento e articulação via ruído** (itens 9-10): "noisy note" como
  forma de acento; tipos de articulação nomeados (breath/scrape/click/
  friction/burst/hiss/crackle).
- **Ruído espectral** (itens 11-13): banda/centro/tilt/ressonância;
  ruído ligado ao pitch da nota (`noise_center = pitch * 4`); harmônico
  vs. inarmônico como eixo `harmonicity`.
- **Degradação** (item 14): `degradation_amount` afetando bit depth,
  sample rate, distorção, dropouts, clipping - contaminação progressiva.
- **Bursts, clouds, trails** (itens 15-17): eventos curtos, nuvens de
  partículas, caudas ruidosas que sobrevivem vários passos (memória
  sonora).
- **Acumulação e decaimento** (itens 18-19): ruído que cresce evento a
  evento até um reset, ou que decai após um pico.
- **Feedback, freeze, capture** (itens 20-22): ruído realimentando sua
  própria quantidade (com limitação); congelar uma configuração
  interessante; capturar um fragmento do que está acontecendo e
  transformá-lo em nova matéria (loop/grão/ruído).
- **Mutation e window** (itens 23-24): pequenas mutações no padrão de
  ruído; janelas temporais em que o ruído age (1 passo a vários
  compassos).
- **Crescendo, threshold, thresholds múltiplos, probabilidade dirigida**
  (itens 25-28): crescimento formal multi-eixo (amplitude+bandwidth+
  modulação+glitch+densidade juntos); um NOISE FIELD lento cruzando um
  limiar dispara um evento sem relógio explícito; múltiplos limiares
  disparam comportamentos diferentes (0.25 sutil, 0.50 acento, 0.75
  glitch, 0.90 ruptura); um campo de ruído contínuo controla a própria
  probabilidade de outros mecanismos (ghost notes, glitches, subdivisão).
- **Noise Field como "clima"** (item 29, o conceito mais ambicioso):
  uma variável lenta que não gera áudio necessariamente, representando o
  "clima interno" do instrumento - baixa = estável/limpo/regular; alta =
  instável/áspero/denso/fragmentado - afetando timing, pitch, timbre,
  glitch, acento e subdivisão ao mesmo tempo.
- **Evento vs. acontecimento** (item 30): evento = tocar uma nota;
  acontecimento = algo altera temporariamente o comportamento do
  sistema. Vocabulário proposto: Burst, Cloud, Rupture, Erosion,
  Contamination, Collapse, Expansion, Freeze, Fragmentation, Dropout,
  Overflow, Drift, Recovery.
- **Arquitetura proposta**: `NOISE SYSTEM` dividido em `AUDIO NOISE`
  (timbre/textura/fricção/burst), `MODULATION` (parâmetros/deriva/
  instabilidade/variação) e `EVENT NOISE` (acontecimentos: ruptura/
  freeze/mutação); acima dos três, um `NOISE FIELD` único alimentando
  `GLOBAL INSTABILITY`, que por sua vez influencia TIMBRE, RHYTHM e
  EVENTS simultaneamente.

## 4. Referências reais (fonte primária, já citadas pelo próprio código-fonte do projeto)

Diferente dos três documentos irmãos, aqui a maior parte das referências
primárias **já estavam corretamente citadas no código do próprio RASGO**,
não precisaram ser adicionadas por conhecimento externo do modelo - a
pesquisa já tinha sido feita, só nunca fora reunida num documento único
nem cruzada com o ANTITOTEM especificamente:

- **Paul Kellet, "economy" pink noise algorithm** (técnica real e
  amplamente documentada em fóruns de DSP/musicdsp.org) - já usada tanto
  em `ANTITOTEM/NoiseFields.h` (integração de três polos) quanto em
  `ColoredNoise.hpp` (RASGO_SYNTH).
- **Ray Wilson, *Noise Cornucopia*** (MFOS - Music From Outer Space,
  projeto DIY real e público) - citado por `GrainyNoise.hpp` e
  `NoiseGateGenerator.hpp` como fonte direta de dois circuitos reais
  (Grainy Noise, Random Gates).
- **Rob Hordijk, circuito Benjolin (Rungler)** - design real, documentado
  e replicado por uma tradição DIY conhecida (Nonlinearcircuits e outros
  clones) - citado por `Rungler.hpp`.
- **Don Buchla, Random Voltage Source (Music Easel/Model 208)**, via o
  redesenho de esquema de Scott Stites (2004) - citado por
  `BuchlaRandomSource.hpp`.
- **Mutable Instruments, Elements (exciter Blow/Strike) e Clouds/Beads
  (modo espectral)** - hardware real, amplamente documentado - citados
  por `BreathExciter.hpp` e `SpectralGhost.hpp` respectivamente.
- **Chip AY-3-8910 (General Instrument)** - arquitetura pública e
  documentada (três tons, um ruído compartilhado, envelope de hardware) -
  citado por `AY8910Chip.hpp`.
- **Instruo ochd**, colaboração com Ben "DivKid" Wilson - módulo real de
  hardware - citado por `OchdBank.hpp`.
- **Shannon, C. (1948). "A Mathematical Theory of Communication."** -
  já citado em `CoherenceCollapse.hpp` e reaproveitado em `CRI-SEQ-001`;
  relevante aqui como a base teórica mais próxima que o projeto tem do
  "Noise Field → Threshold → Event" (itens 26-29 do brief).

**Nota de proveniência**: diferente dos documentos anteriores, a maioria
das citações acima não foi adicionada por conhecimento do modelo nesta
rodada - já existiam, corretas, nos comentários do próprio código-fonte
RASGO_SYNTH. O trabalho desta seção foi de levantamento e cruzamento, não
de pesquisa acadêmica nova. Onde uma citação vier de conhecimento próprio
do modelo (nenhuma nesta seção), isso seria marcado explicitamente, como
nos três documentos irmãos.

## 5. Estado atual vs. o brief

| Conceito do brief | Estado no RASGO |
|---|---|
| Cores de ruído (white/pink/brown/blue/violet/digital) | **Feito** (ANTITOTEM `NoisePalette`, 6 cores reais) e **feito de novo, com motivação diferente** (RASGO_SYNTH `ColoredNoise.hpp`, citando Frap Tools Sapèl) |
| Filtered/resonant noise | **Feito** (ANTITOTEM - NOISE já passa por FILTER/RING no mixer) |
| Impulse/digital noise (clicks, LFSR) | **Feito** (`NoisePalette::bit`; `AY8910Chip.hpp`, LFSR real de 17 bits) |
| Noise com envelope próprio (attack/sustain/release) | **Feito, e depois REDESENHADO no mesmo dia** (19 ago. 2026). 1ª versão: `setNoiseBreathAmount` como camada ADITIVA opcional (0=off), seguindo o gate como attack/release contínuo. Autor testou e apontou o problema real: "o noise deve entrar nos steps do sequencer sem sons longos passando por cima dos acontecimentos" - o gate usado (`running && !muted[currentStep]`) fica true por vários passos seguidos (só cai num passo mutado ou no STOP), então na prática quase nunca "respirava" entre passos, continuando a soar como um só bloco contínuo. Redesenhado pra um "ping" percussivo de verdade: `noiseBreathEnvelope` agora é disparado (=1.0) no MESMO ponto de trigger que `envelope.trigger()` da nota usa em `advanceStep()` (cada passo não-mutado), mais um auto-disparo na borda de subida do gate (mesma lógica que `ContourEnvelope::process()` já usa internamente, achado ao investigar por que um teste automatizado quebrou), e decai a cada sample com um tau proporcional à duração REAL do passo (`samplesPerStep()`, já considera tempo/feel/groove) - não um valor fixo em ms, garantindo que o pulso termine antes do próximo passo em qualquer andamento. Este envelope agora se aplica SEMPRE ao `noiseMix` de base (não só à camada extra) - `noiseTotal = (noiseMix + noiseBreathAmount) * noiseBreathEnvelope` - substituindo de vez o antigo nível constante. `noiseBreathAmount` (o botão BR) passa a significar "profundidade extra no mesmo pulso", não mais "liga a reatividade" (que agora é sempre ativa) |
| Gain-staging do NOISE ("toma conta do áudio") | **Tentado e revertido** (19 ago. 2026) - um corte `*0.42f` (mesmo headroom que a voz já tem) resolvia o volume, mas também enfraquecia o S&H (mesmo sinal, mesmo caminho de nível) - autor: "a função no som" ficou fraca. Revertido; ainda sem solução aplicada, precisa de uma alavanca que não afete S&H (ex.: o ganho padrão do canal NOISE no mixer, não um corte na DSP que atinge tudo igual) |
| UI: botão BR | **Feito** - ao lado do S&H, dentro do próprio widget `NoiseSelector` (mesmo padrão do SWG - liga/desliga, profundidade fixa 0.28, não um dial contínuo). Ambos os botões voltaram ao tamanho original (40px) depois de uma primeira tentativa mais estreita (34px) ter deixado o S&H "fraquinho" visualmente |
| Noise Field ("campo de instabilidade global que afeta timbre, ritmo e eventos", item #29) | **Feito, v1 sem UI** (19 ago. 2026) - autor escolheu este item diretamente como próxima frente. Um único campo COMPARTILHADO (autor: "um só, compartilhado", não um por objeto) - vive em `DualObjectEngine` (`instabilityField`, random walk lento com retorno suave a um piso de 0.2, nunca fica parado em 0), empurrado pra `first`/`fifth` via `SimpleSequencer::setInstability()` a cada sample. Três destinos, todos reaproveitando mecanismos já existentes (mesmo princípio do achado do AQUORBIUM/`geometricAccent` - um valor, várias leituras): **timbre/textura** - soma extra ao `noiseTotal` (até +0.15); **ritmo** - jitter de até ±6% na duração do próximo passo (reaproveita o `randomState` já existente, não um 4º gerador); **eventos** - reduz o limiar de disparo da EXCITAÇÃO (mais sensível quanto mais instável). Sem controle de UI ainda - autônomo, autor ainda não pediu um dial |
| Noise como modulador tímbrico (→ parâmetro, não → áudio) | **Não feito** em nenhum instrumento de forma explícita/nomeada, embora a infraestrutura de modulação já exista separada (`ModulationSources.h`/`ChaosSources.h` no ANTITOTEM) |
| Fast noise / slow noise | **Parcial** - `ChaosSources.h` já tem fontes rápidas e lentas separadas, sem nomear o eixo como tal |
| `noise_smoothness`/correlação temporal | **Feito** (RASGO_SYNTH `Rungler.hpp`, `BuchlaRandomSource.hpp` - saídas correlacionadas de uma fonte comum) - **não feito** no ANTITOTEM |
| Sincronizado ao passo (probability/density/pattern) | **Não feito** em nenhum instrumento de forma dedicada a ruído (existe o análogo pra acento, ver `CRI-ACC-001`) |
| Noise accent / articulação nomeada | **Não feito** como conceito nomeado, embora `BreathExciter.hpp` já produza o efeito sonoro equivalente |
| Spectral noise (banda/centro/tilt) | **Feito** (RASGO_SYNTH `SpectralGhost.hpp`, FFT real com blur de fase) - não portado |
| Noise ligado ao pitch | **Não feito** em nenhum instrumento hoje |
| Harmonic × inharmonic noise | **Não feito** |
| Degradação/contaminação progressiva | **Não feito** como eixo nomeado |
| Bursts / clouds | **Feito** (RASGO_SYNTH `GrainyNoise.hpp`; AQUORBIUM `BiomaGranularStriker.h`, partícula/grão real) |
| Trails, accumulation, decay | **Não feito** em nenhum instrumento |
| Noise feedback (com limitação) | **Feito** (RASGO_SYNTH `Rungler.hpp`, satura por natureza do circuito) |
| Freeze / capture | **Não feito** em nenhum instrumento como mecanismo geral (existe congelamento pontual em contextos específicos não relacionados a ruído, não sondado a fundo nesta rodada) |
| Mutation / window | **Não feito** como eixo de ruído nomeado |
| Noise threshold → event (arquitetura) | **Feito, por outra fonte** (RASGO_SYNTH `NoiseGateGenerator.hpp`, ruído→trigger; `CoherenceCollapse.hpp`, entropia→ruptura) - **não feito** no ANTITOTEM |
| Multiple thresholds → comportamentos diferentes | **Não feito** em nenhum instrumento |
| Noise-driven probability | **Não feito** como eixo de ruído; existe o análogo `ProbabilityMarket.hpp` no RASGO_SYNTH, mas dirigido por uso recente, não por um NOISE FIELD |
| **Noise Field ("clima")** - campo único alimentando timbre+ritmo+eventos | **Feito, sem o nome, por outra fonte** (AQUORBIUM `geometricAccent`, ver `CRI-ACC-001`; `BuchlaRandomSource.hpp`, leituras correlacionadas de um único estado) - **não feito** no ANTITOTEM, e não feito em nenhum lugar a partir de RUÍDO especificamente |
| Vocabulário de acontecimentos (Burst/Rupture/Collapse/Freeze/...) | **Parcial** (RASGO_SYNTH `MaterialAgency.hpp`, família `Rupture` + estado `Fragmented` nomeados, mas vocabulário bem mais estreito que os 13 termos do brief) |

## 6. Próximos passos sugeridos (não decididos - aguardando o autor)

Mesma lição dos três documentos irmãos - priorizar o que já tem
precedente real de código pronto pra adaptar, não o que seria trabalho
inteiramente novo:

1. **Portar `BreathExciter.hpp` (ou o mesmo princípio) pro ANTITOTEM** -
   dar ao NOISE existente um envelope attack/release próprio, separado do
   envelope da nota. É o item de maior precedente direto (código já
   existe, testado, num instrumento irmão) e o mais barato de todos.
2. **Noise como modulador, não só como áudio** - usar o `noiseSignal` que
   `SimpleSequencer.cpp` já calcula (hoje só mixado como áudio) também
   como fonte de modulação de outro parâmetro (ex.: cutoff do FILTER, ou
   a própria EXCITAÇÃO) - reaproveitando um sinal que já existe, não
   criando um novo.
3. **Noise Field explícito** - nomear e generalizar o que já existe
   implicitamente (AQUORBIUM `geometricAccent`, `BuchlaRandomSource.hpp`)
   aplicado a ruído no ANTITOTEM: uma variável lenta única alimentando
   timbre + probabilidade + eventualmente o ACENTO do `CRI-ACC-001` -
   este é o ponto onde os quatro documentos desta família (`CRI-MEL-001`/
   `CRI-SEQ-001`/`CRI-ACC-001`/`CRI-NOI-001`) mais claramente convergem
   pra uma única arquitetura, se o autor decidir ir nessa direção.
4. O restante (accumulation/decay/trails/freeze/capture/mutation/window/
   multiple thresholds/vocabulário completo de acontecimentos) fica
   registrado, sem prioridade definida - mesmo tratamento dado ao
   excedente dos três documentos anteriores.

## 7. Calibração pendente (19 ago. 2026)

Ver a nota geral em `PESQUISA_MELODIA_GENERATIVA.md`, seção 7. Todas em
`SimpleSequencer.cpp`, salvo indicação contrária:

- Ataque do NOISE BREATH: 0.003s (~3ms) - a rampa que corrigiu o clique
  real, mas o TEMPO exato nunca foi comparado contra alternativas.
- Decaimento do NOISE BREATH: 0.7/4.0 da duração do passo atual (tau).
- Contribuição do `instability` no timbre/textura do NOISE: até +0.15.
- Profundidade fixa do botão BR: 0.28.

**Pendência maior, resolvida** (19 ago. 2026): o gain-staging do NOISE
("toma conta do áudio ao ligar o canal") tinha a alavanca certa
identificada (o ganho padrão do próprio canal NOISE no mixer), mas essa
mudança contradizia uma instrução explícita anterior (15 ago. 2026: "os
sliders do mixer... devem iniciar no valor 1.00") - perguntado ao autor
via AskUserQuestion se podia revogar essa instrução só pro NOISE;
resposta: "Pode mudar o padrão do fader (revogar o pedido antigo)".
`mixerGainDefaults[2]` (PRINCIPAL) e o equivalente em CLONE mudaram de
1.0 pra 0.6 - FILTER/RING/SPACE continuam em 1.00 como sempre foi
pedido. Alavanca correta confirmada: o fader do canal escala só o
resultado final, sem tocar em `noiseTotal`/`sampleHoldMix` (o caminho
que S&H compartilha com o ruído cru), então não reintroduz o problema
do corte `*0.42f` revertido antes.

## 8. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, tabela da
seção 5, entrada `CRI-NOI-001`.
