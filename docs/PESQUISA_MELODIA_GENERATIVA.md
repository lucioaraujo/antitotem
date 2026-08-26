# Pesquisa: melodia generativa e o "Melodic Interpreter"

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-MEL-001` nesse arquivo. Origem: trabalho ao vivo em
EXCITAÇÃO (ver `TAREFAS.md`, 20 ago. 2026), mas o conteúdo aqui é
deliberadamente transversal — não específico do ANTITOTEM.

## 1. Por que este documento existe

Ao construir EXCITAÇÃO (a voz generativa tipo-theremin do ANTITOTEM), o
autor apontou repetidamente que um gerador de alturas independentes -
mesmo livre de escala fixa, mesmo com passo limitado - ainda soa como
"gerador de notas", não como "assoviar uma melodia". O autor colou um
brief longo (ver seção 3) descrevendo o que separa as duas coisas, com a
observação-chave: **a arquitetura correta não é
`SEQUENCER → SYNTH`, é `SEQUENCER → MELODIC INTERPRETER → VOZ`** - uma
camada intermediária reutilizável entre instrumentos, não amarrada a
nenhuma voz específica.

## 2. O que já existe no projeto RASGO sobre este assunto

Pesquisa verificada antes de escrever qualquer coisa nova (I2 do funil -
"separar referência primária, interpretação, lembrança e especulação"):

### 2.1 `RASGO_SYNTH/rasgo-synth-core/src/` (prior art de código, já implementado)

- `dsp/ThereminVoice.hpp` - um theremin real: nunca redispara, só desliza
  até um alvo (`glideCoeff` exponencial calculado contra a taxa de
  amostragem real), mais um vibrato pequeno (5-7Hz, fração de semitom)
  modelando o tremor natural da mão. Já portado pra EXCITAÇÃO.
- `sequencer/Scales.hpp` - ~39 escalas/modos reais, com erros de uma fonte
  anterior documentados e corrigidos (HarmonicMinor, Bebop/Byzantine
  duplicados, Arabian/Chinese inventados e removidos).
- `sequencer/HarmonicWanderer.hpp` - 6 técnicas reais de movimento
  harmônico (Coltrane changes, substituição tritônica, mediante cromática/
  Jobim, intercâmbio modal, jazz modal/"So What", backdoor ii-V).
- `dsp/MelodyVoicingBank.hpp` - 5 princípios reais de voicing vertical
  (quartal/McCoy Tyner, planing/Debussy, bitonal/Stravinsky, tríade
  clássica, modal armênio/duduk + fusão quartal ao estilo Tigran
  Hamasyan).
- Não sondados em profundidade ainda: `Euclidean.hpp`, `SieveScale.hpp`,
  `NeoRiemannian.hpp`, `PolymeterEnsemble.hpp`, `MicrotonalTuning.hpp`,
  `CoherenceCollapse.hpp` (detector de "ruptura por coerência excessiva"
  via entropia de Shannon), `BluffSignal.hpp` (modelo de blefe/confiança
  como operação musical).

### 2.2 `RASGO_DOCUMENTATION/ATLAS_DE_REFERENCIA_RASGO(4).md` (pesquisa institucional já registrada)

- **Seção 108 — Suécia, KTH Speech, Music and Hearing**: já reconhecido no
  Atlas como referência em "síntese vocal, percepção, fala, música,
  acústica, expressividade", com pesquisas atuais listadas em "síntese
  contextual, respiração, hesitação, creaky voice, microvariação,
  expressividade". O próprio Atlas já cristalizou um "Conceito Rasgo" a
  partir disso: **"imperfeição pode ser expressiva, não apenas
  degradante"**, com um modelo `SINAL CONTROLADO → MICROVARIAÇÃO →
  HESITAÇÃO → QUEBRA → PRESENÇA`. Isso é o KTH Rule System (ver seção 4)
  já filtrado pela lente do projeto, mas sem a técnica específica nomeada
  ainda.
- **Seção 109 — Noruega, RITMO/fourMs (University of Oslo)**: pesquisa em
  "música, mente, movimento, máquinas, multimodalidade, ritmo, gesto,
  audiovisual" - o eixo "gesto" do brief (seção 3, item 1) já tem
  contraparte institucional reconhecida aqui.
- Tabela geral (linha ~3875) já lista KTH como "voz/expressividade",
  confirmando que a instituição (não só a técnica) já era conhecida do
  Atlas antes deste documento.

### 2.3 Instrumentos irmãos verificados sem material equivalente

Varredura rápida (`grep` por melod/theremin/generative pitch/solo voice)
em AQUORBIUM, TRIOIO, NAVALHA2_JUCE, NAVALHA2_PD, RASGO_MODULAR -
nenhum tem infraestrutura de melodia generativa comparável à do
RASGO_SYNTH hoje. AQUORBIUM tem uma única menção incidental (um módulo
Eurorack chamado "mylar melodies" numa lista de referências de hardware,
sem relação com o assunto deste documento).

## 3. O brief do autor (fonte secundária/interpretativa, não primária)

Colado ao vivo pelo autor (20 ago. 2026), atribuído a uma conversa com o
ChatGPT sobre o RASGO Modular, mas aplicado aqui por decisão explícita do
autor. Marcado como fonte **secundária/interpretativa** (não uma
referência acadêmica primária) - o valor real está em como organiza e
nomeia os problemas, não em alegações de precisão técnica não verificada.
Resumo dos 19 eixos descritos, na ordem original:

1. **Continuidade melódica** - entre duas notas: salto direto, portamento,
   glissando curto/longo, aproximação por cima/baixo, overshoot/
   undershoot, ou nota realmente separada.
2. **Ataque da nota** - limpo, suave, soprado, atrasado, crescente, com
   scoop de pitch, acentuado, ou "nascendo" da nota anterior.
3. **Sustentação** - uma nota longa tem trajetória interna de energia
   (`cresce → estabiliza → vibra → diminui`), não um platô constante.
4. **Final da nota** - corte seco, diminuendo, queda/subida de pitch,
   vibrato desaparecendo, conexão direta à próxima, respiração, silêncio.
5. **Vibrato progressivo** - não um LFO permanente: nasce depois do
   início, com profundidade/velocidade/atraso/irregularidade próprios,
   correlacionados a duração e função da nota (notas curtas ≈ sem
   vibrato).
6. **Fraseado** - o instrumento percebe GRUPOS de notas (início,
   desenvolvimento, clímax, resolução), não eventos isolados - uma
   "Phrase Energy" contínua 0-1 dando arco a vários eventos.
7. **Respiração** - uma "breath_capacity" que diminui ao longo da frase;
   perto do limite, aumenta a chance de encurtar nota, baixar
   intensidade, inserir silêncio ou separar a frase.
8. **Hierarquia das notas** - passagem, estrutural, chegada, tensão,
   ápice, resolução, repetição, bordadura - inferível mesmo sem o
   sequenciador marcar isso explicitamente.
9. **Contorno melódico** - subida, descida, arco, repetição, salto,
   retorno; o instrumento reage à direção, não só ao valor pontual
   ("prosódia melódica").
10. **Registro** - timbre como função da altura (`Timbre = f(pitch)`),
    não um preset fixo - grave mais escuro/lento, agudo mais brilhante/
    tenso/instável.
11. **Intervalo entre notas controla o gesto** - repetição→articulação;
    semitom→glide pequeno; tom→legato; 3ª→conexão normal; 4ª/5ª→gesto
    perceptível; >5ª→salto/glide dramático. Probabilístico, não
    determinístico.
12. **Microafinação** - três camadas: target pitch (nota desejada),
    arrival pitch (onde o gesto chega primeiro), living pitch (oscilação
    durante a sustentação) - "instabilidade controlada", não erro
    aleatório.
13. **Legato** - uma variável `connection` (0=articulado, 1=linha
    contínua), calculada a partir de duração/intervalo/velocidade/posição
    na frase/densidade do sequenciador.
14. **Densidade** - sequenciador denso → reduz ornamentos; sequenciador
    esparso → mais liberdade interpretativa (glissandi, vibrato,
    aproximações, ecos, notas ornamentais).
15. **Ornamentação** - appoggiatura, mordente, nota de aproximação,
    escape, grace note, bend pequeno, turn, glissando - numa camada
    separada ("melodic elaboration") pra não destruir a identidade da
    sequência original.
16. **Memória** - lembrar últimas notas/intervalos/direção/clímax/
    articulação/tempo desde a última pausa/vibrato recente/registro
    recente, pra evitar repetir o mesmo gesto em sequência.
17. **Estados expressivos** - poucos estados internos (REST, BREATH,
    ATTACK, CONNECT, SUSTAIN, INTENSIFY, RELEASE), mais um estado musical
    de escala maior (calm, moving, ascending, tense, climax, release).
18. **Imperfeição correlacionada** ("motor drift"/"gesture inertia") -
    nunca `pitch += random()` isolado; um erro influencia o próximo
    (`+3 cents, +4, +2, 0, -1`), simulando movimento corporal contínuo.
    Especialmente adequado ao theremin (a mão real nunca pula
    instantaneamente).
19. **Arquitetura reutilizável**: `SEQUENCER → MELODIC INTERPRETER →
    VOZ`, não `SEQUENCER → SYNTH`. O sequenciador manda só `pitch, gate,
    velocity, duration, time`; o Melodic Interpreter deriva
    `phrase_position, interval, direction, importance, connection,
    breath, tension, gesture, vibrato, arrival, release`; só então a voz
    gera som. A mesma camada serviria a Theremin, Whistle, Flute, Voice,
    Violin, Synthetic Lead - um motor de interpretação monofônica, não um
    recurso do theremin especificamente.

## 4. Referências acadêmicas reais (fonte primária, verificadas por conhecimento próprio - não busca ao vivo nesta rodada)

Adicionadas porque o brief do autor, embora bem organizado, não cita
nenhuma fonte formal. Estas são pesquisas reais, publicadas, sobre
modelagem computacional de expressividade musical - o mesmo território,
com décadas de trabalho acadêmico por trás:

- **Narmour, E. (1990/1992). *The Analysis and Cognition of Basic
  Melodic Structures* / *...of Melodic Complexity*.** Implication-
  Realization theory: um intervalo pequeno implica continuação
  ("process") na mesma direção; um salto grande implica reversão
  ("registral return"). Mapeia quase 1:1 no item 11 do brief. **Já
  implementado** em EXCITAÇÃO (`excitationPreviousDirection`/
  `excitationPreviousMagnitudeSemitones` em `DualObjectEngine.cpp`, 20
  ago. 2026) - viés de continuação 0.75 (passo pequeno) a 0.25 (salto
  grande), não um sorteio 50/50.
- **Friberg, A., Sundberg, J., Frydén, L. (KTH Speech, Music and
  Hearing).** O "KTH Rule System" - o modelo mais citado de regras de
  performance computacionais, mapeando estrutura (contorno, harmonia,
  métrica) em micro-variações de timing/dinâmica/articulação. Já
  parcialmente reconhecido no projeto via
  `ATLAS_DE_REFERENCIA_RASGO(4).md`, seção 108 (ver 2.2 acima) - este
  documento é o primeiro a nomear a técnica específica, não só a
  instituição.
- **Todd, N. P. M. (1992). "The dynamics of dynamics: a model of musical
  expression."** *JASA*. Kinematic model: trata frases como movimento
  físico (aceleração/desaceleração), gerando rubato a partir de uma curva
  de energia - formalização matemática real do item 6/7 do brief (Phrase
  Energy/respiração).
- **Juslin, P. N. (2003). "Five facets of musical expression: a
  psychologist's perspective on music performance."** *Psychology of
  Music*. O modelo GERMS: Generative rules, Emotional expression, Random
  fluctuations, Motion principles, Stylistic unpredictability - um jeito
  estruturado de pensar a sobreposição de camadas (glide + vibrato +
  drift + fraseado) que o brief descreve de forma mais solta.

**Nota de proveniência**: estas quatro referências foram citadas por
conhecimento próprio do modelo (Claude), não verificadas via busca web
nesta sessão - autor pediu "aprofunde a pesquisa" num momento em que a
sessão estava perto do limite de uso, então a pesquisa foi deliberadamente
mantida como conhecimento já disponível, sem custo de chamadas extras.
Se citações exatas (ano/página/DOI) importarem pra uso formal, precisam de
verificação externa antes.

## 5. O que já está implementado em EXCITAÇÃO (ANTITOTEM) vs. o que não está

Ver `TAREFAS.md` para o histórico completo de decisões ao vivo. Resumo
cruzando os 19 itens da seção 3 com o estado real do código
(`src/core/DualObjectEngine.h`/`.cpp`, 20 ago. 2026).

**Nota de localização (20 ago. 2026, item #19)**: a decisão musical
inteira foi extraída de `DualObjectEngine` pra uma classe nova,
`src/core/MelodicInterpreter.h`/`.cpp` - as referências a
`excitationPitch`/`excitationGlideCoeff`/`excitationRegisterAverage`/
etc. nas linhas abaixo (escritas ANTES da extração) agora vivem lá
dentro sem o prefixo `excitation` (`pitch`/`glideCoeff`/
`registerAverage`/etc., membros privados de `MelodicInterpreter`) - a
matemática/constantes não mudaram, só o endereço. Não reescrevi cada
linha da tabela pra não inflar o diff por uma renomeação mecânica; ver
o comentário de classe em `MelodicInterpreter.h` pro mapeamento
completo.

| Item do brief | Estado |
|---|---|
| #18 Imperfeição correlacionada (motor drift) | **Feito** - passo limitado com piso não-zero, sem saber nomear a técnica na hora |
| #11 Intervalo controla o gesto (Narmour) | **Feito** - viés de continuação/reversão baseado na magnitude do passo anterior |
| #1 Continuidade melódica (glide varia por intervalo) | **Feito, com variedade real de gesto** (19 ago., aprofundado 20 ago. 2026, autor: "precisamos melhorar estacatos e outras formas de articulação e gesto de nota") - `glideCoeff` recomputado a cada disparo a partir da mesma magnitude que o Narmour já calcula (~90ms num passo pequeno até ~430ms num salto de 4 semitons). Novo: `GestureType` (Legato/Staccato/Marcato/Tenuto), sorteado por disparo enviesado pelo contexto já existente (magnitude pequena+liberdade interpretativa/#14 → mais staccato; chegada métrica/#8+salto grande → mais marcato; ápice/#8+clímax de frase/#6 → mais tenuto) - Legato continua majoritário (peso-base fixo), preservando "a real theremin never re-triggers... the glide is ALWAYS on" como comportamento PADRÃO: o pitch nunca para de deslizar por baixo, só o ENVELOPE DE GANHO (`gestureDecay`) muda de forma pra Staccato/Marcato, dando impressão real de separação/ênfase sem quebrar o mecanismo de glide contínuo |
| #5 Vibrato progressivo | **Feito** - profundidade rampeia de 0 a plena ao longo de ~350ms desde o disparo (`excitationNoteAge`), em vez de constante desde o primeiro instante |
| #12 Microafinação (3 camadas) | **Feito** (20 ago. 2026, autor: "falta algo a implementar na melodia? prossiga com inteligência e perspicácia") - as três camadas agora existem de fato e são DISTINTAS: target (`pitch`, o alvo do Narmour walk), arrival (o overshoot/#15, que ultrapassa o alvo e assenta - já existia, agora nomeado como a camada certa), e living (NOVO - `livingPitchPhase`, um LFO senoidal separado do vibrato: ~0.45Hz/~2 cents, bem mais lento e raso que o vibrato de #5, "controlado" por ser uma curva suave determinística, não passeio aleatório - presente o tempo todo, inclusive fora do envelope do vibrato, porque representa a instabilidade do INSTRUMENTO, não um gesto de uma nota específica) |
| #6 Fraseado (Phrase Energy) | **Feito, de verdade** (20 ago. 2026, autor: "vamos deixar o instrumento sofisticado e original nesse quesito") - `excitationPhraseEnergy` novo, DELIBERADAMENTE separado de #7/fôlego (a versão de 19 ago. tratava os dois como o mesmo sinal). Progresso por contagem de notas própria (`excitationPhraseNoteIndex`/`excitationPhraseLength`), não por fôlego (que vaza/recupera continuamente, sem dar uma posição narrativa limpa). Duas formas complementares do mesmo progresso: `excitationPhraseEnergy` (tenda, sobe até o clímax sorteado e desce depois - ênfase dinâmica: até +50% de magnitude do salto, +50% de vibrato, +20% de ganho, +15% de overshoot) e `excitationContourBias` (cresce dentro de cada metade - puxão direcional real: empurra a nota pra LONGE do registro médio subindo até o clímax, puxa de VOLTA pra ele descendo, moldando o CONTORNO, não só decoração). Comprimento de frase (5-13 notas) e posição do clímax (25%-75%) sorteados a CADA frase nova - nenhuma frase igual à anterior |
| #7 Respiração (breath_capacity) | **Feito** (já existia como `excitationBreath` desde 19 ago., cooldown-stretch + `breathGain`; formalizado como item DISTINTO de #6 em 20 ago., não mais o mesmo sinal reaproveitado) - "separar a frase" (a parte que faltava do item) agora é um evento real: ao completar `excitationPhraseLength` notas, um descanso GRANDE (+2x `baseCooldown`) marca a fronteira, distinto do stretching normal de fôlego |
| #9 Contorno melódico maior (arco de várias notas) | **Feito, recalibrado** (20 ago. 2026) - trocou o proxy de fôlego (`phrasePosition = 1 - excitationBreath`) pelo `phraseProgress` real (por contagem de notas, item #6 acima) no mesmo termo do `continueBias` do Narmour - mesma ideia (frase fresca explora mais, frase madura se compromete mais com a direção atual), dado mais preciso agora que existe |
| #16 Memória de gestos | **Feito** (20 ago. 2026, seguindo a recomendação do próprio assistente após #17: "quais outros itens da melodia a implementar" -> "sim") - `excitationGestureHistory` (buffer circular dos últimos 4 gestos, `direction*magnitude` com sinal, gravado a cada disparo) mais `excitationGestureHistoryWrite` (índice do anel). No cálculo do `continueBias` de Narmour (#11), conta-se quantas das 4 entradas do histórico compartilham o mesmo sinal da direção anterior (`sameDirectionCount`) e subtrai-se `monotonyBias = (sameDirectionCount/4) * 0.25` do viés de continuação, dentro dos mesmos limites 0.15-0.85 já existentes - uma frase que já andou repetidamente na mesma direção fica com mais vontade estatística de reverter, sem nunca proibir a repetição (é só um viés, não uma regra rígida, conforme "sem muitas padronizações") |
| #3 Sustentação (trajetória interna) | **Feito, fatia mais barata ("diminui")** (19 ago. 2026) - "cresce" já era o vibrato progressivo (#5), "estabiliza" já é o glide; faltava só "diminui" - `sustainSettle`, reaproveita `excitationNoteAge` (já existia pro vibrato): uma nota sustentada muito tempo sem novo disparo cede até 15% de ganho ao longo de ~3s, em vez de um platô constante |
| #2, #4 Ataque/final com trajetória interna | **Feito, sem redesenhar a arquitetura** (20 ago. 2026, autor: "item #2/#4, #14 e #19, prossiga") - a voz continua sem "fim de nota" tradicional (glide sempre ligado), mas duas trajetórias reais foram construídas EM CIMA disso, sem tocar na arquitetura de glide contínuo. #2 (ataque): `attackPunch`, reinicia a 1.0 em todo disparo e decai em ~50ms, somando até +18% de ganho num transiente pontual - eixo diferente do dip de `articulation` (#13, que REDUZ em saltos grandes; este SOMA sempre, dando à voz um "impacto" que uma voz que só desliza não teria de outro jeito). #4 (final): a última nota de uma frase é conhecida de antemão (lookahead de `phraseNoteIndex+1 >= phraseLength` no disparo, antes de incrementar); enquanto ela soa, `finalEase` cresce ao longo de ~1.2s acalmando o vibrato (até -60%) e escurecendo o timbre (até -40%) - um gesto de FECHAMENTO real (caráter, não só volume), complementar ao silêncio de amplitude que `restHush`/#17 já provoca no mesmo momento |
| #8 Hierarquia de notas | **Feito, duas fatias (ápice + chegada/estrutural)** (19 e 20 ago. 2026, segunda fatia autor: "falta algo a implementar na melodia? prossiga com inteligência e perspicácia") - `registerAverage`/`apexDegree` (19 ago., eixo ESPACIAL: distância do registro recente) seguem como antes. Nova (20 ago.): `metricGlow`/`arrivalStrength` (eixo TEMPORAL: proximidade de um acento métrico real, reaproveitando `Stimulus::accentEvent`, o mesmo Accent Field que já existia) - uma nota que dispara perto de um acento estrutural lê como chegada (ataque até +40% mais firme, vibrato até -25% mais contido, articulação um pouco mais re-articulada), uma nota fora do tempo lê como passagem (comportamento normal). Duas categorias reais das 8 do item agora cobertas por mecanismos DIFERENTES (registro vs. tempo métrico), não a mesma ideia com dois nomes. Faltam ainda: estrutural/tensão/resolução/repetição/bordadura como categorias distintas |
| #10 Timbre por registro | **Feito** - `soundingPitch * 3.0` mapeado direto pra `CmosVoice::setOscillatorShape(0, ...)` no oscilador A (o dominante da mistura, `oscillatorLevels[0]=0.62`); grave = seno puro (shape 0, o padrão que já existia), agudo = dente-de-serra/quadrada (shape 3) - função contínua do registro, não um preset fixo, cobrindo o range inteiro que `setOscillatorShape` já clampa |
| #13 Legato/`connection` (lado da amplitude) | **Feito, com `connection` agora explícita** (19 ago., aprofundado 20 ago. 2026) - `articulation`: dip de volume no ataque, proporcional à magnitude do intervalo (~0.94 legato a ~0.55 salto grande), recuperando em ~35ms. Antes só implícita no tamanho do intervalo; agora `GestureType` (ver #1) É a variável `connection` explícita que faltava - Legato (0=articulado.../1=linha contínua, no sentido do brief) permanece o padrão, mas Staccato/Marcato/Tenuto são valores REAIS e nomeados dessa mesma variável, não um contínuo numérico solto - escolha deliberada de nomear estados em vez de expor um float 0-1 (mesma filosofia "poucos estados" do item #17) |
| #14 Densidade → liberdade interpretativa | **Feito** (20 ago. 2026, autor: "item #2/#4, #14 e #19, prossiga") - segunda leitura do MESMO seguidor de nível já usado pra balancear ganho (`ensembleLevel`, mesmo princípio de "um valor compartilhado lido por vários destinos" já usado no projeto pro Noise Field), invertido em `interpretiveFreedom` (textura densa/alta = pouca liberdade; textura rala/silenciosa = liberdade real). Deliberadamente o OPOSTO de "quanto mais barulho, mais eu grito" (que soaria como competir, não interpretar) - textura densa disciplina a voz, mantendo o `continueBias` perto do que Narmour já implica; textura rala deixa esse mesmo `continueBias` derivar até 35% em direção a uma moeda honesta (0.5), abrindo espaço pra decisões que fogem do esperado - a liberdade age na PRÓPRIA escolha de direção, não só em decoração de volume/vibrato. Compõe modestamente com saltos (+30%) e ornamento/overshoot (+20%) quando há liberdade a gastar |
| #15 Ornamentação | **Feito, fatia mais barata (overshoot/aproximação)** (19 ago. 2026) - `excitationOvershoot`: no disparo, o alvo do glide passa a ser `excitationPitch + direction*magnitude*0.35`, não o alvo puro - a voz "ultrapassa" um pouco na direção do salto (mais em leaps grandes, quase nada em passos pequenos) e decai de volta ao alvo real em ~110ms - um scoop/bend de aproximação real, também fecha o detalhe pendente do item #1 ("overshoot/undershoot" nunca implementado). Não implementa appoggiatura/mordente/grace note/turn discretos - não fazem sentido sem notas discretas nesta voz de glide contínuo |
| #17 Estados expressivos nomeados | **Feito** (20 ago. 2026, autor: "quais outros itens da melodia a implementar" -> "sim") - `ExcitationMicroState` (Rest/Attack/Connect/Sustain/Release, computado por sample a partir de sinais que já existiam: cooldown/idade da nota/distância do glide) e `ExcitationPhraseState` (Calm/Ascending/Climax/Release, a partir de `excitationPhraseEnergy`/#6). Simplificado do brief (5+4 estados em vez de 7+6 - "poucos estados" já era o próprio pedido), mas com um efeito AUDÍVEL real desbloqueado, não só classificação interna: até 20 ago. EXCITAÇÃO nunca "descansava" de verdade entre frases (só sustentava a última nota decaindo); o novo descanso de fronteira de frase (#6/#7) agora vira SILÊNCIO real via `excitationRestHush` (foge a 0 em ~0.4s durante o descanso, recupera em ~0.15s - uma entrada suave, não um corte abrupto) |
| #19 Camada "Melodic Interpreter" separada e reutilizável | **Feito** (20 ago. 2026, autor: "item #2/#4, #14 e #19, prossiga", depois de #6/#7/#9/#16/#17 já feitos e testados - acima do limiar de "2-3 itens validados" que a pesquisa cogitava) - `src/core/MelodicInterpreter.h`/`.cpp`, classe nova sem NENHUMA dependência de `CmosVoice`/JUCE/qualquer motor de síntese específico: recebe um `Stimulus` genérico (variação de atividade, evento de acento, instabilidade, nível/densidade, amount, running/muted) e devolve uma `Voice` (pitch soante, ganho já composto, dica de timbre 0-3, estados micro/macro) - quem chama decide o que FAZER com isso. `DualObjectEngine` ficou só com o que é ESPECÍFICO dela (transformar PRINCIPAL/CLONE em Stimulus, alimentar seu próprio `CmosVoice` com a Voice devolvida). Extração mecânica, comportamento idêntico (mesma matemática/constantes, só o endereço mudou - ver a nota de localização no topo desta seção) - `antitotem_simple_sequencer_tests` passando antes e depois, build limpo com `-Wall -Wextra -Wpedantic -Werror` |

## 6. Próximos passos sugeridos (não decididos - aguardando o autor)

Autor já pediu explicitamente "sem muitas padronizações" (ver
`TAREFAS.md`) - a lição principal desta pesquisa não é implementar os 19
itens, é escolher poucos, bem escolhidos, que já se pagam com pouco
código. Por ordem de custo/benefício estimado, mais barato primeiro:

1. ~~Vibrato progressivo (#5)~~ - **feito** (20 ago. 2026,
   `excitationNoteAge`, rampa de ~350ms).
2. ~~Variedade real de conexão entre notas (#1)~~ - **feito** (mesmo dia,
   `excitationGlideCoeff` recomputado por disparo a partir da magnitude
   já calculada pra Narmour). Ainda falta variar o TIPO de conexão (só
   existe glide contínuo hoje, não salto direto/aproximação/overshoot
   como gestos distintos).
3. ~~`connection`/legato (#13, lado da amplitude)~~ - **feito** (mesmo
   dia, `excitationArticulation` - dip de ataque proporcional à
   magnitude do intervalo, recuperando em ~35ms). Ainda falta uma
   variável `connection` explícita/controlável de fato - hoje é
   implícita, derivada só do tamanho do intervalo.
4. ~~Phrase Energy + respiração (#6/#7)~~ - **feito** (20 ago. 2026,
   autor: "vamos deixar o instrumento sofisticado e original nesse
   quesito"). Ver seção 5 pros dois itens - implementados como sinais
   DISTINTOS pela primeira vez (a versão de 19 ago. reciclava fôlego
   pros dois), com comprimento de frase e posição de clímax sorteados a
   cada frase.
5. ~~Memória de gestos (#16)~~ - **feito** (20 ago. 2026,
   `excitationGestureHistory`, buffer circular de 4 gestos, viés
   `monotonyBias` de até -0.25 no `continueBias` de Narmour contra
   direção repetida).
6. ~~Camada `MelodicInterpreter` separada (#19)~~ - **feito** (20 ago.
   2026, autor: "item #2/#4, #14 e #19, prossiga") -
   `src/core/MelodicInterpreter.h`/`.cpp`, extração mecânica sem
   dependência de `CmosVoice`/JUCE.
7. ~~Ataque/final com trajetória interna (#2/#4)~~ - **feito** (mesmo
   dia) - `attackPunch` (transiente de até +18% em ~50ms a cada
   disparo) e `finalEase` (vibrato/timbre se acalmam ao longo de ~1.2s
   na última nota conhecida de uma frase), ambos sem redesenhar o
   glide contínuo.
8. ~~Densidade → liberdade interpretativa (#14)~~ - **feito** (mesmo
   dia) - `interpretiveFreedom`, segunda leitura de `ensembleLevel`
   (mesmo já usado pra `ensembleGain`), empurra `continueBias` até 35%
   rumo a uma moeda honesta quando a textura está rala.

Com isso, todos os itens desta lista de custo/benefício progressivo
estão feitos. O que resta do brief original (seção 3): #12 microafinação
de verdade (hoje "Parcial", sem a nomenclatura target/arrival/living
explícita), e aprofundar fatias já implementadas mas parciais (#8
hierarquia de notas - só a categoria "ápice"; #1/#13 - só um tipo de
conexão/glide). Nenhum decidido ainda - aguardando o autor.

## 7. Calibração pendente (19 ago. 2026)

Autor: "precisamos elaborar e aprofundar as boas maneiras de
calibração, ligadas a inovação e experimentação" - "isso" (confirmando)
- "também considerar que os itens que foram desenvolvidos no inicio do
processo... como repensá-los, adaptá-los, recalibrá-los". Lista das
constantes desta pesquisa ainda marcadas como estimativa (não medidas/
confirmadas por escuta específica) - todas em `DualObjectEngine.cpp`
salvo indicação contrária:

- Narmour: `continueBias` base 0.75→0.25, magnitude 1-4 semitons.
- Glide: ~90ms (passo pequeno) a ~430ms (salto de 4 semitons).
- Vibrato: profundidade 0.006, taxa 6Hz, rampa de 350ms.
- Cooldown: 60000→11000 amostras base, esticamento até 2.5x com fôlego
  baixo.
- Fôlego: custo 0.12 por disparo, recarga em ~2.5s.
- Articulação: dip 0.94 (passo pequeno) a 0.55 (salto grande),
  recuperação em ~35ms.
- Ganho: `ensembleGain` escala 2.0x, piso/teto 0.3/1.0; trim fixo 0.35.
- Arco de frase (#9): +0.15 no `continueBias`, limites alargados pra
  0.15-0.85.
- Ápice (#8): escala de distância 4.0x, +60% vibrato, +25% ganho.
- Overshoot (#15): 0.35x da magnitude do salto, decaimento em ~110ms.
- Sustentação (#3): até -15% de ganho ao longo de ~3s sem novo disparo.
- Fraseado/Phrase Energy (#6, novo 20 ago. 2026): comprimento de frase
  5-13 notas, posição de clímax 25%-75%, magnitude do salto até +50%,
  contorno (puxão pro/longe do registro) 0.12/0.20 (subindo/descendo),
  overshoot +15%, vibrato +50%, ganho +20%, descanso de fim de frase
  2x `baseCooldown` - nenhum desses 8 números foi comparado por escuta
  entre si nem contra os equivalentes de #8/#9/#15 já existentes.
- Estados nomeados (#17, novo 20 ago. 2026): limiares do micro estado
  (Attack < 0.05s, Release > 1.5s de idade da nota, Connect a partir de
  0.002 de distância de glide) e o silêncio de fronteira
  (`excitationRestHush`, fuga em 0.4s, recuperação em 0.15s) - nenhum
  medido/confirmado por escuta específica ainda.
- Timbre por registro (#10): mapeamento direto `soundingPitch*3.0` - não
  é bem uma "estimativa" (é uma função literal, não um número ajustado),
  mas o RESULTADO sonoro nunca foi confirmado especificamente.
- Memória de gestos (#16, novo 20 ago. 2026): tamanho do histórico (4
  gestos), peso do `monotonyBias` (0.25 no máximo, quando as 4 entradas
  concordam de sinal) - nenhum medido/confirmado por escuta específica
  ainda; em particular, 4 gestos foi um chute de "memória curta o
  bastante pra não virar uma regra rígida de longo prazo", nunca
  comparado com 2, 3 ou 6.
- Ataque/final (#2/#4, novo 20 ago. 2026): punch de ataque +18% em
  ~50ms; trajetória de final ao longo de ~1.2s, -60% de vibrato/-40% de
  timbre no fim - nenhum medido por escuta; em particular a duração de
  1.2s do `finalEase` foi um chute sem relação direta com o tempo real
  de descanso de fronteira (que depende do `baseCooldown` variável, não
  é uma constante fixa), vale reconferir se as duas janelas de tempo
  colidem ou se complementam bem na prática.
- Densidade → liberdade (#14, novo 20 ago. 2026): blend de
  `continueBias` até 35% rumo a 0.5, +30% de magnitude, +20% de
  overshoot - nenhum medido por escuta; a escolha de reusar
  `ensembleLevel` (em vez de um rastreador de densidade dedicado, ex.
  contagem de steps ativos do sequencer) é uma aposta de que loudness
  combinada é um proxy razoável de "quanto está acontecendo", nunca
  comparada com uma medida de densidade mais direta.
- Tipo de gesto (#1/#13 aprofundados, novo 20 ago. 2026): pesos do roll
  (staccato base 0.15 + até 0.25 de liberdade + até 0.2 de passo
  pequeno; marcato base 0.10 + até 0.35 de chegada + até 0.15 de salto;
  tenuto base 0.10 + até 0.25 de ápice + até 0.15 de clímax; legato peso
  fixo 0.65) - nenhum comparado por escuta; a proporção resultante
  aproximada (legato claramente majoritário, os outros três minoritários
  mas não raros) nunca foi ouvida/confirmada. Constantes do envelope:
  piso 0.04, ~100ms marcato / ~160ms staccato - chutes de "curto o
  bastante pra ler como destacado, não abrupto a ponto de estalar".

**Itens do início do processo que merecem um novo olhar**: o próprio
mecanismo de EXCITAÇÃO nasceu antes do Noise Field/Accent Field/DERIVA
existirem - hoje ela já lê `instabilityField` (via o limiar de disparo)
mas os detalhes de fôlego/cooldown/glide acima foram todos calibrados
ANTES dessa conexão existir. Vale reconsiderar se esses números ainda
fazem sentido agora que EXCITAÇÃO reage a mais fontes de estímulo do
que quando foram escolhidos.

## 8. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, tabela da
seção 5, entrada `CRI-MEL-001`.
