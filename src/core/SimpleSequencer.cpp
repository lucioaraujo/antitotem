#include "core/SimpleSequencer.h"

#include <algorithm>
#include <cmath>

namespace antitotem
{
void SimpleSequencer::prepare(double newSampleRate) noexcept
{
    sampleRate = std::max(1.0, newSampleRate);
    voice.prepare(sampleRate);
    filter.prepare(sampleRate);
    filterRight.prepare(sampleRate);
    materialFilter.prepare(sampleRate);
    materialFilterRight.prepare(sampleRate);
    envelope.prepare(sampleRate);
    lfo.prepare(sampleRate);
    noise.reset();
    noiseRight.reset();
    sampleHold.prepare(sampleRate);
    sampleHoldRight.prepare(sampleRate);
    reverb.prepare(sampleRate);
    reverbRight.prepare(sampleRate);
    phaser.prepare(sampleRate);
    phaserRight.prepare(sampleRate);
    flanger.prepare(sampleRate);
    flangerRight.prepare(sampleRate);
    resonator.prepare(sampleRate);
    resonatorRight.prepare(sampleRate);
    leveler.setSampleRate(sampleRate);
    output.prepare(sampleRate);
    reset();
}

void SimpleSequencer::setClockRate(double hertz) noexcept
{
    clockRate = std::clamp(hertz, 0.1, 20.0);
}

void SimpleSequencer::setClockFeel(ClockFeel feel) noexcept
{
    clockFeel = feel;
}

void SimpleSequencer::setGrooveAmount(float normalizedAmount) noexcept
{
    grooveAmount = std::clamp(normalizedAmount, 0.0f, 1.0f);
}

void SimpleSequencer::setScannerDirection(ScannerDirection direction) noexcept
{
    scannerDirection = direction;
    scannerIncrement = direction == ScannerDirection::reverse ? -1 : 1;
    // Reverse starts at the final active address; the other paths retain the
    // present address, allowing a live route change without a hard reset.
    if (direction == ScannerDirection::reverse)
        currentStep = loopEnd - 1;
}

void SimpleSequencer::setTimeSignature(unsigned int beatsPerMeasure, unsigned int beatUnit) noexcept
{
    // Fórmula de compasso real (docs/PESQUISA_COMPASSO_E_METRICA_REAL.md,
    // seção 3) - beatUnit tradicionalmente só faz sentido como potência
    // de 2 (2/4/8/16/32, a figura de nota que vale um tempo), mas não
    // força isso aqui - mesma filosofia de setMetric() logo abaixo
    // ("os números possíveis pode ser de 1 a 16"), aceita o valor real
    // em vez de coagir pra um preset.
    timeSignature.beatsPerMeasure = std::clamp(beatsPerMeasure, 1U, 32U);
    timeSignature.beatUnit = std::clamp(beatUnit, 1U, 32U);
    if (measureStepIndex >= getStepsPerMeasure()) measureStepIndex = 0;
}

void SimpleSequencer::setStepsPerBeat(unsigned int steps) noexcept
{
    stepsPerBeat = std::clamp(steps, 1U, 16U);
    if (measureStepIndex >= getStepsPerMeasure()) measureStepIndex = 0;
}

void SimpleSequencer::setMetaSequencerAmount(float normalizedAmount) noexcept
{
    metaSequencerAmount = std::clamp(normalizedAmount, 0.0f, 1.0f);
}

void SimpleSequencer::setStepRule(std::size_t index, StepRule rule) noexcept
{
    if (index < stepCount) stepRules[index] = rule;
}

void SimpleSequencer::setMetric(unsigned int beats, unsigned int unit) noexcept
{
    // 1-16 for both, not the old 2-16/binary-8-or-4 coercion (19-20 ago.
    // 2026, author: "os numeros possiveis pode ser de 1 a 16 para
    // preencher") - the real value is now kept, so any beats/unit pair
    // the two text boxes can produce is understood, not just the 8
    // hardcoded presets.
    metricBeats = std::clamp(beats, 1U, 16U);
    metricUnit = std::clamp(unit, 1U, 16U);
}

void SimpleSequencer::setRunning(bool shouldRun) noexcept
{
    running = shouldRun;
    targetPlayGate = shouldRun ? 1.0f : 0.0f;
}

void SimpleSequencer::setMasterGain(float normalizedGain) noexcept
{
    output.setMasterGain(normalizedGain);
}

void SimpleSequencer::setFeedbackAmount(float normalizedAmount) noexcept
{
    voice.setFeedbackAmount(normalizedAmount);
}

void SimpleSequencer::setFeedbackSignal(CmosVoice::FeedbackSignal signal) noexcept
{
    voice.setFeedbackSignal(signal);
}

void SimpleSequencer::setFeedbackConnections(unsigned char connections) noexcept
{
    voice.setFeedbackConnections(connections);
}

void SimpleSequencer::setExternalFeedbackAmount(float normalizedAmount) noexcept
{
    externalFeedbackAmount = std::clamp(normalizedAmount, 0.0f, 0.72f);
}

void SimpleSequencer::setOscillatorLevel(std::size_t oscillator, float normalizedLevel) noexcept
{
    voice.setOscillatorLevel(oscillator, normalizedLevel);
}

void SimpleSequencer::setOscillatorShape(std::size_t oscillator, float normalizedShape) noexcept
{
    // Oscillator A (index 0) is special since Accent Field's timbre
    // destination (docs/PESQUISA_ACENTUACAO_GENERATIVA.md "próximos
    // passos" item 3, 19 ago. 2026, author: "sim" - confirmed after a
    // suggested design) was added: this FORMA knob's own value is now
    // the BASE renderSample() nudges each sample with metricAccent, not
    // applied straight to voice here - otherwise this setter and the
    // per-sample accent nudge would fight over the same CmosVoice
    // parameter, the accent contribution winning or losing depending on
    // which happened to run last. Oscillators 1-4 are untouched, forward
    // exactly as before.
    if (oscillator == 0) { oscillatorShapeBaseA = std::clamp(normalizedShape, 0.0f, 3.0f); return; }
    voice.setOscillatorShape(oscillator, normalizedShape);
}

void SimpleSequencer::setOscillatorRatio(std::size_t oscillator, float ratio) noexcept
{
    voice.setOscillatorRatio(oscillator, ratio);
}

void SimpleSequencer::setOscillatorPan(std::size_t oscillator, float normalizedPan) noexcept
{
    voice.setOscillatorPan(oscillator, normalizedPan);
}

void SimpleSequencer::setOscillatorProximity(std::size_t oscillator, float normalizedProximity) noexcept
{
    voice.setOscillatorProximity(oscillator, normalizedProximity);
}

void SimpleSequencer::setOscillatorOrbit(std::size_t oscillator, float normalizedOrbit) noexcept
{
    voice.setOscillatorOrbit(oscillator, normalizedOrbit);
}

void SimpleSequencer::setEnergy(float normalizedEnergy) noexcept
{
    energy = std::clamp(normalizedEnergy, 0.0f, 1.0f);
    voice.setEnergy(energy);
}

void SimpleSequencer::setOscillatorCore(CmosVoice::OscillatorCore core) noexcept
{
    voice.setOscillatorCore(core);
}

void SimpleSequencer::setFilterCutoff(float normalizedCutoff) noexcept { filter.setCutoff(normalizedCutoff); filterRight.setCutoff(normalizedCutoff); }
void SimpleSequencer::setFilterResonance(float normalizedResonance) noexcept { filter.setResonance(normalizedResonance); filterRight.setResonance(normalizedResonance); }
void SimpleSequencer::setFilterCvDepth(float normalizedDepth) noexcept { filter.setCvDepth(normalizedDepth); filterRight.setCvDepth(normalizedDepth); }
void SimpleSequencer::setFilterModeMask(unsigned char mask) noexcept { filter.setModeMask(mask); filterRight.setModeMask(mask); }
void SimpleSequencer::setMaterialFilterCutoff(float normalizedCutoff) noexcept { materialFilter.setCutoff(normalizedCutoff); materialFilterRight.setCutoff(normalizedCutoff); }
void SimpleSequencer::setMaterialFilterResonance(float normalizedResonance) noexcept { materialFilter.setResonance(normalizedResonance); materialFilterRight.setResonance(normalizedResonance); }
void SimpleSequencer::setMaterialFilterDrive(float normalizedDrive) noexcept { materialFilter.setDrive(normalizedDrive); materialFilterRight.setDrive(normalizedDrive); }
void SimpleSequencer::setMaterialFilterAsymmetry(float normalizedAsymmetry) noexcept { materialFilter.setAsymmetry(normalizedAsymmetry); materialFilterRight.setAsymmetry(normalizedAsymmetry); }
void SimpleSequencer::setMaterialFilterMix(float normalizedMix) noexcept { materialFilter.setMix(normalizedMix); materialFilterRight.setMix(normalizedMix); }
void SimpleSequencer::setEnvelopeAttack(float normalizedTime) noexcept { envelope.setAttack(normalizedTime); }
void SimpleSequencer::setEnvelopeDecay(float normalizedTime) noexcept { envelope.setDecay(normalizedTime); }
void SimpleSequencer::setEnvelopeSustain(float normalizedLevel) noexcept { envelope.setSustain(normalizedLevel); }
void SimpleSequencer::setEnvelopeRelease(float normalizedTime) noexcept { envelope.setRelease(normalizedTime); }
void SimpleSequencer::setLfoRate(float hertz) noexcept { lfo.setRate(hertz); }
void SimpleSequencer::setLfoShape(LfoSource::Shape shape) noexcept { lfo.setShape(shape); }
void SimpleSequencer::setLfoFrozen(bool frozen) noexcept { lfo.setFrozen(frozen); }
void SimpleSequencer::reseedLfo() noexcept { lfo.reseed(); }
void SimpleSequencer::setLfoChaosDrive(float normalizedDrive) noexcept { lfo.setChaosDrive(normalizedDrive); }
void SimpleSequencer::setLfoChaosDamping(float normalizedDamping) noexcept { lfo.setChaosDamping(normalizedDamping); }
void SimpleSequencer::setLfoWanderDepth(float normalizedDepth) noexcept { lfo.setWanderDepth(normalizedDepth); }
void SimpleSequencer::setRingMix(float normalizedMix) noexcept { ring.setMix(normalizedMix); }
void SimpleSequencer::setNoiseMix(float normalizedMix) noexcept { noiseMix = std::clamp(normalizedMix, 0.0f, 0.42f); }
// Same 0.42 ceiling as noiseMix itself (see above) - noiseBreathAmount
// ADDS to noiseMix inside renderSample, not replaces it, so capping both
// the same way keeps the combined total in the same general range
// noiseMix alone was already tuned/verified against, rather than letting
// the sum run twice as hot.
void SimpleSequencer::setNoiseBreathAmount(float normalizedAmount) noexcept { noiseBreathAmount = std::clamp(normalizedAmount, 0.0f, 0.42f); }
void SimpleSequencer::setInstability(float normalizedAmount) noexcept { instability = std::clamp(normalizedAmount, 0.0f, 1.0f); }
void SimpleSequencer::setNoiseColour(NoisePalette::Colour colour) noexcept { noise.setColour(colour); noiseRight.setColour(colour); }
void SimpleSequencer::setSampleHoldRate(float hertz) noexcept { sampleHold.setRate(hertz); sampleHoldRight.setRate(hertz); }
void SimpleSequencer::setSampleHoldMix(float normalizedMix) noexcept { sampleHoldMix = std::clamp(normalizedMix, 0.0f, 1.0f); }
void SimpleSequencer::setReverbMix(float normalizedMix) noexcept { reverb.setMix(normalizedMix); reverbRight.setMix(normalizedMix); }
void SimpleSequencer::setReverbFeedback(float normalizedAmount) noexcept { reverb.setFeedback(normalizedAmount); reverbRight.setFeedback(normalizedAmount); }
void SimpleSequencer::setPhaserMix(float normalizedMix) noexcept { phaser.setMix(normalizedMix); phaserRight.setMix(normalizedMix); }
void SimpleSequencer::setPhaserRate(float hertz) noexcept { phaser.setRate(hertz); phaserRight.setRate(hertz); }
void SimpleSequencer::setPhaserDepth(float normalizedDepth) noexcept { phaser.setDepth(normalizedDepth); phaserRight.setDepth(normalizedDepth); }
void SimpleSequencer::setFlangerMix(float normalizedMix) noexcept { flanger.setMix(normalizedMix); flangerRight.setMix(normalizedMix); }
void SimpleSequencer::setFlangerRate(float hertz) noexcept { flanger.setRate(hertz); flangerRight.setRate(hertz); }
void SimpleSequencer::setFlangerDepth(float normalizedDepth) noexcept { flanger.setDepth(normalizedDepth); flangerRight.setDepth(normalizedDepth); }
void SimpleSequencer::setResonatorMix(float normalizedMix) noexcept { resonator.setMix(normalizedMix); resonatorRight.setMix(normalizedMix); }
void SimpleSequencer::setResonatorPitch(float normalizedPitch) noexcept { resonator.setPitch(normalizedPitch); resonatorRight.setPitch(normalizedPitch); }
void SimpleSequencer::setResonatorDamping(float normalizedDamping) noexcept { resonator.setDamping(normalizedDamping); resonatorRight.setDamping(normalizedDamping); }
void SimpleSequencer::setMixChannel(std::size_t channel, MutableMixer::Channel configuration) noexcept { mixer.setChannel(channel, configuration); }
MutableMixer::Channel SimpleSequencer::getMixChannel(std::size_t channel) const noexcept { return mixer.getChannel(channel); }
void SimpleSequencer::captureMixMemory(std::size_t slot) noexcept { mixer.capture(slot); }
void SimpleSequencer::recallMixMemory(std::size_t slot) noexcept { mixer.recall(slot); }

void SimpleSequencer::setStepMuted(std::size_t step, bool shouldMute) noexcept
{
    if (step < stepCount) muted[step] = shouldMute;
}

bool SimpleSequencer::isStepMuted(std::size_t step) const noexcept
{
    return step < stepCount && muted[step];
}

void SimpleSequencer::setStepVoltage(std::size_t step, float normalizedVoltage) noexcept
{
    if (step < stepCount)
        voltages[step] = std::clamp(normalizedVoltage, 0.0f, 1.0f);
}

float SimpleSequencer::getStepVoltage(std::size_t step) const noexcept
{
    return step < stepCount ? voltages[step] : 0.0f;
}

void SimpleSequencer::setStepLevel(std::size_t step, float normalizedLevel) noexcept
{
    if (step < stepCount) levels[step] = std::clamp(normalizedLevel, 0.0f, 1.0f);
}

float SimpleSequencer::getStepLevel(std::size_t step) const noexcept
{
    return step < stepCount ? levels[step] : 0.0f;
}

void SimpleSequencer::setStepEffectSend(std::size_t step, float normalizedSend) noexcept
{
    if (step < stepCount) effectSends[step] = std::clamp(normalizedSend, 0.0f, 1.0f);
}

float SimpleSequencer::getStepEffectSend(std::size_t step) const noexcept
{
    return step < stepCount ? effectSends[step] : 0.0f;
}

void SimpleSequencer::randomizeSteps(float amount) noexcept
{
    const auto blend = std::clamp(amount, 0.0f, 1.0f);
    const auto nextUnit = [this]() noexcept
    {
        randomState ^= randomState << 13U;
        randomState ^= randomState >> 17U;
        randomState ^= randomState << 5U;
        return static_cast<float>(randomState & 0x00ffffffU) / static_cast<float>(0x00ffffffU);
    };

    for (std::size_t step = 0; step < stepCount; ++step)
    {
        const auto cvTarget = nextUnit();
        const auto levelTarget = 0.24f + nextUnit() * 0.76f;
        const auto effectTarget = nextUnit();
        voltages[step] += (cvTarget - voltages[step]) * blend;
        levels[step] += (levelTarget - levels[step]) * blend;
        effectSends[step] += (effectTarget - effectSends[step]) * blend;
        muted[step] = blend > 0.82f && nextUnit() < 0.12f;
    }
}

void SimpleSequencer::setLoopEnd(std::size_t oneBasedStep) noexcept
{
    loopEnd = std::clamp(oneBasedStep, std::size_t { 1 }, stepCount);
    if (currentStep >= loopEnd) currentStep = scannerDirection == ScannerDirection::reverse ? loopEnd - 1 : 0;
}

void SimpleSequencer::reset() noexcept
{
    clockSamples = 0.0;
    voice.reset();
    filter.reset();
    filterRight.reset();
    materialFilter.reset();
    materialFilterRight.reset();
    envelope.reset();
    lfo.reset();
    noise.reset();
    noiseRight.reset();
    sampleHold.reset();
    sampleHoldRight.reset();
    reverb.reset();
    reverbRight.reset();
    phaser.reset();
    phaserRight.reset();
    flanger.reset();
    flangerRight.reset();
    resonator.reset();
    resonatorRight.reset();
    mixer.reset();
    mixReflux = 0.0f;
    noiseBreathEnvelope = 0.0f;
    noiseBreathRising = false;
    noiseGateWasHigh = false;
    accentTail = 0.0f;
    accentRotationPhase = 0.0f;
    accentRotation = 0U;
    accentFatigue = 0.0f;
    measureStepIndex = 0;
    eventBudgetSpent = 0.0f;
    ruleSilenceThisStep = false;
    ratchetActiveThisStep = false;
    ratchetSubIndex = 0;
    ratchetGain = 1.0f;
    leveler.reset();
    output.reset();
    currentStep = scannerDirection == ScannerDirection::reverse ? loopEnd - 1 : 0;
    scannerIncrement = scannerDirection == ScannerDirection::reverse ? -1 : 1;
}

void SimpleSequencer::renderSample(float& left, float& right, float externalSignal) noexcept
{
    if (running && clockSamples++ >= samplesPerStep())
    {
        clockSamples = 0.0;
        // Noise Field's ritmo axis (19 ago. 2026) - a small, bounded
        // timing wobble on the NEXT step's own threshold, scaled by the
        // shared instability value: up to ±6% of a step's duration at
        // full instability, 0 at instability's low resting value in
        // practice. Reuses randomState's existing xorshift stream rather
        // than adding a fourth RNG - it's already unused by the common
        // scanner directions (forward/reverse/pendulum only touch
        // currentStep directly; only memoryAddress calls nextUnit() on
        // this same state), so the correlation this introduces for
        // memoryAddress users is a narrow, accepted edge case, not a new
        // generator to maintain.
        if (instability > 0.0f)
        {
            randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
            const auto jitterRoll = static_cast<float>(randomState) * (1.0f / 2147483648.0f) - 1.0f;
            clockSamples -= static_cast<double>(jitterRoll * instability * 0.06f) * samplesPerStep();
        }
        advanceStep();
    }
    // Microevent / ratchet sub-clock (docs/PESQUISA_SEQUENCER_GENERATIVO.md,
    // seção 7.1, "Ratchets - repetições dentro de um passo com envelope
    // próprio"), 20 ago. 2026, autor: "microevent, pattern/motif". Só
    // age quando `advanceStep()` já decidiu que este passo ratcheia
    // (`ratchetActiveThisStep`, cobrado do event budget lá). Contagem
    // de sub-hits (2-4) derivada do próprio `levels[]` do passo - sem
    // um array novo, um passo mais "forte" naturalmente ratcheia mais.
    // `clockSamples` já reflete a posição real dentro do passo neste
    // ponto (incrementada pelo bloco acima); o sub-hit 0 coincide com o
    // disparo principal que `advanceStep()` já fez (ratchetSubIndex
    // começa em 0), então este bloco só re-dispara a partir do 1º
    // sub-hit seguinte - sem duplicar o ataque original.
    if (running && ratchetActiveThisStep)
    {
        const auto ratchetCount = std::clamp(2 + static_cast<int>(levels[currentStep] * 2.99f), 2, 4);
        const auto subStepSamples = samplesPerStep() / static_cast<double>(ratchetCount);
        const auto expectedSubIndex = std::min(ratchetCount - 1, static_cast<int>(clockSamples / subStepSamples));
        if (expectedSubIndex > ratchetSubIndex)
        {
            ratchetSubIndex = expectedSubIndex;
            envelope.trigger();
            // Decrescendo real (o "envelope de ratchet" clássico) - até
            // -70% de ganho no último sub-hit de um ratchet de 4.
            ratchetGain = 1.0f - static_cast<float>(ratchetSubIndex) / static_cast<float>(ratchetCount) * 0.7f;
        }
    }
    const auto gateStep = std::clamp(1.0f / static_cast<float>(sampleRate * 0.008), 0.0f, 1.0f);
    playGate = targetPlayGate > playGate ? std::min(targetPlayGate, playGate + gateStep)
                                         : std::max(targetPlayGate, playGate - gateStep);
    if (!running && playGate <= 0.0f)
    {
        output.reset();
        left = right = 0.0f;
        return;
    }
    // ruleSilenceThisStep (meta-sequenciador, item StepRule::silence) -
    // uma pausa PONTUAL desta passagem, separada de muted[] (que é uma
    // edição permanente do passo) - ver SimpleSequencer.h.
    const auto noteGate = running && !muted[currentStep] && !ruleSilenceThisStep;
    const auto contour = envelope.process(noteGate);
    // Same rising-edge self-trigger ContourEnvelope::process() just did
    // internally for contour above (`if (gate && !gateWasHigh)
    // trigger();`) - without this, noise stayed silent until the first
    // advanceStep() boundary (>= one full step's duration away), instead
    // of pulsing immediately on PLAY the way the voice itself does. Found
    // by a real regression test failing after the redesign below
    // ("NOISE channel ON with gain restored still lets NOISE MIX audibly
    // reach the output" - the test renders less than one step's worth of
    // samples, so advanceStep() alone never fired).
    if (noteGate && !noiseGateWasHigh) noiseBreathRising = true;
    noiseGateWasHigh = noteGate;
    // Noise breath envelope (docs/PESQUISA_RUIDO_GENERATIVO.md item #2,
    // ports RASGO_SYNTH/dsp/BreathExciter.hpp), REDESIGNED 19 ago. 2026
    // (author: "o noise deve entrar nos steps do sequencer sem sons
    // longos passando por cima dos acontecimentos"). No longer a
    // continuous attack/release FOLLOW of noteGate - that gate stays
    // true across many consecutive steps (only drops on a muted step or
    // STOP), so it rarely dipped between steps at all with typical
    // patterns, still reading as one long wash. Now a percussive "ping":
    // advanceStep() and the rising-edge check above set noiseBreathRising
    // true on every non-muted step (same trigger points as
    // envelope.trigger()/its own self-trigger), and this ramps quickly
    // to 1.0 (real attack time, NOT an instant same-sample jump - found
    // real, author: "gravei um audio e percebi que algo parece estar
    // clipando nos steps" - an unramped jump straight into noiseTotal
    // was a genuine discontinuity/click at every step), then falls back
    // to decaying once it arrives - tau sized as a fraction of the
    // CURRENT step's own duration (samplesPerStep(), tempo/feel/groove
    // already folded in), not a fixed millisecond count, so the burst
    // clears before the next step regardless of tempo instead of
    // bleeding into it.
    if (noiseBreathRising)
    {
        constexpr float noiseBreathAttackSeconds = 0.003f;
        const auto noiseBreathAttackCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (noiseBreathAttackSeconds * sampleRate)));
        noiseBreathEnvelope += (1.0f - noiseBreathEnvelope) * noiseBreathAttackCoeff;
        if (noiseBreathEnvelope > 0.98f) { noiseBreathEnvelope = 1.0f; noiseBreathRising = false; }
    }
    else
    {
        const auto noiseBreathTau = std::max(0.01, samplesPerStep() / sampleRate * 0.7 / 4.0);
        const auto noiseBreathCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (noiseBreathTau * sampleRate)));
        noiseBreathEnvelope -= noiseBreathEnvelope * noiseBreathCoeff;
    }
    const auto lfoSample = lfo.tick();
    const auto whiteLike = noise.tick();
    const auto heldNoise = sampleHold.process(whiteLike);
    const auto noiseSignal = whiteLike + (heldNoise - whiteLike) * sampleHoldMix;
    // Right-channel noise stream, differently seeded - used only for the
    // dedicated NOISE mixer channel below (noiseMono/its own reinjection),
    // never for the voiceLeft/voiceRight addition just below this, so the
    // already-verified FILTER/RING pan behaviour is untouched by it.
    const auto whiteLikeRight = noiseRight.tick();
    const auto heldNoiseRight = sampleHoldRight.process(whiteLikeRight);
    const auto noiseSignalRight = whiteLikeRight + (heldNoiseRight - whiteLikeRight) * sampleHoldMix;
    // Graduated, not the old binary (unit==8 ? 0.82 : 0.88) - that old pair
    // was only a ~1dB dip, inaudible once buried under contour/levels'
    // own swing (author, live, after the 1-16 range shipped: "não altera
    // nada no som" / "nenhum tempo forte ou fraco, fica identico"). Widened
    // to a real dip (0.35-0.65, roughly -3.7 to -9dB) so accented vs
    // unaccented steps actually read as strong/weak, while keeping the
    // same graduated response to the real 1-16 denominator (small unit ->
    // shallower dip, large unit -> deeper dip).
    constexpr float weakAccentAtUnit1 = 0.65f, weakAccentAtUnit16 = 0.35f;
    const auto unitPhase = (static_cast<float>(metricUnit) - 1.0f) / 15.0f;
    // Accent strength contínuo (docs/PESQUISA_ACENTUACAO_GENERATIVA.md
    // item "accent strength contínuo", 19 ago. 2026, author: "avance o
    // acento") - real prior art: AQUORBIUM's GeometricSequencer::Step::
    // accent is `(orbitStep==0 ? 0.92 : 0.58) + cohesion*0.18 +
    // (mutated?0.08:0)` - a base by position PLUS a continuous
    // contribution from real system state, not a fixed two-level
    // constant. Same shape here: the strong beat stays at 1.0 exactly
    // (no regression on the peak level already tuned live, "não altera
    // nada no som"/"nenhum tempo forte ou fraco" - both real past
    // incidents); the WEAK beat's own level now also varies continuously
    // with `instability` (the Noise Field, already shared/pushed in
    // every sample by DualObjectEngine) instead of being a flat function
    // of metricUnit alone - the same shared "weather" driving NOISE and
    // EXCITAÇÃO now also colours which weak beats read as more or less
    // present, tying the three systems together instead of leaving
    // ACENTO as an island.
    // + accentTail: accent inheritance (19 ago. 2026, brief item - "um
    // acento forte pode afetar os passos seguintes... gera uma cauda
    // dinâmica"). Set in advanceStep() only (0.3 right after a strong
    // beat, halved each step after) - only ever adds to the WEAK branch.
    // (currentStep + accentRotation): accent rotation (19 ago. 2026, see
    // accentRotation's own member comment) - which step index counts as
    // strong migrates slowly, metricBeats itself (the cycle length)
    // never changes.
    // 1.0f - accentFatigue: step fatigue (19 ago. 2026, see
    // accentFatigue's own member comment) - the strong beat's own peak
    // recedes a little the longer the SAME rotation position has held
    // it, capped at -0.3 (never below weakAccentAtUnit1's own 0.65
    // ceiling, so a tired strong beat still clearly outranks a weak
    // one), resetting to fully rested whenever accentRotation moves on.
    const auto metricAccent = (currentStep + accentRotation) % metricBeats == 0
        ? 1.0f - accentFatigue
        : std::clamp(weakAccentAtUnit1 - unitPhase * (weakAccentAtUnit1 - weakAccentAtUnit16) + instability * 0.12f + accentTail, 0.0f, 0.95f);
    // Rotation phase advances every sample, speed scaled by instability -
    // see accentRotation's own member comment for the full rationale.
    accentRotationPhase += instability * 0.000004f;
    if (accentRotationPhase >= 1.0f)
    {
        accentRotationPhase -= 1.0f;
        accentRotation = (accentRotation + 1U) % metricBeats;
        // Step fatigue resets when the strong-beat position itself
        // moves (see accentFatigue's own member comment) - a fresh
        // position is fully rested.
        accentFatigue = 0.0f;
    }
    // Accent Field, timbre destination (docs/
    // PESQUISA_ACENTUACAO_GENERATIVA.md "próximos passos" item 3, 19
    // ago. 2026, author: "sim" after a suggested design) - same
    // mechanism item #10 already validated for EXCITAÇÃO
    // (CmosVoice::setOscillatorShape mapped from a continuous source),
    // here nudging oscillator A's FORMA knob (oscillatorShapeBaseA, the
    // author's own setting - see its member comment) by metricAccent
    // instead of replacing it. Deliberately gentler than EXCITAÇÃO's own
    // #10 (±0.25 vs. a full 0-3 span there) - ACENTO's own range is
    // narrower (~0.35-1.0, not 0-1 across a whole register) and this is
    // meant to read as a seasoning on PRINCIPAL/CLONE's already-tuned
    // timbre, not a rewrite of it. Applied every sample, right before
    // tickStereo actually uses it.
    // Widened same day (author: "testei as ultimas implementações mas
    // não percebi as diferenças") - 0.25 was the same mistake already
    // made once before with MÉTRICA's own accent dip ("não altera nada
    // no som"), which only became audible after a real widening, not a
    // subtle one. 0.9, not 0.25 - still well short of EXCITAÇÃO's own
    // full 0-3 span (that voice's whole IDENTITY is its register-timbre
    // link; here it's a seasoning on an already-established sound), but
    // enough to actually read as brighter/darker between strong and weak
    // beats instead of being lost in the mix.
    voice.setOscillatorShape(0, std::clamp(oscillatorShapeBaseA + (metricAccent - 0.65f) * 0.9f, 0.0f, 3.0f));
    const auto voiceFrame = voice.tickStereo(voltages[currentStep], externalSignal * externalFeedbackAmount + mixReflux);
    // Step state gain (docs/PESQUISA_SEQUENCER_GENERATIVO.md, segundo
    // brief, tabela 7.3 - ver o comentário de stepHeat/advanceStep()
    // pro racional completo). Contínuo, não um switch discreto por
    // estado - evita clique audível ao cruzar limiar. dormant/active
    // (heat < 0.55) ficam em 1.0 (frescor normal, sem penalidade);
    // hot (0.55-0.85) ganha até +15%; exhausted (>0.85) recebe até -30%
    // por cima disso, então o pico fica em ~heat 0.85 e desce dali -
    // um passo tocado demais recede de verdade, não só para de crescer.
    const auto stepStateGain = 1.0f + std::clamp(stepHeat[currentStep] - 0.55f, 0.0f, 0.3f) * 0.5f
                                     - std::clamp(stepHeat[currentStep] - 0.85f, 0.0f, 0.15f) * 3.0f;
    // ratchetGain (Microevent/ratchet, ver o comentário do membro em
    // SimpleSequencer.h) - 1.0 fora de um passo `ratchet`/no sub-hit
    // principal, decrescendo real a cada sub-hit seguinte dentro da
    // MESMA duração do passo.
    const auto voiceGain = 0.42f * contour * playGate * levels[currentStep] * metricAccent * stepStateGain * ratchetGain;
    const auto voiceLeft = voiceFrame.left * voiceGain;
    const auto voiceRight = voiceFrame.right * voiceGain;
    // Gated by the mixer's own NOISE channel (index 2) - this pre-RING
    // injection used to run purely off noiseMix, ignoring the mixer's
    // ON/M/S state entirely, so turning the NOISE channel OFF (or muting
    // it, or soloing something else) still left this noise audible
    // through FILTER/RING as long as they were on. Found live, 18 ago.
    // 2026: author left NOISE MIX high, turned the mixer's NOISE channel
    // off, and still heard it ("quando ele está 'off' o noise deveria
    // ficar desligado") - the mixer's NOISE channel is meant to be the
    // master switch for noise, not just for its own dedicated bus.
    const auto noiseChannelActive = mixer.isChannelActive(2);
    // A flat *0.42f headroom trim was tried and REVERTED here earlier
    // the same day (19 ago. 2026) to address "quando clico em on no
    // mixer o som do noise surge muito forte" - noiseSignal (whiteLike,
    // or the S&H-held version when sampleHoldMix > 0) is the SAME signal
    // for both plain noise and S&H, so cutting noiseTotal's LEVEL
    // uniformly also cut S&H's own presence, which the author values at
    // its existing strength ("a função no som" - S&H read weaker after
    // the trim). Loudness needs a different lever than a DSP-level cut
    // that hits every use of noise alike (see setMixChannel's own
    // default gain / the mixer channel fader instead).
    //
    // noiseTotal now ALWAYS follows noiseBreathEnvelope (redesigned same
    // day, author: "o noise deve entrar nos steps do sequencer sem sons
    // longos passando por cima dos acontecimentos") - noiseMix is no
    // longer a flat, always-on level; it (and S&H's own held noise,
    // sharing the same signal) now pulses with each step the same way
    // the voice's own contour does, peaking at the SAME level as before
    // (envelope hits exactly 1.0 at each trigger - this is a TIMING
    // change, not a level cut, so it does not reintroduce the "fraquinho"
    // problem the reverted trim above caused). noiseBreathAmount (the BR
    // toggle) still adds extra depth on top of the same per-step pulse,
    // not a second independent behaviour.
    // + instability * 0.15f: Noise Field's timbre/textura axis (19 ago.
    // 2026) - a modest extra amount of noise presence as the shared
    // field rises, additive on top of whatever the author already set,
    // never overriding it (instability defaults to a low resting value,
    // see DualObjectEngine's own comment, so this stays small in
    // practice, not a constant loud addition).
    const auto noiseTotal = (noiseMix + noiseBreathAmount + instability * 0.15f) * noiseBreathEnvelope;
    const auto noiseGate = noiseChannelActive ? noiseTotal : 0.0f;
    // noiseInjection additionally scales by the NOISE channel's own gain
    // fader (0-1.5) - unlike noiseGate above (reused below for
    // noiseMono/the width term, where mixer.process() and the explicit
    // `* gain` already apply gain exactly once), this pre-RING path never
    // passes through the mixer at all, so it needed its own gain
    // scaling or the fader would only ever be an on/off switch here,
    // never a true fader (author, live, same report: "abaixo tudo... e
    // continuo a escutar chiado" - turning ON off worked, but gain alone
    // still didn't).
    const auto noiseInjection = noiseChannelActive ? noiseTotal * mixer.getChannel(2).gain : 0.0f;
    const auto ringedLeft = ring.process(voiceLeft + noiseSignal * noiseInjection, lfoSample);
    const auto ringedRight = ring.process(voiceRight + noiseSignal * noiseInjection, lfoSample);
    // Accent Field, articulação (docs/PESQUISA_ACENTUACAO_GENERATIVA.md
    // "próximos passos" item 3, 19 ago. 2026, author chose "articulação
    // (filtro/ataque)" over timbre or leaving it undone) - metricAccent
    // now also nudges the same filterCv contour already modulates, not
    // just voiceGain: strong beats (metricAccent=1.0) open the filter a
    // touch more, weak beats (down around 0.35-0.47 with instability)
    // close it a touch more - a real articulation difference between
    // strong/weak beats, not just louder/quieter. 0.65f is the same
    // reference weakAccentAtUnit1 already uses (roughly the old weak-
    // beat midpoint), so this reads as neutral at a typical weak beat
    // and only actually brightens on genuinely strong ones. Modest
    // (±0.1 scale) - an accent on articulation, not a retuned filter.
    // Widened same day (author: "testei as ultimas implementações mas
    // não percebi as diferenças") - 0.1 was too close to contour's own
    // 0.12 swing to read as a distinct articulation difference; 0.3 puts
    // it clearly ahead of contour's own contribution instead of getting
    // lost underneath it.
    const auto filterCv = std::clamp(voltages[currentStep] + contour * 0.12f + (metricAccent - 0.65f) * 0.3f, 0.0f, 1.0f);
    const auto vcfLeft = filter.process(ringedLeft, filterCv);
    const auto vcfRight = filterRight.process(ringedRight, filterCv);
    // MaterialFilter sits in series right after CmosVcf. Always runs - its
    // own internal MIX (default 0) already makes this exactly today's
    // CmosVcf-only signal at the quiet end, the same way REVERB/PHASER/
    // FLANGER have no separate on/off beyond their own MIX either.
    // Everything downstream (differential reinjection, ESPAÇO chain) reads
    // filteredLeft/Right same as before, so this is the only place the
    // stage needed to be spliced in.
    const auto filteredLeft = materialFilter.process(vcfLeft, filterCv);
    const auto filteredRight = materialFilterRight.process(vcfRight, filterCv);
    const auto ringed = (ringedLeft + ringedRight) * 0.5f;
    const auto filtered = (filteredLeft + filteredRight) * 0.5f;
    const auto send = effectSends[currentStep];
    // ESPAÇO now runs in true stereo (mirroring the filter/filterRight
    // pattern above) instead of collapsing to the mono `filtered` average
    // before the chain - reverb/phaser/flanger/resonator used to erase
    // whatever EIXO X pan FILTER/RING had just preserved, the moment a
    // patch routed through them (see TAREFAS.md, 17 ago. 2026).
    const auto phasedLeft = phaser.process(filteredLeft, send);
    const auto phasedRight = phaserRight.process(filteredRight, send);
    const auto flangedLeft = flanger.process(phasedLeft, send);
    const auto flangedRight = flangerRight.process(phasedRight, send);
    const auto spatialLeft = reverb.process(flangedLeft, send);
    const auto spatialRight = reverbRight.process(flangedRight, send);
    // Comb/resonator sits after the reverb, still inside the ESPAÇO bus -
    // distinct processing stage (pitched cavity, not diffuse ambience), not
    // a fifth mixer channel of its own.
    const auto resonatedLeft = resonator.process(spatialLeft, send);
    const auto resonatedRight = resonatorRight.process(spatialRight, send);
    const auto resonated = (resonatedLeft + resonatedRight) * 0.5f;
    // NOISE isn't tied to any single oscillator's EIXO X - it's a separate
    // module, so there is no pan to "preserve" here the way there is for
    // FILTER/RING/SPACE above. noiseRight only gives it its own decorrelated
    // width (not collapsed to a dead-centre mono blob sitting on top of the
    // panned image); it does not make it follow any oscillator's position.
    const auto noiseMono = (noiseSignal + noiseSignalRight) * 0.5f * noiseGate;
    float mixedLeft = 0.0f, mixedRight = 0.0f;
    mixer.process({ filtered, ringed, noiseMono, resonated }, mixedLeft, mixedRight, mixReflux);
    // The mixer receives a stable mono sum for its material channels, while the
    // differential part of the three oscillator voices remains stereo. Gated
    // by FILTER's own ON/M/S (channel 0) - this used to add unconditionally,
    // leaking a faint but real signal even with FILTER/RING/NOISE/SPACE all
    // switched off (author, live: "abaixei todos os volumes... no entanto o
    // clone continua a funcionar baixinho").
    // 0.72 measured as very close to the crossover point where the "quiet"
    // side of a hard pan reaches minimum magnitude (a direct engine probe:
    // raising this to 1.8 made the L/R ratio WORSE, not better - 0.72
    // already sits near the sweet spot for a single isolated oscillator).
    // Each block below now also scales by that channel's own gain fader
    // (`mixer.getChannel(i).gain`, 0-1.5), not just its ON/M/S state -
    // previously these 4 blocks were gated ONLY by isChannelActive(), so
    // dragging a fader to 0 while leaving the channel ON left this
    // stereo-width/pan-preserving term playing at full strength regardless
    // (only toggling OFF/mute actually silenced it). Found live, 18 ago.
    // 2026: author left NOISE MIX high and pulled every NOISE mixer fader
    // down and still heard it ("continuo a escutar chiado") - the mono sum
    // above (mixer.process()) DID respect the fader, but this differential
    // term didn't, so the residual noise was this term alone. Same bug
    // existed for FILTER/RING/ESPAÇO, just never reported because their
    // faders are rarely pulled to 0 while still ON.
    if (mixer.isChannelActive(0))
    {
        const auto gain = mixer.getChannel(0).gain;
        mixedLeft += (filteredLeft - filtered) * 0.72f * gain;
        mixedRight += (filteredRight - filtered) * 0.72f * gain;
    }
    // Same trick, RING's own channel (index 1) - without this, AXIS X (and
    // Y/Z) panning was only ever audible through FILTER's differential
    // above; RING carried nothing but the mono sum, so panning read as
    // "not working" whenever a patch leaned on RING over FILTER (author,
    // live: "o pan dos osciladores não está funcionando").
    if (mixer.isChannelActive(1))
    {
        const auto gain = mixer.getChannel(1).gain;
        mixedLeft += (ringedLeft - ringed) * 0.72f * gain;
        mixedRight += (ringedRight - ringed) * 0.72f * gain;
    }
    // NOISE (index 2) - decorrelated width, not oscillator pan; see comment
    // on noiseMono above.
    if (mixer.isChannelActive(2))
    {
        const auto gain = mixer.getChannel(2).gain;
        mixedLeft += (noiseSignal * noiseGate - noiseMono) * 0.72f * gain;
        mixedRight += (noiseSignalRight * noiseGate - noiseMono) * 0.72f * gain;
    }
    // ESPAÇO (index 3) - closes the gap documented in TAREFAS.md (17 ago.
    // 2026): this channel used to be the one mono-only path that could
    // dilute a patch's pan down to the mixer's own centred default, even
    // with FILTER/RING correctly carrying it right up to this point.
    if (mixer.isChannelActive(3))
    {
        const auto gain = mixer.getChannel(3).gain;
        mixedLeft += (resonatedLeft - resonated) * 0.72f * gain;
        mixedRight += (resonatedRight - resonated) * 0.72f * gain;
    }
    // Nivelamento upstream + proteção de sobrecarga vinculada, antes do teto
    // técnico final de OutputStage - ver SignalLeveler.h/LinkedOverloadProtector.h.
    const auto levelGain = leveler.process(0.5f * (mixedLeft + mixedRight));
    float leveledLeft = 0.0f, leveledRight = 0.0f;
    overloadProtector.process(mixedLeft * levelGain, mixedRight * levelGain, leveledLeft, leveledRight);
    output.process(leveledLeft, leveledRight, left, right);
}

void SimpleSequencer::render(float* left, float* right, std::size_t samples) noexcept
{
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        float leftSample = 0.0f, rightSample = 0.0f;
        renderSample(leftSample, rightSample);
        if (left != nullptr) left[sample] = leftSample;
        if (right != nullptr) right[sample] = rightSample;
    }
}

void SimpleSequencer::advanceStep() noexcept
{
    // Weighted walk (docs/PESQUISA_SEQUENCER_GENERATIVO.md, segundo
    // brief, 19 ago. 2026 - "o playhead tem maior probabilidade de
    // continuar próximo: 4→5 60%, 4→3 20%, 4→7 15%, 4→1 5%... muito
    // diferente de random"), used only by ScannerDirection::
    // memoryAddress below. Two independent uniform rolls, subtracted -
    // a real, simple technique (Irwin-Hall-style: the difference/sum of
    // uniforms approximates a triangular/bell shape) that peaks at 0
    // and falls off toward the extremes, instead of a flat uniform pick
    // across the whole loop. currentStep itself is the centre the walk
    // is measured from, so nearby steps are genuinely more likely than
    // distant ones, not just "equally likely, happens to be close
    // sometimes".
    const auto nextUnit = [this]() noexcept
    {
        // One state update, same cost as before this technique existed -
        // rollA/rollB come from different bit ranges of the SAME
        // update rather than two separate updates, so this consumes
        // randomState at exactly the same rate the old plain-uniform
        // version did.
        randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
        const auto loopEndU = static_cast<unsigned int>(loopEnd);
        const auto rollA = static_cast<int>(randomState % loopEndU);
        const auto rollB = static_cast<int>((randomState >> 12U) % loopEndU);
        const auto offset = rollA - rollB;
        auto next = (static_cast<int>(currentStep) + offset) % static_cast<int>(loopEnd);
        if (next < 0) next += static_cast<int>(loopEnd);
        return static_cast<std::size_t>(next);
    };

    switch (scannerDirection)
    {
        case ScannerDirection::reverse:
            currentStep = currentStep == 0 ? loopEnd - 1 : currentStep - 1;
            break;
        case ScannerDirection::pendulum:
            if (loopEnd == 1) { currentStep = 0; break; }
            if (scannerIncrement > 0 && currentStep + 1 >= loopEnd) scannerIncrement = -1;
            else if (scannerIncrement < 0 && currentStep == 0) scannerIncrement = 1;
            currentStep = static_cast<std::size_t>(static_cast<int>(currentStep) + scannerIncrement);
            break;
        case ScannerDirection::memoryAddress:
        {
            const auto previous = currentStep;
            auto candidate = nextUnit();
            // Repelente (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção
            // 7.3) - um passo muito visitado recentemente (stepHeat
            // alto, o mesmo sinal que já reduz o próprio ganho de
            // áudio via stepStateGain - "um valor compartilhado lido
            // por vários destinos", mesmo princípio já usado pro Noise
            // Field/geometricAccent) fica temporariamente menos
            // convidativo: um segundo sorteio troca o candidato, sem
            // laço - no máximo dois nextUnit() por advanceStep(), custo
            // previsível.
            if (stepHeat[candidate] > 0.7f) candidate = nextUnit();
            // Inércia - um puxão suave (não uma regra rígida) a favor
            // de continuar na mesma direção do último salto real, ver o
            // comentário do membro `scannerMomentumDirection` em
            // SimpleSequencer.h. ~30% do tempo, se o candidato reverteria
            // a direção anterior, reflete o salto de volta a favor dela.
            randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
            const auto momentumRoll = static_cast<float>(randomState % 1000U) / 1000.0f;
            const auto loopEndI = static_cast<int>(loopEnd);
            const auto candidateOffset = static_cast<int>(candidate) - static_cast<int>(previous);
            if (candidateOffset != 0 && momentumRoll < 0.3f
                && (candidateOffset > 0) != (scannerMomentumDirection > 0))
            {
                auto reflected = static_cast<int>(previous) - candidateOffset;
                reflected = ((reflected % loopEndI) + loopEndI) % loopEndI;
                candidate = static_cast<std::size_t>(reflected);
            }
            // Atrator - tempos fortes (a mesma posição que já organiza
            // acento/MÉTRICA no resto do instrumento) puxam o cursor pra
            // perto de si, em vez de sorteados com o mesmo peso de
            // qualquer outro passo. Um sorteio pequeno decide SE o
            // atrator age nesta chamada (não sempre - um atrator
            // constante prenderia o playhead ali); quando age, acha o
            // tempo forte mais próximo (laço limitado a `loopEnd`
            // passos, sem alocação, distância medida no toróide - o
            // caminho mais curto entre voltar ou avançar) e move o
            // candidato UM passo na direção dele, não direto pra cima -
            // um puxão, não um teleporte.
            randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
            const auto attractorRoll = static_cast<float>(randomState % 1000U) / 1000.0f;
            if (attractorRoll < 0.18f && metricBeats > 0)
            {
                auto nearestStrongBeat = candidate;
                auto bestDistance = loopEndI;
                for (std::size_t probe = 0; probe < loopEnd; ++probe)
                {
                    if ((probe + accentRotation) % metricBeats != 0) continue;
                    auto diff = static_cast<int>(probe) - static_cast<int>(candidate);
                    diff = ((diff % loopEndI) + loopEndI) % loopEndI;
                    if (diff > loopEndI / 2) diff -= loopEndI;
                    const auto distance = std::abs(diff);
                    if (distance < bestDistance) { bestDistance = distance; nearestStrongBeat = probe; }
                }
                if (bestDistance > 0 && bestDistance < loopEndI)
                {
                    auto diff = static_cast<int>(nearestStrongBeat) - static_cast<int>(candidate);
                    diff = ((diff % loopEndI) + loopEndI) % loopEndI;
                    if (diff > loopEndI / 2) diff -= loopEndI;
                    const auto towardStrongBeat = diff > 0 ? 1 : -1;
                    candidate = static_cast<std::size_t>((static_cast<int>(candidate) + towardStrongBeat + loopEndI) % loopEndI);
                }
            }
            currentStep = candidate;
            if (loopEnd > 1 && currentStep == previous) currentStep = (currentStep + 1) % loopEnd;
            scannerMomentumDirection = (static_cast<int>(currentStep) - static_cast<int>(previous)) >= 0 ? 1 : -1;
            break;
        }
        case ScannerDirection::forward:
            currentStep = currentStep + 1 >= loopEnd ? 0 : currentStep + 1;
            break;
    }
    // Compasso real (ver o comentário do membro `measureStepIndex` em
    // SimpleSequencer.h) - avança em TODO advanceStep(), independente
    // do modo de scanner e de `loopEnd` - um compasso é uma contagem de
    // TEMPOS reais, não de passos do grid. Envolve sozinho ao cruzar
    // `stepsPerMeasure`, resetando o event budget no mesmo instante -
    // fim de compasso É o gatilho de reset, não um cooldown separado.
    ++measureStepIndex;
    if (measureStepIndex >= getStepsPerMeasure())
    {
        measureStepIndex = 0;
        eventBudgetSpent = 0.0f;
    }
    // Meta-sequenciador / sequenciador de regras (docs/
    // PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1; ver o comentário do
    // enum `StepRule` em SimpleSequencer.h), 20 ago. 2026, autor:
    // "meta-sequenciador/regras". Sorteio autônomo de regra NOVA pro
    // passo recém-alcançado, só com `metaSequencerAmount>0` ("0 = off
    // entirely" - com 0, `stepRules` nunca sai de `normal`, patches
    // existentes tocam exatamente como antes). `normal` continua o
    // peso maior mesmo em amount cheio (45%, reduzido de 55% quando as
    // 3 regras novas de Microevent/Pattern-Motif entraram - ver o
    // comentário do enum `StepRule`) - a maioria dos passos segue
    // tocando sua própria nota, só uma minoria vira comando.
    if (metaSequencerAmount > 0.0f)
    {
        randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
        const auto ruleDriftRoll = static_cast<float>(randomState % 1000U) / 1000.0f;
        if (ruleDriftRoll < 0.02f * metaSequencerAmount)
        {
            randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
            const auto pickRoll = static_cast<float>(randomState % 1000U) / 1000.0f;
            stepRules[currentStep] = pickRoll < 0.45f ? StepRule::normal
                                    : pickRoll < 0.60f ? StepRule::mutate
                                    : pickRoll < 0.70f ? StepRule::silence
                                    : pickRoll < 0.78f ? StepRule::rotate
                                    : pickRoll < 0.88f ? StepRule::ratchet
                                    : pickRoll < 0.94f ? StepRule::invert
                                    : StepRule::retrograde;
        }
    }
    // Executa a regra do passo atual - sempre, mesmo com amount=0 (pra
    // `setStepRule()` chamado de fora continuar funcionando
    // independente do sorteio autônomo acima). `silence` é de graça
    // (reduz complexidade, não soma - "complexidade ≠ densidade");
    // `mutate`/`rotate` gastam do MESMO event budget que DERIVA já usa
    // em Main.cpp (`hasEventBudget()`/`spendEventBudget()`,
    // PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1) - disputam o mesmo
    // orçamento por compasso, não uma reserva própria.
    ruleSilenceThisStep = false;
    // Microevent/ratchet - reseta a CADA passo alcançado, mesmo quando a
    // regra não é `ratchet` (senão um `ratchetSubIndex` velho vazaria
    // pro próximo passo que virasse ratchet). O disparo de verdade dos
    // sub-hits vive no sub-clock de `renderSample()`, que só age quando
    // `ratchetActiveThisStep` está ligado.
    ratchetActiveThisStep = false;
    ratchetSubIndex = 0;
    ratchetGain = 1.0f;
    switch (stepRules[currentStep])
    {
        case StepRule::silence:
            ruleSilenceThisStep = true;
            break;
        case StepRule::ratchet:
            // Custo cobrado UMA VEZ por passagem (não por sub-hit) -
            // mesmo padrão de mutate/rotate. Contagem de sub-hits (2-4)
            // derivada do próprio `levels[]` do passo, sem um array
            // novo - um passo mais "forte" naturalmente ratcheia mais.
            if (hasEventBudget(0.15f)) { spendEventBudget(0.15f); ratchetActiveThisStep = true; }
            break;
        case StepRule::invert:
            // Inversão melódica real - espelha só voltages[] (ALTURA,
            // não acento/gate/send) em torno de 0.5, dentro dos passos
            // ATIVOS. Mais barata que rotate/retrograde (não rearranja
            // ordem, só reflete valor).
            if (hasEventBudget(0.20f))
            {
                spendEventBudget(0.20f);
                for (std::size_t i = 0; i < loopEnd; ++i)
                    voltages[i] = std::clamp(1.0f - voltages[i], 0.0f, 1.0f);
            }
            break;
        case StepRule::retrograde:
            // O motivo tocado de trás pra frente - mesmo conjunto de
            // arrays que `rotate` já usa (inclusive stepRules, mesma
            // ideia auto-referente), std::reverse em vez de
            // std::rotate.
            if (loopEnd > 1 && hasEventBudget(0.30f))
            {
                spendEventBudget(0.30f);
                const auto activeEnd = static_cast<int>(loopEnd);
                std::reverse(voltages.begin(), voltages.begin() + activeEnd);
                std::reverse(levels.begin(), levels.begin() + activeEnd);
                std::reverse(effectSends.begin(), effectSends.begin() + activeEnd);
                std::reverse(muted.begin(), muted.begin() + activeEnd);
                std::reverse(stepRules.begin(), stepRules.begin() + activeEnd);
            }
            break;
        case StepRule::mutate:
            if (hasEventBudget(0.12f))
            {
                spendEventBudget(0.12f);
                randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
                const auto driftA = static_cast<float>(randomState % 1000U) / 1000.0f - 0.5f;
                randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
                const auto driftB = static_cast<float>(randomState % 1000U) / 1000.0f - 0.5f;
                randomState ^= randomState << 13U; randomState ^= randomState >> 17U; randomState ^= randomState << 5U;
                const auto driftC = static_cast<float>(randomState % 1000U) / 1000.0f - 0.5f;
                voltages[currentStep] = std::clamp(voltages[currentStep] + driftA * 0.12f, 0.0f, 1.0f);
                levels[currentStep] = std::clamp(levels[currentStep] + driftB * 0.12f, 0.0f, 1.0f);
                effectSends[currentStep] = std::clamp(effectSends[currentStep] + driftC * 0.12f, 0.0f, 1.0f);
            }
            break;
        case StepRule::rotate:
            // Só os passos ATIVOS ([0, loopEnd)) - inclusive as próprias
            // regras, então uma regra `rotate` migra pra frente com o
            // tempo, um efeito auto-referente real (a regra que causa a
            // rotação também é rotacionada).
            if (loopEnd > 1 && hasEventBudget(0.30f))
            {
                spendEventBudget(0.30f);
                const auto activeEnd = static_cast<int>(loopEnd);
                std::rotate(voltages.begin(), voltages.begin() + 1, voltages.begin() + activeEnd);
                std::rotate(levels.begin(), levels.begin() + 1, levels.begin() + activeEnd);
                std::rotate(effectSends.begin(), effectSends.begin() + 1, effectSends.begin() + activeEnd);
                std::rotate(muted.begin(), muted.begin() + 1, muted.begin() + activeEnd);
                std::rotate(stepRules.begin(), stepRules.begin() + 1, stepRules.begin() + activeEnd);
            }
            break;
        case StepRule::normal:
        default:
            break;
    }
    // Accent inheritance (docs/PESQUISA_ACENTUACAO_GENERATIVA.md, 19
    // ago. 2026, brief item: "um acento forte pode afetar os passos
    // seguintes... o acento principal gera uma cauda dinâmica"). Same
    // strong-beat test metricAccent's own peak uses, not gated by
    // muted[] - a real step position, whether or not this particular
    // step happens to sound. Halved each step after (not reset to 0
    // outright), so the tail fades over 2-3 steps rather than only ever
    // reaching the very next one.
    const auto strongBeatNow = (currentStep + accentRotation) % metricBeats == 0;
    accentTail = strongBeatNow ? 0.3f : accentTail * 0.5f;
    // Step fatigue (see accentFatigue's own member comment) - grows a
    // little each time the CURRENT strong-beat position fires, capped
    // so its peak never recedes below where the weak beat's own top
    // already reaches (weakAccentAtUnit1 = 0.65, see renderSample()) -
    // a tired strong beat still clearly outranks a weak one.
    if (strongBeatNow) accentFatigue = std::min(accentFatigue + 0.03f, 0.3f);
    // Step state - dormant/active/hot/exhausted (docs/
    // PESQUISA_SEQUENCER_GENERATIVO.md, segundo brief, tabela 7.3:
    // "Step state... não feito para steps do ANTITOTEM" - accentFatigue
    // acima é ligado à POSIÇÃO métrica, não à identidade do passo; isto
    // é o eixo que faltava, por índice de passo). stepHeat[] esfria
    // devagar TODO advance (mesmo em passos mutados ou não visitados -
    // um passo esquecido volta a "dormant" com o tempo), esquenta só
    // quando o passo REALMENTE soa (dentro do "if (!muted[...])"
    // abaixo) - repetição vira pressão real: tocado demais recede
    // (exhausted), na frequência certa ganha um pouco de vida (hot),
    // sem penalizar dormant/active (frescor normal). Aplicado em
    // renderSample() via stepStateGain, não aqui.
    for (auto& heat : stepHeat) heat *= 0.985f;
    // ruleSilenceThisStep (meta-sequenciador) - se esta passagem foi
    // silenciada pela regra do passo, ela não "realmente soou": não
    // esquenta stepHeat nem re-dispara envelope/noise breath, mesma
    // lógica que muted[] já usava aqui.
    if (!muted[currentStep] && !ruleSilenceThisStep)
    {
        stepHeat[currentStep] = std::min(stepHeat[currentStep] + 0.16f, 1.0f);
        envelope.trigger();
        // Gancho de exportação MIDI (ver `didStepSoundSincePoll()` em
        // SimpleSequencer.h) - grava o pitch/nível do disparo que
        // acabou de acontecer, pra quem for exportar ler depois.
        stepSoundedPending = true;
        lastSoundingPitch01 = voltages[currentStep];
        lastSoundingLevel = levels[currentStep];
        // Noise breath retrigger (19 ago. 2026, author: "o noise deve
        // entrar nos steps do sequencer sem sons longos passando por
        // cima dos acontecimentos") - a percussive "ping" on every
        // non-muted step, same trigger point as the voice's own
        // envelope.trigger() just above, not a level that follows a
        // mostly-always-true gate. An earlier version tried following
        // (running && !muted[currentStep]) as a continuous attack/
        // release target - that gate stays true for many consecutive
        // steps in a row (it only drops on a muted step or STOP), so in
        // the common case of few/no muted steps it never actually
        // dipped between steps at all, still reading as one long wash -
        // exactly what the author was pointing out. Retriggering to 1.0
        // here and decaying every sample in renderSample() (proportional
        // to the step's own duration, so it clears before the NEXT step
        // regardless of tempo) is what actually makes noise read as
        // "entering with the step" instead of sitting over it. Sets the
        // RISING flag, not the envelope value itself directly (fixed 19
        // ago. 2026, same day - an instant jump to 1.0 here was a real
        // discontinuity/click every step; renderSample() now ramps to
        // 1.0 over a fast ~3ms attack instead - see noiseBreathRising's
        // own comment in SimpleSequencer.h).
        noiseBreathRising = true;
    }
}
double SimpleSequencer::samplesPerStep() const noexcept
{
    const auto supplyClock = 0.46 + static_cast<double>(energy) * 0.78;
    const auto tupleDuration = [this] () noexcept
    {
        switch (clockFeel)
        {
            // Real tuplet ratios (20 ago. 2026, author asked directly:
            // "chegou a verificar na teoria da música como funciona a
            // divisão de tercinas, quintinas, etc?" - the honest answer
            // was no, an earlier pass used an invented (n-1)/n formula,
            // then an arbitrary evenly-spread one, neither checked against
            // real notation theory). Standard convention: n notes fit in
            // the time of the nearest lower power of 2 - triplet 3-in-2,
            // quintuplet 5-in-4, septuplet 7-in-4, nonuplet 9-in-8,
            // undecuplet 11-in-8.
            case ClockFeel::triplet: return 2.0 / 3.0;
            case ClockFeel::quintuplet: return 4.0 / 5.0;
            case ClockFeel::septuplet: return 4.0 / 7.0;
            case ClockFeel::nonuplet: return 8.0 / 9.0;
            case ClockFeel::undecuplet: return 8.0 / 11.0;
            // SWING, not a sextuplet ratio: 6-in-4 (two back-to-back
            // triplets) is mathematically identical to triplet's own 2/3,
            // so as a plain speed multiplier it would be indistinguishable
            // from tercina (author: "encontre outra divisão para sextina
            // que seja diferente da tercina", then, weighing the fraction
            // route against a second option offered in the same breath -
            // "ou encontre outra ideia além do glitch", then confirmed
            // "no lugar da sextina insira a ideia de swing" - this is the
            // resulting enum value, not a disguised tuplet). A 2-step
            // long-short alternation (classic 2:1 swing/shuffle, real
            // rhythmic vocabulary of its own, distinct from both a flat
            // tuplet speed-up and from GLITCH's own irregular 8-step
            // pattern below).
            // Fixed again, not adjustable (20 ago. 2026, author: "deixa o
            // swing somente enquanto botão" - the slider that briefly
            // lived here became GROOVE instead, a general modifier applied
            // below regardless of which feel is active, not exclusive to
            // this button).
            case ClockFeel::swing: return currentStep % 2 == 0 ? 1.0 : 0.5;
            case ClockFeel::glitch:
            {
                // A fixed, cycle-relative instability: it remains legible and
                // repeatable, rather than becoming a non-musical clock RNG.
                constexpr std::array<double, 8> pattern { 1.0, 0.50, 1.0, 1.50, 0.75, 1.25, 0.50, 1.0 };
                return pattern[currentStep % pattern.size()];
            }
            case ClockFeel::straight: break;
        }
        return 1.0;
    }();
    // GROOVE - a small long-short alternation layered on top of
    // tupleDuration regardless of which SUBDIVISÃO feel is active (20
    // ago. 2026, author: "utilise esse slide atual do swing para o
    // groove"). Tempo-preserving: even+odd sums to 2.0 (same as two
    // straight steps) no matter the amount, so it colours the feel
    // without dragging the overall rate.
    const auto grooveEven = 1.0 + static_cast<double>(grooveAmount) * 0.5;
    const auto grooveOdd = 1.0 - static_cast<double>(grooveAmount) * 0.5;
    const auto groove = currentStep % 2 == 0 ? grooveEven : grooveOdd;
    return sampleRate / (clockRate * supplyClock) * tupleDuration * groove;
}

double SimpleSequencer::getAverageSamplesPerStep() const noexcept
{
    // Mesma fórmula de `supplyClock` que `samplesPerStep()` já usa -
    // ver o comentário completo lá (ENERGIA muda a taxa real de passo
    // de 0.46x a 1.24x, não um detalhe cosmético).
    const auto supplyClock = 0.46 + static_cast<double>(energy) * 0.78;
    // Média real do multiplicador de SUBDIVISÃO por feel - constante
    // pros tuplets de verdade (tercina/quintina/septina/nonina/
    // undecina, já que `tupleDuration` não varia por passo nesses
    // casos); média medida do próprio padrão pra GLITCH (8 valores,
    // ver `samplesPerStep()`); média de SWING (alterna 1.0/0.5,
    // portanto 0.75 - diferente de GROOVE, que é tempo-preservante por
    // desenho e por isso nem entra aqui). Manter em sincronia com o
    // `switch` de `tupleDuration` dentro de `samplesPerStep()` se esse
    // switch mudar.
    const auto averageTupleDuration = [this] () noexcept
    {
        switch (clockFeel)
        {
            case ClockFeel::triplet: return 2.0 / 3.0;
            case ClockFeel::quintuplet: return 4.0 / 5.0;
            case ClockFeel::septuplet: return 4.0 / 7.0;
            case ClockFeel::nonuplet: return 8.0 / 9.0;
            case ClockFeel::undecuplet: return 8.0 / 11.0;
            case ClockFeel::swing: return 0.75;
            case ClockFeel::glitch: return (1.0 + 0.50 + 1.0 + 1.50 + 0.75 + 1.25 + 0.50 + 1.0) / 8.0;
            case ClockFeel::straight: break;
        }
        return 1.0;
    }();
    return sampleRate / (clockRate * supplyClock) * averageTupleDuration;
}
} // namespace antitotem
