#pragma once

#include "core/CmosVoice.h"
#include "core/CmosVcf.h"
#include "core/ContourEnvelope.h"
#include "core/LinkedOverloadProtector.h"
#include "core/ModulationSources.h"
#include "core/MaterialEffects.h"
#include "core/MaterialFilter.h"
#include "core/MutableMixer.h"
#include "core/NoiseFields.h"
#include "core/OutputStage.h"
#include "core/SignalLeveler.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace antitotem
{
// A behavioural model of the 40106 -> 4040 -> 4051 sequencer on page 9 of eme.pdf.
class SimpleSequencer
{
public:
    static constexpr std::size_t historicalScannerSteps = 8;
    static constexpr std::size_t stepCount = historicalScannerSteps * 2;
    // 8, not 4 (20 ago. 2026, author: "implementamos mais uma fileira de
    // pulse (oito no total): com tercina, quintina, sextina, septina,
    // nonina, 11ina" - straight/glitch stay the anchors, the other 6 are
    // all tuplet feels; see samplesPerStep()'s own comment for the ratios.
    // swing, not sextuplet (20 ago. 2026, author: "no lugar da sextina
    // insira a ideia de swing" - a plain sextuplet ratio (6-in-4) is
    // mathematically identical to triplet's own 2/3, so the enum now
    // names what it actually is: a long-short shuffle, not a disguised
    // tuplet - see samplesPerStep()'s own comment for the pattern.
    enum class ClockFeel : unsigned char { straight, triplet, quintuplet, swing, septuplet, nonuplet, undecuplet, glitch };
    // The route of the 4040/4051 address, rather than another audio randomizer.
    // memoryAddress is repeatable from the internal state and avoids immediate
    // repeats, so it can become material for DERIVA and recorded form.
    enum class ScannerDirection : unsigned char { forward, reverse, pendulum, memoryAddress };
    // Meta-sequenciador / sequenciador de regras (docs/
    // PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1 - "o conteúdo do
    // sequenciador pode ser COMANDOS sobre outro sequenciador... cada
    // step escolhe uma regra de produção em vez de conter música
    // diretamente"), 20 ago. 2026, autor: "meta-sequenciador/regras".
    // Cada passo carrega uma regra além de seu próprio CV/nível/send -
    // `normal` é o comportamento de sempre (inalterado); os outros seis
    // fazem o passo AGIR sobre a sequência quando alcançado, não só
    // soar: `mutate` desloca o próprio CV/nível/send um pouco (o mesmo
    // "motor drift" já usado em DERIVA, self-aplicado); `silence`
    // silencia esta passagem SEM alterar `muted[]` permanentemente (um
    // descanso pontual, não uma edição); `rotate` desloca em UM passo o
    // conteúdo de todos os passos ATIVOS (`[0, loopEnd)`) - inclusive as
    // próprias regras, então uma regra `rotate` migra pra frente com o
    // tempo, um efeito genuinamente auto-referente. Poucas regras de
    // propósito (não os "step1=ROTATE, step2=MUTATE..." infinitos do
    // brief) - "sem muitas padronizações" continua valendo.
    //
    // Três regras novas (20 ago. 2026, autor: "pode prosseguir com
    // microevent, pattern/motif" - as duas escalas que tinham ficado de
    // fora da arquitetura de 5 escalas por decisão de escopo, ver
    // PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.4):
    // - `ratchet` (Microevent, a escala mais fina - "repetições dentro
    //   de um passo com envelope próprio") - o passo dispara MÚLTIPLOS
    //   sub-hits dentro da SUA PRÓPRIA duração (2-4, derivado do
    //   próprio `levels[]` do passo - sem um array novo), decrescendo
    //   em ganho a cada sub-hit (o "envelope de ratchet" clássico) -
    //   ver `ratchetSubIndex`/`ratchetGain`/`ratchetActiveThisStep` e o
    //   sub-clock em `renderSample()`.
    // - `invert`/`retrograde` (Pattern/Motif, a escala de agrupamento -
    //   "motivos como objetos [...] transformações reais de teoria
    //   motívica: rotação [já feita, `rotate`], inversão, retrógrado,
    //   aumentação, diminuição, fragmentação") - duas das seis
    //   transformações reais do brief, escolhidas por serem as mais
    //   baratas depois de rotação. `invert` espelha só `voltages[]`
    //   (inversão melódica de verdade é sobre ALTURA, não sobre
    //   acento/gate/send) em torno de 0.5 dentro de `[0, loopEnd)`.
    //   `retrograde` inverte a ORDEM de todos os arrays ativos (mesmo
    //   conjunto que `rotate` já usa) via `std::reverse` - tocar o
    //   motivo de trás pra frente, de verdade.
    enum class StepRule : unsigned char { normal, mutate, silence, rotate, ratchet, invert, retrograde };

    void prepare(double newSampleRate) noexcept;
    void setClockRate(double hertz) noexcept;
    void setClockFeel(ClockFeel feel) noexcept;
    // GROOVE - a small long-short alternation layered on top of whichever
    // SUBDIVISÃO feel is active (straight, any tuplet, SWING, or GLITCH),
    // not exclusive to the SWG button (20 ago. 2026, author: "deixa o
    // swing somente enquanto botão, e utilise esse slide atual do swing
    // para o groove" - SWG went back to being a fixed button, this became
    // the general-purpose control instead). 0 = no groove, 1 = strongest.
    void setGrooveAmount(float normalizedAmount) noexcept;
    void setScannerDirection(ScannerDirection direction) noexcept;
    [[nodiscard]] ScannerDirection getScannerDirection() const noexcept { return scannerDirection; }
    // Compasso musical real (docs/PESQUISA_COMPASSO_E_METRICA_REAL.md,
    // `CRI-CMP-001`), 20 ago. 2026, autor: "para mim compasso não tem
    // nada a ver com loop de sequencer" - conceito NOVO e independente
    // de `loopEnd` (FIM DO LOOP, grid de fatias iguais, sem hierarquia)
    // e de `metricBeats` (MÉTRICA, já tem hierarquia de acento mas
    // conflacia "tempo" com "passo" numa variável só). `beatsPerMeasure`/
    // `beatUnit` são a fórmula de compasso real (4/4, 3/4, 6/8...);
    // `stepsPerBeat` é quantos passos do sequenciador cabem em UM
    // tempo - juntos formam `stepsPerMeasure`, um contador PRÓPRIO que
    // avança em `advanceStep()` e dispara um evento real de "fim de
    // compasso" quando cruza, sem depender de `loopEnd` ter terminado.
    // Por objeto (autor, via AskUserQuestion: "Compasso real, por
    // objeto" - PRINCIPAL e CLONE têm `clockRate` PRÓPRIOS e
    // independentes hoje, sem BPM compartilhado, então um compasso
    // musical de verdade não dá pra unificar entre os dois sem
    // sincronizar os clocks primeiro; cada objeto cruza sua própria
    // fronteira de compasso no seu próprio tempo).
    struct TimeSignature { unsigned int beatsPerMeasure = 4; unsigned int beatUnit = 4; };
    void setTimeSignature(unsigned int beatsPerMeasure, unsigned int beatUnit) noexcept;
    void setStepsPerBeat(unsigned int steps) noexcept;
    [[nodiscard]] TimeSignature getTimeSignature() const noexcept { return timeSignature; }
    [[nodiscard]] unsigned int getStepsPerBeat() const noexcept { return stepsPerBeat; }
    [[nodiscard]] unsigned int getStepsPerMeasure() const noexcept { return stepsPerBeat * timeSignature.beatsPerMeasure; }
    // Posição dentro do compasso atual - o chamador (Main.cpp) detecta a
    // fronteira de compasso comparando leituras sucessivas, mesmo
    // idioma já usado pra detectar o wrap de `loopEnd` pra disparar
    // DERIVA (estado de "último valor visto" mora em quem chama, não
    // aqui - ver o comentário de `measureStepIndex` no .cpp).
    [[nodiscard]] unsigned int getMeasureStepIndex() const noexcept { return measureStepIndex; }
    // Event budget (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1 -
    // "um teto compartilhado de eventos... disputado entre
    // sequenciadores - evita que tudo fique complexo ao mesmo tempo"),
    // 20 ago. 2026, autor: "event budget". Por objeto (mesma decisão da
    // fronteira de compasso acima) - cada PRINCIPAL/CLONE tem seu
    // próprio orçamento, resetado no SEU próprio fim de compasso, não
    // um pool único compartilhado entre os dois (o autor sabia dessa
    // troca ao escolher "compasso real, por objeto" via
    // AskUserQuestion). `spendEventBudget()` é chamado por quem gera
    // complexidade (hoje, DERIVA em Main.cpp) antes de aplicar uma
    // mutação; `hasEventBudget()` decide se ainda há espaço nesta
    // medida. Ceiling normalizado (1.0), não uma contagem de eventos
    // bruta - cada "evento" pode custar um valor diferente (uma
    // mutação pequena custa pouco, uma reconfiguração de topologia
    // custa mais), o mesmo espírito de "complexidade ≠ densidade" que
    // o brief original já registrava.
    [[nodiscard]] bool hasEventBudget(float cost = 0.15f) const noexcept { return eventBudgetSpent + cost <= 1.0f; }
    void spendEventBudget(float cost = 0.15f) noexcept { eventBudgetSpent = std::min(1.0f, eventBudgetSpent + cost); }
    [[nodiscard]] float getEventBudgetRemaining() const noexcept { return std::max(0.0f, 1.0f - eventBudgetSpent); }
    // Meta-sequenciador (ver o comentário do enum `StepRule` acima) - 0
    // = desligado por completo (autor: mesmo vocabulário "0 = off
    // entirely" já usado em excitationAmount/grooveAmount/etc. nesta
    // sessão), já que `mutate`/`silence`/`rotate` mudam audivelmente o
    // que soa - não deveria alterar patches existentes por padrão. Sobe
    // duas coisas juntas, não independentes: a chance de UM passo
    // sortear uma regra nova a cada `advanceStep()` (a maioria continua
    // `normal` mesmo em amount alto - ver o roll em `advanceStep()`) e,
    // por construção, a frequência de regras não-`normal` de fato
    // executando quando alcançadas.
    void setMetaSequencerAmount(float normalizedAmount) noexcept;
    [[nodiscard]] float getMetaSequencerAmount() const noexcept { return metaSequencerAmount; }
    void setStepRule(std::size_t index, StepRule rule) noexcept;
    [[nodiscard]] StepRule getStepRule(std::size_t index) const noexcept { return index < stepCount ? stepRules[index] : StepRule::normal; }
    void setMetric(unsigned int beats, unsigned int unit) noexcept;
    void setMasterGain(float normalizedGain) noexcept;
    void setFeedbackAmount(float normalizedAmount) noexcept;
    void setFeedbackSignal(CmosVoice::FeedbackSignal signal) noexcept;
    void setFeedbackConnections(unsigned char connections) noexcept;
    void setExternalFeedbackAmount(float normalizedAmount) noexcept;
    void setOscillatorLevel(std::size_t oscillator, float normalizedLevel) noexcept;
    void setOscillatorShape(std::size_t oscillator, float normalizedShape) noexcept;
    void setOscillatorRatio(std::size_t oscillator, float ratio) noexcept;
    void setOscillatorPan(std::size_t oscillator, float normalizedPan) noexcept;
    void setOscillatorProximity(std::size_t oscillator, float normalizedProximity) noexcept;
    void setOscillatorOrbit(std::size_t oscillator, float normalizedOrbit) noexcept;
    void setEnergy(float normalizedEnergy) noexcept;
    void setOscillatorCore(CmosVoice::OscillatorCore core) noexcept;
    void setFilterCutoff(float normalizedCutoff) noexcept;
    void setFilterResonance(float normalizedResonance) noexcept;
    void setFilterCvDepth(float normalizedDepth) noexcept;
    // Bitmask of CmosVcf::Mode bits - any combination, see CmosVcf.h's own
    // comment on why more than one at once is a real, cheap filter blend.
    void setFilterModeMask(unsigned char mask) noexcept;
    // MaterialFilter (src/core/MaterialFilter.h) sits in series right after
    // CmosVcf. No separate on/off - MaterialFilter's own internal MIX
    // crossfade already makes 0 exactly today's CmosVcf-only signal (not
    // an approximation - the same code path, verified in
    // tests/SimpleSequencerTests.cpp), so a bypass bool would only
    // duplicate what the slider's own 0 end already does (author, live,
    // 17 ago. 2026: "não é só ligar/desligar, é ligar/escalonar/
    // desligar" - matches how REVERB/PHASER/FLANGER's own MIX already
    // works, no separate switch there either).
    void setMaterialFilterCutoff(float normalizedCutoff) noexcept;
    void setMaterialFilterResonance(float normalizedResonance) noexcept;
    void setMaterialFilterDrive(float normalizedDrive) noexcept;
    void setMaterialFilterAsymmetry(float normalizedAsymmetry) noexcept;
    void setMaterialFilterMix(float normalizedMix) noexcept;
    void setEnvelopeAttack(float normalizedTime) noexcept;
    void setEnvelopeDecay(float normalizedTime) noexcept;
    void setEnvelopeSustain(float normalizedLevel) noexcept;
    void setEnvelopeRelease(float normalizedTime) noexcept;
    void setLfoRate(float hertz) noexcept;
    void setLfoShape(LfoSource::Shape shape) noexcept;
    // Only meaningful for CHAOS/WANDER - see LfoSource's own comment.
    void setLfoFrozen(bool frozen) noexcept;
    void reseedLfo() noexcept;
    void setLfoChaosDrive(float normalizedDrive) noexcept;
    void setLfoChaosDamping(float normalizedDamping) noexcept;
    void setLfoWanderDepth(float normalizedDepth) noexcept;
    void setRingMix(float normalizedMix) noexcept;
    void setNoiseMix(float normalizedMix) noexcept;
    void setNoiseColour(NoisePalette::Colour colour) noexcept;
    // Ruído com envelope próprio (docs/PESQUISA_RUIDO_GENERATIVO.md item
    // #2, porta a ideia real de RASGO_SYNTH/dsp/BreathExciter.hpp),
    // reaproveitando o ruído já colorido pelo NOISE COR existente em vez
    // de uma fonte branca à parte. IMPORTANTE (redesenhado 19 ago. 2026,
    // autor: "o noise deve entrar nos steps do sequencer sem sons longos
    // passando por cima dos acontecimentos"): 0 (padrão) NÃO significa
    // mais "idêntico a antes" - o próprio noiseMix agora SEMPRE dispara
    // e decai a cada passo (mesmo ponto de trigger do envelope.trigger()
    // da nota, decaimento proporcional à duração real do passo, não um
    // valor fixo em ms), substituindo o antigo comportamento de nível
    // constante. Este parâmetro só ADICIONA profundidade extra ao MESMO
    // pulso por passo, não liga/desliga a reatividade em si (que agora é
    // sempre ativa).
    void setNoiseBreathAmount(float normalizedAmount) noexcept;
    // Noise Field (19 ago. 2026, docs/PESQUISA_RUIDO_GENERATIVO.md item
    // #29 - "campo de instabilidade global que afeta timbre, ritmo e
    // eventos"). Pushed in every sample from DualObjectEngine (the ONE
    // shared value the author asked for, not computed independently per
    // object) - not something this class evolves on its own. 0 = calm,
    // no effect (matches this session's 0=off convention); used in
    // renderSample() to add a small amount of extra noise presence
    // (timbre/textura axis) and in advanceStep() to add a small bounded
    // timing jitter (ritmo axis) - the third axis (eventos) lives in
    // DualObjectEngine itself, nudging EXCITAÇÃO's own trigger
    // threshold, not here.
    void setInstability(float normalizedAmount) noexcept;
    void setSampleHoldRate(float hertz) noexcept;
    void setSampleHoldMix(float normalizedMix) noexcept;
    void setReverbMix(float normalizedMix) noexcept;
    void setReverbFeedback(float normalizedAmount) noexcept;
    void setPhaserMix(float normalizedMix) noexcept;
    void setPhaserRate(float hertz) noexcept;
    void setPhaserDepth(float normalizedDepth) noexcept;
    void setFlangerMix(float normalizedMix) noexcept;
    void setFlangerRate(float hertz) noexcept;
    void setFlangerDepth(float normalizedDepth) noexcept;
    // Comb/resonator - distinct from the reverb above (see MaterialEffects.h's
    // CombResonator): a single short delay whose length reads as a pitch, not
    // as ambience.
    void setResonatorMix(float normalizedMix) noexcept;
    void setResonatorPitch(float normalizedPitch) noexcept;
    void setResonatorDamping(float normalizedDamping) noexcept;
    void setMixChannel(std::size_t channel, MutableMixer::Channel configuration) noexcept;
    [[nodiscard]] MutableMixer::Channel getMixChannel(std::size_t channel) const noexcept;
    void captureMixMemory(std::size_t slot) noexcept;
    void recallMixMemory(std::size_t slot) noexcept;
    void setStepMuted(std::size_t step, bool shouldMute) noexcept;
    [[nodiscard]] bool isStepMuted(std::size_t step) const noexcept;
    void setStepVoltage(std::size_t step, float normalizedVoltage) noexcept;
    [[nodiscard]] float getStepVoltage(std::size_t step) const noexcept;
    void setStepLevel(std::size_t step, float normalizedLevel) noexcept;
    [[nodiscard]] float getStepLevel(std::size_t step) const noexcept;
    void setStepEffectSend(std::size_t step, float normalizedSend) noexcept;
    [[nodiscard]] float getStepEffectSend(std::size_t step) const noexcept;
    // A controlled perturbation of the 16 CV/AMP/FX cells. It remains bounded
    // and repeatable from the engine's internal state, never a raw audio RNG.
    void randomizeSteps(float amount = 0.65f) noexcept;
    // Reseeds the internal xorshift used by randomizeSteps() and the MEM
    // scanner route. The default (0xA17E70U) stays fixed/reproducible for
    // tests; the app calls this once at launch with real entropy so a
    // restart doesn't always perturb/scan the exact same way. 0 would leave
    // xorshift stuck at 0 forever, so it's rejected in favour of 1.
    void seedRandom(unsigned int seed) noexcept { randomState = seed != 0U ? seed : 1U; }
    // Digital counterpart to selecting "reset on step" in a one-hot scanner.
    void setLoopEnd(std::size_t oneBasedStep) noexcept;
    [[nodiscard]] std::size_t getLoopEnd() const noexcept { return loopEnd; }
    [[nodiscard]] std::size_t getCurrentStep() const noexcept { return currentStep; }
    // Gancho de exportação MIDI/partitura (docs/
    // PESQUISA_COMPASSO_E_METRICA_REAL.md, seção 5 - "serve de ponte pra
    // MIDI/partitura"), 20 ago. 2026, autor: "é possível extrair a
    // partitura da melodia? ou a partitura rítmica, ou completa" -
    // "consome e limpa" (como um evento de um disparo, não um estado
    // contínuo): true UMA VEZ depois de um passo REALMENTE soar (não
    // `muted[]`, não silenciado por `StepRule::silence`), falso depois
    // de lido. Granularidade de POLLING (uma vez por callback de áudio,
    // não por sample) é suficiente pra exportação de partitura - a
    // resolução mais fina que qualquer subdivisão real do sequenciador
    // já produz. Se `advanceStep()` disparar mais de uma vez entre duas
    // leituras (clock muito rápido pro tamanho do buffer de áudio), só a
    // ÚLTIMA fica registrada - uma perda aceita, documentada, não um bug
    // silencioso.
    [[nodiscard]] bool didStepSoundSincePoll() noexcept
    {
        const auto result = stepSoundedPending;
        stepSoundedPending = false;
        return result;
    }
    [[nodiscard]] float getLastSoundingPitch01() const noexcept { return lastSoundingPitch01; }
    [[nodiscard]] float getLastSoundingLevel() const noexcept { return lastSoundingLevel; }
    // Acento como fonte de EXCITAÇÃO (docs/PESQUISA_ACENTUACAO_GENERATIVA.md,
    // "próximos passos" item 2 - accent_source = STRUCTURAL, "acento do
    // step → articulação" já previsto no próprio brief de CRI-SEQ-001),
    // 19 ago. 2026. Same strong-beat test renderSample() already uses
    // for metricAccent's own peak (currentStep % metricBeats == 0),
    // exposed read-only so DualObjectEngine can use accent as a second,
    // structural stimulus for EXCITAÇÃO - distinct from the raw
    // audio-derivative one it already reacts to.
    [[nodiscard]] bool isMetricAccentStep() const noexcept { return (currentStep + accentRotation) % metricBeats == 0; }
    void setRunning(bool shouldRun) noexcept;
    [[nodiscard]] bool isRunning() const noexcept { return running; }
    void reset() noexcept;
    void renderSample(float& left, float& right, float externalSignal = 0.0f) noexcept;
    void render(float* left, float* right, std::size_t samples) noexcept;
    // Gancho de exportação MIDI (docs/PESQUISA_MIDI_E_PARTITURA.md), 20
    // ago. 2026, autor: "no midi está super rápido" - bug real
    // encontrado: `samplesPerStep()` (a fonte de verdade que já rege o
    // áudio) NÃO é só `clockRate` - ela combina CLOCK com ENERGIA
    // (`supplyClock`, 0.46x-1.24x) e SUBDIVISÃO (`tupleDuration`, varia
    // por feel). A exportação MIDI original ignorava os dois, usando só
    // `clockRate/stepsPerBeat*60` - por isso o BPM saía errado (mais
    // rápido que o real, sobretudo em ENERGIA baixa). Este getter expõe
    // a média REAL de amostras por passo (mesma fórmula de
    // `samplesPerStep()`, mas com a MÉDIA do multiplicador de
    // SUBDIVISÃO em vez do valor do passo atual - GLITCH/SWING variam
    // por passo, mas GROOVE já é tempo-preservante por design, "soma
    // sempre 2.0", então não precisa entrar aqui) - quem exporta MIDI
    // converte isso em segundos/BPM sem precisar reimplementar a
    // fórmula de `samplesPerStep()` por fora.
    [[nodiscard]] double getAverageSamplesPerStep() const noexcept;

private:
    void advanceStep() noexcept;
    [[nodiscard]] double samplesPerStep() const noexcept;

    std::array<float, stepCount> voltages { 0.12f, 0.28f, 0.46f, 0.64f,
                                             0.82f, 0.56f, 0.35f, 0.18f,
                                             0.18f, 0.35f, 0.56f, 0.82f,
                                             0.64f, 0.46f, 0.28f, 0.12f };
    std::array<bool, stepCount> muted {};
    std::array<float, stepCount> levels { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                          1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<float, stepCount> effectSends { 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f,
                                               0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f };
    double sampleRate = 44100.0;
    double clockRate = 2.0;
    double clockSamples = 0.0;
    ClockFeel clockFeel = ClockFeel::straight;
    // Defaults to 0 (no groove) - unlike SWG's own fixed pattern, GROOVE
    // is a general modifier layered on top of every feel, so it should be
    // silent/off until the author actually reaches for it.
    float grooveAmount = 0.0f;
    ScannerDirection scannerDirection = ScannerDirection::forward;
    // Inércia do playhead (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção
    // 7.3 "playhead inércia/atratores/repelentes", 20 ago. 2026) - sinal
    // (+1/-1) do último salto real em ScannerDirection::memoryAddress,
    // lido por advanceStep() pra enviesar o PRÓXIMO salto a favor de
    // continuar na mesma direção (mesma "process implica continuação"
    // do viés de Narmour que a EXCITAÇÃO já usa pra pitch, item #11,
    // PESQUISA_MELODIA_GENERATIVA.md - a mesma técnica, aplicada aqui ao
    // playhead em vez de altura). Não usado pelos outros modos de
    // scanner (forward/reverse/pendulum já têm direção fixa/própria).
    int scannerMomentumDirection = 1;
    unsigned int metricBeats = 4, metricUnit = 4;
    // Compasso real (ver o comentário do membro público `TimeSignature`
    // acima) - 4/4 por padrão, `stepsPerBeat=4` casa com os 16 passos
    // do grid por padrão (`stepsPerMeasure` = 16), mas os dois seguem
    // independentes de `loopEnd`/`metricBeats` a partir daqui.
    // `measureStepIndex` avança em `advanceStep()` (não em
    // `renderSample()` - um evento por PASSO real, não por sample) e
    // ENVOLVE (wrap) sozinho ao cruzar `stepsPerMeasure`, disparando o
    // reset do event budget abaixo no mesmo instante - quem chama
    // detecta a fronteira comparando leituras sucessivas de
    // `getMeasureStepIndex()`, mesmo idioma já usado pro wrap de
    // `loopEnd` disparar DERIVA.
    TimeSignature timeSignature {};
    unsigned int stepsPerBeat = 4;
    unsigned int measureStepIndex = 0;
    // Event budget (ver o comentário do membro público acima) - 0 =
    // orçamento cheio, some até 1.0 (ceiling) conforme
    // `spendEventBudget()` é chamado; reseta pra 0 no mesmo instante em
    // que `measureStepIndex` envolve.
    float eventBudgetSpent = 0.0f;
    // Meta-sequenciador (ver o comentário do getter/setter público
    // acima) - `stepRules` por índice de passo (todos `normal` até
    // alguém chamar `setStepRule()` ou o roll autônomo em
    // `advanceStep()` sortear algo diferente). `ruleSilenceThisStep`
    // é um flag TRANSIENTE (recomputado a cada `advanceStep()`, nunca
    // persistido) que `renderSample()` lê pra decidir `noteGate` -
    // separado de `muted[]` porque `silence` é uma pausa PONTUAL desta
    // passagem, não uma edição permanente do passo.
    float metaSequencerAmount = 0.0f;
    std::array<StepRule, stepCount> stepRules {};
    bool ruleSilenceThisStep = false;
    // Microevent / ratchet (ver o comentário do enum `StepRule` acima) -
    // `ratchetActiveThisStep` decidido em `advanceStep()` (junto do
    // event budget, uma vez por passo); o sub-clock real que dispara os
    // sub-hits vive em `renderSample()`, comparando `clockSamples`
    // contra frações de `samplesPerStep()`. `ratchetSubIndex` conta
    // quantos sub-hits já dispararam nesta passagem (evita re-disparar
    // o mesmo sub-hit em samples consecutivos); `ratchetGain` é o
    // multiplicador de ganho do sub-hit ATUAL, lido por `renderSample()`
    // junto de `stepStateGain`/`levels[]` - decrescendo real, não um
    // efeito de áudio separado.
    bool ratchetActiveThisStep = false;
    int ratchetSubIndex = 0;
    float ratchetGain = 1.0f;
    // Gancho de exportação MIDI (ver o comentário do getter público
    // `didStepSoundSincePoll()` acima) - setado no MESMO ponto que já
    // decide "este passo soou de verdade" (dentro de `advanceStep()`,
    // junto de `stepHeat`/`envelope.trigger()`), lido de fora em
    // granularidade de callback de áudio.
    bool stepSoundedPending = false;
    float lastSoundingPitch01 = 0.5f;
    float lastSoundingLevel = 1.0f;
    CmosVoice voice;
    CmosVcf filter, filterRight;
    MaterialFilter materialFilter, materialFilterRight;
    ContourEnvelope envelope;
    LfoSource lfo;
    NoisePalette noise;
    // Right-channel counterpart, differently seeded, used only for the
    // dedicated NOISE mixer channel's own stereo image - never mixed into
    // the pre-filter voiceLeft/voiceRight addition below, so the already
    // verified FILTER/RING pan behaviour stays untouched.
    NoisePalette noiseRight { 0x9e3779b9U };
    SampleHold sampleHold, sampleHoldRight;
    RingModulator ring;
    // FILTER and RING already carry the oscillators' EIXO X pan through
    // their own stereo differential (filteredLeft/Right, ringedLeft/Right).
    // reverb/phaser/flanger/resonator used to process only the mono
    // `filtered` average, silently collapsing that pan back to centre for
    // the ESPAÇO mixer channel - these Right instances let the whole chain
    // run in stereo instead, mirroring the filter/filterRight pattern.
    MaterialReverb reverb, reverbRight;
    PhaseField phaser, phaserRight;
    FlangerField flanger, flangerRight;
    CombResonator resonator, resonatorRight;
    MutableMixer mixer;
    SignalLeveler leveler;
    LinkedOverloadProtector overloadProtector;
    OutputStage output;
    float externalFeedbackAmount = 0.0f;
    float energy = 0.72f;
    float noiseMix = 0.0f;
    float noiseBreathAmount = 0.0f;
    float instability = 0.0f;
    // Accent Field, timbre destination (19 ago. 2026) - the FORMA-A
    // knob's own value (setOscillatorShape(0, ...) now stores here
    // instead of forwarding straight to voice), nudged by metricAccent
    // each sample in renderSample() before actually being applied.
    // Default matches CmosVoice's own oscillatorShapes[0] default (0.0,
    // pure sine) so a patch that never touches FORMA-A sounds identical
    // to before this existed.
    float oscillatorShapeBaseA = 0.0f;
    // Accent inheritance (docs/PESQUISA_ACENTUACAO_GENERATIVA.md, brief
    // item "um acento forte pode afetar os passos seguintes... o acento
    // principal gera uma cauda dinâmica"), 19 ago. 2026. Set in
    // advanceStep() when landing on a strong beat, halved on every
    // subsequent step - a real decaying tail, not a second accent.
    float accentTail = 0.0f;
    // Accent rotation (docs/PESQUISA_ACENTUACAO_GENERATIVA.md, brief
    // item "o padrão de acentos pode girar sem mudar as notas"), 19
    // ago. 2026 - also the first real overlap with docs/
    // PESQUISA_DERIVA_GENERATIVA.md's own "deriva métrica" ("o centro
    // perceptivo dos acentos muda lentamente... sem necessariamente
    // mudar o tamanho do ciclo"), same mechanism serving both. Speed
    // scales with `instability` (the shared Noise Field) - at its
    // typical resting value (~0.2) this rotates roughly once every
    // 20-30s (estimate, not measured), true "weather", not a fixed
    // clock. accentRotation shifts WHICH step counts as the strong beat
    // (see isMetricAccentStep()/renderSample()'s own metricAccent), not
    // metricBeats itself - the metric identity stays, only its phase
    // migrates.
    float accentRotationPhase = 0.0f;
    unsigned int accentRotation = 0;
    // Step fatigue (docs/PESQUISA_SEQUENCER_GENERATIVO.md, segundo
    // brief, seção 7.4 - "um step muito repetido pode ficar
    // temporariamente menos provável... recupera lentamente"), 19 ago.
    // 2026. Real prior art: RASGO_SYNTH's ProbabilityMarket.hpp (preço
    // que cai com uso frequente, sobe com ausência, sempre com piso/teto
    // - nunca extingue nem vira certeza). Adaptado aqui sem um sistema
    // de "preço" persistente novo - reaproveita accentRotation (já
    // existe): a MESMA posição de tempo forte tocando ciclo após ciclo
    // sem a rotação mudar é "uso frequente"; quando accentRotation
    // muda, a posição é "nova" de novo (fresca) e a fadiga zera. Cresce
    // um pouco a cada vez que o tempo forte atual dispara, com teto -
    // nunca reduz o pico abaixo do que o tempo fraco já alcança no seu
    // próprio topo, preservando a distinção forte/fraco mesmo cansado.
    float accentFatigue = 0.0f;
    // Step state - dormant/active/hot/exhausted, by step INDEX rather
    // than metric position (see advanceStep()'s own comment for the
    // full rationale). One float per step, 0 (cold/dormant) to 1
    // (fully exhausted) - mapped to an audible gain via stepStateGain
    // in renderSample().
    std::array<float, stepCount> stepHeat {};
    // The breath's own decay envelope value (0-1), separate from
    // envelope's ADSR - see setNoiseBreathAmount above. Retriggered both
    // in advanceStep() (each new non-muted step) and on the rising edge
    // of noteGate in renderSample() (noiseGateWasHigh below) - the same
    // two trigger points ContourEnvelope::process() itself uses
    // internally for the voice's own contour, so noise starts pulsing
    // immediately on PLAY, not only after the first step boundary is
    // reached.
    float noiseBreathEnvelope = 0.0f;
    // A trigger sets THIS true instead of jumping noiseBreathEnvelope
    // straight to 1.0 (found real, 19 ago. 2026, author: "gravei um
    // audio e percebi que algo parece estar clipando nos steps") - an
    // instant same-sample jump from wherever the envelope had decayed to
    // up to 1.0 is a genuine amplitude discontinuity multiplying straight
    // into noiseTotal, exactly at every step boundary - a real click/
    // overshoot, not a false alarm. renderSample() ramps toward 1.0 over
    // a fast ~3ms attack while this is true (still reads as a percussive
    // "ping", just without the single-sample jump), then switches to the
    // existing tempo-proportional decay once it arrives.
    bool noiseBreathRising = false;
    bool noiseGateWasHigh = false;
    float sampleHoldMix = 0.78f;
    float mixReflux = 0.0f;
    float playGate = 0.0f;
    float targetPlayGate = 0.0f;
    std::size_t currentStep = 0;
    std::size_t loopEnd = stepCount;
    int scannerIncrement = 1;
    unsigned int randomState = 0xA17E70U;
    bool running = false;
};
} // namespace antitotem
