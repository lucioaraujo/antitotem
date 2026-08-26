# Pesquisa: deriva como deslocamento de estado, não aleatoriedade

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-DRF-001` nesse arquivo. Origem: quinto brief colado
ao vivo pelo autor (19 ago. 2026), mesma fonte de `CRI-MEL-001`/
`CRI-SEQ-001`/`CRI-ACC-001`/`CRI-NOI-001` ("mais um dialogo com o chatgpt
agora sobre deriva: aprofunde, analise, documente"). Quinto irmão da
mesma família de documentos - mas o único, até agora, em que o *tema* já
é um recurso real e nomeado no próprio ANTITOTEM (o botão DERIVA), não um
conceito inteiramente novo.

## 1. Por que este documento existe

O brief do autor propõe uma distinção central: **modulação transforma o
presente; deriva transforma aquilo que o instrumento considera seu
estado normal**. Antes de escrever qualquer coisa nova, é preciso
verificar quanto disso já existe - e a resposta, neste caso, é
"surpreendentemente muito". O botão DERIVA (CLONE: `deriveFromMemory()`/
`captureDerivationMemory()`; PRINCIPAL tem uma cópia paralela do mesmo
mecanismo) já não é um "randomizador" - é um sistema com memória,
momentum e mutação de topologia, só nunca documentado com este
vocabulário nem comparado a uma teoria explícita.

## 2. O que já existe: DERIVA no próprio ANTITOTEM (leitura completa do mecanismo)

`captureDerivationMemory()` (autor, live: fixa uma **âncora** - CV/AMP/FX
de cada passo, razões dos osciladores, e preenche `topologyMemory` (um
buffer circular) inteiro com a topologia de roteamento ATUAL) é
literalmente o evento **DRIFT CAPTURE** do brief, implementado antes de o
brief existir. `derivationMotion` zera junto - a deriva recomeça do
repouso a partir daquele ponto.

`deriveFromMemory()`, disparado automaticamente uma vez por loop (`
timerCallback()`: quando o passo volta a 0, com o botão ligado e o
sequenciador rodando) ou em pontos específicos de uma gravação
(`advanceRecordingForm()`):

- **Momentum real**: `derivationMotion` é um random walk LIMITADO
  (`clamp(-0.55, 0.55)`, não elástico ao centro, mas também não livre
  sem fim) que se acumula a cada disparo - e retroalimenta a PRÓPRIA
  intensidade do próximo disparo (`activeDepth = userDepth * (0.62 +
  abs(derivationMotion))`). Isto é exatamente o "momentum" do brief:
  *"Se o instrumento está há algum tempo aumentando densidade, existe uma
  tendência de continuar"* - aqui generalizado pra intensidade de deriva
  como um todo, não só densidade.
- **Deriva com memória, não `random()` independente**: cada passo (CV/
  AMP/FX) desliza de seu valor ATUAL rumo a um NOVO alvo aleatório, a uma
  taxa (`drift`) proporcional a `activeDepth` - `cv += (cvTarget - cv) *
  drift`, não um salto. É literalmente a fórmula do brief
  (`0.20 → 0.24 → 0.31...`, carrega o estado anterior) já implementada,
  por passo, há muito tempo.
- **Deriva de topologia com memória real ("rule drift"/"topological
  drift")**: `topologyMemory` é um buffer circular de roteamentos
  passados; a cada disparo, com probabilidade dependente de
  `activeDepth`, o próximo roteamento é puxado da MEMÓRIA (um estado já
  visitado) em vez de aleatório puro (`historicalRoute`), e às vezes
  sofre uma mutação de 1 bit. Isso é ao mesmo tempo **residual drift**
  (o sistema pode voltar a um estado antigo, mas raramente idêntico ao
  atual) e **rule drift** (o roteamento entre módulos - não um parâmetro
  contínuo - é o que muda).
- **Deriva cruzada real ("forces"/correlação entre parâmetros)**: a
  densidade de conexões ativas na topologia (`routeDensity`) empurra o
  alvo do FEEDBACK GAIN e dos sends de efeito (REVERB/PHASER/FLANGER) -
  um parâmetro estrutural (topologia) influenciando parâmetros contínuos
  (ganho, sends), não canais isolados.
- **Deriva de afinação**: razão dos osciladores desliza multiplicativamente
  (`ratio * 2^exponent`), e o EXPOENTE da mutação já soma
  `derivationMotion * 0.18` - a MESMA variável de momentum acima também
  colore a deriva de afinação, não uma fonte separada.
- **Cadência de deriva ("drift_rate")**: o disparo automático é por
  CICLO/LOOP (volta ao passo 0), não por sample nem por passo - já
  corresponde à granularidade "bars/phrases" que o brief propõe como uma
  das opções de `drift_rate`.

## 3. O que já existe: prior art real em RASGO_SYNTH/AQUORBIUM

- **AQUORBIUM `BiomaBrain::nextDrift()`** (`core/BiomaBrain.cpp`) - o
  achado mais forte fora do ANTITOTEM. Uma leitura MUITO próxima do
  "Drift Field" do brief: um único registrador correlacionado (mesma
  técnica do `BuchlaRandomSource.hpp` já citado em
  `PESQUISA_RUIDO_GENERATIVO.md` - várias leituras ponderadas
  DIFERENTES do mesmo estado, não geradores independentes) combinado
  (72%/28%) com uma LFO senoidal lenta própria por organismo
  (`ratio[index]`, cada um numa razão de frequência distinta) -
  `result[index] = wave*0.72 + heldCorrelation[index]*0.28`. Isso é,
  literalmente, "um campo compartilhado, várias leituras levemente
  diferentes dele, cada uma na sua própria velocidade" - a arquitetura
  central que o brief chama de Drift Field, já em produção.
- **RASGO_SYNTH `BuchlaRandomSource.hpp`** (já citado em
  `PESQUISA_RUIDO_GENERATIVO.md`) - mesmo princípio, reaproveitado aqui
  como precedente da correlação estado-único/múltiplas-leituras que tanto
  `BiomaBrain` quanto o "Drift Field" do brief descrevem.
- **RASGO_SYNTH `engine/GenerativeArc.hpp`** (já citado em
  `PESQUISA_MELODIA_GENERATIVA.md` como "nível frase, walk markoviano") -
  não relido a fundo nesta rodada; nome e uso anterior sugerem
  precedente de deriva em nível de FRASE (a "SLOW DRIFT" do brief),
  merece leitura futura antes de qualquer decisão de reaproveitamento.
- **"Motor drift"/"gesture inertia"** (já nomeado e implementado em
  EXCITAÇÃO, `CRI-MEL-001` item #18, e já citado como técnica pendente
  de generalizar pro sequenciador principal em `CRI-SEQ-001`) - o passo
  Narmour-enviesado de EXCITAÇÃO É uma deriva de pitch local, só nunca
  batizada assim. O item #9 desta mesma sessão (arco de frase via
  `excitationBreath`) também já é, na prática, uma forma de "deriva
  dinâmica"/"deriva de comportamento" (o instrumento se comporta diferente
  cedo vs. tarde numa frase) implementada antes deste documento existir.
- **Noise Field** (`CRI-NOI-001`, implementado nesta mesma sessão,
  `DualObjectEngine::instabilityField`) - já é, por definição, um "Drift
  Field" real: um valor único (`instabilityField`), um random walk LENTO
  com retorno suave a um piso de repouso (0.2) - a mesma família de
  "elastic drift" que o brief descreve (`tende ao centro`), alimentando
  três destinos ao mesmo tempo (timbre/ritmo/eventos). O Accent Field
  (`CRI-ACC-001`, item 3, também implementado hoje) já lê esse MESMO
  campo pra articulação e timbre do ACENTO. Ou seja: **o "Drift Field" do
  brief já existe, com esse nome ou não, e já está conectado a dois
  sistemas diferentes antes deste documento ter sido escrito.**

## 4. O brief do autor (fonte secundária/interpretativa, não primária)

Colado ao vivo (19 ago. 2026), mesma origem dos quatro anteriores.
Resumo dos eixos principais (texto completo preservado na mensagem
original):

- **Definição central**: `random` muda sem continuidade; `drift` muda
  carregando o estado anterior (`0.20→0.24→0.31→0.37...`, uma
  trajetória, não uma sequência de saltos independentes).
- **13 territórios de deriva**: temporal (microtiming atrasa/antecipa
  progressivamente), métrica (centro perceptivo dos acentos migra,
  4/4→3+3+2→5+3 sem mudar o ciclo), swing/groove (reto→ternário→
  assimétrico), densidade (ganha/perde eventos, não necessariamente
  crescendo), dinâmica (desloca o CENTRO de energia, não só cresce/
  diminui), pitch (notas/afinação/centro tonal/registro migram),
  tímbrica (brilho/noise/harmonicidade/ressonância migram), espectral
  (energia se desloca pelo espectro), probabilística (as próprias
  probabilidades mudam com o tempo), estrutural/regras (o instrumento
  altera seu próprio comportamento, não só parâmetros), espacial (pan/
  largura/profundidade migram), articulação (ligado→separado→fragmentado
  progressivamente), e "deriva de comportamento" (combinação das
  anteriores = mudança de "personalidade").
- **Deriva de parâmetro vs. deriva de sistema**: um valor que se move
  (`cutoff: 1000→1800Hz`) é diferente de um REGIME que muda (`regular/
  limpo/previsível` → `irregular/ruidoso/fragmentado`).
- **Drift Field**: uma variável contínua (`-1..+1`) influenciando vários
  subsistemas ao mesmo tempo, cada um respondendo de forma NÃO-linear e
  distinta (`timing responde muito, pitch pouco, noise só acima de 0.5,
  glitch só acima de 0.75`) - "regiões de comportamento", não um mapa
  linear único.
- **Drift rate**: granularidade da deriva - segundos, compassos, frases,
  minutos.
- **Drift inertia/momentum**: quanto maior a inércia, mais difícil mudar
  de direção; `current_drift = previous_drift + force - resistance` -
  um modelo quase físico.
- **Forças e atratores**: em vez de fixar um valor, somar tendências
  (força pra cima, gravidade de volta, ruído) - e o sistema pode ter
  REGIÕES preferenciais (atratores) às quais tende a retornar, orbitando
  entre elas em vez de um passeio puramente aberto.
- **Âncoras**: elementos que resistem à deriva (tempo, nota fundamental,
  duração de ciclo, motivo) - permitem que parte do sistema se
  transforme enquanto outra parte permanece reconhecível. "Release from
  anchor" (soltar uma âncora de propósito) pode funcionar como
  acontecimento formal.
- **Elasticidade**: `elastic drift` (volta ao centro), `free drift`
  (pode se afastar indefinidamente dentro de limites), `bounded drift`
  (livre entre mín/máx), `attractor drift` (migra entre regiões
  preferenciais).
- **Deriva e memória/resíduo**: o sistema pode evitar ou favorecer
  retornar a regiões já visitadas; depois de uma fase ruidosa, pode não
  voltar EXATAMENTE ao estado limpo original - um resíduo/cicatriz
  (`residual drift`/"trace"), ideia que o autor liga explicitamente à
  estética RASGO ("ruptura, rasgo, desgaste, arqueologia sonora").
- **Deriva topológica**: as próprias CONEXÕES entre módulos mudam
  lentamente (`LFO→FILTER` vira `LFO→FILTER+PITCH` vira `NOISE→FILTER,
  LFO→PITCH`) - quase uma deriva da arquitetura do patch.
- **Vocabulário de acontecimentos ligados à deriva**: DRIFT START/
  ACCELERATION/REVERSAL/CAPTURE/FREEZE/RELEASE, ANCHOR/UNANCHOR, RETURN,
  ESCAPE, ATTRACTOR CHANGE, RESIDUE.
- **Arquitetura proposta**: `DRIFT ENGINE` dividido em SLOW DRIFT (nível
  macroforma), LOCAL DRIFT (nível gesto) e RULE DRIFT (nível
  comportamento), convergindo num `DRIFT FIELD` único que afeta TIMING/
  PITCH/TIMBRE/NOISE/ACCENT/DENSITY/GROOVE/EVENTS.

## 5. Estado atual vs. o brief

| Conceito do brief | Estado no RASGO |
|---|---|
| Definição básica (drift = muda carregando estado anterior) | **Feito** (DERIVA do ANTITOTEM, `cv += (target-cv)*drift`, por passo, há muito tempo - só nunca nomeado com esta teoria) |
| Deriva de parâmetro (CV/AMP/FX/razão de osciladores/sends) | **Feito** (DERIVA, todos esses exatos parâmetros já derivam) |
| Deriva estrutural/de regras (topologia de roteamento) | **Feito** (`topologyMemory`/mutação de 1 bit - a "regra" de conexão entre módulos muda, não só um valor) |
| Deriva topológica (conexões entre módulos migram) | **Feito, parcial** (roteamento de feedback entre PRINCIPAL/CLONE já muda; roteamento LFO→destino como no exemplo do brief, não) |
| Drift Field (um campo único, vários destinos, respostas não-lineares distintas) | **Feito** (Noise Field, `CRI-NOI-001`, já implementado hoje - falta só a resposta NÃO-linear por destino que o brief pede; hoje todos os destinos leem o mesmo valor de forma proporcional) |
| Drift rate (granularidade segundos/compassos/frases) | **Feito** (DERIVA dispara por LOOP; Noise Field evolui por sample/segundos - já dois "rates" diferentes coexistindo) |
| Momentum (`current = previous + force - resistance`) | **Feito** (`derivationMotion`, random walk limitado que retroalimenta a própria intensidade do próximo disparo) |
| Forças somadas (upward/downward/return/noise/external) | **Parcial** (a topologia empurra feedback/sends - uma força real; não modelado como soma explícita de vetores nomeados) |
| Atratores (regiões preferenciais, o sistema orbita entre elas) | **Feito** (19 ago. 2026) - três regiões nomeadas ao longo do eixo de `derivationMotion` (limpo/tonal -0.4, neutro 0.0, denso/ruidoso +0.4); depois do passo de random walk de sempre, um puxão suave (0.12) rumo ao atrator mais próximo - orbita entre as três regiões em vez de vagar uniformemente, sem travar o caráter aleatório existente |
| Âncoras / release from anchor | **Não feito** como conceito explícito - `captureDerivationMemory()` já FIXA uma âncora (o estado capturado), mas nada hoje é declarado "resistente" à deriva enquanto o resto se move |
| Elasticidade (elastic/free/bounded/attractor drift) | **Bounded, parcial** - `derivationMotion` é limitado (`clamp -0.55..0.55`), não elástico ao centro; Noise Field É elástico (retorna a 0.2); os dois padrões já coexistem no projeto sem nome comum |
| Deriva com memória/resíduo (não retorna exatamente ao estado antigo) | **Feito** (`topologyMemory` favorece histórico mas raramente idêntico ao atual - resíduo real, não nomeado) |
| Deriva tímbrica/espectral | **Parcial** (sends de efeito derivam; timbre de oscilador/filtro em si, não) |
| Deriva métrica (centro perceptivo dos acentos migra, sem mudar o ciclo) | **Feito** (19 ago. 2026, via `CRI-ACC-001`/"rotação de acento" - `SimpleSequencer::accentRotation`, desloca qual passo conta como forte, `metricBeats` nunca muda; velocidade escalada por `instability`, ~20-30s por rotação no repouso típico - implementado como item da pesquisa de ACENTO, registrado aqui também por ser literalmente a mesma ideia) |
| Deriva de densidade | **Não feito** como eixo isolado (existe indiretamente via routeDensity, não como conceito próprio) |
| Deriva probabilística (as próprias chances mudam) | **Não feito** - nenhum sistema RASGO hoje tem probabilidades que elas mesmas derivam (mais próximo: `ProbabilityMarket.hpp` do RASGO_SYNTH, já citado em `PESQUISA_RUIDO_GENERATIVO.md`, preço com reversão à média - mecanismo adjacente, não a mesma coisa) |
| Deriva espacial (pan/largura/profundidade) | **Não feito** |
| Deriva de articulação | **Não feito** como eixo isolado (adjacente: EXCITAÇÃO já tem articulação por INTERVALO, não por deriva temporal) |
| Vocabulário de acontecimentos (CAPTURE/FREEZE/RELEASE/ANCHOR/...) | **Parcial** - CAPTURE já existe e já se chama assim (`captureDerivationMemory`); FREEZE existe só no CAOS/VAGA LFO, não ligado à deriva; o resto não nomeado |
| Drift Field one shared value, múltiplos destinos NÃO-lineares por destino | **Parcial** (AQUORBIUM `BiomaBrain::nextDrift`, correlação real 72/28% por organismo, cada um com razão própria - a peça mais próxima da "resposta distinta por destino" que existe no projeto) |

## 6. Próximos passos sugeridos (não decididos - aguardando o autor)

Diferente dos quatro documentos irmãos, aqui a maior parte do trabalho
já está feita - o valor agora está em NOMEAR e CONECTAR o que já existe,
mais que construir do zero:

1. ~~Atratores~~ - **feito** (19 ago. 2026). Três regiões nomeadas
   (limpo/tonal -0.4, neutro 0.0, denso/ruidoso +0.4) ao longo do eixo
   de `derivationMotion`, com um puxão suave (0.12) rumo à mais próxima
   depois do random walk de sempre.
2. **Âncora explícita** - **feito, com realocação por GLT** (19 ago.
   2026). `derivationAnchor` (era fixo em `i==0`) - o CV do passo
   apontado por ele nunca deriva no loop de steps, fica exatamente no
   valor capturado, enquanto AMP/FX dele e tudo dos outros 15 passos
   continua derivando normalmente. Autor: "a previsibilidade de sempre
   começar uma nova configuração no tempo 1 pra mim é uma padronização
   desnecessária... talvez se o glitch decidir?" - confirmado "no step
   1". Implementado: a âncora só realoca na TRANSIÇÃO da SUBDIVISÃO pra
   GLT (não a cada ciclo enquanto continua em GLT - um "ponto fixo" que
   nunca fica quieto deixaria de significar algo), uma causa real e
   audível pra mudança em vez de um sorteio silencioso. Ainda sem evento
   "UNANCHOR" dedicado - GLT já cumpre esse papel na prática.
3. ~~Conectar o Noise Field à DERIVA~~ - **feito** (19 ago. 2026, autor:
   "precisamos que os sistemas dentro do instrumentos sejam
   inteligentes versáteis com boa capacidade de enxergar os fluxos" ->
   "isso"). `DualObjectEngine` ganhou `getInstabilityField()`/
   `nudgeInstability()`; `deriveFromMemory()` (as duas cópias) lê o
   campo compartilhado pra dar mais energia ao próprio passo de random
   walk (até +0.15 no tamanho do passo, escalado por `instability`) e,
   depois do puxão dos atratores, empresta um pouco de energia de volta
   pro campo (`abs(derivationMotion) * 0.02`, só uma vez por ciclo -
   acumula devagar, não é um pico). Nenhum dos dois sistemas manda no
   outro - cada um continua no seu próprio ritmo (DERIVA uma vez por
   loop, o campo todo sample com seu próprio retorno elástico a 0.2), só
   passam a se apoiar um pouco. Achado ao implementar: `ObjectFiveComponent`
   (a classe do CLONE) só guardava referência ao `fifth` (metade
   `SimpleSequencer`), não ao `DualObjectEngine` inteiro - precisou
   ganhar essa referência nova (`dualEngine`) pra função conseguir ler/
   escrever no campo compartilhado.
4. **Alcance da DERIVA estendido** - **feito, quase completo** (19 ago.
   2026, pergunta do autor: "há um ponto do antitotem a implementar...
   por exemplo slider de parametros, os osciladores, lfo, adsr,
   sequencer, etc"). Rodada 1: ADSR (attack/decay/sustain/release) e
   taxa do LFO, mesmo padrão de random walk em torno do valor atual.
   Rodada 2 (autor: "métrica, noise, groove, subdivisão" / "pans dos
   osciladores" / "filtro"): NOISE MIX, GROOVE, filtro (cutoff +
   resonância) e pans dos osciladores (-1..1, faixa própria) no mesmo
   padrão contínuo; MÉTRICA e SUBDIVISÃO precisaram de um mecanismo
   NOVO - são seleções discretas (botões em grupo de rádio), não
   sliders, então em vez de um blend contínuo usam um SALTO
   probabilístico (mesmo espírito do `routeMutates` da topologia, que já
   existia) - chance escalada por `activeDepth` de pular pra uma seleção
   nova, com `setToggleState` desligando a anterior automaticamente
   (mesmo radio group). Rodada 3 (autor: "noise cor também"): NOISE COR
   adicionado com o mesmo salto probabilístico - mais simples que
   MÉTRICA/SUBDIVISÃO porque `noiseSelector.select(index, true)` já
   dispara `onSelection` sozinha (chama `setNoiseColour` por dentro),
   sem precisar do passo manual de sync que os outros dois tiveram que
   fazer. Ainda não tocado: ENERGIA, MASTER - candidatos futuros, mesmo
   padrão.
5. ~~Instâncias paralelas + âncoras combináveis~~ - **feito** (19 ago.
   2026, autor: "as duas, testei o instrumentos, está ficando bem
   interessante essas novas implementações"). Três "motions"
   independentes agora, cada uma seu próprio random walk + atratores
   (A: original, steps/topologia/osciladores; B: mais lenta/calma, 2
   atratores, ADSR/LFO/NOISE MIX/GROOVE/filtro; C: mais rápida/inquieta,
   3 atratores mais largos, pans dos osciladores/MÉTRICA/SUBDIVISÃO/
   NOISE COR) - o instrumento evolui como processos paralelos com
   caráter distinto, não um bloco só se movendo junto. Âncoras: pool de
   2 posições (`derivationAnchors`) em vez de uma única - cada evento
   GLT ocupa a próxima posição livre, round-robin depois de cheio.
6. O restante (deriva probabilística, espacial, espectral, de
   articulação como eixo próprio, resposta não-linear por destino no
   Noise Field) fica registrado, sem prioridade definida.
7. ~~Assincronia real entre parâmetros + botões VCF A/B/C~~ - **feito**
   (19 ago. 2026, autor, depois de testar o lote das instâncias
   paralelas: "ainda me dá a impressão que as alterações estão
   acontecendo todas no mesmo momento"). Diagnóstico: cada bloco da
   DERIVA disparava em TODO ciclo de `deriveFromMemory()`, mesmo tendo
   Motions com velocidades diferentes - a velocidade só afetava o
   TAMANHO do passo do random walk por baixo, não SE um slider mudava
   de forma visível naquele ciclo. Por isso três motions com
   velocidades distintas ainda pareciam um bloco só se movendo junto.
   Correção: cada bloco (steps/osciladores/FX de A; ADSR/LFO/NOISE
   MIX/GROOVE/filtro de B; pans de C) ganhou seu próprio sorteio de
   "chance de agir" (`nextDerivationUnit() < chance`, valor distinto por
   bloco - ver seção 7 pros números), independente do sorteio de
   qualquer outro bloco - os que já eram seleções discretas
   (MÉTRICA/SUBDIVISÃO/NOISE COR) já tinham esse padrão desde a rodada
   4 acima e não precisaram de nada novo. O resultado é assíncrono de
   verdade: em qualquer ciclo dado, alguns parâmetros mudam e outros
   não, não porque um é "mais lento" e sim porque cada um jogou seu
   próprio dado. Junto: os botões VCF-style A/B/C (multi-select,
   `derivationLayers`, já existiam na UI desde a rodada anterior mas
   sem função) agora desligam de fato o grupo de blocos da Motion
   correspondente quando destacados - espaço independente por
   PRINCIPAL/CLONE, no mesmo lugar do botão DERIVA.
8. ~~Alcance estendido a ROTAS ATIVAS/MATÉRIA/CAOS + mixer~~ - **feito**
   (19 ago. 2026, autor: "rotas ativas não percebo" / "matéria também
   não" / "CAOS também não" / "sliders horizontais do mixer também
   não"). Achado: ROTAS ATIVAS/MATÉRIA/CAOS são o MESMO painel de 16
   sliders (`detailControls[0..15]`: 0-8 S&H/reverb/phaser/flanger/
   resonador, 9-12 MATÉRIA - cutoff/reson/drive/asym, 13-15 CAOS/VAGA -
   drive/damping/depth), nunca tinha participado da DERIVA - três
   reclamações, um gap só. `mixGain`/`mixPan`/`mixReflux` (os "sliders
   vermelhos" do mixer, 4 canais - FILTER/RING/NOISE/SPACE) idem.
   Implementado, mesmo padrão de memória por slider que steps/rates já
   usam (`derivationDetail[16]`, `derivationMixGain/Pan/Reflux[4]`,
   capturados em `captureDerivationMemory()`), com o clamp de cada um
   respeitando seu próprio range (`detailControls` 0..1; `mixGain`
   0..1.5; `mixPan` -1..1; `mixReflux` 0..0.72 - usar o range errado
   estouraria ou prenderia o slider). `detailControls` entra no grupo
   Motion A (textura/timbre, mesma família de osciladores/FX); mixer
   entra no grupo Motion B (balanço/caráter sonoro, mesma família de
   ADSR/filtro/groove). Achado técnico: `ObjectFiveComponent` (CLONE)
   sincroniza esses dois grupos através de lambdas locais ao construtor
   (`updateDetails`, `updateMixerChannel`), não acessíveis de
   `deriveFromMemory()` - resolvido com `juce::sendNotificationSync` em
   vez de `dontSendNotification` (deixa o `onValueChange` de cada
   slider fazer o sync sozinho), o mesmo truque que NOISE COR já usava
   (`noiseSelector.select(index, true)`). `MainComponent` (PRINCIPAL)
   tem `syncDetails()`/`syncMixer()` como métodos de verdade, mas usa o
   mesmo `sendNotificationSync` por uniformidade entre as duas cópias -
   um pouco redundante (`syncMixer()` roda de novo a cada um dos 12
   sliders do mixer, não só uma vez), mas barato o bastante (4 leituras
   de float) por só disparar no máximo uma vez por loop de 16 passos.
9. ~~Magnitude por evento de CV/AMP/FX recalibrada~~ - **feito** (19
   ago. 2026, autor, depois de testar o lote acima: "CV 16 steps muda
   somente o slider verde, bem pouco, o fx e amp não percebo ainda
   alterações"). CV/AMP/FX compartilham a MESMA fórmula de `drift` e o
   mesmo sorteio de chance dentro do loop de steps - se só CV mostrava
   algo, a causa não podia ser lógica (todos os três usam o idêntico
   `drift`), tinha que ser magnitude. Confirmado: a constante antiga
   (`0.025 + rand*0.105`, vezes `activeDepth` ~0.3-0.5 com
   DERIVA·PROFUNDIDADE no padrão 0.46) resultava num passo absoluto de
   milésimos por evento - visível só na barra vertical mais alta e mais
   longa (CV, "alongue os sliders verticais verdes"), invisível nas
   barras horizontais curtas (AMP/FX). Não era um bug desta sessão, era
   uma constante antiga (pré-data toda a rodada de assincronia) nunca
   recalibrada - só ficou evidente agora que o bloco inteiro já não
   roda em TODO ciclo. Corrigido: `0.14 + rand*0.40` (antes `0.025 +
   rand*0.105`), ~5-6x maior.
10. ~~AUTO: configuração de deriva por item autônomo~~ - **feito** (20
    ago. 2026, autor: "acho que vamos ter que criar um cérebro capaz de
    gerenciar item por item... vejo que as alterações são muito sutis,
    mais realizadas por parametrizações via código que algo inteligente
    capaz de vivenciar os fluxos e tomar decisões" / "penso em algo que
    cada item é autônomo" / "cada slide, cada knob, cada botão" / "pode
    fazer a C, mas sem destruir também o que já temos que é outra
    configuração possível"). Prior art correto pra este pedido: não
    `BiomaBrain::nextDrift()` (correlaciona vários organismos num campo
    compartilhado - o oposto de autônomo), e sim `Ecosystem.h` - cada
    organismo carrega sua própria `Strategy`/`ListeningMode`/`Integrity`,
    ninguém governa ninguém, cada um decide a partir do próprio estado.
    Implementado como uma QUARTA configuração (botão "AUTO", índice 3 do
    mesmo array VCF-style que já tinha A/B/C, `derivationLayers[3]`,
    desligado por padrão) - não uma variação de Motion A/B/C, uma
    configuração inteira alternativa: A/B/C continuam 100% intactos no
    ramo `else` de `deriveFromMemory()`, sem nenhuma linha alterada
    ("sem destruir também o que já temos que é outra configuração
    possível"). Mecanismo: `driftAutonomousItem()` (método novo, mesmo
    corpo nas duas cópias) dá a CADA slider individual (não grupo) uma
    "fome" própria (`hunger`) que cresce a cada ciclo em que aquele item
    não age e reseta quando age - tanto a chance de agir quanto o
    tamanho do salto escalam com a própria fome, então um item quieto há
    muito tempo fica mais provável de agir E dá um salto maior quando
    finalmente age. Sem coordenador central: nenhum item sabe da
    existência dos outros, cada um só lê seu próprio par fome/memória.
    Cobertura: os 16 CV/AMP/FX individualmente (não como um grupo só),
    5 rates, 3 FX, 16 ROTAS ATIVAS/MATÉRIA/CAOS, 12 sliders do mixer, 5
    pans, LFO, NOISE MIX, GROOVE, filtro cutoff/resonância, 4 ADSR, e os
    3 botões discretos (MÉTRICA/SUBDIVISÃO/NOISE COR) com a mesma fome
    aplicada a um salto em vez de um blend. Achado técnico: grupos que
    o modo A/B/C já cobria sem precisar de uma memória capturada (ADSR/
    LFO/NOISE MIX/GROOVE/filtro/pans/FX derivavam livremente em torno do
    valor ATUAL, não de um valor capturado) precisaram ganhar uma
    memória nova só pro modo AUTO (`derivationEffects/Envelope/Lfo/
    NoiseMix/Groove/FilterCutoff/FilterResonance/Pans`), já que
    `driftAutonomousItem()` sempre precisa de uma âncora própria por
    item - resetada em `captureDerivationMemory()` junto com todas as
    fomes (zeradas ali, não em `deriveFromMemory()`).
11. ~~FIM DO LOOP + CONEXÕES ENTRE OBJETOS~~ - **feito** (20 ago. 2026,
    retomando dois itens do próprio pedido original ainda não feitos:
    "os sliders vermelhos do mixer... FIM DO LOOP... cada um a sua
    maneira" e "conexoes entre objetos?" - este segundo com uma
    correção do autor no meio do trabalho: "isso não é pra duplicar,
    somente para que haja variação de deriva nos seus controles",
    confirmando que a implementação deveria existir só uma vez, não
    espelhada em CLONE). FIM DO LOOP (`loopSwitches`/`setLoopEnd`,
    seleção discreta 1-16, existe nas duas cópias) entrou no mesmo
    padrão de MÉTRICA/SUBDIVISÃO/NOISE COR - Motion C no modo A/B/C,
    fome própria (`hungerLoopEnd`) no modo AUTO. CONEXÕES ENTRE OBJETOS
    (`gainToFifth`/`gainToFirst`/`auxToFirst`/`auxToFifth`,
    `routesToFifth`/`routesToFirst`) implementado SÓ em `MainComponent`
    - esses controles vivem no `dualEngine` compartilhado, nunca
    tiveram cópia em `ObjectFiveComponent`, então não há nada pra
    "duplicar" pro lado do CLONE. Os 4 sliders de gain/aux entraram no
    grupo Motion B (mesmo padrão de blend do mixer, faixa 0..0.72,
    mesma que a UI já usa) tanto no modo A/B/C quanto no AUTO
    (`driftAutonomousItem`). Os 8 toggles de rota (`routesToFifth[4]` +
    `routesToFirst[4]`) são independentes (sem radio group, ao
    contrário de MÉTRICA/SUBDIVISÃO) - em vez de um salto pra uma
    seleção única, a DERIVA aqui sorteia um dos 8 e INVERTE seu estado
    (liga se tava desligado, desliga se tava ligado), tanto no modo
    A/B/C (chance fixa escalada por `activeDepthB`) quanto no AUTO
    (fome própria, `hungerObjectRoute`, compartilhada pelos 8 - não uma
    fome por botão).
12. ~~Bug: FIM DO LOOP travava em 1 (deadlock autoinfligido)~~ - **feito**
    (20 ago. 2026, autor: "travou no 1 do fim do loop, tanto no clone
    como no principal"). Causa: `deriveFromMemory()` só dispara quando o
    playhead VOLTA ao passo 0 depois de sair dele; com `loopEnd = 1` o
    playhead nunca sai do passo 0, então a condição de disparo nunca
    mais fica verdadeira - a própria DERIVA se travava ao sortear 1 pra
    si mesma. Autor sugeriu a correção antes dela terminar: "talvez
    definir que a deriva atue do 2 ao 16, nunca somente no 1". Corrigido
    nos 4 pontos onde FIM DO LOOP é sorteado - faixa `[2,16]`, nunca `1`.
13. ~~CV/16 STEPS: reshape de contorno + recalibração geral~~ - **feito**
    (20 ago. 2026, autor: "gostaria de mais variação nos sliders do cv
    16 steps, eles alteram mas sempre com o mesmo gráfico" / "a variação
    é sutil" / "que seja possível variar pouco ou bastante"). Achado:
    o random walk independente por passo preserva a ORDEM relativa
    entre os passos quase sempre (um jitter pequeno raramente faz dois
    passos se cruzarem) - por isso os valores mudavam mas o contorno
    geral do gráfico continuava parecendo o mesmo. Adicionado um
    "reshape" ocasional: troca de posição entre dois passos inteiros
    (CV+AMP+FX juntos, não só CV, pra manter a "voz" de cada passo
    coerente) - embaralha o próprio DESENHO, não só os valores dentro
    dele (passos ancorados ficam de fora no modo A/B/C; AUTO não tem
    esse conceito). Junto: passo de CV/AMP/FX recalibrado de novo
    (`0.14+rand*0.40` → `0.22+rand*0.55`, mesmo padrão de "achado real"
    das rodadas anteriores) e o helper `driftAutonomousItem()` também
    (fome cresce mais rápido, chance e mistura maiores). Pedido
    "pouco ou bastante": a chance de reshape tinha um piso alto demais
    (`0.22 + activeDepth*0.35`, 22% mesmo com profundidade baixa) -
    corrigido pra `0.03 + activeDepth*0.55`, que agora acompanha o
    knob DERIVA·PROFUNDIDADE de perto (quase nada perto de 0, bastante
    perto de 1), a mesma alavanca que já controla a magnitude do resto
    da DERIVA.
14. ~~Botões VCF/CORE dos osciladores + M1-4 (recall)~~ - **feito** (20
    ago. 2026, autor auditando botão por botão: "os botões do vcf não
    estão conectados a deriva" / "verifique se os 3 botões dos
    osciladores se conectam a deriva" / "e os botoes de memoria captura
    também" - as duas primeiras entraram direto (mesmo padrão dos
    toggles/saltos já existentes); a terceira, por ser um recall de
    ESTADO INTEIRO (não deriva incremental), foi por AskUserQuestion
    antes de implementar - escolhida "Só M1-4 (RECALL, nunca CAPTURE)".
    - `filterModeButtons` (LP/BP/HP/NOTCH combináveis, multi-select):
      mesmo mecanismo de INVERTER um sorteado que já existia pros
      toggles de rota de CONEXÕES ENTRE OBJETOS.
    - `coreSwitches` (40106/8038/4069UB, radio group): salto discreto,
      mesmo espírito de MÉTRICA/SUBDIVISÃO.
    - `mixMemorySlots` (M1-4): achado real ao investigar - um slot
      nunca capturado tem `enabled = false` nos 4 canais por padrão
      (`MutableMixer::Channel`'s próprio default), então recall nele
      silenciaria o mixer inteiro. Novo `mixMemoryCaptured[4]` (bool por
      slot, marcado só no `onClick` real de CAPTURE, nunca pela DERIVA)
      evita sortear um slot vazio - se o índice sorteado não foi
      capturado, o ciclo simplesmente não faz nada. CAPTURE em si NUNCA
      é chamado pela DERIVA (chamar `recallMixMemory()` direto, sem
      passar pelo `onClick` do botão, elimina de vez o risco de um
      timing acidentalmente disparar captura em vez de recall). Memória
      de deriva do mixer (`derivationMixGain/Pan/Reflux`) também
      atualizada no recall, senão o próximo ciclo de deriva incremental
      puxaria os sliders de volta pro valor pré-recall.
15. ~~Participação por título~~ - **feito** (20 ago. 2026, autor: "tive
    uma ideia para as seleções dos conteúdos a fazerem parte dos item a
    partiticar da deriva, ao lado de cada título um pequeno botão
    toogle (se o item on ele participa da deriva)... isso permite um
    controle por parte do usuário?"). Confirmado via AskUserQuestion:
    vale pros dois modos (A/B/C e AUTO) de uma vez, todos os ~16 títulos
    de uma vez (não um subconjunto primeiro). Escopo explicitamente
    excluído pelo autor durante a implementação: "os itens do
    cabeçalho não participam" (RUN/STOP/RESET/RECORD, abas SOUND/
    SEQUENCE/MIX, TUTORIAL/ABOUT, idioma - pura navegação), "não quero
    deriva no master, nem no osciloscopio e nem no mixer objetos"
    (nenhum dos três nunca esteve no escopo mesmo), e VARIAÇÃO (já
    excluído numa rodada anterior).

    16 `juce::ToggleButton` novos (15 em CLONE, +1 `participateConnections`
    só em PRINCIPAL - CONEXÕES ENTRE OBJETOS não existe em CLONE), sem
    texto (só um quadrado 10x10), `componentID("core")`, ligados por
    padrão. Posicionados no CANTO de cada label já existente
    (`label.getRight()-10, label.getY()`), lidos DEPOIS de todo o resto
    do `resized()` já ter calculado os bounds finais - nenhuma linha de
    layout existente precisou mudar (autor: "acho que o botão pode ser
    pequeno pra não alterar o layout"). Agrupamento por título
    compartilhado onde fazia sentido (LFO+NOISE MIX sob "MODULAÇÃO";
    cutoff+resonância+VCF sob "VCF"; CORE+rates+pans dos osciladores sob
    "OSC"), não um toggle por mecanismo interno.

    Mecanismo: cada bloco já existente ganhou `participateXxx.getToggleState()
    && ` na frente da sua própria condição (`if (nextDerivationUnit() <
    chance)`) - uma alteração cirúrgica, não uma reestruturação; blocos
    sem condição própria (for-loops/chamadas diretas de
    `driftAutonomousItem()`) ganharam um `if (participateXxx...)` novo
    envolvendo o statement. Achado real: PORTAS DE FEEDBACK (o bitmask
    de rotas/`routeMutates`) rodava incondicionalmente ANTES de
    qualquer camada A/B/C/AUTO, e sua saída (`routeDensity`) é lida por
    blocos DEPOIS dele (FX/steps) - precisou virar uma variável mutável
    declarada por fora com default 0, só computada de fato dentro do
    novo `if (participateRoutes...)`, pra continuar existindo com um
    valor neutro quando desligado sem quebrar quem lê depois.

    Nota de processo: a primeira tentativa de aplicar os ~40 gates via
    script continha alguns `prepend_text` com quebra de linha embutida,
    que aumentam o número de linhas FÍSICAS do arquivo sem mudar o
    tamanho da lista Python usada internamente - a verificação inicial
    via `sed -n` (que lê números de linha do arquivo já modificado)
    pareceu apontar pra um bug real, mas era só uma discrepância entre
    numeração de linha "antes" vs. "depois" da própria execução do
    script; `grep` por texto (não por número de linha) confirmou que
    todos os gates foram aplicados no lugar certo.

## 7. Calibração pendente (19 ago. 2026)

Ver a nota geral em `PESQUISA_MELODIA_GENERATIVA.md`, seção 7. Todas em
`Main.cpp`, salvo indicação contrária:

- Atratores: força do puxão 0.12, posições -0.4/0.0/+0.4.
- ADSR/LFO/NOISE MIX/GROOVE/filtro (padrão comum): faixa de alvo
  `0.2 + activeDepth*0.3`, taxa de mistura `0.05 + activeDepth*0.20`.
- Pans dos osciladores: faixa de alvo mais larga (`0.4 + activeDepth*0.6`,
  -1..1) que o padrão comum acima - nunca comparada contra o padrão por
  escuta, só escolhida por ter faixa maior (-1..1 vs 0..1).
- MÉTRICA/SUBDIVISÃO/NOISE COR (seleções discretas): chance de salto
  `0.06 + activeDepth*0.18`.
- Conexão Noise Field ↔ DERIVA: `instability` soma até +0.15 no passo de
  `derivationMotion`; `derivationMotion` devolve `abs(valor) * 0.02` ao
  campo por ciclo.
- Instâncias paralelas B/C (novo, 19 ago. 2026): velocidades (`0.08 +
  userDepth*0.12 + instability*0.08` pra B, `0.24 + userDepth*0.30 +
  instability*0.20` pra C), posições dos atratores (-0.3/+0.3 pra B,
  -0.5/0.0/+0.5 pra C), força do puxão (0.18 pra B, 0.06 pra C) - os
  números que dão o caráter "calma" vs. "inquieta" a cada uma nunca
  foram comparados por escuta contra a Motion A original nem entre si.
- Pool de âncoras: 2 posições, escolhidas sem comparar contra 1 ou 3 por
  escuta.
- Chance de agir por bloco (novo, 19 ago. 2026): steps 0.85, osciladores
  (rates) 0.5, FX 0.45 (Motion A); ADSR 0.55, LFO 0.65, NOISE MIX 0.6,
  GROOVE 0.5, filtro cutoff 0.45, filtro resonância 0.4 (Motion B); pans
  0.5 (Motion C, MÉTRICA/SUBDIVISÃO/NOISE COR mantêm sua própria chance
  de salto já calibrada como pendente acima). Números escolhidos só pra
  garantir que nenhum bloco fique preso em "sempre" ou "quase nunca"
  (steps mais alto por ser o efeito mais estruturalmente central da
  DERIVA), nunca comparados por escuta entre si.
- Magnitude por evento, recalibrada no mesmo dia (autor, testando o
  build anterior: "ainda não vejo variação nos sliders do CV, 16 steps,
  conexão entre objetos, em alguns outros sliders"). Achado: a chance
  de agir por bloco reduz a FREQUÊNCIA de eventos sem aumentar o quanto
  cada evento move o valor - blocos com chance baixa (ex. filtro
  resonância, 0.4) ficaram proporcionalmente MENOS perceptíveis, não só
  mais assíncronos, o oposto do que se buscava. Corrigido: a taxa de
  mistura em direção ao alvo (antes uniforme `0.05 + profundidade*0.20`
  em quase todo bloco) agora escala na direção OPOSTA da chance - blocos
  que agem com menos frequência dão um passo maior quando agem
  (osciladores 0.09+0.32, FX 0.10+0.28, ADSR 0.08+0.30, LFO 0.07+0.25,
  NOISE MIX 0.08+0.28, GROOVE 0.09+0.30, filtro cutoff 0.10+0.32, filtro
  resonância 0.11+0.34, pans 0.09+0.30) - lê como um evento decisivo, não
  como um gotejamento mais lento. Números ainda não comparados por
  escuta uns contra os outros. Nota à parte, não uma correção: CV/16
  STEPS usa a chance mais alta (0.85) e já tinha o passo maior desde
  antes (fórmula própria, não a comum) - se ainda não visível, o suspeito
  mais provável é o DERIVA·PROFUNDIDADE estar baixo ou o botão DERIVA
  em si (não os A/B/C, que são camadas por CIMA dele) estar desligado,
  não a chance/magnitude. "Conexão entre objetos" nunca participou da
  DERIVA (vive em `DualObjectEngine`, não em `deriveFromMemory()`) -
  não é uma regressão desta rodada, é um alcance ainda não implementado
  (ver seção 6, item 6 e a pendência de PRINCIPAL/CLONE mixer red
  sliders/FIM DO LOOP no item de "próximos passos" do TAREFAS.md).
- AUTO (novo, 20 ago. 2026): fome cresce `+0.05` por ciclo (item que
  nunca age chega no teto `1.0` em 20 ciclos), chance de agir
  `0.04 + fome*0.5` (varia de 4% recém-acalmado a 54% no teto de fome),
  taxa de mistura `0.08 + fome*0.5` (8% a 58%). Botões discretos
  (MÉTRICA/SUBDIVISÃO/NOISE COR): mesma curva de fome, chance de salto
  `0.03 + fome*0.4`. Nenhum desses 4 números foi comparado por escuta
  contra os equivalentes do modo A/B/C nem entre si - só escolhidos pra
  garantir que a fome realmente mude o comportamento perceptível (de
  "quase nunca, passo pequeno" pra "frequente, passo grande"), não
  testados quanto ao RITMO em que os itens se revezam de verdade.

**Itens ANTIGOS que pedem revisão explícita, por serem literalmente o
núcleo original da DERIVA, nunca recalibrados desde então**: a fórmula
de `derivationMotion` em si (`(nextDerivationUnit()-0.5) * (0.16 +
userDepth*0.24)`), a chance de mutação de topologia (`0.18 +
activeDepth*0.66`), e a leitura da memória histórica (`0.78 -
activeDepth*0.32`) - todas datam de ANTES desta sessão (o próprio
`captureDerivationMemory()`/`deriveFromMemory()` já existiam prontos
quando `PESQUISA_DERIVA_GENERATIVA.md` foi escrito). Com atratores,
step/LFO/ADSR/NOISE/GROOVE/filtro/pans, MÉTRICA/SUBDIVISÃO/NOISE COR e
a conexão ao Noise Field TODOS somados por cima dessas fórmulas
originais hoje, vale ouvir a DERIVA como um todo de novo - o
comportamento agregado pode já ter passado do ponto em que essas
constantes de base foram escolhidas.

## 8. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, tabela da
seção 5, entrada `CRI-DRF-001`.
