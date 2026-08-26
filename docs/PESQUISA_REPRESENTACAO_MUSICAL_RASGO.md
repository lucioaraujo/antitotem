# Pesquisa: representação musical intermediária própria do RASGO ("RASGO Score")

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-SCR-001` nesse arquivo. Nono documento da mesma
família (CRI-MEL-001/SEQ-001/ACC-001/NOI-001/DRF-001/CMP-001/REL-001/
MID-001), continuação direta de `CRI-MID-001` (exportação MIDI) - não
uma correção de bug, e sim uma nova linha de pesquisa aberta por um
segundo diálogo do autor com o ChatGPT.

## 1. Por que este documento existe

Logo depois da correção do bug 3 do `CRI-MID-001` (andamento variável,
ver `PESQUISA_MIDI_E_PARTITURA.md`, seção 3.7), o autor colou uma nova
conversa com o ChatGPT e pediu: "vejo que temos uma nova linha de
pesquisa a explorar, analise, aprofunde, documente." A conversa parte
de uma pergunta implícita (o que existe além de MIDI pra representar
música de forma mais completa) e chega numa proposta concreta: uma
**representação musical intermediária própria do RASGO** - não MIDI,
não MusicXML, um formato novo com três camadas simultâneas (SCORE/
PERFORMANCE/INTERPRETATION), capaz de alimentar MIDI, MusicXML e o
próprio motor DSP do RASGO como três projeções diferentes da mesma
fonte.

Este documento é puramente de pesquisa - nenhuma implementação foi
pedida ou feita aqui (diferente do brief do MIDI, "implemente...",
este pedido foi explicitamente "analise, aprofunde, documente").

## 2. O que já existe no projeto RASGO sobre este assunto

### 2.1 `CRI-MID-001` já expõe o problema exato que a conversa descreve

`MidiCapture`/`writeMidiCaptureToFile()` (`Main.cpp`, ver
`PESQUISA_MIDI_E_PARTITURA.md`) já são, na prática, um caso mínimo
exatamente do problema que o ChatGPT descreve: MIDI como "linguagem de
performance/eventos", não uma descrição completa da música. O
`MidiCapture::NoteEvent` grava só quatro campos por nota
(`startSeconds`, `endSeconds`, `midiNote`, `velocity`) - qualquer coisa
além disso (articulação, gesto, caráter, microafinação) já se perde
antes mesmo de tocar o disco.

### 2.2 O funil de perda de dado já existe dentro do próprio ANTITOTEM - com números reais

Não é preciso especular sobre "o que MIDI não capta" - dá pra medir
exatamente, porque `MelodicInterpreter::tick()` (ver
`PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md` e
`PESQUISA_MELODIA_GENERATIVA.md`) já computa, POR SAMPLE, muito mais
informação do que qualquer camada seguinte preserva. Três níveis, cada
um mais pobre que o anterior:

**Nível 1 - estado interno completo (privado, computado todo sample,
nunca exposto fora da classe)**: `GestureType` (Legato/Staccato/
Marcato/Tenuto), `articulation` (dip de amplitude em saltos grandes),
`arrivalStrength`/`metricGlow` (chegada métrica vs. passagem),
`apexDegree` (distância do registro recente - ápice vs. bordadura),
`phraseEnergy`/`phraseState`/`phraseFinalNote` (posição na frase),
`characterEnergy`/`characterSoftness`/`characterBrightness` (vetor de
caráter, drift lento tipo "clima"), `livingPitchPhase` (microafinação
contínua, ~2 cents), `attackPunch`, `overshoot` (scoop de chegada),
`restHush`/`breath` (fôlego e silêncio de fronteira), e uma
`vibratoDepth` calculada localmente em `tick()` que nem chega a virar
membro (fórmula: `0.006 × (1+apexDegree×0.6) × (1+phraseEnergy×0.5) ×
(1-finalEase×0.6) × (1-arrivalStrength×0.25) × (0.7+characterEnergy×
0.6)` - cinco eixos expressivos diferentes multiplicados numa única
profundidade de vibrato).

**Nível 2 - `Voice` (struct pública, devolvida por `tick()`, já uma
redução grande)**: só `soundingPitch`, `gain`, `timbreBrightness`
(0..3, já uma MISTURA de vários eixos do nível 1 - brilho de registro +
trajetória + caráter + intensidade, somados numa única dimensão
escalar), `micro`/`phrase` (os enums de estado, não os números
contínuos que os geram), `justTriggered`. Tudo o que gerou esses
números - por que o gain subiu, por que o timbre clareou - já não é
recuperável só olhando o `Voice`.

**Nível 3 - `MidiCapture::NoteEvent` (o que de fato é exportado)**:
`startSeconds`/`endSeconds` (quantizado só pela granularidade de
polling, ver `PESQUISA_MIDI_E_PARTITURA.md`, seção 3.2),
`midiNote` = `pitch01ToMidiNote(soundingPitch)` (arredondado pro
semitom mais próximo - perde os ~2 cents de `livingPitchPhase`, o
overshoot de chegada, o vibrato inteiro), `velocity` =
`velocityFromLevel(gain)` (um inteiro 1-127 - perde toda a curva de
`GestureType`/articulação que produziu aquele gain), `track` (só
identifica PRINCIPAL/CLONE/EXCITAÇÃO, nada de caráter).

Ou seja: o próprio ANTITOTEM já é uma prova de conceito viva do
argumento central da conversa - "MIDI pode ser apenas um formato de
entrada/saída. A representação interna pode ser muito mais rica" - só
que hoje essa representação interna rica simplesmente não é
PERSISTIDA em lugar nenhum; ela existe por um sample e desaparece.

### 2.3 `TimeSignature`/compasso real (`CRI-CMP-001`) já separa intenção de execução

`PESQUISA_COMPASSO_E_METRICA_REAL.md` já fez, num escopo bem menor
(só compasso), exatamente o tipo de distinção que a conversa propõe em
escala maior: `TimeSignature{beatsPerMeasure, beatUnit}` é uma
descrição de INTENÇÃO estrutural (a fórmula de compasso), separada de
qualquer execução real (`clockRate`, `supplyClock`/ENERGIA,
`tupleDuration`/SUBDIVISÃO) - o mesmo tipo de separação "SCORE vs.
PERFORMANCE" que a seção 3.5/3.6 abaixo formaliza em maior escala.

## 3. A pesquisa do autor + ChatGPT (segunda fonte secundária, 20 ago. 2026)

Resumo consolidado da conversa colada pelo autor - não é conhecimento
próprio verificado ao vivo nesta rodada, é uma fonte secundária citada
com a mesma cautela já aplicada em `CRI-REL-001` seção 3 pro
"Interpretation Engine".

### 3.1 MIDI 2.0

Corrige várias limitações do MIDI 1.0 (resolução maior, informação por
nota, controladores mais ricos, perfis de dispositivo), mais adequado
pra performance expressiva (especialmente combinado com MPE/controle
por nota) - mas continua sendo, fundamentalmente, uma linguagem de
PERFORMANCE/EVENTOS, não uma descrição completa da música. Não muda a
conclusão central da conversa.

### 3.2 MusicXML e MEI - formatos de partitura estruturada

MusicXML representa muito mais que MIDI: notas, vozes, compassos,
articulações, acentos, dinâmicas, glissandi, ornamentos, fermatas,
técnicas instrumentais, swing, e distingue explicitamente informação
SONORA de informação NOTACIONAL (permite registrar diferença entre
duração escrita e realização interpretativa). Elementos citados na
conversa: `accent`, `strong-accent`, `staccato`, `tenuto`, `slur`,
`glissando`, `slide`, `trill`, `mordent`, `fermata`, `dynamics`,
`swing`. MEI (Music Encoding Initiative) vai ainda mais longe pro lado
musicológico/estrutural - um modelo rico pra codificar documentos
musicais inteiros, incluindo relações temporais e semânticas.
Relevante mas já coberto pela decisão existente de `CRI-MID-001`: não
reimplementar engraving dentro do ANTITOTEM, deixar pro software de
notação (MuseScore e afins) - MusicXML seria um SEGUNDO formato de
saída possível no mesmo espírito do MIDI atual, não uma substituição.

### 3.3 AMT (Automatic Music Transcription) - áudio para representação simbólica

A área de pesquisa que persegue `áudio → análise → notas/tempo/vozes/
instrumentos/estrutura`. Modelos neurais modernos não precisam pensar
primeiro em MIDI - podem transformar áudio direto numa sequência de
tokens musicais (`NOTE_ON`, `PITCH_67`, `TIME_SHIFT`, `VELOCITY`,
`NOTE_OFF`...) ou tokens mais sofisticados ainda. A representação
INTERNA de um sistema desses pode ser muito mais rica que MIDI, mesmo
quando MIDI é só uma das saídas possíveis dele.

### 3.4 Como Suno provavelmente se encaixa (especulação sinalizada como tal na própria conversa)

O Suno Studio expõe publicamente um fluxo `audio → stem separation →
select stem → Get MIDI → audio analysis → MIDI` - ou seja, existe
claramente um sistema de transcrição áudio-pra-MIDI no produto. Mas a
conversa é explícita que não há informação pública suficiente pra
afirmar que o MODELO GERADOR principal da Suno usa MIDI internamente -
é mais provável que use alguma representação latente/tokens de áudio,
o que explicaria por que ele preserva timbre, textura, voz,
articulação, espaço, ataques, ruído, microtiming e detalhes de
performance que MIDI não descreve bem. Citado aqui só como analogia de
por que uma representação rica compensa - não é uma referência técnica
verificada, e o ANTITOTEM não tem (nem está cogitando ter) infra de ML
pra fazer algo parecido.

### 3.5 A proposta central: "RASGO Musical Event" / "RASGO Score"

O núcleo da conversa, e o que de fato importa pro RASGO: em vez de
tentar "substituir MIDI por MusicXML", criar uma representação musical
intermediária PRÓPRIA. Cada evento musical carregaria campos como:

```text
pitch, onset, duration, velocity
accent, articulation, legato, microtiming
timbre (brightness, roughness, noise)
gesture (glide, vibrato, attack, release)
phrase_position, structural_weight, tension
interpretation (dolce, grazioso, agitato, maestoso, ...)
confidence, source
```

E o formato funcionaria nos dois sentidos:

```text
AUDIO → RASGO SCORE           (análise)
RASGO SCORE → INTERPRETER → AUDIO   (geração)
```

com três saídas possíveis a partir da mesma fonte:

```text
                    RASGO REPRESENTATION
                           │
            ┌──────────────┼──────────────┐
            ↓              ↓              ↓
          MIDI          MusicXML        RASGO DSP
            ↓              ↓              ↓
           DAW          PARTITURA      PERFORMANCE
```

### 3.6 Três camadas simultâneas: Score / Performance / Interpretation

A parte mais concreta da proposta - guardar, pro MESMO evento, três
leituras diferentes ao mesmo tempo em vez de escolher uma:

```text
SCORE          - intenção escrita (ex.: "semínima C5, mf, legato")
PERFORMANCE    - o que realmente aconteceu (onset +17ms, duração
                 491ms, pico de amplitude .61, overshoot de pitch +9
                 cents)
INTERPRETATION - leitura semântica do acontecimento (dolce .7,
                 cantabile .8, tension .2)
```

Com uma consequência prática pra notação: guardar `score onset` (ex.:
"3.000 beats") e `performed onset` (ex.: "3.071 beats") separadamente
significa não precisar destruir o rubato pra produzir uma partitura -
o MusicXML já reconhece conceitualmente essa diferença entre valor
notado e realização interpretativa. A conversa também nomeia o
problema mais difícil da transcrição real: não é só reconhecer notas,
é decidir "isto é uma colcheia com rubato ou uma semicolcheia
atrasada?", "estas duas notas pertencem à mesma voz?", "isso é um
glissando ou várias notas rápidas?" - a passagem de PERFORMANCE REAL
pra INTENÇÃO NOTACIONAL.

## 4. Onde isso já bate com o que o ANTITOTEM tem hoje

A comparação mais direta: a proposta de três camadas (SCORE/
PERFORMANCE/INTERPRETATION) já existe, embrionariamente e sem esse
nome, espalhada pelo código:

- **SCORE** (intenção estrutural): `TimeSignature`/`stepsPerBeat`
  (`CRI-CMP-001`), `voltages[]`/`levels[]` do sequenciador (o que CADA
  PASSO "deveria" tocar), `StepRule` (mutate/silence/rotate/ratchet/
  invert/retrograde - já uma forma de dizer "este passo teve uma
  intenção estrutural diferente do normal").
- **PERFORMANCE** (o que realmente soou): exatamente o que
  `MidiCapture::NoteEvent` já captura hoje (`startSeconds`/
  `endSeconds` reais, `midiNote`/`velocity` reais) - mas SÓ isso, sem
  o resto do nível 1 da seção 2.2.
- **INTERPRETATION** (leitura semântica): é literalmente o vetor de
  caráter já implementado (`characterEnergy`/`characterSoftness`/
  `characterBrightness`, `CRI-REL-001` seção 6.1) mais `GestureType` e
  `phraseState` - três eixos contínuos já rotulados na documentação
  existente com nomes tradicionais como região no espaço ("dolce ≈
  suavidade alta+energia baixa+brilho baixo"), o mesmo espírito exato
  do `interpretation: dolce=.62, espressivo=.31` do exemplo da
  conversa.

Ou seja: o RASGO já GERA as três camadas ao vivo, sample a sample -
só nunca as persiste juntas em lugar nenhum. A lacuna não é
conceitual, é de PERSISTÊNCIA/SERIALIZAÇÃO.

## 5. Proposta de adaptação para o RASGO (não implementada - só desenhada)

Sem inventar nomes novos onde o ANTITOTEM já tem os campos certos -
uma "RASGO Musical Event" pro ANTITOTEM especificamente poderia ser,
literalmente, uma extensão de `MidiCapture::NoteEvent` com os campos
que já existem hoje dentro de `MelodicInterpreter`/`SimpleSequencer`
mas nunca saem da classe:

```text
RasgoMusicalEvent
  # SCORE (já existe, CRI-CMP-001)
  measureIndex, beatIndex, timeSignature

  # PERFORMANCE (já existe, CRI-MID-001, sem mudança)
  startSeconds, endSeconds, midiNote, velocity, track

  # PERFORMANCE, estendido (existe no nível 1 do MelodicInterpreter,
  # hoje descartado antes de chegar no MidiCapture)
  gestureType, articulationDip, livingPitchOffsetCents, vibratoDepth,
  attackPunch, overshoot

  # INTERPRETATION (existe, CRI-REL-001 seção 6.1)
  characterEnergy, characterSoftness, characterBrightness,
  phraseState, phraseEnergy, arrivalStrength, apexDegree
```

Isso NÃO substituiria `MidiCapture` - continuaria existindo em
paralelo, como hoje `MidiCapture` existe ao lado de `WavRecorder` sem
competir com ele. O `.mid` continuaria sendo a saída compatível com
DAW/software de notação; um formato próprio (provável candidato mais
simples: um `.json`/`.csv` por tomada, gravado junto do `.mid`/`.wav`
já existentes, mesmo timestamp) seria a representação "mestra" mais
rica, da qual o MIDI já exportado hoje é uma PROJEÇÃO com perda -
exatamente o diagrama SCORE/PERFORMANCE/INTERPRETATION → MIDI da
conversa, só que a fonte já é o próprio motor de geração do ANTITOTEM,
não um resultado de análise de áudio.

Isso conecta diretamente com uma vantagem que `CRI-MID-001` já registrou
(seção 3.3 de `PESQUISA_MIDI_E_PARTITURA.md`): como o ANTITOTEM GERA a
música (em vez de precisar analisar áudio genérico), ele nunca
precisaria da parte mais difícil da conversa - decidir "isto é
glissando ou notas rápidas?", "que voz é essa?" - porque já SABE a
resposta no instante em que gera o evento. O trabalho de AMT
(transcrição de áudio real) não se aplicaria ao ANTITOTEM; se aplicaria
só a um cenário bem diferente (analisar uma gravação externa, de outro
instrumento), fora de escopo aqui.

## 6. Por que NÃO perseguir AMT (áudio → representação simbólica) agora

Registrado explicitamente como fora de escopo, não como "não pensado":

- Exigiria infraestrutura de ML (modelos de transcrição neural,
  treino/inferência) que o projeto não tem e que este documento não
  está propondo construir.
- O ANTITOTEM não PRECISA disso pro próprio uso - ele já conhece o
  evento no instante em que o gera (mesmo argumento do item 5 acima e
  de `CRI-MID-001`), então "áudio → RASGO Score" só faria sentido se o
  objetivo fosse analisar gravações de OUTRAS fontes (áudio externo,
  outro instrumento) - um projeto completamente diferente.
- A comparação com Suno/AMT serve só como contexto de por que uma
  representação rica importa em geral, não como um roteiro de
  implementação a seguir aqui.

**Atualização (20 ago. 2026)**: esse "projeto completamente diferente"
citado acima passou a existir de verdade - `RASGO/MARAVI/` (autor: "uso
o suno, extraio os stems dos instrumentos... gostaria de algo capaz de
extrair com mais fidelidade... qual o caminho?"). O conteúdo desta
seção e da seção 3 inteira foi trazido pra lá (`MARAVI/MARAVI_CONCEITO.md`,
seção 4, autor: "traga os detalhes para o maravi") como a pesquisa de
base que justifica o projeto novo, junto de uma verificação de licença/
proveniência do Spotify Basic Pitch (Apache 2.0, datasets de treino
públicos e citados) e uma primeira instalação funcional (venv Python,
backend ONNX). Registrado como `CRI-AMT-001` no funil de criação.

## 7. Perguntas em aberto para o autor

1. Vale a pena persistir essa representação estendida (seção 5) como
   um arquivo real por tomada (ao lado do `.wav`/`.mid` já
   existentes), ou isso só importa se/quando existir um consumidor
   real pra ela (ex.: uma ferramenta de replay/análise futura)? Sem um
   consumidor, seria dado morto - mesmo risco já registrado em outras
   pesquisas desta sessão (auditoria de "implementação sem aplicação
   direta").
2. Se decidido persistir, qual formato? JSON (legível, fácil de
   inspecionar, mas verboso) vs. binário simples (mais compacto, sem
   ferramenta de inspeção pronta) - nenhum dos dois teria custo de
   implementação alto, é mais uma questão de uso pretendido.
3. ~~MusicXML como uma SEGUNDA saída (ao lado do `.mid` já exportado)
   faz sentido agora, ou só quando/se a representação estendida da
   seção 5 já existir pra alimentá-lo com mais do que MIDI já carrega
   hoje?~~ - **decidido e feito** (20 ago. 2026, autor: "ótimo, vamos
   implementar o musicxml") - implementado SEM esperar a representação
   estendida da seção 5; ver seção 9 pro que isso significa na prática
   (o `.musicxml` resultante ainda não ganha articulação/interpretação
   sobre o `.mid`, só pitch/ritmo QUANTIZADO em figuras de nota reais e
   dinâmica - um ganho real mesmo sem a seção 5, mas menor do que seria
   com ela).
4. Existe algum caso de uso concreto imaginado pro "replay"/regeração
   a partir da representação (RASGO SCORE → INTERPRETER → AUDIO da
   conversa), ou o valor imediato é só de DOCUMENTAÇÃO/ANÁLISE de uma
   tomada já gravada?

## 8. Calibração pendente

Herdada de `PESQUISA_MIDI_E_PARTITURA.md` (mapeamento pitch→MIDI, BPM,
velocity - a exportação MusicXML reusa os MESMOS eventos capturados,
ver seção 9) - nada de novo introduzido pela quantização em si, que é
determinística (não estatística/aproximada, ver seção 9.2). Se/quando a
representação estendida da seção 5 for implementada, herdaria
calibração nova pros campos de interpretação (quais faixas numéricas de
`characterEnergy`/`arrivalStrength`/etc. merecem virar qual rótulo
tradicional tipo "dolce", se algum rótulo chegar a ser exposto) - ainda
não implementada, ver seção 9.4.

## 9. Implementação: exportação MusicXML (20 ago. 2026)

Autor: "ótimo, vamos implementar o musicxml" - resposta direta à
pergunta 3 da seção 7. Atrelada ao MESMO REC já existente, junto do
`.wav`/`.mid` (zero UI nova, mesmo precedente de `CRI-MID-001`):
`finishMidiRecording()` agora também chama `writeMusicXmlCaptureToFile`
depois de `writeMidiCaptureToFile`, escrevendo um terceiro arquivo
`ANTITOTEM_<timestamp>.musicxml` no mesmo diretório/timestamp.

### 9.1 Reaproveitamento total da captura já existente

Nenhum campo novo foi adicionado a `MidiCapture::NoteEvent` - a
exportação MusicXML lê exatamente os mesmos `NoteEvent`/`TempoEvent`
que `MidiCapture::finish()` já devolvia pro MIDI (a mesma "fonte
única, duas projeções" do diagrama da seção 3.5). `buildTempoSegments`/
`secondsToTicksPiecewise` (as duas funções que a correção do andamento
variável já tinha criado, ver `PESQUISA_MIDI_E_PARTITURA.md`, seção
3.7) foram extraídas pra escopo de arquivo em `Main.cpp` exatamente pra
serem compartilhadas pelas duas exportações, em vez de duplicar a
lógica de segmentos de tempo.

### 9.2 Quantização determinística pra uma grade de "steps"

A diferença real entre o `.mid` e o `.musicxml`: o MIDI grava ticks
brutos (contínuos, 960 por semínima); o MusicXML precisa de FIGURAS DE
NOTA reais (semínima/colcheia/etc.), que só existem numa grade
discreta. Cada `NoteEvent.startSeconds`/`endSeconds` é convertido pra
"steps" (mesma unidade de `stepsPerBeat`/`TimeSignature`, CRI-CMP-001)
via `secondsToStep()` - a MESMA conversão segundos→ticks já usada pro
MIDI, só dividida por `stepTicks = ticksPerQuarterNote/stepsPerBeat` e
arredondada. Isso é uma conversão DETERMINÍSTICA, não uma tentativa de
transcrição/adivinhação - o instrumento já sabe exatamente que grid
gerou cada nota (mesmo argumento da seção 5).

Overlaps introduzidos pelo arredondamento (uma nota quantizada
"encostando" ou passando da anterior) são resolvidos encostando o
início da nota seguinte no fim já quantizado da anterior
(`note.startStep = std::max(note.startStep, cursor)`), nunca deixando
duas notas quantizadas se sobreporem na partitura.

### 9.3 Decomposição em figuras de nota, ligaduras através da barra de compasso

Qualquer duração quantizada (em steps) é decomposta numa sequência de
figuras de nota válidas via um algoritmo guloso: a cada passo, acha a
maior potência de dois de steps que caiba no espaço restante DENTRO DO
COMPASSO ATUAL (nunca cruza a barra sem dividir - a mesma decomposição
já cuida de fechar/abrir `<measure>` automaticamente), com uma extensão
pontuada (nota com ponto) quando `base + base/2` também couber. Quando
uma nota real (não silêncio) precisa de mais de um pedaço - seja porque
sua duração não é uma potência de dois exata, seja porque cruza uma
barra de compasso - os pedaços saem ligados por `<tie>` (elemento
sonoro) e `<tied>` dentro de `<notations>` (elemento visual/notacional)
nos pontos certos (start/stop/start+stop). Verificado com um cenário
sintético cobrindo justamente esse caso (nota cruzando a barra do
compasso 1 pro 2) - ver seção 9.5.

Silêncios (lacunas entre notas, e o preenchimento até o fim do último
compasso de cada parte) passam pelo MESMO algoritmo de decomposição,
como itens sem pitch.

### 9.4 O que fica de fora desta primeira versão

- **Articulação real não sai no arquivo** - `GestureType`/
  `articulation`/`arrivalStrength` (nível 1 da seção 2.2) ainda não
  chegam até `MidiCapture::NoteEvent`, então não há `<staccato/>`/
  `<tenuto/>`/`<accent/>` no MusicXML, mesmo o formato suportando isso
  nativamente (seção 3.2). Extensão natural, não decidida - depende da
  seção 5 (`RasgoMusicalEvent`) ou de um campo mínimo adicionado só pra
  isso.
- **Dinâmica SIM, de `velocity`** - `<pp/>` a `<fff/>` por faixa de
  velocity (7 faixas, limiares arbitrários nunca calibrados por
  audição/leitura real da partitura resultante), emitida só quando
  muda de uma nota pra outra (evita marcar toda nota individualmente).
- **Clave fixa de sol pras três partes** - PRINCIPAL/CLONE/EXCITAÇÃO
  cobrem um registro amplo (~4.3 oitavas via `pitch01ToMidiNote`);
  notas graves vão aparecer com bastante linha suplementar. Simplifica-
  ção deliberada de uma primeira versão, não uma limitação técnica do
  formato (MusicXML suporta troca de clave por compasso).
  Só sustenidos, nunca bemóis (`midiNoteToXmlPitch` sempre escolhe a
  grafia sustenida) - mesma arbitrariedade já registrada pra
  `pitch01ToMidiNote`.
- **Só correto quando `TimeSignature.beatUnit == 4`** - `divisions` é
  definido como `stepsPerBeat` (assumindo que um tempo = uma semínima);
  como não há UI pra mudar `TimeSignature` (decisão deliberada de
  `CRI-MID-001`), isso nunca é um problema na prática hoje, mas ficaria
  incorreto se `beatUnit` mudasse por código sem essa função acompanhar.

### 9.5 Verificação (sem UI ao vivo)

Mesma restrição de sempre (nunca usar entrada sintética de teclado/
mouse contra o app real). Verificação feita por um harness C++
descartável (`/tmp/.../scratchpad/musicxml_check.cpp`, mesma lógica
traduzida pra tipos padrão sem JUCE) alimentado com um cenário
sintético cobrindo os casos de risco: nota cruzando barra de compasso
(gerou corretamente duas notas ligadas, uma pontuada de cada lado da
barra), múltiplas trocas de dinâmica em sequência, mudança real de
andamento no meio do take (dois `<sound tempo>` em compassos
diferentes), e uma trilha inteiramente silenciosa (EXCITAÇÃO sem
nenhum evento, virou compassos de silêncio puro - `<rest>` cobrindo o
compasso inteiro). Saída validada como XML bem formado via
`xml.dom.minidom` do Python e inspecionada nota a nota - todos os
valores batem com o cálculo manual esperado. O código real em
`Main.cpp` compila limpo (`-Wall -Wextra -Wpedantic -Werror`) e é a
mesma lógica, mas o build real com JUCE nunca foi exercitado ao vivo
(sem gravar uma tomada de verdade e abrir o `.musicxml` resultante num
software de notação) - pendente de teste real pelo autor.

### 9.6 Limitação confirmada por teste real: EXCITAÇÃO uniformiza no MusicXML, o MIDI não

Autor testou com uma tomada real (`ANTITOTEM_2026-08-20_13-04-32.mid`/
`.musicxml`) e relatou: "há uma diferença de como o midi e o musicxml
compreendem as durações das notas da música" - depois precisou o achado
sozinho: "o musicxml gerou uma partitura humanamente mais fácil de
ler, porém uniformizou as durações dos eventos melódicos, o que não
acontece no midi, porém o midi gera uma partitura de difícil leitura".
Diagnóstico confirmado analisando os dois arquivos (parser binário
manual pro `.mid`, `xml.dom.minidom` pro `.musicxml`, mesmo método já
usado pros bugs 1/2 do MIDI):

- **PRINCIPAL/CLONE quantizam quase sem perda** - a trilha PRINCIPAL
  tem só 2 notas reais nesta tomada (235 ticks e 19012 ticks); no
  `.musicxml` viraram 1 nota de 1 step (235/240≈0.98, arredonda pra 1 -
  erro de ~2%) e uma sequência ligada de 79 steps (12+3+16×4,
  exatamente 19012/240≈79.2) cruzando 5 compassos com `<tie>` correto.
  Isso funciona bem porque PRINCIPAL/CLONE são travados no grid do
  sequenciador por construção - cada passo real já dura um múltiplo
  quase exato de `stepTicks` (240 ticks = 1/4 de tempo).
- **EXCITAÇÃO não é travada em grid nenhum - e é aí que a uniformização
  acontece** - as primeiras 8 notas reais desta trilha duraram 72, 121,
  250, 112, 114, 115, 117, 118 ticks. Convertido pra steps (÷240):
  0.30, 0.50, 1.04, 0.47, 0.48, 0.48, 0.49, 0.49 - TODAS arredondam pra
  1 step (o piso mínimo de `secondsToStep`/decomposição, ver seção 9.2)
  exceto a de 250 ticks, que também arredonda pra 1. Resultado: as 147
  notas reais de EXCITAÇÃO desta tomada saíram TODAS como `16th` no
  `.musicxml` (confirmado contando os elementos `<type>` do arquivo
  real) - a variação real de duração (72 a 250 ticks, quase 3.5x de
  diferença) inteira absorvida pelo arredondamento, porque o "step"
  (1/4 de tempo, a mesma unidade que faz sentido pra PRINCIPAL/CLONE,
  que TÊM 4 passos por tempo por design) é uma grade grossa demais pra
  uma voz que dispara continuamente por conta própria (cooldown/
  activity de `MelodicInterpreter`, sem relação nenhuma com
  `stepsPerBeat`).

Ou seja: não é um bug de conversão (os dois arquivos concordam
matematicamente dentro da precisão de cada formato - MIDI mede em
ticks contínuos, 1/960 de semínima; MusicXML mede em steps quantizados,
1/4 de tempo) - é uma DIFERENÇA REAL DE RESOLUÇÃO entre a grade que
serve bem pra PRINCIPAL/CLONE e a taxa de disparo, muito mais fina e
irregular, de EXCITAÇÃO. Item 4 de `PESQUISA_MIDI_E_PARTITURA.md` já
tinha antecipado isso em teoria ("EXCITAÇÃO em notação discreta é uma
aproximação real... perdendo o glide/vibrato/microafinação real que
soa no áudio") - este teste real confirma e quantifica o efeito prático
específico sobre RITMO/DURAÇÃO (não só afinação): a aproximação
"achata" toda a variação de EXCITAÇÃO numa figura só, exatamente o
troca-troca legibilidade↔fidelidade que o autor nomeou.

Três caminhos possíveis foram levantados:

1. **Grade mais fina só pra EXCITAÇÃO** (`<divisions>` maior nessa
   parte especificamente, MusicXML permite `divisions` por parte -
   ex.: 32 ou 64 em vez de 4) - recuperaria parte da variação real, mas
   arriscaria piorar exatamente o problema que motivou o MusicXML
   (legibilidade): mais figuras diferentes por compasso (32avos/64avos
   avulsos), possivelmente MENOS legível que hoje, não mais.
2. **Manter como está, documentado** - aceitar que MusicXML é uma
   leitura "limpa"/estrutural e o MIDI continua sendo a fonte de
   verdade rítmica fina pra quem quiser essa nuance (abrir os dois
   lado a lado, cada um serve um propósito diferente - nem todo
   trabalho de partitura precisa da fidelidade rítmica completa de
   EXCITAÇÃO).
3. **Notação diferente pra EXCITAÇÃO** (não decidida em detalhe) - em
   vez de tentar notar cada disparo individual, agrupar visualmente
   por gesto/frase (ex.: uma nota mais longa com articulação/símbolo
   de trêmulo ou glissando indicando "atividade rápida aqui", sem
   pretender notar cada evento) - mais fiel ao ESPÍRITO da voz (um
   glide contínuo, nunca discreto de verdade) mas exigiria lógica nova,
   não só ajuste de grade.

**Decidido** (20 ago. 2026, `AskUserQuestion`): opção 2, "manter como
está" - MIDI continua sendo a fonte de verdade rítmica fina; MusicXML
fica como leitura limpa/estrutural, cada arquivo servindo um propósito
diferente. Nenhuma mudança de código necessária - o comportamento atual
de `writeMusicXmlCaptureToFile` já é o comportamento final pra
EXCITAÇÃO nesse aspecto. Opções 1/3 ficam registradas caso o autor
mude de ideia no futuro, mas não são um próximo passo ativo.

## 10. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, tabela da
seção 3 e entrada `CRI-SCR-001`.
