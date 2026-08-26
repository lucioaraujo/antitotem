# Pesquisa: exportação MIDI e extração de partitura

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`). Registrado como
`CRI-MID-001`. Continuação direta de `PESQUISA_COMPASSO_E_METRICA_REAL.md`
(`CRI-CMP-001`) - a motivação original de `TimeSignature`/`stepsPerBeat`
("talvez precisemos desse conceito no futuro que desejar trabalhar com
midi e extração de partituras") finalmente ganhou um consumidor real.

## 1. Por que este documento existe

Depois da auditoria de código sem aplicação direta (mesmo dia, ver
`TAREFAS.md`), o autor perguntou: "é possível extrair a partitura da
melodia?", depois "ou a partitura rítmica, ou completa", e por fim:
"implemente o timeSignature [e] a criação de partituras a partir da
possibilidade de converter o sinal em midi. É possível?". Este
documento registra a resposta (sim, com ressalvas reais) e a
arquitetura escolhida.

## 2. O que já existe no projeto RASGO sobre este assunto

- **`WavRecorder`** (`Main.cpp`, já existia) - o precedente de
  arquitetura mais direto: uma classe de captura com `start()`/`stop()`/
  `write()`, atrelada ao botão REC já existente, quantizada ao início/
  fim do loop de PRINCIPAL (`recordingArmed`/`recordingActive`/
  `recordingStopPending` em `timerCallback()`). `MidiCapture` (seção 4)
  espelha essa mesma máquina de estados deliberadamente, em vez de
  inventar um ciclo de vida novo.
- **JUCE `juce::MidiFile`/`juce::MidiMessageSequence`** - já parte de
  `juce_audio_basics` (dependência que o projeto já tem via
  `juce_audio_formats`/`juce_audio_utils`, usadas por `WavRecorder`) -
  suporte real de escrita de arquivo `.mid` sem precisar de nenhuma
  biblioteca nova.
- **`TimeSignature`/`stepsPerBeat`** (`CRI-CMP-001`, implementado mais
  cedo no mesmo dia) - existiam como infraestrutura sem nenhum
  consumidor real (confirmado por auditoria via grep, ver `TAREFAS.md`)
  até este documento - o motivo original pro qual foram criados.
- **Mapeamento CV→altura já estabelecido**: `excitationPitch`/
  `voltages[]` (0-1) já eram entendidos em todo o projeto como "1
  semitom = 1/51.6" (citado em `PESQUISA_MELODIA_GENERATIVA.md` desde o
  item #11/Narmour) - `CmosVoice::tickStereo`'s próprio mapeamento
  cv×4.3-oitavas. Reaproveitado aqui pra converter pitch01 em nota MIDI,
  não uma escolha nova.

## 3. Decisões de design

### 3.1 Por que reaproveitar REC em vez de UI nova

O painel de controles já é extremamente denso e minuciosamente ajustado
a pixel (ver o histórico de comentários em `layoutTransportColumn()`/
`layoutHeader...` em `Main.cpp`, dezenas de citações do autor sobre
espaçamento exato). Sem conseguir testar visualmente (regra permanente
desta sessão: nunca usar entrada sintética de teclado/mouse pra testar
o app), inserir um botão novo nesse layout seria um risco real de
regressão visual não detectável antes do autor testar. Solução:
`MidiCapture` vive inteiramente atrelada ao ciclo de vida do botão REC
que já existe - toda vez que o autor grava um WAV, ganha também um
`.mid` da mesma tomada, sem nenhuma superfície de UI nova. Isso também
resolveu a pergunta pendente "melodia, ritmo, ou completa": as três
trilhas (PRINCIPAL/CLONE/EXCITAÇÃO) sempre vão juntas no mesmo arquivo
- softwares de notação (MuseScore e afins) já mostram/escondem trilhas
individualmente, então a decisão de "só melodia" vs. "completa" fica
pro autor, na notação, não precisa ser feita aqui.

### 3.2 Granularidade de polling, não captura por sample

`SimpleSequencer::didStepSoundSincePoll()`/
`DualObjectEngine::didExcitationTriggerSincePoll()` são "consome e
limpa" (como um evento de disparo), lidos UMA VEZ por callback de
áudio (não por sample) de dentro de `getNextAudioBlock()`. Resolução
típica (~10ms num buffer de 512 amostras a 44.1kHz) é bem mais fina que
qualquer subdivisão rítmica real que o sequenciador produz - suficiente
pra notação, não pretende ser sample-accurate. Limitação aceita e
documentada: se `advanceStep()` disparasse mais de uma vez entre duas
leituras (clock configurado absurdamente rápido pro tamanho do buffer),
só o último disparo ficaria registrado - um cenário extremo, não o uso
normal do instrumento.

### 3.3 Por que MIDI e não partitura/MusicXML direto

Gerar notação de verdade (engraving: agrupamento de compasso,
hastes/feixes, escolha de clave, ligaduras, quantização rítmica
inteligente) é um problema especializado e grande por si só - motores
de notação profissionais (MuseScore, Sibelius, Finale) já resolvem isso
muito melhor do que valeria reconstruir aqui. A escolha certa é gerar
um MIDI limpo e correto (pitch/tempo/compasso reais) e deixar a
notação para um software que já faz isso bem - MuseScore (gratuito) já
importa MIDI e produz uma partitura legível automaticamente.
Reinventar engraving ficaria fora de escopo desta pesquisa.

### 3.4 Três trilhas monofônicas

PRINCIPAL, CLONE e EXCITAÇÃO são cada uma uma linha melódica só (nunca
duas notas simultâneas na mesma trilha) - `MidiCapture::noteOn()` fecha
qualquer nota pendente na mesma trilha antes de abrir a próxima,
correto por construção, não uma simplificação.

### 3.5 BPM e compasso reais, primeiro consumidor de TimeSignature

O metaevento de compasso do MIDI usa `TimeSignature::beatsPerMeasure`/
`beatUnit` diretamente. Como nada ainda chama `setTimeSignature()`/
`setStepsPerBeat()` (ver `PESQUISA_COMPASSO_E_METRICA_REAL.md`, item
1), o padrão de fábrica (4/4, `stepsPerBeat=4`) já produz um resultado
honesto - 16 passos = 1 compasso de 4/4, a leitura mais natural pra um
grid de 16 passos, sem exigir nenhum ajuste do autor. Ajustar isso pra
performances com outros compassos continua exigindo UI que ainda não
existe (ver seção 5).

**Bug real encontrado e corrigido no mesmo dia** (autor testou: "no
midi está super rápido"): a primeira versão calculava `bpm = (CLOCK em
Hz / stepsPerBeat) × 60`, tratando `clockRate` como se já fosse
literalmente "passos por segundo". Não é - `SimpleSequencer::
samplesPerStep()` (a fonte de verdade que já rege o áudio de verdade)
é:

```
samplesPerStep = sampleRate / (clockRate × supplyClock) × tupleDuration × groove
supplyClock = 0.46 + ENERGIA × 0.78        (0.46x a 1.24x!)
tupleDuration = depende da SUBDIVISÃO ativa (reto=1.0, tercina=2/3,
                swing alterna 1.0/0.5 por passo, glitch é um padrão
                fixo de 8 valores, etc.)
groove = tempo-preservante por desenho (par+ímpar sempre soma 2.0)
```

A versão original ignorava `supplyClock` e `tupleDuration` por
completo - o BPM calculado saía sempre mais rápido que o real,
sobretudo em ENERGIA baixa (`supplyClock` cai até 0.46x, quase 2.2x
mais rápido que o real). Corrigido com um novo
`SimpleSequencer::getAverageSamplesPerStep()` - mesma fórmula real de
`supplyClock`, mais uma MÉDIA do multiplicador de SUBDIVISÃO (constante
pros tuplets de verdade, valor medido do próprio padrão fixo pro
GLITCH, 0.75 pro SWING - `GROOVE` fica de fora de propósito, já que é
tempo-preservante por desenho). BPM agora vem de
`60 / (secondsPerStep × stepsPerBeat)`, a mesma cadeia de causalidade
que já toca o áudio, não uma reimplementação paralela arriscada.

**Sobre o "3/4" que o autor também relatou**: `TimeSignature` continua
SEMPRE 4/4 neste código (nada muda `beatsPerMeasure`/`beatUnit` do
padrão de fábrica) - o metaevento de compasso escrito no `.mid` nunca
poderia ser "3/4". A hipótese mais provável é que o software de notação
usado reinterpretou/adivinhou o compasso sozinho a partir do padrão
rítmico das notas - um efeito colateral plausível do BPM errado
distorcendo a grade de tempo que a nota real ocupava. Não confirmado
(o autor não testou de novo depois da correção do BPM até este
documento).

### 3.6 Como o instrumento entende figuras de nota (breve→semifusa) e SWING/GROOVE/GLITCH

Autor perguntou, ao vivo, dois esclarecimentos importantes que valem
registro permanente aqui (não só na resposta do chat):

**"Como o instrumento entende a divisão musical de escrita musical -
partitura - com as divisões: breve, semibreve, mínima, semínima,
colcheia, semicolcheia, fusa, semifusa e as subdivisões?"** - o
instrumento NÃO tem constantes nomeadas pra essas figuras - ele deriva
o equivalente pela razão `TimeSignature.beatUnit` ÷ `stepsPerBeat`:
`beatUnit` diz qual figura vale UM tempo (4=semínima, 8=colcheia,
2=mínima, 1=semibreve - a mesma convenção real da fórmula de compasso);
`stepsPerBeat` diz quantos passos do grid cabem nesse tempo. Cada passo
= 1/`stepsPerBeat` da figura `beatUnit`. Com o padrão de fábrica (4/4,
`stepsPerBeat=4`): cada passo = 1/4 de semínima = **semicolcheia** (a
leitura clássica "sequenciador de 16 passos = 16 semicolcheias por
compasso de 4/4"); 4 passos = 1 semínima (o tempo); 16 passos = 1
semibreve (o compasso inteiro). A escada real breve→semibreve→
mínima→semínima→colcheia→semicolcheia→fusa→semifusa é sempre binária
(cada uma exatamente metade da anterior) - é exatamente isso que
`beatUnit`/`stepsPerBeat` capturam, mesmo sem os nomes em português no
código. Esse mesmo par é o que faz o BPM da seção 3.5 funcionar
corretamente uma vez calculado certo.

**"Como ele entende o swing, o groove e o glitch?"** - os três
modificam a duração REAL de cada passo (via `samplesPerStep()`), mas
são conceitualmente diferentes:

- **SWING** (uma opção de SUBDIVISÃO/`ClockFeel`, não um controle
  próprio) - alterna cada passo entre duração normal (pares) e metade
  (ímpares) - o shuffle clássico. NÃO é tempo-preservante: média
  0.75x, então SWING sozinho acelera o andamento médio geral - por
  isso `getAverageSamplesPerStep()` (seção 3.5) precisa saber disso pra
  calcular um BPM correto.
- **GROOVE** (um slider próprio, 0-1, somado por cima de QUALQUER
  SUBDIVISÃO ativa) - mesma alternância par/ímpar, mas desenhada de
  propósito pra ser tempo-preservante (`grooveEven + grooveOdd` soma
  sempre exatamente 2.0) - colore o balanço sem arrastar o andamento
  médio, por isso fica de fora do cálculo de BPM.
- **GLITCH** (outra opção de SUBDIVISÃO/`ClockFeel`) - um padrão FIXO e
  repetível de 8 valores (`1.0, 0.5, 1.0, 1.5, 0.75, 1.25, 0.5, 1.0`,
  indexado por `passo % 8`) - não é aleatório, sempre o mesmo ciclo -
  uma instabilidade rítmica real mas previsível, média ≈0.94x.

### 3.7 Andamento variável dentro de uma mesma tomada

Depois da correção do bug 1 (seção 3.5), o autor perguntou: "se houver
variação de clock durante a gravação o registro midi vai entender
como?" A resposta, antes desta seção, era não - `finishMidiRecording()`
calculava UM `bpm` só, no FIM da tomada, e aplicava esse valor único a
TODA a conversão segundos→ticks (uma única `secondsToTicks` lambda, um
único `tempoMetaEvent` no tick 0). Os timestamps em segundos gravados
por `MidiCapture` sempre foram certos (`elapsedSeconds` acumula tempo
real, sample a sample); só a conversão pra ticks (a unidade que o MIDI
realmente grava) que era uniforme - então qualquer mudança ao vivo de
CLOCK/ENERGIA/SUBDIVISÃO durante a tomada produzia um `.mid` com
proporções erradas do início ao fim, não só no trecho que mudou (o erro
"vaza" pra trás e pra frente porque uma única razão bpm/60 escala a
tomada inteira). Autor confirmou o pedido de correção: "isso, fazer a
mudança de andamento" (20 ago. 2026).

Correção implementada, sem UI nova:

- `MidiCapture::TempoEvent { atSeconds, bpm }` - um ponto onde o BPM
  efetivo mudou de verdade.
- `MidiCapture::recordTempo(bpm)` - chamado a cada callback de áudio
  (`getNextAudioBlock()`, junto de `midiCapture.advance()`), só grava
  um novo ponto quando o BPM se moveu ≥0.5 do último ponto gravado
  (limiar arbitrário, evita encher a trilha de tempo com ruído de
  ponto-flutuante de callback em callback quando o BPM está
  efetivamente parado).
- `MainComponent::computeCurrentBpm()` - a mesma conta que já existia
  inline em `finishMidiRecording()` (CLOCK×ENERGIA×SUBDIVISÃO via
  `sequencer.getAverageSamplesPerStep()`), fatorada pra fora pra poder
  ser chamada tanto por callback (`recordTempo`) quanto uma última vez
  no fim da tomada (`finishMidiRecording()`, ANTES de `midiCapture.
  stop()` - `recordTempo` só aceita enquanto `active`).
- `MidiCapture::finish()` agora devolve `Capture { notes, tempos }` (um
  struct simples) em vez de só `vector<NoteEvent>`.
- `writeMidiCaptureToFile()` reescrita: constrói uma lista de
  `TempoSegment { startSeconds, startTicks, bpm }` por soma cumulativa
  de ticks entre pontos consecutivos (mesma lógica que uma DAW usa pra
  mover marcadores de tempo), escreve UM `tempoMetaEvent` por segmento
  na posição de tick correta (em vez de um só no tick 0), e troca a
  antiga `secondsToTicks` de razão única por uma versão que primeiro
  acha o segmento certo (busca linear - poucos segmentos esperados por
  tomada, não vale binária) e só then aplica a razão bpm/60 daquele
  trecho. Múltiplos `set_tempo` meta-events em posições de tick
  diferentes é o vocabulário nativo do SMF pra automação de tempo - o
  mesmo que DAWs (Logic/Ableton/Reaper) escrevem ao exportar automação
  e que software de notação (MuseScore e afins) já sabe importar e
  reinterpretar como mudanças de andamento/rall./accel.
- CLONE continua fora do cálculo de BPM (clock próprio e independente
  - mesma limitação já registrada na seção 3.5/`PESQUISA_COMPASSO_E_
  METRICA_REAL.md` seção 6 item 1, não alterada por esta correção).

Não testado ao vivo ainda - pendente de uma tomada real com mudança de
CLOCK/ENERGIA/SUBDIVISÃO no meio pra confirmar por escuta/inspeção do
`.mid` resultante.

## 4. O que foi implementado

- **`SimpleSequencer.h`/`.cpp`**: `didStepSoundSincePoll()`,
  `getLastSoundingPitch01()`, `getLastSoundingLevel()` - gravados no
  MESMO ponto que já decide "este passo soou de verdade" (dentro de
  `advanceStep()`, junto de `stepHeat`/`envelope.trigger()`).
- **`DualObjectEngine.h`/`.cpp`**: `didExcitationTriggerSincePoll()`,
  `getExcitationLastTriggerPitch01()` - agregados a partir de
  `MelodicInterpreter::Voice::justTriggered` (item 6.3 de
  `PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md`, que já existia
  desde mais cedo no mesmo dia) dentro de `render()`, já que esse sinal
  é computado por SAMPLE e `render()` só devolve áudio pro chamador -
  aqui é o único lugar onde vira um evento de granularidade de
  callback.
- **`Main.cpp`**: `MidiCapture` (captura thread-safe, mesma forma de
  `WavRecorder`), `pitch01ToMidiNote()`/`velocityFromLevel()`
  (conversão), `writeMidiCaptureToFile()` (escreve o `.mid` real via
  `juce::MidiFile`), `finishMidiRecording()` (fecha+exporta, chamado em
  todo ponto que já finaliza/cancela REC - seguro mesmo com zero
  eventos capturados). Arquivo nomeado `ANTITOTEM_<timestamp>.mid`, mesmo
  diretório e timestamp que o `.wav` irmão (`ANTITOTEM_RECORDINGS_DIR`
  ou `~/Music/Antitotem Objeto Sonoro`).
- **`SimpleSequencer::getAverageSamplesPerStep()`** (novo, correção de
  bug do mesmo dia - ver seção 3.5) - BPM real de exportação, usando a
  mesma cadeia CLOCK×ENERGIA×SUBDIVISÃO que já toca o áudio, não uma
  reimplementação ingênua.
- **`MidiCapture::TempoEvent`/`recordTempo()`/`MainComponent::
  computeCurrentBpm()`** (novo, mesmo dia - ver seção 3.7) - andamento
  variável dentro de uma mesma tomada: um `set_tempo` meta-event MIDI
  por trecho onde o BPM efetivo mudou de verdade, em vez de um único
  valor aplicado à tomada inteira.

## 5. Limitações conhecidas e próximos passos (não decididos)

1. **Sem UI pra ajustar `TimeSignature`/`stepsPerBeat`** - decisão
   deliberada desta sessão (ver seção 3.1) por risco de layout sem
   poder testar visualmente. Se o autor quiser um compasso diferente de
   4/4 no MIDI exportado, hoje precisaria ser feito por código. Opção
   mais segura sugerida: um controle compacto e novo (não inserido no
   layout já denso), talvez um atalho de teclado cíclico (mesmo
   precedente de Shift+C pro toggle CLONE/PRINCIPAL) em vez de um botão
   visível.
2. **Sem calibração de afinação real** - o mapeamento pitch01→MIDI
   (ancorado em C2, `+pitch01×51.6` semitons) nunca foi comparado contra
   uma afinação de concerto real; o resultado pode soar "certo" na
   melodia relativa mas em nomes de nota arbitrários.
3. **Granularidade de bloco, não sample-accurate** (ver seção 3.2) -
   aceitável pra notação, não pretende virar uma ferramenta de
   transcrição rítmica de precisão profissional.
4. **EXCITAÇÃO em notação discreta é uma aproximação real** - a voz
   desliza continuamente (glide sempre ligado, nunca re-dispara de
   verdade); o MIDI exportado usa o PITCH-ALVO de cada disparo como se
   fosse uma nota discreta, perdendo o glide/vibrato/microafinação real
   que soa no áudio. Aceito como a aproximação correta pra notação (uma
   partitura real também não desenha glissandi contínuos nota a nota),
   mas vale deixar registrado que o MIDI não é uma transcrição
   sample-perfeita do que EXCITAÇÃO realmente faz.
5. ~~Não testado ao vivo ainda~~ - **testado, achou dois bugs reais,
   ambos corrigidos no mesmo dia** (20 ago. 2026). Bug 1, BPM errado
   ("no midi está super rápido") - ver seção 3.5, confirmado corrigido
   pelo autor: "melhorou". Bug 2, mute do object mixer ignorado
   ("quando há algum instrumento mutado no object mixer ele produz
   notas no midi") - autor gravou uma tomada com PRINCIPAL e CLONE
   mutados (só EXCITAÇÃO audível); inspecionando o `.mid` exportado
   (script Python ad-hoc, parse manual do formato binário) confirmou
   251 notas de PRINCIPAL + 124 de CLONE mesmo mutados - causa: o mute
   do mixer só zera a SAÍDA de áudio final
   (`DualObjectEngine::render()`), `SimpleSequencer`/
   `MelodicInterpreter` continuam disparando por dentro por design
   (pra retomar exatamente de onde estavam ao desmutar - mesmo
   precedente já documentado pro `running`/STOP). Os ganchos de captura
   MIDI viviam exatamente aí dentro, sem saber do mute do mixer.
   Corrigido em `Main.cpp`: `didStepSoundSincePoll()`/
   `didExcitationTriggerSincePoll()` continuam sendo chamados SEMPRE
   (drena o flag mesmo mutado, evita vazar um disparo fantasma quando
   desmutar depois), mas `midiCapture.noteOn()` só é chamado quando o
   mute correspondente (`principalMute`/`cloneMute`/`excitationMute`)
   NÃO está ativo. Reteste deste segundo bug ainda pendente.
6. **`getAverageSamplesPerStep()` é uma MÉDIA, não uma conta exata por
   nota** (limitação nova, introduzida pela própria correção do bug 1
   acima) - pra SWING/GLITCH, o multiplicador de duração varia por
   passo de verdade (não é constante como nos tuplets reais); o BPM
   exportado usa a MÉDIA desse padrão, então o `.mid` de uma performance
   em SWING/GLITCH vai ter um BPM globalmente correto mas passos
   individuais ligeiramente mais rápidos/lentos que o valor médio
   informado - aceitável pra notação (a própria notação real de swing
   já é uma aproximação sobre uma grade binária), mas vale saber que
   não é exato nota a nota nesses dois feels específicos.
7. **Mute de `SimpleSequencer::running`/STOP não auditado** - o mesmo
   raciocínio do bug 2 pode se aplicar ao transporte parado: o disparo
   de EXCITAÇÃO (`MelodicInterpreter::tick()`) não é condicionado a
   `stimulus.running`, só o ganho final é - em teoria, um disparo
   fantasma poderia entrar na captura MIDI com o transporte parado, se
   `activity`/`cooldown` ainda permitissem um disparo. Não confirmado,
   registrado por precaução depois de encontrar o bug 2 por um caminho
   parecido - vale investigar se o autor notar notas MIDI durante
   trechos com STOP pressionado.
8. ~~Andamento fixo, calculado uma vez só no fim da tomada~~ -
   **decidido e feito** (20 ago. 2026, autor: "se houver variação de
   clock durante a gravação o registro midi vai entender como?" ->
   "isso, fazer a mudança de andamento") - ver seção 3.7 pro design e
   implementação completos (`MidiCapture::TempoEvent`/`recordTempo()`,
   `MainComponent::computeCurrentBpm()`, `writeMidiCaptureToFile()`
   reescrita com segmentos de tempo cumulativos). Não testado ao vivo
   ainda.

## 6. Calibração pendente

- Mapeamento pitch01→MIDI: âncora C2 (nota 36), alcance 51.6 semitons
  (4.3 oitavas) - nunca confirmado por escuta/afinação real.
- Velocity: `level×127` linear, piso 1 - nunca comparado contra a
  percepção real de dinâmica.
- BPM derivado de `getAverageSamplesPerStep()` (corrigido 20 ago. 2026,
  ver seção 3.5) - a fórmula agora é a certa, mas os números que ela
  usa (`supplyClock` 0.46-1.24x, médias de SUBDIVISÃO) nunca foram
  comparados por escuta contra o andamento real percebido ouvindo o
  instrumento tocar - herdados de `samplesPerStep()`, nunca calibrados
  lá também.
- Resolução de ticks: 960 por semínima (`ticksPerQuarterNote`) - padrão
  comum da indústria, não uma escolha específica testada aqui.

## 7. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, entrada
`CRI-MID-001`.
