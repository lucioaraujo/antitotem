# Pesquisa: EXCITAÇÃO como agente relacional — temperamento, timbre camaleão e diálogo com o ambiente

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`). Registrado como
`CRI-REL-001` nesse arquivo. Origem: trabalho ao vivo em EXCITAÇÃO (ver
`TAREFAS.md`, 20 ago. 2026) — continuação direta de
`PESQUISA_MELODIA_GENERATIVA.md`, mas um assunto grande o bastante pra
merecer documento próprio: ali a voz aprendeu a se articular e frasear
sozinha; aqui a pergunta é como ela se relaciona com o que está ao redor
dela.

## 1. Por que este documento existe

Depois de fechar os itens #1-#19 do brief original de melodia (ver
`PESQUISA_MELODIA_GENERATIVA.md`, seção 5 — todos "Feito" a esta altura),
o autor levantou quatro frentes novas na mesma conversa, em mensagens
sucessivas:

- "ainda precisamos melhorar estacatos e outras formas de articulação e
  gesto de nota" — resolvido diretamente, sem precisar de pesquisa nova
  (ver `TAREFAS.md`, entrada "Tipo de gesto/articulação"; `GestureType`
  Legato/Staccato/Marcato/Tenuto).
- "também um leque mais variado de timbres, o instrumento talvez precise
  agora ser mais camaleão"
- "estágios de ânimo, espírito, interpretação, sensibilidade"
- "como disse no ambiente que vive e existe, precisa interagir,
  coexistir, influenciar e ser influenciado"
- "talvez precisamos nos aprofundar mais sobre os instrumentos
  melódicos (instrumentos extremamente intensos como violino ou flauta
  podem nos ajudar), mais teoria musical, estudarmos e trazermos mais
  das características musicais, o papel da melodia, orquestração (como
  ele dialoga"
- depois, mais palavras soltas confirmando o escopo: "expressividade,
  gestualidade, intenção, interpretação, criação, contraponto",
  "improviso"
- e a pista que abriu a parte mais importante desta pesquisa: "creio que
  o rasgo synth já andou pesquisando teoria musical, composição" —
  confirmada (ver seção 2).

Estas quatro frentes (timbre camaleão / estados de ânimo / ecossistema-
ambiente / orquestração-diálogo) não são quatro assuntos separados — a
pesquisa em RASGO_SYNTH (seção 2) mostra que no projeto-irmão elas já
colapsaram numa coisa só: um agente com **temperamento** (sensibilidade,
resistência, fadiga, memória) que **escolhe um verbo relacional**
(seguir/contrapor/negociar/etc.) em resposta ao que ouve de outro corpo,
e essa escolha **é** o que determina timbre, presença e ânimo audível —
não três sistemas paralelos, um sistema só visto de três ângulos.

## 2. O que já existe no projeto RASGO sobre este assunto

Verificado por dois forks de pesquisa nesta sessão (20 ago. 2026) — não
por memória de sessões anteriores, para evitar citar API que pode ter
mudado.

### 2.1 O achado central: `IntermodularOrchestra.hpp` já põe O MESMO THEREMIN numa relação real

`RASGO_SYNTH/rasgo-synth-core/src/engine/IntermodularOrchestra.hpp`
(citado em `docs/pt/Orquestra_Relacional_Fase_121.md` e
`docs/INTERMODULAR_LAB.md`), fase 121, **validado** (8/8 CTests, 20/20
regressão de seed, zero clipping, true peak entre -1.7 e -1.1 dBTP;
escuta A/B ao vivo na seed 11: 76 compassos observados, 16 respostas, 16
resistências, 16 retornos, presença nos 77 compassos sem nenhuma entrada/
saída forçada). O mesmo `ThereminVoice.hpp` já portado pra EXCITAÇÃO é o
nó "intérprete" de uma relação de três:

```
bus real da bateria -> theremin -> campo harmônico
        ^                              |
        +---- dinâmica/atraso futuro --+
```

- A bateria publica **emissão observada de verdade** (RMS, pico, razão
  audível, densidade de ataque, desvio de pulso, pressão) — não uma
  flag de "tocando/não tocando".
- O theremin "deixa de receber a nota melódica central como alvo" — ele
  tem sensibilidade, permeabilidade, fadiga, memória e resistência
  PRÓPRIAS, e traduz o gesto da bateria através de um de **seis verbos**:
  **seguir, transformar, contrapor, agredir, ocultar, negociar** — só
  então propõe um pitch independente.
- O campo harmônico pode aceitar, tensionar, resistir ou reconfigurar a
  proposta do theremin; quando reconfigura, a tônica muda de verdade.
- A consequência volta pro início do laço, modificando a velocidade da
  bateria e/ou o atraso futuro — uma influência **de mão dupla** real,
  não um efeito que só sai de um lado.
- Citação que motivou esse trabalho, do diagnóstico da fase 120
  (`Reavaliacao_Modular_Pos_Fase_120.md`, linha 226): *"Theremin |
  Personalidade audível, mas normalmente segue a melodia central |
  Separar 'theremin intérprete' de 'sombra'; fazê-lo ouvir e responder
  sem copiar."* — exatamente o mesmo diagnóstico que valeria pra
  EXCITAÇÃO hoje (ver seção 5).

### 2.2 `Metodo_RASGO.md` — o vocabulário transversal do projeto

`RASGO_SYNTH/docs/pt/Metodo_RASGO.md` (645 linhas) formaliza a
diferença entre **ativação** (uma flag), **presença** (RMS real) e
**agência** (mudar a atenção/relação/decisão/forma de outro
participante) — três coisas frequentemente confundidas.

- **16 verbos relacionais reais**: escutar, responder, seguir,
  contrariar, imitar, reinterpretar, penetrar, contaminar, agredir,
  proteger, ceder, ocultar, interromper, negociar, recusar, devolver —
  "cada organismo realiza esses verbos segundo sua personalidade; não
  há um único algoritmo de 'agressão' ou 'resposta'."
- **"Contrato mínimo de uma relação intermodular"** — um checklist de
  10 pontos que toda relação real precisa declarar: participantes e
  direção inicial; o que cada um pode perceber/publicar; o verbo da
  relação; intensidade/alcance/atraso/duração; permeabilidade/
  resistência de quem recebe; tradução entre domínios; memória/fadiga/
  cicatriz acumulada; condição de transformação/encerramento;
  consequência sonora/formal observável; proveniência.
- **8 estados de vida de um módulo**: disponível → selecionado →
  manifestado → audível → saliente → influente → afetado →
  transformador — uma escada real de "o código existe" até "deixou uma
  consequência composicional duradoura", útil pra medir se qualquer
  coisa nova em EXCITAÇÃO chegou a valer, não só se compilou.
- Citação direta pro eixo "estados de ânimo/sensibilidade" que o autor
  pediu: *"Cada corpo possui uma sensibilidade própria. Não precisa
  escutar tudo nem responder sempre. Pode ser sensível a ataque,
  densidade, altura, aspereza, silêncio, posição, repetição, risco,
  memória, regra ou gesto; pode possuir limiar, saturação, fadiga,
  curiosidade, resistência e tempo de reação. Essa diferença de escuta
  é parte de sua personalidade."*
- Critério de 5 perguntas antes de importar uma técnica de outro
  instrumento (relevante porque é exatamente o que este documento está
  fazendo, do RASGO_SYNTH pro ANTITOTEM): o problema é realmente
  compartilhado ou só parecido de nome; que comportamento da origem NÃO
  deve atravessar a fronteira; que tradução torna a adaptação NECESSÁRIA
  (não só possível) no destino; a adaptação amplia a personalidade do
  destino ou puxa ele artificialmente pra origem; a diferença pode ser
  provada por escuta A/B cega.

### 2.3 `MaterialAgency.hpp` / `OrganismAudibility.hpp` — papel relacional e agência tímbrica

`RASGO_SYNTH/rasgo-synth-core/src/engine/MaterialAgency.hpp` — 7
famílias (core, texture, instrument, voice, polyphony, rhythm, rupture),
cada uma podendo ocupar um de três papéis por seção: **primeiro plano /
interlocutor / fundo**, escolhido pelas mesmas estratégias
(follow/transform/counter/interrupt/conceal/negotiate), com crossfade de
continuidade de 320ms — "há passagem de agência, não chaveamento bruto
de mute." Cada família carrega temperamento próprio (já citado em
sessões anteriores para `Rupture`, confirmado agora como usado por
**todas** as 7 famílias): ousadia, persistência, resposta ao silêncio,
resistência, sociabilidade — sorteados por seed, não fixos.

`OrganismAudibility.hpp` — `enterSection(density, familyForeground,
familyInterlocutor)`: escala um multiplicador de ganho conforme a
disposição (`Contest ×1.09, Claim ×1.06, Yield ×0.95, Withhold ×0.93,
Listen ×1.01`), com `motion *= 1.0 - density*0.10` — literalmente "como
eu me comporto dado quem mais está falando agora", sensível à
densidade — o mesmo eixo que o item #14 (densidade → liberdade
interpretativa) já implementou em EXCITAÇÃO, mas aqui nomeado e
relacional (contra um interlocutor específico), não um escalar solto.

**Confirmado como lacuna real, não só em ANTITOTEM**: nenhuma das duas
classes acima implementa variação de TIMBRE por agência/ânimo — o nome
"Auditoria de Agência Tímbrica" (fase 120) é sobre distribuição de
primeiro-plano na mixagem, não sobre cor tímbrica variável. O
RASGO_SYNTH tem o mesmo buraco que o ANTITOTEM tem hoje (timbre só como
`f(registro)`, item #10) — "camaleão" de verdade não existe em nenhum
canto do ecossistema RASGO ainda. Ver seção 6.2 para uma proposta.

### 2.4 AQUORBIUM `Ecosystem.h` — ambiente e percepção mútua real

`AQUORBIUM/src/core/Ecosystem.h` (219 linhas) — vocabulário de ambiente
já real, mas de uma família conceitual ligeiramente diferente (biológica/
territorial, não musical-relacional):

- `enum class Presence { Present, Leaving, Absent, Entering, Trapped }`
  — a presença de um organismo no espaço é ela mesma uma máquina de
  estados.
- `enum class Strategy { None, Territorial, Sessile, Migratory,
  Predator, Prey }` — cada organismo carrega até 2 estratégias
  simultâneas.
- `struct Environment { dayPhase, seasonPhase, temperature, daylight,
  matingPulse }` — um ambiente compartilhado de verdade, separado de
  qualquer organismo individual.
- `strategyForce()` (`Ecosystem.cpp`, linhas ~473-556) é o mecanismo
  real de influência mútua: `Predator` pontua `Prey` visíveis por
  `visibility - distance*0.08` e mira no melhor alvo; `Prey` soma
  vetores de repulsão de todo `Predator` visível dentro de um
  `fearRadius`. Os dois leem `underwaterVisibility()` — organismos
  literalmente se percebem e se guiam um pelo outro, não só por um campo
  global.

Vocabulário útil pro eixo "ambiente" do pedido do autor, mas menos
diretamente portável pra uma voz solista monofônica do que a Orquestra
Relacional (seção 2.1) — `Ecosystem.h` pressupõe posição espacial 2D e
múltiplos organismos coexistindo, categorias que EXCITAÇÃO não tem
(ela é uma voz, não uma população). Citado aqui por completude/
proveniência, não como base principal da proposta da seção 6.

### 2.5 O que falta confirmar (lacunas reais, não deste projeto)

- Nenhuma fonte pesquisada (RASGO_SYNTH ou AQUORBIUM) tem vocabulário
  de **articulação discreta** (staccato/legato/marcato/spiccato) —
  confirmado pelos dois forks desta sessão via grep vazio em todo
  `rasgo-synth-core/src`. Já resolvido em ANTITOTEM hoje sem depender
  desta pesquisa (`GestureType`, ver `PESQUISA_MELODIA_GENERATIVA.md`).
- Nenhuma fonte tem técnica idiomática de violino/flauta (posição/
  pressão de arco, respiração/embocadura) codificada — a seção 6.3
  abaixo traz isso de conhecimento próprio (teoria musical real, não
  verificada por busca externa nesta sessão), não de prior art do
  projeto.

## 3. Segunda fonte secundária: "Interpretation Engine" (autor + ChatGPT, 20 ago. 2026)

**Nota de proveniência** (mesmo padrão da seção 3 de
`PESQUISA_MELODIA_GENERATIVA.md`): texto colado ao vivo pelo autor, de
uma conversa com o ChatGPT sobre RASGO Modular. Fonte **secundária/
interpretativa** — a base real por trás dela é séculos de prática de
indicações expressivas na notação musical ocidental (dolce, agitato,
maestoso etc. são termos históricos reais, não invenção do ChatGPT); o
valor do texto está em como ORGANIZA esses termos pra um sistema
generativo, não em alegação de rigor acadêmico verificado nesta sessão.
Autor, ao colar: "anote isso, aprofunde, analise".

### 3.1 A ideia central: intenção não é preset, é vetor

*"Dolce" não é `velocity = 45`.* É uma região num espaço contínuo de
dimensões expressivas (energy, tension, weight, softness, brightness,
agitation, stability, playfulness, intimacy...). Isso permite
**interpolação** (uma frase pode migrar de `dolce` pra `agitato` sem
trocar de preset) e **combinação** (`dolce` 70% + `misterioso` 30%, ou
um caráter de base com um gesto local de sforzando por cima) — em vez
de um enum travado que só permite um estado por vez.

### 3.2 Cinco camadas que coexistem, não uma hierarquia única

O texto separa deliberadamente:

- **CHARACTER** — estado expressivo duradouro (dolce, grazioso,
  giocoso, maestoso, misterioso, agitato, furioso...) — dura uma
  frase ou várias.
- **ARTICULATION** — modo de formar notas (legato, staccato, portato,
  tenuto, marcato) — **já implementado em ANTITOTEM hoje** como
  `GestureType`, com os MESMOS quatro nomes (ver seção 3.5).
- **GESTURE (local)** — evento pontual (sforzando, fortepiano,
  rinforzando, breath, glissando) — um pico isolado, não um estado.
- **PROCESS** — transformação ao longo do tempo (crescendo,
  diminuendo, accelerando, ritardando, morendo, fragmentation).
- **TEMPORAL ATTITUDE** — relação com o tempo (rubato, strict, laid-
  back, pushed, elastic).

Um evento pode ter as cinco simultaneamente: *"CHARACTER=dolce,
ARTICULATION=portato, GESTURE local=sforzando ocasional,
PROCESS=crescendo, TEMPORAL=rubato"* — a distinção entre "estado que
dura" e "evento que acontece uma vez" é a mesma que já separa
`PhraseState` (macro, dura a frase) de `attackPunch`/`MicroState`
(pontual, um disparo) em EXCITAÇÃO hoje, sem essa nomenclatura ainda.

### 3.3 Tradução por instrumento — a mesma intenção, resultado diferente por voz

Ponto crucial, citado quase literalmente porque bate direto com a
seção 6.2 desta pesquisa (escrita ANTES de ler este texto, de forma
independente — uma boa confirmação cruzada): *"dolce em flauta
virtual: mais breath, ataque suave, pouco ruído agudo, vibrato tardio,
legato alto. No theremin: glide suave, pitch stability maior, vibrato
pequeno, harmônicos reduzidos, attack lento."* — a intenção é comum
(um vocabulário compartilhado entre instrumentos do ecossistema RASGO),
a realização é específica de cada voz. Isso é literalmente o "SEQUENCER
→ MELODIC INTERPRETER → VOZ" do brief original (item #19,
`PESQUISA_MELODIA_GENERATIVA.md`), com uma camada de tradução
explícita entre intenção abstrata e parâmetro de síntese.

### 3.4 Mecanismos de sustentação (o que evita virar caricatura)

- **Contexto muda o resultado**: o mesmo `sforzando` num trecho
  pianíssimo não deve soar igual ao mesmo sforzando dentro de um
  fortíssimo — a interpretação lê `current_dynamic`/`phrase_position`/
  `density`/`previous_event` antes de decidir a magnitude real.
- **Orçamento de expressão** (`expression_budget`): se tudo é
  expressivo o tempo todo, nada se destaca — cada gesto (acento forte,
  portamento grande, vibrato fundo, ornamento) tem um "custo", e uma
  frase tem capacidade limitada.
- **Memória anti-caricatura**: 3 sforzandi seguidos reduzem a chance do
  próximo; muito tempo sem vibrato aumenta a chance dele aparecer —
  mesma família de ideia que `gestureHistory`/monotonyBias (item #16)
  já usa pra direção melódica, generalizada aqui pra QUALQUER gesto
  expressivo, não só direção.
- **Deriva de intenção** (`Expression Drift`): tranquillo → inquieto →
  agitato → furioso, sem troca abrupta de preset — a MESMA arquitetura
  que `instabilityField` (Noise Field) já implementa em
  `DualObjectEngine.cpp` (passeio lento com puxão de volta a um
  repouso), só que aplicada a um conteúdo diferente.
- **Resíduo expressivo**: depois de uma seção furiosa, o retorno ao
  tranquillo carrega um resíduo de tensão por alguns compassos, não
  reseta instantaneamente — o instrumento "carrega o que acabou de
  acontecer", a mesma lógica de `sustainSettle`/`excitationBreath`
  (nunca um corte abrupto, sempre uma curva de recuperação).
- **Modelo de expectativa**: o sistema observa repetição/direção/
  métrica e estima "o que parece provável agora", depois decide
  cumprir, atrasar, exagerar ou quebrar essa expectativa — isto é
  literalmente a teoria de Narmour (Implication-Realization, já citada
  na seção 4 de `PESQUISA_MELODIA_GENERATIVA.md` e já implementada no
  item #11/`continueBias`), generalizada aqui de "direção de pitch" pra
  qualquer dimensão musical (ritmo, densidade, harmonia).
- **Personalidade do performer**: um nível ACIMA da intenção — dois
  "performers" diferentes tocam o mesmo `dolce` de formas diferentes
  (um mais contido, outro mais dado a portamento/rubato) — o análogo
  direto do temperamento por família de `MaterialAgency.hpp` (seção
  2.3), aqui pensado por VOZ em vez de por família de módulos.

### 3.5 Onde isso já bate com o que EXCITAÇÃO tem hoje

Análise pedida pelo autor ("analise"). Boa notícia: boa parte do
vocabulário já existe em código, só não estava nomeada com esta
taxonomia:

| Camada do texto | Já existe em EXCITAÇÃO como |
|---|---|
| ARTICULATION | `GestureType` (Legato/Staccato/Marcato/Tenuto) — implementado HOJE, mesmos 4 nomes, antes de ler este texto |
| GESTURE local (sforzando) | `attackPunch` (item #2) — pico de energia efêmero no ataque, já é um proto-sforzando real |
| PROCESS (crescendo/diminuendo/morendo) | `PhraseState` (Climax≈pico, Release≈resolução) e `excitationRestHush`/#17 (o "morendo" real de fim de frase) |
| META/expressivity multiplicador | `interpretiveFreedom` (#14) — já controla literalmente "quanto a interpretação se desvia do esperado" |
| Memória anti-caricatura | `gestureHistory`/monotonyBias (#16) — mas só mede repetição de DIREÇÃO, não repetição de qualquer gesto expressivo (3 marcatos seguidos, por exemplo) — gap real e barato de estender |
| Expression Drift | `instabilityField` (Noise Field) — mesma arquitetura de campo-que-deriva-e-volta-a-um-repouso, conteúdo diferente |
| Resíduo expressivo | `sustainSettle`/`breathGain` — nunca corte abrupto, sempre curva |
| Modelo de expectativa | Narmour/`continueBias` (#11) — já existe pra direção de pitch, não generalizado pra outras dimensões |

**O que falta de verdade**: a camada **CHARACTER** propriamente dita —
um vetor pequeno e PERSISTENTE (mais lento que uma frase — talvez
várias frases, um "clima" de sessão) que module várias das peças acima
AO MESMO TEMPO, coerentemente. Hoje cada peça (vibrato, gesto, timbre,
liberdade) deriva de um sinal LOCAL diferente (fase da frase, ápice,
densidade) — nenhuma delas deriva de um estado de ânimo persistente e
COMPARTILHADO entre todas. Essa é a peça central ainda ausente.

## 5. A lacuna real em EXCITAÇÃO hoje

Comparado ao theremin da Orquestra Relacional (seção 2.1), EXCITAÇÃO
(`MelodicInterpreter`) hoje:

- Escuta PRINCIPAL/CLONE como um **borrão único** (`Stimulus::
  rawLevel`/`rawActivityDelta` já somam `abs(lastFirst)+abs(lastFifth)`
  antes de chegar no intérprete) — não há "o outro corpo" com quem
  EXCITAÇÃO tem uma relação, só uma média.
- Não tem verbo — density→freedom (#14) já é uma reação contínua
  (mais/menos liberdade), mas nunca vira uma ESCOLHA nomeada como
  "seguir" ou "contrapor".
- Não tem memória DA RELAÇÃO — `gestureHistory` (#16) lembra os
  últimos gestos da própria EXCITAÇÃO, não como PRINCIPAL/CLONE reagiu
  a ela.
- Não tem consequência de volta — EXCITAÇÃO lê `instabilityField` mas
  nunca o modifica; DERIVA é quem tem essa via (`nudgeInstability`).
  EXCITAÇÃO nunca influencia PRINCIPAL/CLONE, só é influenciada por
  eles.
- Timbre só é `f(registro)` (#10) + a pequena aresta de gesto adicionada
  hoje (#1/#13) — nenhuma variação por temperamento/ânimo/contexto
  relacional.

Isto é exatamente o diagnóstico que a fase 120 do RASGO_SYNTH já fez do
PRÓPRIO theremin antes da fase 121: *"Personalidade audível, mas
normalmente segue a melodia central [...] fazê-lo ouvir e responder sem
copiar."* EXCITAÇÃO hoje está no estado pré-fase-121 do seu próprio
irmão de código.

## 6. Proposta de adaptação (item 6.1 implementado 20 ago. 2026 — 6.2/6.3 aguardando decisão)

Seguindo o critério de 5 perguntas do `Metodo_RASGO.md` (seção 2.2):
o problema É compartilhado (a mesma classe de voz, o mesmo
`ThereminVoice.hpp` de origem); o que NÃO deve atravessar é a
complexidade de 7 famílias/`GamePlan`/`OpenModuleArchive` inteiros — ANTITOTEM
tem UMA voz solista, não uma orquestra de módulos, então a tradução
certa é o núcleo relacional (verbo + temperamento + consequência), não
o aparato inteiro.

### 6.1 Um vetor de caráter, não um verbo travado (eixo "espírito/interpretação") — FEITO

**Implementado 20 ago. 2026** (autor: "continuamos as implementações",
seguindo a própria recomendação deste documento — "o item de menor
risco/maior payoff"). Em `MelodicInterpreter.h`/`.cpp`: três membros
`characterEnergy`/`characterSoftness`/`characterBrightness` (0-1,
repouso em 0.5), cada um derivando sozinho por sample com a MESMA
arquitetura do Noise Field (`instabilitySeed`/passo pequeno
correlacionado + puxão de volta a um repouso) - `characterSeed`
dedicada, para não alterar o consumo de `seed` nos disparos já
calibrados. Roda sempre (mesmo com `amount=0` ou fora de disparos - um
"clima" do instrumento, não um gesto de nota), matching exatamente a
"Persistência" já especificada abaixo.

Os três destinos propostos, todos aplicados: `characterEnergyMultiplier`
(0.7-1.3) multiplica `magnitude`/`attackPunch`/`vibratoDepth`;
`characterSoftness` aumenta o peso de Marcato/Staccato no roll de
`GestureType` quando BAIXA (e Tenuto quando ALTA) e aprofunda o dip de
`articulation` quando BAIXA; `characterBrightness` soma direto (não
multiplica) em `timbreBrightness`, até ±0.6 em torno do neutro - o
segundo eixo tímbrico independente do registro que a seção 6.2 ainda
propõe construir de forma mais rica (arco/respiração). Getters públicos
em `MelodicInterpreter` e forwarding em `DualObjectEngine`
(`getExcitationCharacterEnergy()`/`Softness()`/`Brightness()`), sem
consumidor de UI ainda - mesmo padrão "infraestrutura primeiro" de
tudo mais nesta sessão. Build limpo, testes passando, comportamento
inicial idêntico ao anterior (repouso em 0.5 → todos os multiplicadores
partem neutros). Não ouvido ao vivo ainda - ver `TAREFAS.md`.

Versão original desta proposta (escrita antes da seção 3) cogitava um
`enum` de verbos (Seguir/Contrapor/Ocultar/Negociar), um por frase —
revisado depois de ler a seção 3.1: um enum trava a interpretação num
estado só por vez, exatamente o que o texto do ChatGPT argumenta contra
("dolce não equivale a um preset"). Proposta revisada, incorporando
3.1-3.4:

- Um vetor pequeno e CONTÍNUO — não os 16+ eixos do texto original (isso
  seria exatamente a "padronização excessiva" que o autor já pediu pra
  evitar em outro momento desta sessão), só o suficiente pra modular o
  que já existe de forma coerente. Candidato mínimo, cada eixo já com
  destino natural no código atual:
  - **energia** → magnitude do salto, attackPunch, vibratoDepth (mais
    alto = gestos maiores/mais firmes, mesmo eixo que #14/#6 já usam
    parcialmente, agora com uma fonte adicional e mais lenta).
  - **suavidade** (inverso de tensão) → profundidade do dip de
    `articulation`, peso de Marcato/Staccato no roll de `GestureType`
    (mais suave = articulation dip menor, menos marcato).
  - **brilho** → soma direto em `timbreBrightness`, o segundo eixo
    tímbrico independente do registro (#10) que a seção 6.2 já propõe.
- Verbos/nomes tradicionais (dolce, agitato, misterioso...) não
  precisam virar um enum interno — podem ficar só como REGIÕES
  nomeadas desse espaço pequeno pra documentação/depuração (ex.: dolce
  ≈ suavidade alta + energia baixa + brilho baixo), sem forçar o código
  a escolher uma categoria exclusiva - o vetor real permite estar "70%
  dolce, 30% misterioso" sem essa mistura precisar de um terceiro nome.
- **Persistência**: mais lento que uma frase (#6) — um "clima" que dura
  várias frases, a diferença real entre "estado de ânimo" (o que o
  autor pediu) e "posição dentro da frase atual" (o que #6/#17 já
  fazem). Arquitetura direta a reaproveitar: `instabilityField` (Noise
  Field) já é exatamente um "campo que deriva lentamente e puxa de
  volta a um repouso" (ver 3.4/3.5, "Expression Drift") - o mesmo
  princípio, aplicado a um conteúdo (caráter, não instabilidade) e
  possivelmente por sample rate mais lento ainda.
- **Fonte do drift**: em vez de puramente autônomo (como
  `instabilityField`), poderia também ler o mesmo contexto que #14 já
  lê (`ensembleLevel`) e outros sinais do ambiente - mais um lugar onde
  "influenciado pelo ambiente" (seção 6.3) e "caráter persistente" se
  encontram naturalmente.

### 6.2 Timbre por caráter + técnica idiomática real (eixo "camaleão") — parcialmente FEITO

**Implementado 20 ago. 2026** (autor: "prossiga"): o item mais barato
desta seção, **acoplamento intensidade↔timbre** - `MelodicInterpreter`
agora soma `apexDegree*0.3 + phraseEnergy*0.25` em `timbreBrightness`,
reaproveitando os MESMOS sinais que já escalam `apexGain`/`phraseGain`
(nenhum estímulo novo, só um segundo destino). O eixo de arco (sul
tasto/sul ponticello) já tinha sido coberto de fato pelo
`characterBrightnessOffset` do item 6.1 (soma independente do registro
em `timbreBrightness`) - a mesma ideia, implementada antes de eu voltar
a este item.

RASGO_SYNTH confirmou não ter uma solução pronta pra "reaproveitar"
aqui (seção 2.3) — esta parte precisa vir de teoria/técnica real de
instrumentos acústicos, adaptada, não de código já existente no
ecossistema:

- **Posição/pressão de arco (cordas)**: sul tasto (perto do braço,
  quente/suave, mais fundamental, menos harmônicos) → sul ponticello
  (perto do cavalete, vítreo/metálico, rico em harmônicos agudos) é o
  eixo clássico de "camaleão" tímbrico num instrumento de arco — mapeia
  bem num parâmetro contínuo de "brilho" já existente
  (`timbreBrightness`, hoje só função de registro), mas como um SEGUNDO
  eixo independente (não substituindo #10, somando-se a ele). **Feito
  via `characterBrightnessOffset` (item 6.1)**, ver acima.
- **Respiração/embocadura (sopros), AINDA NÃO FEITO**: ar não-tonal
  (breath noise) antes/durante o ataque — RASGO_SYNTH já tem
  `BreathExciter.hpp` (Mutable Instruments Elements) e ANTITOTEM já tem
  `NoisePalette` (6 cores) sem função de modulador ainda (CRI-NOI-001,
  `PESQUISA_RUIDO_GENERATIVO.md`) — uma ponte real, mas mais cara que o
  resto de 6.2: `MelodicInterpreter` não sintetiza áudio (fronteira
  deliberada, ver o comentário de classe), então uma textura de ruído
  de verdade precisaria de uma fonte nova ligada ao `excitationVoice`
  em `DualObjectEngine`, não só um número a mais na Voice. Ficou de
  fora desta rodada por esse motivo - risco/escopo maior que o resto
  de 6.1/6.2/6.3, que só reorganizavam sinais já existentes. Um verbo
  "Ocultar" poderia somar uma pitada de ruído respiratório (baixo,
  textural) em vez de só reduzir ganho, lendo como uma voz que
  "sussurra" em vez de simplesmente "fica mais quieta".
  - "flautando" (flauta): tom aéreo, rico em ar, pobre em harmônicos —
    outro ponto no mesmo eixo ruído-vs-tom.
  - Overblowing/harmônicos: saltos de registro reais em instrumentos de
    sopro mudam TIMBRE abruptamente, não só altura — um eco possível
    do salto grande que já dispara Marcato hoje (#1/#13): dar a saltos
    grandes também uma mudança timbral mais discreta/brusca, não só
    contínua por registro.
- **Acoplamento intensidade↔timbre — FEITO** (ver acima): em cordas e
  sopros reais, tocar mais forte/rápido não só aumenta volume, aumenta
  BRILHO (mais harmônicos agudos excitados).

### 6.3 Consequência de volta (eixo "influenciar e ser influenciado") — parcialmente FEITO

**Implementado 20 ago. 2026** (autor: "prossiga"): a primeira bala
desta seção - `MelodicInterpreter::Voice` ganhou `justTriggered` (true
só no sample de um disparo real; o intérprete não sabe nada de
`instabilityField`, só REPORTA o evento, mantendo a fronteira de
classe intacta). `DualObjectEngine::render()` lê isso e chama
`nudgeInstability(0.004f)` a cada disparo real de EXCITAÇÃO -
deliberadamente pequeno (o nudge de DERIVA em Main.cpp é
`abs(derivationMotion)*0.02`, até ~0.011 típico; este é ~1/3 disso),
pra não competir com o papel de DERIVA no mesmo campo, só ecoar. A
segunda bala (pipeline relacional geral) segue fora de escopo:

- ~~Espelhar `nudgeInstability`~~ - **feito**, ver acima.
- Mais ambicioso (fora do escopo de uma sessão, citado só como direção
  futura): um pipeline relacional geral como o que a fase 123 do
  RASGO_SYNTH já propôs — `observação → interpretação → ação →
  resistência → consequência → vestígio` — adaptado a dois corpos só
  (PRINCIPAL/CLONE vs. EXCITAÇÃO) em vez de vários.

## 7. Perguntas em aberto para o autor

Antes de escrever qualquer código desta proposta (diferente da
articulação, que já foi direto por ser aprofundamento sem risco
arquitetural):

1. ~~O vetor de caráter (6.1)~~ — **feito** (20 ago. 2026, autor:
   "continuamos as implementações") - energia/suavidade/brilho, drift
   lento tipo Noise Field, os três aplicados aos destinos já mapeados.
   Pergunta original ainda em aberto: os três eixos bastam, ou faltam
   eixos que o autor considera essenciais depois de ouvir?
2. ~~O timbre por caráter/intensidade (6.2)~~ — **feita a fatia barata**
   (20 ago. 2026, autor: "prossiga") - acoplamento intensidade↔brilho.
   Ainda em aberto: o eixo de ruído respiratório tipo sul-ponticello/
   flautando, mais caro (precisa de uma fonte de ruído nova ligada a
   `excitationVoice`, `MelodicInterpreter` não sintetiza áudio) -
   prioridade ainda não definida.
3. ~~A consequência de volta (6.3)~~ — **feita a primeira bala** (20
   ago. 2026, autor: "prossiga") - `nudgeInstability(0.004)` a cada
   disparo real de EXCITAÇÃO. O pipeline relacional geral (segunda
   bala, mais ambicioso) segue fora de escopo, sem prioridade.
4. Vale abrir uma frente irmã, fora do escopo imediato de EXCITAÇÃO:
   revisitar `IntermodularOrchestra.hpp` diretamente como referência de
   implementação (não só de conceito) antes de escrever o código de
   6.1-6.3, já que ele resolve um problema quase idêntico com código
   real, testado e ouvido?
5. A seção 3.4 traz "orçamento de expressão" e "modelo de expectativa
   generalizado" como mecanismos de sustentação de longo prazo — mais
   ambiciosos que o vetor de caráter em si. Ficam registrados como
   direção futura (não implementados agora), ou o autor já quer discutir
   algum deles em detalhe?

## 8. Calibração pendente (20 ago. 2026)

Ver a nota geral em `PESQUISA_MELODIA_GENERATIVA.md`, seção 7 (mesmo
pedido do autor, 19 ago. 2026). Constantes desta pesquisa, ambas em
`MelodicInterpreter.cpp`/`DualObjectEngine.cpp`, nenhuma confirmada por
escuta:

- Vetor de caráter (6.1): coeficiente de deriva por sample (0.00001 do
  roll, puxão de volta 0.0000004 - mais lento que qualquer outro
  seguidor do projeto, nunca testado contra uma sessão real longa o
  bastante pra ver os eixos se mover de verdade, mesma incerteza já
  registrada pra `formEnergy` em `PESQUISA_SEQUENCER_GENERATIVO.md`);
  `characterEnergyMultiplier` 0.7-1.3; peso de suavidade nos rolls de
  `GestureType` (±0.10/±0.15); `characterBrightnessOffset` ±0.6.
- Acoplamento intensidade↔timbre (6.2): `apexDegree*0.3 +
  phraseEnergy*0.25` somado em `timbreBrightness` - nunca comparado
  contra os equivalentes de ganho (`apexGain`/`phraseGain`) que
  inspiraram a escolha, nem contra o `characterBrightnessOffset` que já
  soma no mesmo lugar (três termos independentes, nunca ouvidos juntos
  pra saber se se acumulam bem ou brigam).
- Consequência de volta (6.3): nudge de `instabilityField` fixo em
  0.004 por disparo - escolhido como "~1/3 do nudge típico de DERIVA",
  nunca medido se esse fator realmente lê como "um eco menor", só como
  um número proporcionalmente menor no papel.

## 9. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, entrada
`CRI-REL-001`.
