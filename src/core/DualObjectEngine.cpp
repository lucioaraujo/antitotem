#include "core/DualObjectEngine.h"

#include <algorithm>

namespace antitotem
{
void DualObjectEngine::prepare(double sampleRate) noexcept
{
    first.prepare(sampleRate); fifth.prepare(sampleRate); output.prepare(sampleRate);
    auxiliaryOne.prepare(sampleRate); auxiliaryTwo.prepare(sampleRate);
    auxiliaryOne.reset(); auxiliaryTwo.reset();
    excitationVoice.prepare(sampleRate);
    excitationVoice.setOscillatorCore(CmosVoice::OscillatorCore::functionForms);
    excitationVoice.reset();
    melodicInterpreter.prepare(sampleRate);
    formSampleRate = sampleRate;
    first.setExternalFeedbackAmount(0.56f); fifth.setExternalFeedbackAmount(0.56f);
    lastFirst = lastFifth = firstToFifthCapacitor = fifthToFirstCapacitor = 0.0f;
}

void DualObjectEngine::setExcitationAmount(float normalizedAmount) noexcept
{
    excitationAmount = std::clamp(normalizedAmount, 0.0f, 1.0f);
}

void DualObjectEngine::setRunning(bool shouldRun) noexcept
{
    first.setRunning(shouldRun); fifth.setRunning(shouldRun);
    // EXCITAÇÃO has no transport of its own - it only glides toward
    // whatever pitch it last triggered, so without this it would keep
    // sounding (gliding/vibrating) forever after STOP, since nothing else
    // in its own signal chain ever goes to true silence (author, 19 ago.
    // 2026: "após o stop o som do excit continua audivel"). Gated the same
    // way excitationMuted already is - an instant hard cut on OUTPUT gain
    // only, not a reset - so its internal pitch-walk/breath state keeps
    // evolving quietly underneath, same precedent as mute.
    running = shouldRun;
}

void DualObjectEngine::setObjectConnection(float object1To5, float object5To1) noexcept
{
    firstToFifth = std::clamp(object1To5, 0.0f, 0.72f);
    fifthToFirst = std::clamp(object5To1, 0.0f, 0.72f);
}

void DualObjectEngine::setConnectionRoutes(unsigned char object1To5, unsigned char object5To1) noexcept
{
    firstToFifthRoutes = object1To5 & 0x0fU;
    fifthToFirstRoutes = object5To1 & 0x0fU;
}

void DualObjectEngine::setAuxiliaryMix(float toObject1, float toObject5) noexcept
{
    auxiliaryToFirst = std::clamp(toObject1, 0.0f, 0.72f);
    auxiliaryToFifth = std::clamp(toObject5, 0.0f, 0.72f);
}

void DualObjectEngine::render(float* left, float* right, std::size_t samples) noexcept
{
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        // Noise Field (see the member comment in DualObjectEngine.h) -
        // a slow, bounded random walk with a soft pull back toward a
        // mostly-calm resting point (0.2, not 0 - a real "weather" is
        // rarely perfectly still), computed once per sample and pushed
        // into both objects before they render this same sample. Updated
        // and pushed unconditionally (cheap - one LCG step, one clamp,
        // two setInstability calls) - same "always tracked, never stale"
        // precedent as MelodicInterpreter's own always-on bookkeeping
        // (ensembleLevel/breath, see MelodicInterpreter::tick()).
        instabilitySeed = instabilitySeed * 1664525u + 1013904223u;
        const auto instabilityRoll = static_cast<float>(instabilitySeed >> 8) / static_cast<float>(1u << 24) * 2.0f - 1.0f;
        instabilityField += instabilityRoll * 0.00002f;
        instabilityField += (0.2f - instabilityField) * 0.0000005f;
        instabilityField = std::clamp(instabilityField, 0.0f, 1.0f);
        first.setInstability(instabilityField);
        fifth.setInstability(instabilityField);
        // Form/State + History (docs/PESQUISA_SEQUENCER_GENERATIVO.md,
        // seção 7.1, "Arquitetura de cinco escalas") - ver o comentário
        // completo dos membros públicos `FormState`/`getFormHistoryAt`
        // em DualObjectEngine.h. Segunda leitura, em escala de tempo
        // MUITO mais lenta, do mesmo event budget que PRINCIPAL/CLONE já
        // tracked pra si mesmos - não um novo estímulo, só uma janela
        // de tempo diferente sobre um sinal que já existia.
        const auto spendNow = (1.0f - first.getEventBudgetRemaining() + 1.0f - fifth.getEventBudgetRemaining()) * 0.5f;
        formEnergyPrevious = formEnergy;
        formEnergy += (spendNow - formEnergy) * static_cast<float>(1.0 / (30.0 * formSampleRate));
        const auto formRising = formEnergy > formEnergyPrevious;
        const auto newFormState = formEnergy < 0.15f ? FormState::Calm
                                 : formEnergy > 0.75f ? FormState::Peak
                                 : formRising ? FormState::Rising
                                 : formFatigue > 0.4f ? FormState::Recovering
                                 : FormState::Falling;
        if (newFormState != formState)
        {
            formState = newFormState;
            formHistory[static_cast<std::size_t>(formHistoryWrite)] = formState;
            formHistoryWrite = (formHistoryWrite + 1) % static_cast<int>(formHistory.size());
            if (formState == FormState::Peak)
            {
                // História informando consequência - conta quantas das
                // últimas 4 transições já eram Peak ANTES desta (por
                // isso stepsAgo começa em 1, não 0 - a transição atual
                // acabou de ser gravada acima).
                unsigned int recentPeaks = 0;
                for (std::size_t stepsAgo = 1; stepsAgo < formHistory.size(); ++stepsAgo)
                    if (getFormHistoryAt(stepsAgo) == FormState::Peak) ++recentPeaks;
                if (recentPeaks >= 2) nudgeInstability(-0.08f);
                formFatigue = std::min(formFatigue + 0.2f, 1.0f);
            }
        }
        formFatigue -= formFatigue * static_cast<float>(1.0 / (45.0 * formSampleRate));
        float firstLeft = 0.0f, firstRight = 0.0f, fifthLeft = 0.0f, fifthRight = 0.0f;
        const auto auxiliaryA = auxiliaryOne.tick(0.21f + 0.18f * std::abs(lastFifth));
        const auto auxiliaryB = auxiliaryTwo.tick(0.66f + 0.18f * std::abs(lastFirst));
        const auto fromFifth = routeSignal(lastFifth, fifthToFirstRoutes, fifthToFirstCapacitor) * fifthToFirst;
        const auto fromFirst = routeSignal(lastFirst, firstToFifthRoutes, firstToFifthCapacitor) * firstToFifth;
        first.renderSample(firstLeft, firstRight, fromFifth + auxiliaryA * auxiliaryToFirst);
        fifth.renderSample(fifthLeft, fifthRight, fromFirst + auxiliaryB * auxiliaryToFifth);
        lastFirst = 0.5f * (firstLeft + firstRight);
        lastFifth = 0.5f * (fifthLeft + fifthRight);
        // Ensemble loudness follower for EXCITAÇÃO's own gain balancing
        // below (~70ms) - kept outside the excitationAmount gate, cheap
        // enough to always run, so the level is already current whenever
        // the slider gets raised.
        // EXCITAÇÃO - toda a decisão musical (pitch/ganho/timbre/estados)
        // mora em MelodicInterpreter (item #19, docs/
        // PESQUISA_MELODIA_GENERATIVA.md - "Camada 'Melodic Interpreter'
        // separada e reutilizável", extraída 20 ago. 2026, autor: "item
        // #2/#4, #14 e #19, prossiga"). Esta engine só monta o Stimulus
        // genérico a partir do que É específico dela (PRINCIPAL/CLONE
        // combinados) e decide o que FAZER com a Voice devolvida (aqui,
        // alimentar um CmosVoice) - ver o comentário de classe em
        // MelodicInterpreter.h pro racional completo da fronteira, e
        // MelodicInterpreter.cpp pro histórico de design item por item
        // (#1, #3, #5, #6, #7, #8, #9, #10, #13, #14, #15, #16, #17, #2,
        // #4), preservado nos comentários originais só movidos pra lá.
        MelodicInterpreter::Stimulus excitationStimulus;
        excitationStimulus.rawLevel = std::abs(lastFirst) + std::abs(lastFifth);
        excitationStimulus.rawActivityDelta = std::abs(lastFirst - excitationPreviousFirst) + std::abs(lastFifth - excitationPreviousFifth);
        excitationPreviousFirst = lastFirst;
        excitationPreviousFifth = lastFifth;
        // Accent as a second, structural stimulus (docs/
        // PESQUISA_ACENTUACAO_GENERATIVA.md "próximos passos" item 2 -
        // "acento do step → articulação"), 19 ago. 2026, author: "avance o
        // acento". Edge-detected here (fires once per accented step, not
        // every sample it stays true) - either object's own strong beat
        // counts, matching how the raw-level stimulus above already reads
        // from both.
        const auto firstAccented = first.isMetricAccentStep();
        const auto fifthAccented = fifth.isMetricAccentStep();
        excitationStimulus.accentEvent = (firstAccented && !excitationPreviousFirstAccented) || (fifthAccented && !excitationPreviousFifthAccented);
        excitationPreviousFirstAccented = firstAccented;
        excitationPreviousFifthAccented = fifthAccented;
        excitationStimulus.instability = instabilityField;
        excitationStimulus.amount = excitationAmount;
        excitationStimulus.running = running;
        excitationStimulus.muted = excitationMuted;
        // tick() sempre roda (bookkeeping barato - ensembleLevel/activity/
        // cooldown/breath/restHush - continua atualizado mesmo com
        // amount==0, mesmo motivo de sempre no código original: reerguer
        // o slider depois de um trecho baixo não deve comparar contra um
        // valor velho/errado). O bloco caro de verdade (pitch/glide/
        // vibrato) já se auto-limita lá dentro (early return se
        // amount<=0) - ver o comentário no início de
        // MelodicInterpreter::tick().
        const auto excitationVoiceOutput = melodicInterpreter.tick(excitationStimulus);
        // Consequência de volta (docs/PESQUISA_ORQUESTRA_RELACIONAL_E_
        // TEMPERAMENTO.md, seção 6.3 - "influenciar e ser influenciado"),
        // 20 ago. 2026, autor: "prossiga". Espelha `nudgeInstability`
        // (já existia, só usado por DERIVA em Main.cpp) - EXCITAÇÃO
        // ganha uma via própria e deliberadamente PEQUENA (0.004, bem
        // menor que o nudge de DERIVA, `abs(derivationMotion)*0.02`, até
        // ~0.011 típico) de influenciar o mesmo campo compartilhado a
        // cada disparo real seu - um eco genuíno, não um mecanismo novo,
        // pra não competir com o papel de DERIVA no mesmo campo.
        if (excitationVoiceOutput.justTriggered)
        {
            nudgeInstability(0.004f);
            // Gancho de exportação MIDI (ver `didExcitationTriggerSincePoll()`
            // em DualObjectEngine.h) - grava o pitch do disparo que
            // acabou de acontecer, granularidade de callback de áudio.
            excitationTriggerPending = true;
            excitationLastTriggerPitch01 = excitationVoiceOutput.soundingPitch;
        }
        float excitationLeft = 0.0f, excitationRight = 0.0f;
        // excitationVoice.tickStereo() (um CmosVoice inteiro, mesma classe
        // de custo dos osciladores de first/fifth) só roda com amount>0 -
        // author reported ALSA underruns right after EXCITAÇÃO was first
        // added; running a full extra voice's worth of DSP every sample
        // even while inaudible was pure waste regardless of whether it
        // was the actual cause.
        if (excitationAmount > 0.0f)
        {
            excitationVoice.setOscillatorShape(0, excitationVoiceOutput.timbreBrightness);
            const auto excitationFrame = excitationVoice.tickStereo(excitationVoiceOutput.soundingPitch, 0.0f);
            excitationLeft = excitationFrame.left * excitationVoiceOutput.gain;
            excitationRight = excitationFrame.right * excitationVoiceOutput.gain;
        }
        float finalLeft = 0.0f, finalRight = 0.0f;
        const auto firstGain = objectChannels[0].mute ? 0.0f : objectChannels[0].gain;
        const auto fifthGain = objectChannels[1].mute ? 0.0f : objectChannels[1].gain;
        output.process((firstLeft * firstGain + fifthLeft * fifthGain) * 0.5f + excitationLeft,
                        (firstRight * firstGain + fifthRight * fifthGain) * 0.5f + excitationRight, finalLeft, finalRight);
        if (left != nullptr) left[sample] = finalLeft;
        if (right != nullptr) right[sample] = finalRight;
    }
}

float DualObjectEngine::routeSignal(float source, unsigned char routes, float& capacitorState) noexcept
{
    capacitorState += (source - capacitorState) * 0.008f;
    float sum = 0.0f;
    int count = 0;
    const auto add = [&] (unsigned char flag, float value) {
        if ((routes & flag) != 0) { sum += value; ++count; }
    };
    add(direct, source);
    add(diode, std::abs(source) * 2.0f - 1.0f);
    add(capacitor, capacitorState);
    add(pulse, source >= 0.0f ? 1.0f : -1.0f);
    return count > 0 ? sum / static_cast<float>(count) : 0.0f;
}
} // namespace antitotem
