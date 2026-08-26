# Pesquisa: sequenciador generativo em camadas

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-SEQ-001`. Irmão direto de
[`PESQUISA_MELODIA_GENERATIVA.md`](PESQUISA_MELODIA_GENERATIVA.md)
(`CRI-MEL-001`) - o autor conectou os dois explicitamente (ver seção 5).

## 1. Por que este documento existe

Depois do brief sobre interpretação melódica, o autor colou um segundo
brief (mesma origem: "também uma conversa com o chatgpt"), desta vez
sobre o SEQUENCIADOR em si - não a voz que interpreta seus eventos, mas
as camadas generativas que poderiam produzir/deformar os eventos antes
disso. Pedido explícito: "verificar e aprofundar" - mesma exigência de
rigor do documento anterior (checar o que o projeto já tem, citar fontes
reais, separar interpretação de fato verificado).

## 2. O que já existe no projeto RASGO sobre este assunto

### 2.1 `RASGO_SYNTH/rasgo-synth-core/src/sequencer/` (prior art de código)

- **`Euclidean.hpp`** - implementação real do algoritmo de Bjorklund
  (distribuição uniforme de `k` pulsos em `n` passos), citando
  corretamente que essa é a lógica por trás do "ritmo euclidiano"
  (Steve Reich e incontáveis drum machines) - o próprio brief do autor
  já usa a notação certa (`E(5,8)`), que bate exatamente com o paper de
  referência real: **Toussaint, G. (2005). "The Euclidean Algorithm
  Generates Traditional Musical Rhythms."** *Proceedings of BRIDGES*.
- **`PolymeterEnsemble.hpp`** - polimetria real já implementada: vozes
  independentes com seus próprios comprimentos de ciclo em passos
  (conjunto coprimo, ex. "melodia 11, baixo 7, harmonia 5, percussão 8"
  - quase idêntico ao exemplo do brief atual, "pitch pattern 16, accent
  pattern 7, vibrato pattern 5, glide pattern 11"), reencontro num
  macroperíodo real (MMC dos comprimentos), mais um "pulso não-isócrono"
  (intervalos livres somando o comprimento do ciclo) citando **London,
  J. (2004). *Hearing in Time: Psychological Aspects of Musical
  Meter*** - referência acadêmica real pra métrica não-isócrona. Também
  já implementa "aksak" (célula assimétrica turca/balcânica real) e um
  "polyrhythm divider" próprio (XOR de divisões de um único relógio
  compartilhado - mecanismo DIFERENTE de polimetria de verdade, o
  próprio comentário do arquivo faz questão de distinguir os dois).
- **`CoherenceCollapse.hpp`** - já implementa o que o brief atual chama
  de "Entropia": entropia de Shannon real
  (`H = -Σp·log2(p)`, **Shannon, C. (1948). "A Mathematical Theory of
  Communication."** *Bell System Technical Journal*) sobre a
  distribuição de classes de altura das últimas N notas - dispara uma
  "ruptura por coerência excessiva" quando a entropia fica baixa demais
  por barras consecutivas, não um efeito pontual.
- **`GenerativeArc.hpp`** - já opera no nível mais alto que o brief
  chama de "PHRASE": um walk markoviano sobre papéis estruturais
  (Intro/Rising/Peak/Falling/Coda), com "tension"/"energy" implícitos
  na energia rolada por seção - conceitualmente paralelo a
  `tension`/`CALM → TENSION → CLIMAX → RELEASE` do brief atual, embora
  em escala de peça inteira, não de frase curta.
- Ainda não sondados a fundo neste levantamento:
  `sequencer/StepSequencer.hpp` (provavelmente já tem alguma forma de
  probabilidade/condição por passo, dado o nome), `SieveScale.hpp`
  (sieves de Xenakis - poderiam informar subdivisões não-binárias),
  `NeoRiemannian.hpp`, `MicrotonalTuning.hpp`, `BluffSignal.hpp`.

### 2.2 Instrumentos irmãos além do RASGO_SYNTH (autor perguntou: "o que temos nos outros instrumentos sobre o assunto")

Varredura por termos do brief (euclidean/ratchet/polymeter/swing/groove/
humaniz/entropy/probability/glitch/máscara) em AQUORBIUM, TRIOIO,
NAVALHA2_JUCE, NAVALHA2_PD, RASGO_MODULAR. Dois achados reais, diretamente
relevantes - nenhum tinha sido lido antes deste documento:

- **AQUORBIUM - `GeometricSequencer.h`/`.cpp` + `GEOMETRIC_SEQUENCER_
  DESIGN.md`** (marco 1, já implementado, não especulativo - código
  real lido por completo, não só o design doc). Um sequenciador
  generativo real com exatamente o modelo de camadas que o brief atual
  descreve, construído de forma independente (sem ler este documento
  nem o brief). Peças diretamente equivalentes:
  - `EuclideanPattern` - mesmo algoritmo de Bjorklund, mesma fonte
    primária já citada aqui (Toussaint), até 32 posições, com rotação -
    "gera e gira" é literalmente a "Rotação" (#20) e o "Euclidean como
    força aplicada" (#21) do brief, já funcionando.
  - `Step` - `struct` com `gate, acento, grau, pitch` e 4 CVs
    normalizadas - **já tem um campo de ACENTO por passo separado do
    gate**, exatamente o `accent_weight` (#1) que o ANTITOTEM não tem
    hoje (ver 2.3 abaixo).
  - `GeometricSequencer` - 5 "lanes" independentes sobre uma FORMA
    geométrica compartilhada (não ciclos de comprimentos diferentes
    como `PolymeterEnsemble.hpp` do RASGO_SYNTH - um mecanismo de
    polimetria genuinamente diferente, cada lane com fase própria lendo
    a mesma geometria: "relação de cardume sem cinco sequências
    idênticas").
  - `GenerativeInput` (densidade, coesão, perigo, raiz, âncora,
    registro, seção) vindo do ecossistema Bioma do próprio Aquorbium -
    e `PerformanceIntent` (densidade rítmica, rotação, viés intervalar,
    mutação) com um parâmetro `influence` contínuo (0=autônomo puro,
    1=intenção de performance inteira, intermediário SOMA viés ao
    estado ecológico em vez de trocar de motor) - modelo real e já
    testado pra exatamente o tipo de "quanto controle humano vs.
    quanto generativo" que qualquer implementação dos eixos do brief
    atual (#3 probabilidade, #4 condições, #25 morph) vai precisar
    decidir.
  - **Detalhes confirmados lendo o `.cpp`**: `EuclideanPattern::
    generate()` não usa o algoritmo recursivo clássico de Bjorklund
    (o de `RASGO_SYNTH/Euclidean.hpp`), usa uma construção por resíduo
    circular (`(step*pulseCount) % stepCount`) - matematicamente
    equivalente, mais simples de auditar, comentário do próprio código
    já documenta a escolha. `accent` tem uma fórmula concreta e real
    pra "peso de acento" (#1 do brief): downbeat (`orbitStep==0`) recebe
    0.92 base, os outros passos ativos 0.58, ambos somando um bônus de
    coesão (até +0.18) e de mutação (+0.08) - um exemplo funcional de
    "acento controla mais do que só volume" sem precisar inventar do
    zero. `mutation`/`danger` decidem uma CHANCE de mutação por evento
    (não uma caminhada correlacionada tipo "motor drift" da EXCITAÇÃO -
    é uma escolha independente por evento, uma técnica genuinamente
    diferente pro mesmo problema de humanização, ambas válidas).
    **Coincidência real**: `nextRandomUnit()` usa exatamente as mesmas
    constantes de LCG (`1664525u`/`1013904223u`, Numerical Recipes) que
    a EXCITAÇÃO já usava antes de eu ler este código - convergência
    independente pra um LCG padrão bem conhecido, não cópia.
  - **Explicitamente fora de escopo no marco 1 do Aquorbium** (mesma
    lista, quase palavra por palavra, de itens que este documento
    também registra como não feitos): "swing, memória de passos
    editável, quatro valores persistentes por passo... matriz de
    permutação" - confirma que mesmo o instrumento mais avançado da
    família nessa frente ainda não resolveu swing/permutação, não é
    peculiaridade do ANTITOTEM ficar pra trás nisso.
- **`RASGO_MODULAR/RASGO_MODULAR.md`** (documento vivo de arquitetura,
  v0.1, 16 ago. 2026 - 4 dias antes desta pesquisa): o ambiente modular
  próprio da família RASGO, com objetivo explícito de ser "base
  tecnológica para outros instrumentos Rasgo" e "biblioteca de módulos
  reutilizáveis" - é provavelmente o CONTEXTO original da conversa que
  gerou o brief do autor (a primeira mensagem já dizia "ele fala mais
  sobre o rasgo modular"). Tem uma taxonomia de categorias de módulo
  que já nomeia boa parte do vocabulário do brief:
  - Categoria **TIME**: clock, divider, multiplier, duration, pulse,
    **swing**, drift, irregular clock, phase, temporal field.
  - Categoria **DECISION**: chance, Bernoulli, comparator, logic, rule,
    threshold, branch, **probabilistic gate**, conflict, **conditional
    routing** - nomeia exatamente #3 (Probabilidade) e #4 (Condições)
    do brief atual como uma categoria própria de módulo, não um detalhe
    de implementação.
  - Categoria **SEQUENCE**: step, grid, pattern, roll, random sequence,
    chord sequence, physical sequence, generative path, recorded
    sequence, recursive sequence - com o princípio já registrado
    "sequenciar é uma família de comportamentos", o mesmo espírito do
    brief atual tratando glitch/stutter/fill como uma família de
    operações, não um efeito único.
  - Este documento é conceitual/taxonômico (categorias de módulo), não
    uma implementação como o `GeometricSequencer` - mas confirma que a
    LINGUAGEM do brief atual já é reconhecida e nomeada no projeto,
    independente deste documento.

### 2.3 `ANTITOTEM/src/core/SimpleSequencer.h` (o que o sequenciador deste instrumento já tem)

Modelo por passo já existente, 16 passos (`stepCount`):

- `voltages[]` - altura/CV por passo (já existe).
- `levels[]` - nível/velocity por passo (já existe - cobre parte de
  "microdinâmica" do brief).
- `muted[]` - gate on/off por passo (já existe - cobre "trigger
  probability" só no extremo binário, sem probabilidade real).
- `effectSends[]` - quanto cada passo envia pros efeitos (já existe,
  não tem equivalente direto no brief, é uma dimensão própria do
  ANTITOTEM).

O que o brief descreve e o ANTITOTEM **não tem hoje**: peso de acento
por passo separado de `levels[]` (ACENTO/MÉTRICA, ver `TAREFAS.md`,
opera em padrão GLOBAL - a cada N passos - não por passo individual),
probabilidade de disparo, condições (a cada 2/3 ciclos, depende do
passo anterior), ratchets, subdivisão não-binária por passo, swing
variável no tempo (SWING hoje é um valor único de GROOVE, não uma curva
por passo), rotação, morph entre padrões, glitch como família de
operações (GLITCH hoje só existe como um `ClockFeel`, um multiplicador
de tempo fixo por posição no ciclo - não as operações descritas na
seção 3.15 abaixo).

## 3. O brief do autor (fonte secundária/interpretativa, não primária)

Mesma ressalva do documento irmão: organizado e nomeado com clareza,
mas sem citações formais - valor está na estrutura do pensamento, não em
alegações de precisão técnica. Resumo dos eixos, agrupados como o
próprio brief sugeriu no final (STEP / CYCLE / PHRASE):

### Nível STEP (atributo de um passo individual)

1. **Acentuação** (`accent_weight`) - controla simultaneamente velocity,
   duração, brilho, ataque, quantidade de vibrato, probabilidade de
   ornamentação, intensidade de portamento - acento deixa de ser só
   volume.
2. **Microdinâmica** - variação passo a passo (a escala mais fina de
   "Dinâmica", que o brief separa em 3 escalas: micro/meso/macro).
3. **Probabilidade** - não um número só: trigger probability, accent
   probability, mutation probability, subdivision probability, pitch/
   timing deviation probability, ornament probability - um campo
   probabilístico por passo.
4. **Condições** - toca sempre / a cada N ciclos / só se o anterior
   tocou (ou não tocou) / só na primeira ou última passagem / quando
   densidade ou energia cruzam um limiar.
5. **Subdivisão** - cada passo pode abrir um nível temporal inferior,
   inclusive não-binário (3, 5, 7, 9, 11) - "um passo pode gerar um
   pequeno burst".
6. **Ratchets** - repetições dentro de um passo com envelope próprio
   (crescendo/decrescendo, acelerando/desacelerando) - "quase um
   microfraseador".
7. **Ghost notes** - eventos secundários de baixa velocity entre
   eventos principais - "num instrumento melódico isso pode virar nota
   de aproximação".
8. **Flam** - dois eventos quase simultâneos com poucos ms de
   separação - "no theremin talvez pitch scoop, grace note, ataque
   duplo".
9. **Microtiming como função, não ruído** - acentos levemente
   antecipados, notas fracas atrasadas, fim de frase desacelerado,
   início adiantado.
10. **Elasticidade temporal / "time borrowing"** - um passo pode
    "roubar" duração do seguinte, mantendo o período total.

### Nível CYCLE (comportamento de um ciclo/grupo de passos)

11. **Dinâmica meso** - crescendo/decrescendo/ondas dentro de um bloco
    de passos (envelopes generativos sobre blocos).
12. **Groove como campo de deslocamento temporal** - duas
    temporalidades: "grid time" e "performed time", cada posição com
    seu próprio `timing_offset` (fixo, probabilístico, aprendido,
    matemático, dependente de densidade/frase).
13. **Swing** - não um valor fixo (`swing = 62%`), um "swing field": uma
    curva temporal (`58 61 63 66 62...`), podendo variar por compasso,
    aumentar gradualmente, ser diferente por voz, ser negativo ou
    irregular.
14. **Dependência entre passos** ("gramática temporal") - step 3
    "conhece" o que aconteceu no step 2: forte→tende a fraco em
    seguida; 3 notas seguidas→aumenta chance de silêncio; salto
    melódico grande→reduz chance de outro salto; ratchet→evita ratchet
    logo depois.
15. **Densidade** (0-1 abstrato) - controla não só quantos passos tocam,
    mas subdivisão, ghost notes, duração, probabilidade, complexidade
    rítmica.
16. **Humanização com memória** - `timing/velocity/duration/pitch
    drift` correlacionados (`+2, +4, +7, +5, +1, -2 ms`), não
    `random(-10,+10)` independente a cada evento - mesma técnica
    ("motor drift"/"gesture inertia") já usada no documento irmão pra
    pitch, agora generalizada pra qualquer parâmetro.
17. **Glitch como família de operações** sobre uma janela de eventos
    (repetir, omitir, inverter, congelar, truncar, duplicar, deslocar,
    subdividir, permutar, retriggerar, comprimir, esticar) - não um
    efeito de áudio, uma transformação estrutural sobre a sequência de
    eventos.
18. **Stutter** - repetição controlada e acelerável, útil em transições.
19. **Fill** - reconhecer fim de ciclo e aumentar complexidade
    (densidade, subdivisão, ornamentação, faixa de altura, glitch,
    densidade de acento) - gera macroforma.
20. **Rotação** - deslocar o padrão sem alterar seu conteúdo estrutural
    - variação que preserva identidade.
21. **Euclidean patterns como força aplicada**, não só gerador inicial -
    um parâmetro contínuo de "influência euclidiana" (0-1) que aproxima
    ou afasta a sequência existente de uma distribuição euclidiana.
22. **Polimetria** - camadas diferentes em números de passos diferentes
    (pitch/accent/vibrato/glide cada um com seu próprio ciclo),
    reencontrando-se depois.
23. **Polirritmia** - o mesmo relógio-base subdividido internamente por
    módulos diferentes (3, 5, 7) simultaneamente.
24. **Máscaras** - cada comportamento (NOTE/ACCENT/GLITCH/RATCHET/TIE)
    como uma camada booleana independente que se combina com as
    outras, em vez de tudo armazenado num único step monolítico.
25. **Morph** - interpolação contínua (0-1) entre dois padrões inteiros
    (ritmo, acentos, velocity, groove, pitch, probabilidade,
    subdivisão).

### Nível PHRASE (trajetória de escala maior)

26. **Macrodinâmica** - arco de intensidade sobre uma frase inteira
    (`pp → mf → f → p`).
27. **Entropia** - conceito global, baixa (repetição/regularidade) a
    alta (mutação/irregularidade) - "complexidade ≠ densidade": pode
    haver poucos eventos extremamente imprevisíveis.
28. **Tensão** (`tension = 0-1`) - afeta intervalos, dissonância,
    antecipações, acentos, densidade, duração, subdivisão,
    instabilidade temporal, glitch - com uma curva
    `CALM → TENSION → CLIMAX → RELEASE`.

### Arquitetura proposta

Uma cadeia de camadas (`BASE SEQUENCE → STRUCTURAL ANALYSIS → ACCENT
FIELD → DYNAMIC FIELD → GROOVE/MICROTIMING → SUBDIVISION/RATCHET →
PROBABILITY/CONDITIONS → MUTATION → GLITCH → HUMANIZATION → EVENT
STREAM`), algumas atuando em paralelo, mais a distinção de três níveis
já usada pra organizar este resumo (**o step possui atributos; o ciclo
possui comportamento; a frase possui trajetória**).

### A ponte que o autor já traçou com o documento irmão

Registrada explicitamente pelo próprio autor, não interpretação minha:
"acento do step → articulação, groove → timing da execução, densidade →
ornamentação, tensão da frase → vibrato/glissando/dinâmica, glitch →
ruptura deliberada da continuidade melódica" - ou seja, este documento
alimentaria o `MELODIC INTERPRETER` do documento irmão, não o substitui.

## 4. Cruzamento com o que a EXCITAÇÃO já faz (ANTITOTEM)

A EXCITAÇÃO não lê o sequenciador de passos diretamente hoje - ela reage
à atividade combinada de PRINCIPAL+CLONE (ver `TAREFAS.md`, 20 ago.
2026), então os conceitos de STEP/CYCLE deste documento não se aplicam a
ela diretamente ainda. Onde este material já se conecta com trabalho
real:

- **#16 Humanização com memória** = exatamente a mesma técnica do "motor
  drift"/passo limitado já implementado no passeio da EXCITAÇÃO (ver
  `PESQUISA_MELODIA_GENERATIVA.md`, item #18/já feito) - o brief atual
  só generaliza a mesma ideia pra outros parâmetros além de pitch.
- **#27 Entropia** já tem prior art real e citável (`CoherenceCollapse.
  hpp`, Shannon 1948) - poderia se conectar ao MÉTRICA/ACENTO ou ao
  próprio disparo de EXCITAÇÃO como um limiar alternativo/complementar
  ao detector de derivada atual.
- **ACENTO (MÉTRICA)** do ANTITOTEM já é, estruturalmente, uma versão
  bem mais simples do #1 (`accent_weight`) - hoje só controla volume a
  cada N passos; o brief sugere que um peso de acento poderia controlar
  MUITAS outras coisas ao mesmo tempo (timbre, vibrato, probabilidade de
  ornamento).
- **SUBDIVISÃO/SWING/GROOVE** do ANTITOTEM (ver `TAREFAS.md`, sessão de
  20 ago. 2026 inteira) já cobrem parte do que o brief chama de
  "Swing"/"Groove"/"Subdivisão" no nível CYCLE - mas como valores
  ÚNICOS por vez (um SUBDIVISÃO ativo, um GROOVE amount), não como
  campos/curvas variando no tempo (o "swing field" do brief).

## 5. Próximos passos (não decididos - aguardando o autor)

Este documento é deliberadamente só registro + verificação nesta rodada
- nenhuma implementação de código ainda, dado o tamanho real do
material (28 eixos). Coerente com "sem muitas padronizações" já pedido
pelo autor antes: a lição não é implementar tudo, é escolher poucos
pontos de entrada baratos. Candidatos que reaproveitariam código já
existente sem exigir uma arquitetura de camadas nova:

- Estender o "motor drift" que a EXCITAÇÃO já usa pra pitch (#16) e
  aplicar a mesma técnica ao GROOVE/timing do sequenciador principal -
  reaproveita a mesma lição, código quase idêntico.
- Conectar `CoherenceCollapse`-style entropy (#27) como uma segunda
  fonte de "estímulo" pra EXCITAÇÃO, complementando o detector de
  derivada atual - não exige nova infraestrutura de sequenciador, só um
  novo tipo de gatilho.
- Qualquer coisa no nível STEP/CYCLE (probabilidade, condições,
  ratchets, máscaras) exigiria mexer na estrutura de dados do
  `SimpleSequencer` em si (`voltages[]`/`levels[]`/`muted[]` etc.) - bem
  mais invasivo, não recomendado como primeiro passo.
- **Código do `GeometricSequencer` lido por completo** (20 ago. 2026,
  autor: "prossiga"). Achado importante que muda a recomendação: é uma
  classe de TAXA DE EVENTO, não de taxa de áudio - `advance()` é
  chamado uma vez por evento externo (ecológico ou de performance), sem
  clock, sem processamento de áudio dentro. O `SimpleSequencer` do
  ANTITOTEM é o oposto - tudo roda dentro de `renderSample()`, taxa de
  áudio, chamado uma vez por sample. Reuso direto (herdar/incluir a
  classe inteira) exigiria uma camada adaptadora; mas as PEÇAS
  algorítmicas isoladas (a própria `EuclideanPattern`, a fórmula de
  `accent` por downbeat/coesão/mutação, o padrão de "chance de mutação"
  independente por evento) já são puras (sem estado de áudio), então
  são diretamente portáveis como técnica/referência de implementação,
  mesmo sem compartilhar código C++ de fato. Continua valendo como
  candidato a módulo compartilhado de verdade (`CRI-DSP-001`/
  arquitetura de saída comum já é precedente de promoção), mas a
  adaptação de taxa de evento→áudio precisaria ser resolvida primeiro -
  não é um "importar e pronto".

## 7. Segundo brief, mais profundo (19 ago. 2026)

Autor colou um segundo brief do ChatGPT sobre o mesmo tema ("mais um
dialogo com o chatgpt, agora sobre sequencer: aprofunde, analise,
documente") - não um tema novo, uma segunda passada mais profunda sobre
o MESMO sequenciador, por isso continua registrado sob `CRI-SEQ-001`,
não um ID novo. Onde o primeiro brief (seção 3) organizava STEP/CYCLE/
PHRASE como três escalas de atributo, este vai mais fundo em
ARQUITETURA: o sequenciador como grafo, o playhead como um agente físico
(momentum/atratores), o step como zona de potencialidade (não valor
fixo), estados globais do sistema, ecologia entre instrumentos, e uma
distinção conceitual nova entre GLITCH (perturbação pontual) e
**RASGO/RUPTURE** (ruptura da própria continuidade estrutural).

### 7.1 Resumo dos eixos novos (não repetindo os 28 já registrados na seção 3)

- **Três concepções simultâneas de sequenciador**: grid (grade fixa),
  flow (corrente deformável), behavior (decide como continuar a partir
  do que aconteceu) - o próprio caminho como material musical, não só
  o conteúdo.
- **Step como zona de potencialidade**: em vez de valores fixos, um
  step carrega chances (`trigger 0.80, accent 0.35, ratchet 0.15,
  glitch 0.05, ornament 0.40`) - resolvidas no momento do evento, não
  pré-determinadas.
- **Step state** (DORMANT/ACTIVE/HOT/EXHAUSTED/LOCKED/MUTATING) e
  **step memory** (quantas vezes tocou, última vez, últimos vizinhos) -
  produz "fadiga": um step muito repetido fica temporariamente menos
  provável, recupera devagar.
- **Sequenciamento condicional real** (`IF previous_step_triggered THEN
  probability=.25`) e **causalidade entre eventos** (acento aumenta
  chance de silêncio depois; ratchet reduz chance de outro ratchet) -
  uma gramática de causa/efeito entre passos, não regras isoladas.
- **Playhead como agente físico**: comportamentos (forward/backward/
  ping-pong/random/walk/skip/jump/branch/spiral), **weighted walk**
  (vizinhos mais prováveis que saltos distantes), **inércia** (tendência
  de continuar na mesma direção), **atratores** (alguns steps "puxam" o
  cursor de volta) e **repelentes** (um step recém-visitado fica
  temporariamente menos provável).
- **Sequenciador topológico/grafo**: passos como nós conectados por
  arestas com peso/probabilidade, as próprias conexões podendo mudar
  com o tempo ("topological drift" do sequenciador).
- **Comprimento de ciclo vivo** (`cycle drift`: 16→15→16→14→17) e
  **ciclo local por camada** (pitch com 13 passos, acento com 7,
  velocity com 9 - o evento final é a INTERSEÇÃO, produzindo
  combinações nunca programadas explicitamente) - e **fase** (mesmo
  padrão, deslocado no tempo - "phase drift").
- **Sequenciador multicamada** (trigger/pitch/accent/gate/timing/
  timbre/noise/glitch como sequências paralelas e independentes) e
  **máscaras hierárquicas** (global→frase→ciclo→step, um evento só
  acontece se passar por todos os níveis).
- **Meta-sequenciador / sequenciador de regras**: o conteúdo do
  sequenciador pode ser COMANDOS sobre outro sequenciador
  (`step1=NORMAL, step2=ROTATE, step3=MUTATE`), ou cada step escolhe uma
  regra de produção (`transpose+5, silence, mutate`) em vez de conter
  música diretamente.
- **Tipos de mutação**: local (um step), regional (um grupo),
  estrutural (comprimento/direção/roteamento), comportamental (muda uma
  regra) - distintos por ESCALA, não um "mutate" genérico.
- **Mutation memory/hereditariedade**: guardar o original, permitir
  retorno parcial (`30% original, 70% mutante`) - "morphing histórico" -
  e padrões que nascem de outros preservando algumas características
  ("linhagens").
- **Novelty vs. repetition**: dois eixos distintos, não opostos de um
  só (novelty mede distância do passado; entropy mede imprevisibilidade
  - conceitos diferentes mesmo correlacionados).
- **Motivos como objetos**: identificar células (`A = X.XX`), tratá-las
  como material e aplicar transformações reais de teoria motívica
  (rotação, inversão, retrógrado, aumentação, diminuição, fragmentação).
- **Silêncio como objeto**, não `gate=off`: duração/peso/probabilidade/
  função próprios, tipos nomeados (REST/BREATH/GAP/BREAK/DROP/VOID), e
  **acumulação de silêncio** como processo formal (denso→cheio de
  buracos→fragmentado→quase vazio).
- **Bursts vs. rarefação**: densidade média estável, mas eventos
  organizados em explosão-e-silêncio em vez de distribuição uniforme.
- **Tempo elástico** (passos que se esticam/comprimem mantendo o total)
  e **gravidade temporal** (pontos métricos que atraem eventos
  próximos, outros que repelem) - groove nasce de física, não de swing
  fixo.
- **Groove Field**: não um valor de swing, um campo com
  deslocamento/tendência de acento/tendência de duração/tendência de
  velocity POR POSIÇÃO.
- **Sequencer event com duração** (uma região de mutação durando 4
  compassos, uma deriva durando minutos) - liga sequenciamento a forma.
- **Estados globais nomeados** (STABLE/MOVING/DENSE/SPARSE/FRAGMENTED/
  CHAOTIC/FROZEN/RECOVERING) com transições que já são, em si, uma forma
  musical - e **estado emergente** (o sistema INFERE o estado a partir
  de entropia+densidade+desvio de timing, em vez de o estado ser uma
  ordem externa).
- **Feedback e ecologia entre instrumentos**: a saída sonora
  realimenta o próprio sequenciador (muito denso→reduz eventos); um
  instrumento reage à atividade de outro (A toca muito→B silencia;
  acento de A dispara mutação em B) - **competição** (energia total
  limitada, se A cresce outros cedem) e **cooperação** (call and
  response com transformações reais, não repetição literal).
- **Event budget / complexity budget**: um teto compartilhado de
  eventos (ou de complexidade) por compasso, disputado entre
  sequenciadores - evita que tudo fique complexo ao mesmo tempo, o
  "problema clássico de sistemas generativos" nas palavras do autor.
- **Attention**: o sistema concentra complexidade em UM eixo por vez
  (ritmo agora, timbre depois), não em todos simultaneamente.
- **Palimpsesto/resíduo/trace**: mutação nunca apaga o estado anterior
  por completo - vestígios permanecem. **Cicatriz**: depois de uma
  ruptura grande, o sistema "recupera" mas não volta exatamente ao
  estado anterior. **Erosão** (perde informação aos poucos) vs.
  **sedimentação** (acumula aos poucos) como forças opostas.
- **RASGO/RUPTURE, distinto de GLITCH**: glitch é uma perturbação;
  ruptura ALTERA A CONTINUIDADE do sistema em si (grade, roteamento,
  comprimento, tempo, memória, topologia) - uma distinção conceitual
  que o autor marca como "muito importante".
- **Arquitetura de cinco escalas**: Microevent (ratchet/grace/glitch/
  subdivisão) → Step (nota/acento/gate/condição) → Pattern/Motif
  (agrupamento/identidade) → Form/State (trajetória do sistema) → uma
  quinta camada, **History** (não só onde o sistema está, por onde já
  passou) - unificando memória/deriva/erosão/resíduo/mutação/
  recuperação/rasgo numa única "temporalidade do instrumento".

### 7.2 Prior art novo encontrado (não citado na seção 2)

- **`AQUORBIUM/core/Ecosystem.h`** - achado mais forte desta rodada.
  `Presence` (`Present/Leaving/Absent/Entering/Trapped`) é
  literalmente **step state** (item da seção 7.1) já implementado, só
  aplicado a organismos, não a steps. `Strategy`
  (`Territorial/Sessile/Migratory/Predator/Prey`) e `ListeningMode::
  CollectiveDrift` são **ecologia entre agentes** (competição/
  cooperação/predação) real, já em produção - exatamente o território
  dos itens #44-50 do brief novo (feedback entre instrumentos,
  competição, cooperação). `Integrity{health, crackStress, cracked,
  shattered}` é uma **cicatriz real**: saúde degrada, racha, no limite
  se estilhaça - o mesmo princípio de "erosão com consequência
  permanente" que os itens #55-58 (palimpsesto/cicatriz/erosão) do
  brief descrevem, implementado antes deste documento.
- **`RASGO_SYNTH/engine/ProbabilityMarket.hpp`** (já citado em
  `PESQUISA_RUIDO_GENERATIVO.md`, não cruzado com sequenciador até
  agora) - é, com outras palavras, exatamente **step fatigue**/
  **repetition pressure vs. novelty** (itens #3-4 e #28-29 da seção
  7.1): "um mecanismo usado com frequência recentemente fica mais raro
  (preço cai); um que não dispara há vários renders fica mais provável
  (preço sobe)", nunca extinto (piso 0.03) nem certeza (teto 0.97),
  sempre puxado de volta ao baseline - a mesma tensão novelty↔repetition
  que o brief propõe como duas forças concorrentes, já real.
- **DERIVA do próprio ANTITOTEM** (`PESQUISA_DERIVA_GENERATIVA.md`,
  `CRI-DRF-001`, mesma sessão) - `topologyMemory` (buffer circular de
  roteamentos passados, puxado com probabilidade) já é, ao mesmo tempo,
  **sequenciador topológico com memória** (item da seção 7.1) e
  **resíduo/trace** (o sistema raramente volta EXATAMENTE ao estado
  anterior) - achado antes deste documento, agora cruzado com ele.
- **`RASGO_SYNTH/engine/MaterialAgency.hpp`** (já citado em
  `PESQUISA_RUIDO_GENERATIVO.md`) - a família nomeada `Rupture` e o
  estado `MaterialFocus::Fragmented` ("event-shaped rather than
  section-shaped") são o precedente mais próximo já registrado no
  projeto pra distinção GLITCH×RUPTURE que este brief agora nomeia
  explicitamente.

### 7.3 Estado atual vs. este segundo brief

| Conceito | Estado |
|---|---|
| Step state (dormant/active/hot/exhausted) | **Feito** (20 ago. 2026) - `stepHeat[]` novo em `SimpleSequencer` (`std::array<float, stepCount>`), um float por passo em vez de por posição métrica (diferente de `accentFatigue`, que já existia mas é ligado à métrica, não à identidade do passo). Esfria devagar TODO `advanceStep()` (`*0.985`, mesmo em passos mutados/não visitados), esquenta só quando o passo REALMENTE soa (`+0.16`, dentro do `if (!muted[...])`). Aplicado em `renderSample()` via `stepStateGain` contínuo (não um switch discreto por estado, evita clique audível): dormant/active (heat < 0.55) neutros, hot (0.55-0.85) até +15%, exhausted (>0.85) até -30% por cima disso - pico em ~0.85, desce dali, um passo tocado demais recede de verdade em vez de só parar de crescer |
| Step fatigue / repetition pressure vs. novelty | **Feito, adjacente** (`ProbabilityMarket.hpp`, preço com reversão à média) - **feito no ANTITOTEM** (19 ago. 2026, `accentFatigue` - conectado ao `accentRotation` já existente em vez de um sistema de "preço" novo: cresce a cada vez que a posição de tempo forte ATUAL dispara, reseta quando a rotação muda pra uma posição nova/"descansada"; pico do tempo forte recede até -0.3, nunca abaixo do teto do tempo fraco) |
| Playhead weighted walk | **Feito** (19 ago. 2026) - `ScannerDirection::memoryAddress`'s `nextUnit()` reescrito: distribuição triangular real (dois valores de faixas de bits diferentes do mesmo update de xorshift, subtraídos), pico no deslocamento 0 a partir do `currentStep` atual, caindo pras extremidades - vizinho genuinamente mais provável que distante, não uniforme. Regressão real pega por teste automatizado (dois updates de RNG por chamada desalinhava a sequência determinística de outro teste) - corrigido consumindo o estado do mesmo jeito que a versão antiga |
| Playhead inércia / atratores / repelentes | **Feito** (20 ago. 2026, autor: "primeiro vamos finalizar as implementações em aberto do sequencer" -> "Playhead: inércia/atratores/repelentes") - `advanceStep()`'s `ScannerDirection::memoryAddress` ganhou três camadas sobre o weighted walk (`nextUnit()`) já existente, todas reaproveitando sinais/técnicas que já existiam: **repelente** - candidato com `stepHeat` alto (passo recentemente/muito visitado, o mesmo valor que já reduz `stepStateGain`) sofre um segundo sorteio, sem laço; **inércia** - ~30% de chance de refletir o salto de volta a favor da direção do último salto real (mesma técnica do viés de Narmour da EXCITAÇÃO, item #11, aplicada ao playhead em vez de pitch), novo membro `scannerMomentumDirection`; **atrator** - ~18% de chance por chamada de puxar o candidato um passo em direção ao tempo forte mais próximo (mesma posição de MÉTRICA/acento que já organiza o resto do instrumento), nunca um teleporte direto |
| Sequenciador topológico/grafo com memória | **Feito, adjacente** (DERIVA do ANTITOTEM já faz isso pro ROTEAMENTO entre PRINCIPAL/CLONE, não pra passos de uma sequência) |
| Ciclo/fase local por camada (polimetria de verdade) | **Feito** (`PolymeterEnsemble.hpp`, RASGO_SYNTH - já citado na seção 2.1) - **não feito** no ANTITOTEM |
| Ecologia entre instrumentos (competição/cooperação/feedback) | **Feito** (`Ecosystem.h`, AQUORBIUM - `Strategy::Predator/Prey`, `CollectiveDrift`) - **não feito** entre instrumentos RASGO diferentes (só dentro do próprio Aquorbium) |
| Cicatriz/erosão com consequência permanente | **Feito** (`Ecosystem::Integrity` - health/crackStress/cracked/shattered) - **não feito** no ANTITOTEM |
| Event budget / complexity budget compartilhado | **Feito, por objeto** (20 ago. 2026, autor: "event budget", escopo confirmado via AskUserQuestion: "Compasso real, por objeto") - depende do compasso real (`CRI-CMP-001`) finalmente implementado como groundwork (ver `PESQUISA_COMPASSO_E_METRICA_REAL.md`, agora com código de verdade). `SimpleSequencer` ganhou `hasEventBudget(cost)`/`spendEventBudget(cost)`/`getEventBudgetRemaining()` - ceiling normalizado 1.0, reseta no mesmo instante em que `measureStepIndex` envolve (fim de compasso REAL, não `loopEnd`). Em `Main.cpp`, os 5 "eventos" que mais afetam complexidade em cada `deriveFromMemory()` (reconfiguração de topologia/rotas, custo 0.35 - o exemplo canônico de ruptura real; rodadas inteiras de Motion AUTO/A/B/C, 0.25 cada) checam o orçamento antes de agir e gastam quando agem - custo por RODADA, não por parâmetro individual (instrumentar cada `driftAutonomousItem()` isoladamente exigiria dezenas de pontos a mais). Como A/B/C são combináveis (blocos `if` sequenciais, não else-if), competem de verdade pelo MESMO orçamento quando mais de uma camada está ligada - "disputado entre sequenciadores" literal, dentro de UM único objeto |
| GLITCH × RASGO/RUPTURE como distinção nomeada | **Parcial** (`MaterialAgency.hpp`'s família `Rupture`, mais estreita que a distinção completa proposta aqui) |
| DERIVA (topologia com memória) × MATERIAL AGENCY (Rupture) nomeados como a mesma família | **Feito** (20 ago. 2026) - a topologia de rotas de feedback da DERIVA (`nextRoute`/`topologyMemory`/`routeMutates` em `deriveFromMemory()`, PESQUISA_DERIVA_GENERATIVA.md) e `AgencyFamily::Rupture` (`MaterialAgency.hpp`) resolvem o MESMO problema - "quando romper um estado estável, lembrando de onde já esteve" - com formas bem diferentes: DERIVA é UM bitmask contínuo que mistura rota histórica (`topologyMemory`) e rota atual, mutando por uma chance própria (`routeMutates`, escalada por `activeDepth`); Rupture é um AGENTE numa ecologia de vários (`AgencyFamily::Core/Texture/Instrument/Voice/Polyphony/Rhythm/Rupture`), com temperamento próprio (`boldness`/`persistence`/`silenceResponse`/`resistance`/`sociability`) e disposição (`Dormant/Event/Withhold/Yield/Listen/Claim/Contest`), explicitamente "event-shaped rather than section-shaped" - interrompe no instante que se manifesta, não reserva uma seção inteira pra si (código do próprio `MaterialAgency.hpp`: "reserving a whole section for a rare event produced a silent foreground"). A distinção real: DERIVA não tem "disposição" nem compete com outros agentes - é um processo solitário com memória; Rupture é social, um entre vários pedindo a vez. Achado ao comparar: a `accentFatigue`/`stepHeat` do ANTITOTEM (item já feito acima) são mais parecidas com o CONCEITO de "temperamento que resiste a repetição" que o `MaterialAgency` já tem do que a própria DERIVA é. |
| Meta-sequenciador / sequenciador de regras | **Feito, fatia real, ampliada** (20 ago. 2026, autor: "meta-sequenciador/regras", ampliado mesmo dia com "microevent, pattern/motif") - `SimpleSequencer` ganhou `enum class StepRule { normal, mutate, silence, rotate, ratchet, invert, retrograde }` por passo, além do próprio CV/nível/send que o passo já carregava. `normal` é o comportamento de sempre; os outros seis fazem o passo AGIR sobre a sequência quando alcançado, não só soar: `mutate` desloca o CV/nível/send DO PRÓPRIO passo; `silence` pausa esta passagem pontualmente, sem editar `muted[]` permanentemente; `rotate`/`retrograde`/`invert` são as transformações de Pattern/Motif (ver linha "Arquitetura de 5 escalas" abaixo pro detalhe); `ratchet` é Microevent (idem). `mutate`/`rotate`/`ratchet`/`invert`/`retrograde` gastam do MESMO event budget que DERIVA já usa em Main.cpp - disputam o mesmo orçamento por compasso, não uma reserva própria; `silence` é de graça. Regras são atribuídas por sorteio autônomo em `advanceStep()`, gated por `metaSequencerAmount` (0 = off entirely) - `normal` continua maioria (45%, reduzido de 55% quando as 3 regras novas entraram) mesmo em amount cheio. Sem UI exposta ainda - infraestrutura primeiro, mesmo padrão já usado pra EXCITAÇÃO/Noise Field. Ainda não é o meta-sequenciador completo do brief (sem endereçamento por UI, sem regras de produção tipo transpose+N/branch condicional) - uma fatia real e funcional, não o conceito inteiro |
| Arquitetura de 5 escalas com History | **Feito, cinco escalas reais** (20 ago. 2026, autor: "prossiga" - a última das "três tarefas grandes"; escopo completado no mesmo dia, autor: "microevent, pattern/motif") - Step já existia por inteiro (`SimpleSequencer`: voltages/levels/muted/effectSends/`StepRule`, item acima desta tabela). `DualObjectEngine`: `FormState` (Calm/Rising/Peak/Falling/Recovering) - a QUARTA escala, trajetória do INSTRUMENTO inteiro (não de um objeto só) - derivada de `formEnergy`, um seguidor MUITO lento (constante de tempo ~30s) da média de quanto do event budget de PRINCIPAL/CLONE anda sendo gasto; classificado por limiar + DIREÇÃO, evitando flicker. `getFormHistoryAt(stepsAgo)` é a QUINTA escala, History de verdade - buffer circular de 4 TRANSIÇÕES reais, "não só onde o sistema está, por onde já passou". Consequência real: quando 2+ das últimas transições History já eram Peak, um novo Peak empurra `instabilityField` pra BAIXO (`nudgeInstability(-0.08)`) - o instrumento "cansa" de ficar ocupado, fadiga que recupera em ~45s (item "recuperação"/"erosão" do brief). Microevent e Pattern/Motif, que tinham ficado de fora por decisão de escopo, agora também reais: `StepRule` (`SimpleSequencer`) ganhou `ratchet` (Microevent - o passo dispara 2-4 sub-hits DENTRO da sua própria duração, contagem derivada do próprio `levels[]`, decrescendo real de ganho a cada sub-hit via novo sub-clock em `renderSample()`) e `invert`/`retrograde` (Pattern/Motif - duas das seis "transformações reais de teoria motívica" do brief, além de `rotate` já feita: `invert` espelha `voltages[]` em torno de 0.5 dentro de `[0, loopEnd)`, `retrograde` inverte a ORDEM de todos os arrays ativos via `std::reverse`, o motivo tocado de trás pra frente). As três novas regras gastam do mesmo event budget (ratchet 0.15, invert 0.20, retrograde 0.30) - StepRule agora tem 7 casos (normal 45%/mutate 15%/silence 10%/rotate 8%/ratchet 10%/invert 6%/retrograde 6% no sorteio autônomo). Aumentação/diminuição/fragmentação (as 3 transformações motívicas restantes do brief) e identidade/comparação de padrão entre si ficam de fora - decisão de escopo, não o conceito completo |

### 7.4 Próximos passos deste segundo brief (não decididos)

Mesma lição de sempre - poucos pontos de entrada baratos, não a
arquitetura inteira:

1. ~~Step fatigue no ANTITOTEM~~ - **feito** (19-20 ago. 2026) - ver
   tabela 7.3 ("Step state"/"Step fatigue"). Esta entrada ficou sem o
   risco quando escrita; marcada agora só por consistência com o resto
   da lista.
2. ~~Weighted walk pro playhead~~ - **feito** (19 ago. 2026) - ver
   tabela 7.3. Mesma nota da entrada acima.
3. ~~Conectar DERIVA (topologia com memória) e MATERIAL AGENCY
   (Rupture) explicitamente~~ - **feito** (20 ago. 2026) - ver tabela
   7.3. Achado real, não só nomeação: a distinção que mais importa não
   é "os dois lidam com ruptura" (verdade superficial), é que Rupture é
   um AGENTE social (compete, tem disposição, pode ser recusado por
   outros) enquanto DERIVA é um processo solitário com memória - a
   `accentFatigue`/`stepHeat` do ANTITOTEM (já feitas) se parecem mais
   com o "temperamento que resiste à repetição" do `MaterialAgency` do
   que a própria DERIVA.
4. ~~Playhead inércia/atratores/repelentes~~ - **feito** (20 ago. 2026,
   autor: "primeiro vamos finalizar as implementações em aberto do
   sequencer" -> "Playhead: inércia/atratores/repelentes") - ver tabela
   7.3.
5. ~~Event budget / complexity budget~~ - **feito, por objeto** (20 ago.
   2026, autor: "event budget") - ver tabela 7.3. Escopo POR OBJETO, não
   um pool único compartilhado entre PRINCIPAL/CLONE - a troca que o
   autor escolheu via AskUserQuestion, já que os dois têm `clockRate`
   independentes hoje (sem BPM sincronizado, um budget genuinamente
   compartilhado exigiria resolver isso primeiro).
6. ~~Meta-sequenciador / sequenciador de regras~~ - **feito, fatia real,
   ampliada** (20 ago. 2026, autor: "meta-sequenciador/regras",
   ampliado mesmo dia com "microevent, pattern/motif") - ver tabela
   7.3. `StepRule` (normal/mutate/silence/rotate/ratchet/invert/
   retrograde) por passo, gastando do mesmo event budget que DERIVA.
   Sem UI exposta, sem regras de produção tipo transpose/branch do
   brief original - uma fatia real, não o conceito completo.
7. ~~Arquitetura de 5 escalas com History~~ - **feito, cinco escalas
   reais** (20 ago. 2026, autor: "prossiga", depois "microevent,
   pattern/motif") - ver tabela 7.3. `FormState` (a quarta escala,
   trajetória do instrumento) + History (a quinta, buffer de
   transições) real, com uma consequência audível de verdade
   (`nudgeInstability` quando Peak se repete). Microevent (`ratchet`,
   sub-hits dentro de UM passo) e Pattern/Motif (`invert`/`retrograde`,
   duas transformações reais além de `rotate`) agora também reais -
   aumentação/diminuição/fragmentação (as 3 transformações restantes) e
   identidade/comparação de padrão entre si ficam de fora, decisão de
   escopo.
8. O restante (event budget entre instrumentos de verdade -
   compartilhado, não por objeto -, ecologia RASGO-wide,
   aumentação/diminuição/fragmentação, identidade de padrão comparável)
   fica registrado, sem prioridade definida - são mudanças de
   arquitetura reais, não fatias baratas.

**Ganchos técnicos pros itens do item 8** (20 ago. 2026, autor: "quando
terão prioridade?" -> "registre" - sem data/prioridade real, mas com um
caminho técnico já identificado pra cada um, pra não se perderem):

- **Fragmentação** tem o gancho mais pronto dos três: precisa de um
  conceito de "limite de motivo" diferente do `loopEnd` inteiro - o
  compasso real já implementado (`CRI-CMP-001`,
  `stepsPerMeasure`/`measureStepIndex`, independente de `loopEnd`) já
  dá exatamente isso. Um motivo poderia ser "um compasso dentro do
  loop" em vez de precisar de um sistema de marcação novo do zero.
- **Aumentação/diminuição** é a mais cara das três - mexe em DURAÇÃO,
  não em valor, então precisaria tocar a camada de clock/timing
  (`samplesPerStep()` por passo, hoje uniforme pro objeto inteiro), um
  risco bem maior que `invert`/`retrograde`/`ratchet` (que só
  reorganizam arrays já existentes, sem tocar o clock).
- **Identidade de padrão comparável** faz mais sentido depois que a
  History (`DualObjectEngine::getFormHistoryAt()`, hoje só guarda
  transições de `FormState`) crescer pra guardar um retrato real do
  CONTEÚDO do padrão (ex.: um resumo/hash de `voltages[]`), não só o
  estado macro - só aí "esse padrão parece com um de 5 minutos atrás"
  vira uma pergunta que dá pra responder de verdade.

Com isso, as "três tarefas grandes" anunciadas no início desta
sub-thread (meta-sequenciador, event budget, arquitetura de 5 escalas)
têm as três com pelo menos uma fatia real implementada, e a arquitetura
de 5 escalas chegou a completar as cinco escalas de verdade (ainda que
cada uma numa fatia reduzida do conceito original) - nenhuma foi
resolvida por completo (nenhum dos três briefs originais cabia numa
sessão sem virar over-engineering), mas cada uma ganhou o pedaço mais
barato/melhor fundamentado que reaproveitava o máximo de infraestrutura
já existente.

## 8. Calibração pendente (19 ago. 2026)

Ver a nota geral em `PESQUISA_MELODIA_GENERATIVA.md`, seção 7 (mesmo
pedido do autor, mesma data). Constantes desta pesquisa ainda não
confirmadas por escuta, ambas em `SimpleSequencer.cpp`:

- Weighted walk do playhead: distribuição triangular via dois valores
  de faixas de bits diferentes do mesmo update de xorshift (não uma
  distribuição medida/calibrada, só uma escolha de técnica real).
- Step fatigue: `accentFatigue` +0.03 por disparo, teto 0.3.
- Step state (novo, 20 ago. 2026): esfriamento `*0.985` por
  `advanceStep()`, aquecimento `+0.16` por disparo real, limiares 0.55
  (hot) / 0.85 (exhausted), ganho até +15%/-30% - nenhum desses seis
  números foi comparado por escuta contra `accentFatigue` nem entre si.

- Playhead inércia/atratores/repelentes (novo, 20 ago. 2026): repelente
  dispara com `stepHeat > 0.7`; inércia reflete com 30% de chance;
  atrator age com 18% de chance por chamada - três números escolhidos
  por "convite, não regra" (nenhum tão alto que dominasse o weighted
  walk de base), nenhum medido/confirmado por escuta específica ainda.
  Em particular, as chances dos três mecanismos NUNCA foram comparadas
  entre si (por que 30% e não 18% pra inércia, por exemplo) nem contra
  o efeito visual esperado no playhead.

Nenhum item ANTIGO específico desta pesquisa a recalibrar - as quatro
implementações (weighted walk, step fatigue, step state, inércia/
atratores/repelentes) nasceram nesta mesma sessão, não têm histórico
anterior pra revisitar.

- Event budget (novo, 20 ago. 2026): ceiling normalizado 1.0; custo de
  reconfiguração de topologia 0.35; custo de uma rodada de Motion (AUTO/
  A/B/C) 0.25 cada - nenhum desses números foi comparado por escuta.
  Escolha deliberada de gastar por RODADA (não por parâmetro individual
  dentro dela) nunca validada contra a granularidade mais fina que o
  brief original sugeria ("cada 'evento' pode custar um valor
  diferente").
- Meta-sequenciador (novo, 20 ago. 2026): chance de sorteio autônomo de
  regra `0.02 * metaSequencerAmount` por `advanceStep()`; pesos da
  regra sorteada (55% normal/20% mutate/15% silence/10% rotate); custo
  de `mutate` 0.12, de `rotate` 0.30 (mesma escala do custo de topologia
  de DERIVA, mas nunca comparado contra ele por escuta); amplitude do
  drift de `mutate` (±0.06 por parâmetro, metade de 0.12) - nenhum
  desses seis números foi medido/confirmado por escuta específica
  ainda, e `metaSequencerAmount` em si nunca saiu de 0 (default) em
  teste real.
- Arquitetura de 5 escalas / Form-State (novo, 20 ago. 2026, em
  `DualObjectEngine.cpp`): constante de tempo de `formEnergy` ~30s;
  limiares 0.15 (Calm)/0.75 (Peak); limiar de `formFatigue` pra
  Recovering 0.4; incremento de fadiga por Peak +0.2, teto 1.0,
  recuperação em ~45s; nudge de `instabilityField` -0.08 quando 2+ das
  últimas 3 transições já eram Peak - nenhum desses sete números foi
  medido/confirmado por escuta, e a MAIOR incerteza de todas: a
  constante de ~30s nunca foi testada contra uma sessão real de escuta
  longa o bastante pra realmente ver `formEnergy` sair de 0 e um
  `FormState` diferente de Calm acontecer - todo o mecanismo é teórico
  até uma escuta de vários minutos confirmar que ele se move na escala
  de tempo certa.
- Microevent/Pattern-Motif (novo, 20 ago. 2026, `SimpleSequencer.cpp`):
  contagem de sub-hits do ratchet (2-4, derivada de `levels[]`,
  multiplicador 2.99 nunca comparado contra um mapeamento alternativo);
  decrescendo de ganho do ratchet (até -70% no último sub-hit); pesos
  redistribuídos do sorteio autônomo de `StepRule` (normal 45%/mutate
  15%/silence 10%/rotate 8%/ratchet 10%/invert 6%/retrograde 6%); custos
  de event budget das 3 regras novas (ratchet 0.15, invert 0.20,
  retrograde 0.30, este último igualado a `rotate` por "mesma classe de
  complexidade estrutural", nunca comparado por escuta) - nenhum desses
  números foi medido/confirmado por escuta específica ainda, e como
  `metaSequencerAmount` continua em 0 por padrão (sem UI), nenhuma das 7
  regras de `StepRule` foi ouvida em uso real nesta sessão.

## 9. Terceiro brief: vocabulário de subdivisão/métrica (20 ago. 2026)

Autor colou um quarto diálogo com o ChatGPT (mesmo padrão dos briefs
das seções 3 e 7 - "mais um diálogo... que merece aprofundamento"),
desta vez sobre subdivisão rítmica, quiálteras e métrica, explicitamente
enquadrado pro sequenciador generativo do RASGO ("Para um sequenciador
generativo como o que estávamos discutindo para o RASGO").

### 10.1 O que o brief traz

Nomenclatura padrão de figuras (semibreve=1 até semifusa=1/64,
subdivisão binária regular em árvore: semínima → 2 colcheias → 4
semicolcheias → 8 fusas → 16 semifusas) e subdivisão ternária
(tercina, "3 no espaço de 2") como caso mais comum de **quiáltera**
(tuplet) - a família completa vai de duína (2 no espaço de 3) até
n-inas arbitrárias (quintina, sextina, septina, nonina, decina...).
Aumento por ponto (semínima pontuada = 1/4+1/8 = 3 colcheias; dois
pontos = 1/4+1/8+1/16) e ligadura (soma de figuras através de pulsos/
tempos/compassos) tratados como formas de gerar durações assimétricas
SEM precisar de quiáltera - "duração e subdivisão não precisam ser a
mesma coisa".

Compasso simples (pulso se subdivide em 2: 2/4, 3/4, 4/4) vs. composto
(pulso se subdivide em 3: 6/8, 9/8, 12/8) - diferente de simplesmente
escrever uma tercina num compasso simples. Métricas assimétricas (5/8,
7/8, 9/8, 11/8...) com AGRUPAMENTO interno explícito, e o ponto mais
importante do brief: "numerador do compasso e agrupamento perceptivo
não são a mesma coisa" - um 7/8 pode ser 2+2+3, 3+2+2 ou 2+3+2, cada
agrupamento com um "feel" diferente apesar do mesmo numerador.
Polirritmia (subdivisões simultâneas diferentes entre vozes, ex.: 3
contra 2, 4 contra 3, 5 contra 4) fechando o quadro.

### 10.2 Proposta de esquema de dados (a parte mais reaproveitável)

O brief propõe separar seis conceitos que costumam ficar embolados
numa implementação ingênua de sequenciador:

- **resolution** - menor unidade da grade (ex.: 1/16)
- **subdivision** - quantas partes num pulso (ex.: 2, 3, 5, 7)
- **grouping** - como essas partes se agrupam perceptivamente (ex.: 7 =
  2+2+3) - DISTINTO do numerador do compasso
- **tuplet** - quiáltera excepcional dentro de uma duração específica
  (ex.: 5:4, 7:4, 3:2)
- **meter** - organização dos pulsos em compassos, incluindo métricas
  assimétricas (4/4, 7/8, 5/4)
- **polyrhythm** - duas ou mais subdivisões simultâneas (dentro do
  MESMO compasso/ciclo)
- **polymeter** - dois ou mais compassos/ciclos INTEIROS diferentes
  rodando ao mesmo tempo (não é o mesmo que polyrhythm - a distinção
  syntagma vs. paradigma entre "subdivisão diferente dentro do mesmo
  ciclo" e "ciclos inteiros diferentes simultâneos")

Exemplo de step com esse vocabulário: `step_duration=1/4, subdivision=7,
grouping=3+2+2, accent_pattern=X..X.X.` ou `step_duration=1/8,
tuplet=5:4, ratchet=5`. O brief aponta 5/7/9/11/13 como subdivisões
"particularmente interessantes" pra um sequenciador generativo - criam
padrões não triviais sem cair em subdivisão irracional.

### 10.3 Cruzamento com o que já existe

- `RASGO_SYNTH/rasgo-synth-core/src/sequencer/Euclidean.hpp` já
  implementa GROUPING irregular (Bjorklund) - o vocabulário do brief dá
  nome formal a algo que o código já faz.
- `PolymeterEnsemble.hpp` já é POLYMETER de verdade (compassos/ciclos
  inteiros diferentes rodando simultâneos) - também já nomeado assim
  antes de existir essa formalização.
- `SimpleSequencer::StepRule::ratchet` (seção 7, "Microevent") já é uma
  forma de TUPLET dentro de um step (sub-hits dentro de uma duração
  fixa), embora nunca tenha sido nomeado com esse vocabulário até agora.
- POLYRHYTHM (subdivisões diferentes simultâneas dentro do mesmo ciclo,
  não canais/ciclos inteiros diferentes) não tem prior art direto
  identificado ainda no RASGO_SYNTH nem no AQUORBIUM.
- Conexão cruzada com `CRI-AMT-001` (MARAVI): `beat_grid.
  detect_beat_divisions()` já separa resolution/subdivision na direção
  oposta - DETECTAR a partir de áudio em vez de GERAR - hoje limitado a
  candidatos `{1,2,3,4,6,8}` (família binária + tercina/sextina). As
  subdivisões ímpares que este brief destaca (5,7,9) seriam a extensão
  natural se algum áudio de teste real algum dia pedir - mesmo
  vocabulário, os dois lados do mesmo problema (gerar vs. reconhecer).

### 10.4 Estado

Registrado como vocabulário/mapa conceitual, não como decisão de
arquitetura nem item de implementação - nenhuma prioridade definida,
nenhum código escrito. Fica como referência pra quando POLYRHYTHM (o
único conceito do brief sem prior art já existente) ou uma extensão de
GROUPING além do Euclidiano forem retomados.

## 10. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, entrada
`CRI-SEQ-001`.
