#pragma once

#include "core/MelodicInterpreter.h"
#include "core/OutputStage.h"
#include "core/SimpleSequencer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace antitotem
{
// Two autonomous objects, connected through deliberately visible cross-routes.
// Their physical historic layouts remain separately documented; this is the
// first digital modular study, not a claim of exact reconstruction.
class DualObjectEngine
{
public:
    enum ConnectionRoute : unsigned char { direct = 1, diode = 2, capacitor = 4, pulse = 8 };

    void prepare(double sampleRate) noexcept;
    void setRunning(bool shouldRun) noexcept;
    void setObjectConnection(float object1To5, float object5To1) noexcept;
    void setConnectionRoutes(unsigned char object1To5, unsigned char object5To1) noexcept;
    void setAuxiliaryMix(float toObject1, float toObject5) noexcept;
    // The one genuinely single/shared gain in the whole engine: applied to
    // the already-summed (object1+object5)*0.5 signal, downstream of each
    // object's own SimpleSequencer::setMasterGain (which only scales that
    // object's own pre-mix stage). MASTER in the UI should call this, not
    // either object's own setMasterGain.
    void setMasterGain(float normalizedGain) noexcept { output.setMasterGain(normalizedGain); }
    // PRINCIPAL/CLONE treated as a 2-channel mix at the point they're
    // summed into the final output - same M+gain vocabulary as
    // SimpleSequencer's own 4-channel MutableMixer, one level up (no S:
    // with only 2 elements, soloing one is exactly the same as muting the
    // other, so it added nothing - author, live: "apesar de que o 's'
    // como está é a mesma coisa que o 'on'... deixa só o M"). Index 0 is
    // object1 (PRINCIPAL), index 1 is object5 (CLONE). Deliberately does
    // NOT touch lastFirst/lastFifth (what each object sends into the
    // other's cross-feedback input) - muting one object's own audible
    // output should not also silence its influence on the other through
    // the feedback ports/routes, those stay a separate, still-visible
    // concern from CONEXÃO ENTRE OBJETOS.
    struct ObjectChannel { float gain = 1.0f; bool mute = false; };
    void setObjectChannel(std::size_t index, ObjectChannel value) noexcept
    {
        if (index >= objectChannels.size()) return;
        value.gain = std::clamp(value.gain, 0.0f, 1.0f);
        objectChannels[index] = value;
    }
    [[nodiscard]] SimpleSequencer& object1() noexcept { return first; }
    [[nodiscard]] SimpleSequencer& object5() noexcept { return fifth; }
    void render(float* left, float* right, std::size_t samples) noexcept;
    // EXCITAÇÃO (20 ago. 2026, author: "algo generativo, não performático
    // ... que inventasse melodias a partir de estimulos e acontecimentos
    // dos fluxos do que acontece nos controles que já existem") - a third,
    // dedicated CmosVoice - "é um solista, e uma voz" (monophonic, one
    // line, phrased rather than firing constantly) - that listens to how
    // much PRINCIPAL/CLONE's own combined output is CHANGING sample to
    // sample (its derivative, not its raw level - author: "creio que seja
    // melhor que ele reaja a estimulos sutis"), and rolls a chance,
    // weighted by excitationAmount, of turning a detected wobble into a
    // new pitch, spaced out by a cooldown so it phrases like a soloist
    // instead of retriggering every sample. 0 = off entirely (author:
    // "zerado é ele desligado"); higher values lower the sensitivity
    // threshold, shorten the cooldown, and raise this voice's own output
    // gain, so it "gets more excited" gradually as the author described.
    //
    // Two earlier versions tried and replaced live, both same day:
    // (1) listened to combined OUTPUT AMPLITUDE via two followers at
    // different speeds (an onset detector) - author tested it and
    // reported "só tem a mesma nota" (stuck on one note): this
    // instrument's own texture stays fairly sustained once running, so
    // amplitude rarely swings enough after the first attack to trigger
    // again.
    // (2) listened for step-advance EVENTS (getCurrentStep() changing) -
    // never tested live; author redirected first ("creio que seja melhor
    // que ele reaja a estimulos sutis" / "é um solista") before hearing
    // it, reasoning that locking triggers to the sequencer's own clock
    // grid would read as mechanical, not like a soloist reacting to
    // nuance - a fair point independent of whether (2) would have worked.
    //
    // Pitch itself stays free/chromatic (author, asked directly whether to
    // quantize to a scale: "não sem escalas prontas, é um instrumentos
    // inteligente, que pode se valer de qualquer escala, nota timbre,
    // ritmo, duração, gesto, etc" -> confirmed via AskUserQuestion:
    // "Cromático livre, sem restrição"). What actually makes this read as
    // a theremin isn't scale quantization, it's glide: this engine's own
    // sibling project (RASGO_SYNTH/rasgo-synth-core/src/dsp/
    // ThereminVoice.hpp, author: "acho que há um theremin no rasgo synth")
    // never re-triggers a note, only ever glides toward a target, plus a
    // small always-on vibrato modelling a hand's natural tremor - both
    // borrowed here (excitationCurrentPitch chases excitationPitch, the
    // target, via a real exponential glideCoeff computed from the actual
    // sample rate the same way ThereminVoice.hpp does; excitationVibrato
    // is a small sine LFO nudging the sounding pitch, not the target).
    void setExcitationAmount(float normalizedAmount) noexcept;
    // Independent from excitationAmount, same M-button vocabulary as
    // PRINCIPAL/CLONE (objectChannels) - mute silences without discarding
    // whatever the slider is set to (20 ago. 2026, author: "crie o botão
    // de mute para o excit no object mixer").
    void setExcitationMute(bool shouldMute) noexcept { excitationMuted = shouldMute; }
    // Estados expressivos nomeados (item #17, docs/
    // PESQUISA_MELODIA_GENERATIVA.md - "poucos estados internos [...],
    // mais um estado musical de escala maior"), 20 ago. 2026, autor:
    // "quais outros itens da melodia a implementar" -> "sim" (concordando
    // em fazer #17 antes de #16). Simplificado do brief (5 estados em
    // vez de 7 pro micro, 4 em vez de 6 pro macro - "poucos estados" já
    // era o próprio pedido do brief) mas COMPUTADO de verdade a cada
    // sample/disparo a partir de sinais que já existiam (idade da nota,
    // progresso do glide, fôlego, energia de frase), não um enum
    // decorativo sem efeito nenhum no som - ver o comentário de
    // `excitationPhraseBoundaryResting` logo abaixo pro efeito audível
    // real que isso desbloqueou (silêncio de verdade entre frases, que
    // não existia antes).
    // 20 ago. 2026 (item #19, autor: "item #2/#4, #14 e #19, prossiga") -
    // os enums em si agora vivem em MelodicInterpreter (extração da
    // camada de decisão musical, ver o comentário de classe em
    // MelodicInterpreter.h); aliases aqui pra não quebrar a API pública
    // já exposta por estes getters.
    using ExcitationMicroState = MelodicInterpreter::MicroState;
    using ExcitationPhraseState = MelodicInterpreter::PhraseState;
    [[nodiscard]] ExcitationMicroState getExcitationMicroState() const noexcept { return melodicInterpreter.getMicroState(); }
    [[nodiscard]] ExcitationPhraseState getExcitationPhraseState() const noexcept { return melodicInterpreter.getPhraseState(); }
    // Vetor de caráter (docs/PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md,
    // seção 6.1), 20 ago. 2026 - forwarding getters, mesmo padrão dos
    // dois acima; leitura pública pra uma UI/outro sistema futuro,
    // ainda sem consumidor além do próprio MelodicInterpreter.
    [[nodiscard]] float getExcitationCharacterEnergy() const noexcept { return melodicInterpreter.getCharacterEnergy(); }
    [[nodiscard]] float getExcitationCharacterSoftness() const noexcept { return melodicInterpreter.getCharacterSoftness(); }
    [[nodiscard]] float getExcitationCharacterBrightness() const noexcept { return melodicInterpreter.getCharacterBrightness(); }
    // Gancho de exportação MIDI/partitura (docs/
    // PESQUISA_COMPASSO_E_METRICA_REAL.md, seção 5), 20 ago. 2026, autor:
    // "é possível extrair a partitura da melodia?" - mesmo padrão
    // "consome e limpa" de `SimpleSequencer::didStepSoundSincePoll()`,
    // agregado aqui a partir de `Voice::justTriggered` (item 6.3) que já
    // é computado por SAMPLE dentro de `render()` - render() só devolve
    // áudio pro chamador, então este é o ÚNICO lugar onde esses disparos
    // por sample viram um evento de granularidade de callback.
    [[nodiscard]] bool didExcitationTriggerSincePoll() noexcept
    {
        const auto result = excitationTriggerPending;
        excitationTriggerPending = false;
        return result;
    }
    [[nodiscard]] float getExcitationLastTriggerPitch01() const noexcept { return excitationLastTriggerPitch01; }
    // Noise Field ↔ DERIVA connection (docs/PESQUISA_DERIVA_GENERATIVA.md,
    // seção 6, item 3 - "hoje são dois sistemas de 'campo lento'
    // paralelos e desconhecidos um do outro"), 19 ago. 2026, author:
    // "precisamos que os sistemas dentro do instrumentos sejam
    // inteligentes versáteis com boa capacidade de enxergar os fluxos" ->
    // "isso" (confirmando a conexão especificamente, não uma unificação
    // maior). Read-only getter so DERIVA (Main.cpp) can bias its own
    // derivationMotion by the shared field, and nudgeInstability lets a
    // real DERIVA event push back onto the field - a two-way, but still
    // gentle, coupling; neither system is put in charge of the other.
    [[nodiscard]] float getInstabilityField() const noexcept { return instabilityField; }
    void nudgeInstability(float amount) noexcept { instabilityField = std::clamp(instabilityField + amount, 0.0f, 1.0f); }
    // Arquitetura de cinco escalas (docs/PESQUISA_SEQUENCER_GENERATIVO.md,
    // seção 7.1, item "Arquitetura de cinco escalas": Microevent -> Step
    // -> Pattern/Motif -> Form/State -> History, "unificando memória/
    // deriva/erosão/resíduo/mutação/recuperação/rasgo numa única
    // temporalidade do instrumento"), 20 ago. 2026, autor: "prossiga"
    // (a última das "três tarefas grandes", depois de event budget e
    // meta-sequenciador). Step já existia por inteiro (SimpleSequencer:
    // voltages/levels/muted/effectSends/StepRule). Esta é a QUINTA
    // escala, Form/State - a trajetória do INSTRUMENTO inteiro (não de
    // um objeto só, ao contrário de Step/event budget/meta-sequenciador,
    // que são por PRINCIPAL/CLONE) - mais History, a memória de por onde
    // essa trajetória já passou. Microevent (ratchet/grace/glitch) e
    // Pattern/Motif (identidade de padrão comparável entre si) ficam de
    // fora desta fatia - genuinamente ausentes ainda, não uma correção
    // de algo que já existia (ver PESQUISA_SEQUENCER_GENERATIVO.md,
    // seção 7.4, pro racional completo da escolha de escopo).
    //
    // formEnergy: um seguidor MUITO lento (dezenas de segundos, não os
    // ~70ms/300ms de qualquer outro seguidor deste arquivo) da média de
    // quanto do event budget de PRINCIPAL/CLONE anda sendo gasto - não
    // um sinal novo, uma segunda leitura em escala de tempo diferente do
    // que `hasEventBudget()`/`spendEventBudget()` já medem por compasso
    // (mesmo princípio "um valor compartilhado, várias leituras" já
    // usado pro Noise Field). FormState nomeia essa trajetória (Calm/
    // Rising/Peak/Falling/Recovering, "poucos estados" de novo) usando
    // limiar + DIREÇÃO (subindo ou descendo), não só limiar puro - o
    // mesmo cuidado que distingue Rising de Falling numa PhraseState já
    // existente (MelodicInterpreter) evitaria aqui um flicker sem
    // sentido perto da fronteira entre dois estados.
    enum class FormState { Calm, Rising, Peak, Falling, Recovering };
    [[nodiscard]] FormState getFormState() const noexcept { return formState; }
    // History - não só onde o sistema está, por onde já passou (a
    // citação do brief). Buffer circular pequeno (4, mesma filosofia
    // "poucos estados" - não um log ilimitado) das ÚLTIMAS transições
    // reais de FormState (gravado só quando o estado MUDA, não a cada
    // sample) - `stepsAgo=0` é a transição mais recente.
    [[nodiscard]] FormState getFormHistoryAt(std::size_t stepsAgo) const noexcept
    {
        if (stepsAgo >= formHistory.size()) return FormState::Calm;
        const auto index = (static_cast<std::size_t>(formHistoryWrite) + formHistory.size() - 1U - stepsAgo) % formHistory.size();
        return formHistory[index];
    }

private:
    SimpleSequencer first, fifth;
    CmosVoice auxiliaryOne, auxiliaryTwo;
    // functionForms = a pure sine core (std::sin in CmosVoice's own
    // shapeCore) - the cleanest, most theremin-like of the three cores,
    // chosen live after reading CmosVoice.h together with the author. Set
    // in prepare(), not here - CmosVoice has no constructor taking a core.
    CmosVoice excitationVoice;
    float excitationAmount = 0.0f;
    bool excitationMuted = false;
    // Mirrors the transport's own running state (setRunning) - EXCITAÇÃO
    // has no note-off/envelope of its own (it only glides between
    // triggers), so without this its last note would keep sounding forever
    // after STOP. Gates output gain the same instant-hard-cut way
    // excitationMuted does; internal pitch-walk/breath state is left to
    // keep evolving underneath, same precedent as mute.
    bool running = true;
    // Toda a decisão musical (pitch/ganho/timbre/estados) vive agora em
    // MelodicInterpreter (item #19, 20 ago. 2026 - ver o comentário de
    // classe em MelodicInterpreter.h pro racional completo da extração).
    // O que sobra aqui é só o que é ESPECÍFICO desta ensemble
    // (PRINCIPAL/CLONE) - virar lastFirst/lastFifth num Stimulus genérico
    // é trabalho de DualObjectEngine, não do intérprete.
    MelodicInterpreter melodicInterpreter;
    float excitationPreviousFirst = 0.0f, excitationPreviousFifth = 0.0f;
    // Accent as a second, structural stimulus (docs/
    // PESQUISA_ACENTUACAO_GENERATIVA.md "próximos passos" item 2), 19
    // ago. 2026 - tracked so render() can edge-detect exactly when
    // either object LANDS on a strong beat (SimpleSequencer::
    // isMetricAccentStep(), true for that step's whole duration - these
    // remember the PREVIOUS sample's state so the nudge fires once per
    // accented step, not every sample it stays true).
    bool excitationPreviousFirstAccented = false, excitationPreviousFifthAccented = false;
    // Noise Field (19 ago. 2026, docs/PESQUISA_RUIDO_GENERATIVO.md item
    // #29 - "campo de instabilidade global que afeta timbre, ritmo e
    // eventos", author's own words when asked which noise-research idea
    // to pursue next). ONE shared value for the whole instrument (author,
    // asked directly: "um só, compartilhado" over one per object) - a
    // slow, autonomous "weather" (0=calm, drifts up toward instability
    // and back down on its own, not user-triggered) computed once here
    // and pushed into both first/fifth via SimpleSequencer::
    // setInstability() every sample, plus used directly below to nudge
    // EXCITAÇÃO's own trigger threshold. Real prior art for "one shared
    // value read by several destinations at once" already exists in the
    // project: AQUORBIUM's geometricAccent (BiomaEngine.cpp) feeds
    // hardness/particleDensity/irregularity/excitation from a single
    // source, and BuchlaRandomSource.hpp reads one LFSR state through
    // several different weighted combinations rather than several
    // independent generators - this follows the same principle. v1, no
    // UI control yet (fully autonomous) - author chose the concept, not
    // yet the exposed surface; ships DSP first per this session's own
    // established pattern (EXCITAÇÃO, NOISE BREATH both did the same).
    // All constants below are estimates, not measured/heard yet.
    float instabilityField = 0.0f;
    unsigned int instabilitySeed = 0xc2b2ae35u;
    // Form/State + History (ver o comentário dos membros públicos
    // `FormState`/`getFormHistoryAt` acima) - `formSampleRate` guardado
    // porque `prepare()` só recebe a taxa real uma vez, e o seguidor de
    // `formEnergy` precisa dela pra calcular sua constante de tempo (o
    // mesmo motivo que já justificava `excitationSampleRate` antes de
    // ser removido - agora um consumidor real de verdade voltou a
    // precisar).
    double formSampleRate = 44100.0;
    float formEnergy = 0.0f;
    float formEnergyPrevious = 0.0f;
    FormState formState = FormState::Calm;
    std::array<FormState, 4> formHistory { FormState::Calm, FormState::Calm, FormState::Calm, FormState::Calm };
    int formHistoryWrite = 0;
    // Fadiga de forma - cresce um pouco a cada transição PRA Peak,
    // recupera devagar. Efeito real (não só contabilidade): quando o
    // instrumento andou passando por Peak repetidas vezes recentemente
    // (lido de `formHistory`, a própria História informando uma
    // consequência), o novo Peak empurra `instabilityField` um pouco
    // pra BAIXO em vez de pra cima - o instrumento "cansa" de ficar
    // ocupado e acalma seu próprio clima, uma "recuperação" real (item
    // do brief: "erosão... cicatriz... o sistema recupera mas não volta
    // exatamente ao estado anterior") em vez de decorativa.
    float formFatigue = 0.0f;
    // Gancho de exportação MIDI (ver `didExcitationTriggerSincePoll()`
    // acima) - agregado em render() a partir de `Voice::justTriggered`.
    bool excitationTriggerPending = false;
    float excitationLastTriggerPitch01 = 0.5f;
    OutputStage output;
    std::array<ObjectChannel, 2> objectChannels {};
    float firstToFifth = 0.0f, fifthToFirst = 0.0f;
    float auxiliaryToFirst = 0.0f, auxiliaryToFifth = 0.0f;
    float lastFirst = 0.0f, lastFifth = 0.0f;
    float firstToFifthCapacitor = 0.0f, fifthToFirstCapacitor = 0.0f;
    unsigned char firstToFifthRoutes = capacitor, fifthToFirstRoutes = capacitor;

    [[nodiscard]] static float routeSignal(float source, unsigned char routes, float& capacitorState) noexcept;
};
} // namespace antitotem
