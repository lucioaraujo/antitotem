# Pesquisa: compasso musical real, separado do loop do sequenciador

Estado: `research` (funil de criação RASGO, ver
`RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, seção 3, estágio I2).
Registrado como `CRI-CMP-001` nesse arquivo. Sexto documento da mesma
família (CRI-MEL-001/SEQ-001/ACC-001/NOI-001/DRF-001), mas o primeiro
motivado por uma correção direta do autor, não por um brief colado.

## 1. Por que este documento existe

Discutindo o item 2 dos "próximos passos" grandes do sequenciador
(event budget "por compasso"), o autor perguntou: "o que o app entende
como compasso? um ciclo do sequencer?" A resposta, depois de ler o
código, foi não - o ANTITOTEM não tem NENHUM conceito chamado
"compasso" hoje; existem dois ciclos independentes (`loopEnd`/FIM DO
LOOP e `metricBeats`/MÉTRICA) que se aproximam da ideia sem serem ela.
O autor então corrigiu de frente: "para mim compasso não tem nada a
ver com loop de sequencer" - e apontou uma motivação concreta pra
fazer isso direito agora: "talvez precisemos desse conceito no futuro
que desejar trabalhar com midi e extração de partituras". Este
documento pesquisa o que "compasso" realmente significa em teoria
musical e propõe como adequar isso ao ANTITOTEM sem contaminar (nem
ser contaminado por) os mecanismos rítmicos que já existem.

## 2. O que já existe no projeto RASGO sobre este assunto

Verificado em `src/core/SimpleSequencer.h`/`.cpp` antes de propor
qualquer coisa nova:

- **`loopEnd`** (1-16, FIM DO LOOP): quantos dos 16 passos tocam antes
  do playhead voltar pro passo 0. Um grid de fatias IGUAIS - não
  carrega nenhuma hierarquia de peso entre elas. É o que hoje dispara
  a DERIVA (`active == 0 && lastDerivationStep != 0`) - mais próximo
  do conceito de **frase** que de compasso.
- **`metricBeats`** (1-16, MÉTRICA): o ciclo usado por `metricAccent`/
  `strongBeatNow` (`(currentStep + accentRotation) % metricBeats == 0`)
  pra decidir qual passo é "tempo forte". Isso já é uma hierarquia de
  acento real - mas sem separar "quantos passos por tempo" de "quantos
  tempos por compasso" (são a mesma variável), sem noção de
  simples/composto, e sem mapear pra nenhuma figura de nota.
- **SUBDIVISÃO** (RET/3:2/5:4/SWG/7:4/9:8/11:8/GLT): razões de
  subdivisão dentro de um passo (polirritmia), um eixo ortogonal a
  compasso - não confundir os dois.
- **CLOCK** (`clockRate`/`clock`, 0.1-20.0): a taxa interna do
  sequenciador. Não está explicitamente em BPM nem amarrada a nenhuma
  figura de nota - é um número de Hz cru.

Nenhum desses três mecanismos foi tocado por este documento - a
proposta abaixo é um conceito NOVO e separado, não uma correção dos
existentes.

## 3. O que "compasso" significa de verdade (teoria musical, conhecimento
próprio - não busca ao vivo nesta rodada)

- **Fórmula de compasso** (*time signature*): uma fração (4/4, 3/4,
  6/8, 5/4, 7/8...). O número de cima é quantos **tempos** (*beats*)
  cabem no compasso; o de baixo é qual figura de nota vale UM tempo (4
  = semínima, 8 = colcheia, 2 = mínima).
- **Hierarquia de peso entre os tempos** - isso É o compasso, não é
  decoração. Em 4/4: tempo 1 forte, tempo 3 meio-forte, tempos 2 e 4
  fracos. Em 3/4: tempo 1 forte, 2 e 3 fracos.
- **Simples × composto**: em compasso simples (2/4, 3/4, 4/4) cada
  tempo se divide em 2; em composto (6/8, 9/8, 12/8) cada tempo se
  divide em 3 (o "tempo" em si é uma semínima pontuada = 3 colcheias).
  Muda onde o acento cai e como os agrupamentos são notados/beamed.
- **Duração real**: só existe em segundos quando BPM e figura-por-
  tempo estão fixados - `duração = tempos_por_compasso × (60/BPM) ×
  (4/figura_do_tempo)`.
- **Anacruse**: um compasso incompleto antes do primeiro tempo forte
  "de verdade" (upbeat/pickup).
- **Fórmula de compasso pode mudar no meio da peça** (métrica mista,
  ex.: alternar 5/8 e 7/8) - comum em música experimental, não é uma
  exceção rara.

## 4. Por que isso não é `loopEnd` nem `metricBeats`

- `loopEnd` é resolução de grid (quantas fatias), sem hierarquia de
  peso - mais perto de "quantos compassos cabem numa frase" (se cada
  fatia fosse um compasso) do que de um compasso em si.
- `metricBeats` já tem hierarquia de acento, mas conflaciona "tempo"
  (beat) com "passo" (step) numa variável só, não distingue
  simples/composto, e não sabe que figura de nota cada passo
  representa.
- Nenhum dos dois expõe um evento de "fim de compasso" que um
  exportador de MIDI/partitura possa usar pra desenhar barras de
  compasso - só existe o teste por-sample `strongBeatNow` e o wrap do
  `loopEnd`.

## 5. Proposta de adequação (não implementada ainda)

Um conceito novo, que não substitui nada:

- **`TimeSignature { beatsPerMeasure, beatUnit }`** - a fórmula de
  compasso real (4/4, 3/4, 6/8...), com uma lista de presets comuns
  mais um par manual pra fórmulas incomuns.
- **`stepsPerBeat`** - quantos passos do sequenciador cabem em UM
  tempo real. Separado de `beatsPerMeasure` (diferente de
  `metricBeats`, que hoje mistura os dois papéis).
- **`stepsPerMeasure = stepsPerBeat × beatsPerMeasure`** - um contador
  NOVO, independente de `loopEnd`, disparando um evento real de "fim
  de compasso" quando cruzado.
- **Simples × composto**: muda o agrupamento interno de `stepsPerBeat`
  (2 vs. 3) pra quem for gerar a notação de verdade depois.
- **Serve de ponte pra MIDI/partitura**: fórmula de compasso como
  metaevento MIDI real, posição das barras de compasso, duração de
  cada passo expressa em figura de nota (não "passo 7 de 16", e sim
  "semicolcheia") - exatamente o vocabulário que um exportador MIDI ou
  gerador de MusicXML precisa.

## 6. Próximos passos sugeridos (não decididos - aguardando o autor)

1. ~~Confirmar se `TimeSignature`/`stepsPerBeat`/`stepsPerMeasure` deve
   viver no `SimpleSequencer`~~ - **decidido e feito** (20 ago. 2026,
   via `AskUserQuestion`: "Compasso real, por objeto") - vive no
   `SimpleSequencer` (`struct TimeSignature { beatsPerMeasure, beatUnit
   }`, `stepsPerBeat`, `measureStepIndex`), um por objeto (PRINCIPAL/
   CLONE), não uma camada de metadados separada - motivo real: cada
   objeto tem `clockRate` PRÓPRIO e independente hoje (sem BPM
   compartilhado), então um compasso musical de verdade não dava pra
   unificar entre os dois sem sincronizar os clocks primeiro; o autor
   escolheu aceitar essa limitação (PRINCIPAL e CLONE cruzam suas
   próprias fronteiras de compasso em momentos diferentes) em vez de
   construir sincronização de tempo agora.
2. ~~Decidir se o event budget... reseta por compasso real ou por
   frase~~ - **decidido e feito** (mesma pergunta/resposta acima) - por
   compasso real, por objeto - ver
   `PESQUISA_SEQUENCER_GENERATIVO.md`, seção 7.3/7.4 pro mecanismo
   completo (`hasEventBudget()`/`spendEventBudget()`,
   `SimpleSequencer.h`/`.cpp`).
3. ~~Nenhuma implementação de exportação MIDI/partitura em si ainda~~ -
   **decidido e feito** (20 ago. 2026, autor: "é possível extrair a
   partitura da melodia? ou a partitura rítmica, ou completa" ->
   "implemente... a criação de partituras a partir da possibilidade de
   converter o sinal em midi"). Exportação MIDI real implementada,
   atrelada ao botão REC já existente (sem UI nova): `MidiCapture`
   (`Main.cpp`) grava eventos de nota de PRINCIPAL/CLONE/EXCITAÇÃO em
   três trilhas separadas enquanto REC está ativo, e escreve um arquivo
   `.mid` real (`juce::MidiFile`) no mesmo instante em que a gravação
   de WAV termina - mesmo diretório, mesmo timestamp no nome, os dois
   arquivos da mesma tomada reconhecíveis como um par. `TimeSignature`/
   `stepsPerBeat` (item 1 acima) e o CLOCK (Hz) da UI viram o BPM e o
   metaevento de compasso real do MIDI - o primeiro consumidor de
   verdade dessa infraestrutura. Não é exportação de partitura em si
   (MusicXML/engraving) - o MIDI resultante é pensado pra ser importado
   em software de notação (MuseScore e afins, que já convertem MIDI em
   partitura de verdade automaticamente) - reinventar engraving dentro
   do ANTITOTEM ficaria fora de escopo. Ver
   `PESQUISA_MIDI_E_PARTITURA.md` pro racional completo desta decisão e
   da arquitetura de captura.

## 7. Calibração pendente

- `TimeSignature` padrão: 4/4 (`beatsPerMeasure=4`, `beatUnit=4`),
  `stepsPerBeat=4` - casa com os 16 passos do grid por padrão
  (`stepsPerMeasure=16`), uma escolha de "não muda nada perceptível pra
  quem nunca mexer nisso", nunca comparada por escuta contra outros
  defaults.
- Event budget: ceiling 1.0, custo de rotas/topologia 0.35, custo de
  rodada de Motion (AUTO/A/B/C) 0.25 cada - ver
  `PESQUISA_SEQUENCER_GENERATIVO.md`, seção 8, pro racional completo
  (nenhum desses números foi medido/confirmado por escuta ainda).
- Exportação MIDI (novo, 20 ago. 2026): mapeamento pitch01→nota MIDI
  ancorado em C2 (nota 36) mais `pitch01*51.6` semitons - ver
  `PESQUISA_MIDI_E_PARTITURA.md`, seção de calibração, pro racional
  completo (nenhum desses números foi confirmado por escuta/afinação
  real ainda).

## 8. Registro no funil de criação

Ver `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`, tabela da
seção 3 e entrada `CRI-CMP-001`.
