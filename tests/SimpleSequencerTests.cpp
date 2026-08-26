#include "core/SimpleSequencer.h"
#include "core/DualObjectEngine.h"
#include "core/ContourEnvelope.h"
#include "core/MaterialEffects.h"
#include "core/ModulationSources.h"
#include "core/OutputStage.h"
#include "core/MaterialFilter.h"
#include "core/ChaosSources.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
}

int main()
{
    antitotem::ContourEnvelope contour;
    contour.prepare(1000.0);
    contour.setAttack(0.0f); contour.setDecay(0.0f); contour.setSustain(0.30f); contour.setRelease(0.0f);
    const auto attackFirst = contour.process(true);
    const auto attackPeak = contour.process(true);
    require(attackFirst > 0.0f && attackFirst < 1.0f, "ADSR attack rises rather than jumping directly to peak");
    require(attackPeak == 1.0f, "ADSR reaches its attack peak");
    const auto decayValue = contour.process(true);
    require(decayValue < 1.0f && decayValue >= 0.30f, "ADSR decay leaves the peak toward sustain");
    const auto sustained = contour.process(true);
    require(std::abs(sustained - 0.30f) < 0.0001f, "ADSR holds the requested sustain level");
    float released = contour.process(false);
    for (int i = 0; i < 16; ++i) released = contour.process(false);
    require(released == 0.0f, "ADSR release reaches digital silence in its selected duration");
    (void) contour.process(true);
    const auto retriggered = contour.process(true);
    require(retriggered > 0.0f, "ADSR retriggers after a completed release");

    antitotem::SimpleSequencer sequencer;
    sequencer.prepare(100.0);
    sequencer.setClockRate(1.0);
    sequencer.setRunning(true);
    std::vector<float> clockWindow(110);
    sequencer.render(clockWindow.data(), nullptr, 70);
    require(sequencer.getCurrentStep() == 0, "the selected channel is held before the energy-shaped clock period ends");
    sequencer.render(clockWindow.data(), nullptr, 40);
    require(sequencer.getCurrentStep() == 1, "the mux address advances after one energy-shaped clock period");
    sequencer.setLoopEnd(1);
    sequencer.render(clockWindow.data(), nullptr, 110);
    require(sequencer.getCurrentStep() == 0, "a one-step return holds the scanner at its chosen origin");
    sequencer.setLoopEnd(antitotem::SimpleSequencer::stepCount);
    sequencer.reset();
    sequencer.setLoopEnd(3);
    sequencer.render(clockWindow.data(), nullptr, 110);
    sequencer.render(clockWindow.data(), nullptr, 110);
    sequencer.render(clockWindow.data(), nullptr, 110);
    require(sequencer.getCurrentStep() == 0, "reset-on-step creates a shorter mutable trajectory without removing CVs");
    sequencer.setLoopEnd(99);
    require(sequencer.getLoopEnd() == antitotem::SimpleSequencer::stepCount, "loop end is safely constrained to the scanner range");

    antitotem::SimpleSequencer scanner;
    scanner.prepare(100.0); scanner.setClockRate(1.0); scanner.setLoopEnd(3); scanner.setRunning(true);
    scanner.setScannerDirection(antitotem::SimpleSequencer::ScannerDirection::reverse);
    require(scanner.getCurrentStep() == 2, "reverse scanner begins at the final active address");
    scanner.render(clockWindow.data(), nullptr, 110);
    require(scanner.getCurrentStep() == 1, "reverse scanner descends through the selected loop");
    scanner.setScannerDirection(antitotem::SimpleSequencer::ScannerDirection::pendulum);
    scanner.reset(); scanner.render(clockWindow.data(), nullptr, 110);
    require(scanner.getCurrentStep() == 1, "pendulum scanner begins in the forward direction");
    scanner.render(clockWindow.data(), nullptr, 110);
    require(scanner.getCurrentStep() == 2, "pendulum scanner reaches the last active address");
    scanner.render(clockWindow.data(), nullptr, 110);
    require(scanner.getCurrentStep() == 1, "pendulum scanner returns without wrapping abruptly");
    scanner.setScannerDirection(antitotem::SimpleSequencer::ScannerDirection::memoryAddress);
    for (int tick = 0; tick < 8; ++tick)
    {
        const auto before = scanner.getCurrentStep();
        scanner.render(clockWindow.data(), nullptr, 110);
        require(scanner.getCurrentStep() < 3, "memory addressing remains inside the active loop");
        require(scanner.getCurrentStep() != before, "memory addressing avoids immediate address repetition");
    }

    antitotem::SimpleSequencer tuplets;
    tuplets.prepare(100.0); tuplets.setClockRate(1.0); tuplets.setRunning(true);
    std::vector<float> temporalWindow(128);
    tuplets.setClockFeel(antitotem::SimpleSequencer::ClockFeel::triplet);
    tuplets.render(temporalWindow.data(), nullptr, 80);
    require(tuplets.getCurrentStep() == 1, "triplets advance inside the duration of a straight pulse");
    tuplets.reset(); tuplets.setClockFeel(antitotem::SimpleSequencer::ClockFeel::quintuplet);
    tuplets.render(temporalWindow.data(), nullptr, 96);
    require(tuplets.getCurrentStep() == 1, "quintuplets advance inside the duration of a straight pulse");
    tuplets.reset(); tuplets.setClockFeel(antitotem::SimpleSequencer::ClockFeel::glitch);
    tuplets.render(temporalWindow.data(), nullptr, 120);
    require(tuplets.getCurrentStep() == 1, "glitch mode begins from a legible straight pulse");
    tuplets.render(temporalWindow.data(), nullptr, 60);
    require(tuplets.getCurrentStep() == 2, "glitch mode has a stable, testable short second pulse");
    sequencer.setStepVoltage(3, 3.0f);
    require(sequencer.getStepVoltage(3) == 1.0f, "CV is safely constrained");
    require(sequencer.getStepVoltage(99) == 0.0f, "invalid CV reads are safe");
    sequencer.setStepMuted(0, true);
    require(sequencer.isStepMuted(0), "a selected step can be muted without removing its CV");
    sequencer.setStepMuted(99, true);
    require(!sequencer.isStepMuted(99), "invalid mute requests are safe");
    sequencer.setStepMuted(0, false);
    std::vector<float> audio(400);
    sequencer.render(audio.data(), nullptr, audio.size());
    bool audible = false;
    for (const auto sample : audio)
    {
        audible = audible || std::abs(sample) > 0.0001f;
        require(std::isfinite(sample), "the output remains finite");
        require(std::abs(sample) <= 0.851f, "the technical output stage prevents clipping");
    }
    require(audible, "the oscillator produces a non-silent signal");

    antitotem::CmosVcf lowpass, bandpass, highpass, notch;
    for (auto* filter : { &lowpass, &bandpass, &highpass, &notch })
    {
        filter->prepare(48000.0); filter->setCutoff(0.48f); filter->setResonance(0.32f);
    }
    lowpass.setModeMask(antitotem::CmosVcf::Mode::lowpass);
    bandpass.setModeMask(antitotem::CmosVcf::Mode::bandpass);
    highpass.setModeMask(antitotem::CmosVcf::Mode::highpass);
    notch.setModeMask(antitotem::CmosVcf::Mode::notch);
    float lowSignature = 0.0f, bandSignature = 0.0f, highSignature = 0.0f, notchSignature = 0.0f;
    for (int sample = 0; sample < 512; ++sample)
    {
        const auto impulse = sample == 0 ? 1.0f : 0.0f;
        lowSignature += lowpass.process(impulse, 0.5f);
        bandSignature += bandpass.process(impulse, 0.5f);
        highSignature += highpass.process(impulse, 0.5f);
        notchSignature += notch.process(impulse, 0.5f);
    }
    require(std::abs(lowSignature - bandSignature) > 0.01f, "LPF and BPF project distinct filter responses");
    require(std::abs(lowSignature - highSignature) > 0.01f, "LPF and HPF project distinct filter responses");
    require(std::abs(notchSignature - highSignature) > 0.01f, "notch is a distinct state-variable filter route");

    // Multi-select mode mask (17 ago. 2026, author: "dois ou mais") - low/
    // band/high are already computed every sample regardless of mode, so
    // combining bits is a real, normalised blend, not a trick. LPF+HPF
    // together should land close to NOTCH (both average low and high),
    // and clearly away from LPF alone.
    {
        antitotem::CmosVcf lowHighBlend;
        lowHighBlend.prepare(48000.0); lowHighBlend.setCutoff(0.48f); lowHighBlend.setResonance(0.32f);
        lowHighBlend.setModeMask(antitotem::CmosVcf::Mode::lowpass | antitotem::CmosVcf::Mode::highpass);
        antitotem::CmosVcf notchReference;
        notchReference.prepare(48000.0); notchReference.setCutoff(0.48f); notchReference.setResonance(0.32f);
        notchReference.setModeMask(antitotem::CmosVcf::Mode::notch);
        antitotem::CmosVcf lowReference;
        lowReference.prepare(48000.0); lowReference.setCutoff(0.48f); lowReference.setResonance(0.32f);
        lowReference.setModeMask(antitotem::CmosVcf::Mode::lowpass);
        double blendVsNotch = 0.0, blendVsLow = 0.0;
        for (int sample = 0; sample < 512; ++sample)
        {
            const auto impulse = sample == 0 ? 1.0f : 0.0f;
            const auto blended = lowHighBlend.process(impulse, 0.5f);
            blendVsNotch += std::abs(blended - notchReference.process(impulse, 0.5f));
            blendVsLow += std::abs(blended - lowReference.process(impulse, 0.5f));
        }
        require(blendVsNotch < 1e-6, "LPF+HPF selected together is bit-identical to NOTCH - both are the same normalised low+high blend");
        require(blendVsLow > 0.01, "LPF+HPF selected together is clearly distinct from LPF alone");
    }

    // Regression for a real numeric-instability bug found live 17 ago. 2026
    // while investigating "PAN dos osciladores mais fraco que o esperado"
    // (TAREFAS.md): the naive state-variable topology above has an unstable
    // pole whenever coefficient * damping exceeds 1 - which happens right
    // where resonance sits near 0 (its highest damping) and cutoff sits
    // near its max (its highest coefficient). low/band then diverge
    // exponentially; float holds that divergence (finite but huge, ~1e37
    // observed live) for thousands of samples before it would finally go
    // non-finite, so isfinite() alone does not catch it. This has to be
    // tested on CmosVcf directly, not through SimpleSequencer's full
    // render() - the downstream leveler/limiter/OutputStage still clamps
    // the *final* mix to the technical ceiling either way, which let an
    // earlier, weaker version of this test pass even against the
    // unpatched filter.
    antitotem::CmosVcf edgeFilter;
    edgeFilter.prepare(44100.0);
    edgeFilter.setCutoff(1.0f);
    edgeFilter.setResonance(0.0f);
    edgeFilter.setCvDepth(0.0f);
    float edgeMaxAbs = 0.0f;
    double edgePhase = 0.0;
    for (int sample = 0; sample < 5000; ++sample)
    {
        const auto in = static_cast<float>(std::sin(edgePhase));
        edgePhase += 2.0 * 3.14159265358979 * 220.0 / 44100.0;
        edgeMaxAbs = std::max(edgeMaxAbs, std::abs(edgeFilter.process(in, 0.5f)));
    }
    require(edgeMaxAbs < 10.0f,
            "VCF at resonance 0 / cutoff max stays a stable filter instead of an unstable exponential runaway");

    antitotem::CmosVoice spatialVoice;
    spatialVoice.prepare(48000.0);
    spatialVoice.setOscillatorLevel(1, 0.0f); spatialVoice.setOscillatorLevel(2, 0.0f);
    spatialVoice.setOscillatorPan(0, -1.0f);
    float leftEnergy = 0.0f, rightEnergy = 0.0f;
    for (int sample = 0; sample < 1024; ++sample)
    {
        const auto frame = spatialVoice.tickStereo(0.48f);
        leftEnergy += std::abs(frame.left); rightEnergy += std::abs(frame.right);
    }
    require(leftEnergy > rightEnergy * 3.0f, "a negative oscillator X position creates a real left-weighted stereo signal");

    // MaterialFilter (src/core/MaterialFilter.h) - prototype-stage module,
    // not yet wired into SimpleSequencer/the UI (see TAREFAS.md, "Retomar a
    // engine musical", 17 ago. 2026). Verified in isolation first, same
    // gate as OSC4/OSC5 before it.
    {
        float worstAbs = 0.0f;
        for (float resonance : { 0.0f, 0.5f, 1.0f })
        for (float cutoff : { 0.0f, 0.5f, 1.0f })
        for (float drive : { 0.0f, 0.5f, 1.0f })
        for (float asymmetry : { 0.0f, 0.5f, 1.0f })
        {
            antitotem::MaterialFilter filter;
            filter.prepare(44100.0);
            filter.setCutoff(cutoff); filter.setResonance(resonance);
            filter.setDrive(drive); filter.setAsymmetry(asymmetry);
            double phase = 0.0;
            for (int sample = 0; sample < 4000; ++sample)
            {
                const auto in = static_cast<float>(std::sin(phase));
                phase += 2.0 * 3.14159265358979 * 220.0 / 44100.0;
                const auto out = filter.process(in, 0.5f);
                require(std::isfinite(out), "MaterialFilter output stays finite across its full cutoff/resonance/drive/asymmetry range");
                worstAbs = std::max(worstAbs, std::abs(out));
            }
        }
        require(worstAbs < 5.0f, "MaterialFilter's asymmetric feedback saturator keeps the output bounded even at maximum resonance/drive");

        antitotem::MaterialFilter clean, driven;
        clean.prepare(44100.0); clean.setDrive(0.0f); clean.setAsymmetry(0.0f); clean.setResonance(0.2f);
        driven.prepare(44100.0); driven.setDrive(1.0f); driven.setAsymmetry(1.0f); driven.setResonance(0.2f);
        double difference = 0.0, phase = 0.0;
        for (int sample = 0; sample < 2000; ++sample)
        {
            const auto in = static_cast<float>(std::sin(phase));
            phase += 2.0 * 3.14159265358979 * 220.0 / 44100.0;
            difference += std::abs(clean.process(in, 0.5f) - driven.process(in, 0.5f));
        }
        require(difference > 1.0, "DRIVE and ASYMMETRY audibly change MaterialFilter's output, not decorative parameters");

        // MIX=0 must return the input completely unchanged, sample for
        // sample - the whole reason SimpleSequencer doesn't need a
        // separate on/off for this stage (17 ago. 2026: "não é só ligar/
        // desligar, é ligar/escalonar/desligar", same as REVERB/PHASER/
        // FLANGER's own MIX). Deliberately maxed-out DRIVE/RESONANCE/
        // ASYMMETRY here - if MIX=0 leaked even a trace of the driven
        // signal, this combination is where it would show up loudest.
        antitotem::MaterialFilter transparent;
        transparent.prepare(44100.0);
        transparent.setCutoff(0.5f); transparent.setResonance(1.0f);
        transparent.setDrive(1.0f); transparent.setAsymmetry(1.0f);
        transparent.setMix(0.0f);
        double transparentDifference = 0.0, transparentPhase = 0.0;
        for (int sample = 0; sample < 2000; ++sample)
        {
            const auto in = static_cast<float>(std::sin(transparentPhase));
            transparentPhase += 2.0 * 3.14159265358979 * 220.0 / 44100.0;
            transparentDifference += std::abs(transparent.process(in, 0.5f) - in);
        }
        require(transparentDifference < 1e-6, "MaterialFilter at MIX=0 returns the input completely unchanged, even at maximum drive/resonance/asymmetry");
    }

    // ChaosField (src/core/ChaosSources.h) - double-well chaotic CV source,
    // same prototype-stage gate as MaterialFilter above.
    {
        for (float damping : { 0.05f, 0.5f, 1.0f })
        for (float drive : { 0.0f, 0.5f, 1.0f })
        for (float rate : { 0.02f, 4.0f, 400.0f })
        {
            antitotem::ChaosField chaos;
            chaos.prepare(44100.0);
            chaos.setRate(rate); chaos.setDrive(drive); chaos.setDamping(damping);
            for (int sample = 0; sample < 4000; ++sample)
            {
                const auto out = chaos.tick();
                require(std::isfinite(out) && std::abs(out) <= 1.0f,
                        "ChaosField stays a bounded, finite CV across its full rate/drive/damping range");
            }
        }

        antitotem::ChaosField moving;
        moving.prepare(44100.0); moving.setRate(20.0f); moving.setDrive(0.8f); moving.setDamping(0.15f);
        float minSeen = 1.0f, maxSeen = -1.0f;
        for (int sample = 0; sample < 20000; ++sample)
        {
            const auto out = moving.tick();
            minSeen = std::min(minSeen, out); maxSeen = std::max(maxSeen, out);
        }
        require(maxSeen - minSeen > 0.3f, "ChaosField actually moves through its range rather than sitting at a fixed point");

        antitotem::ChaosField seedA, seedB;
        seedA.prepare(44100.0); seedA.setRate(30.0f); seedA.setDrive(0.7f); seedA.setDamping(0.2f);
        seedB.prepare(44100.0); seedB.setRate(30.0f); seedB.setDrive(0.7f); seedB.setDamping(0.2f);
        bool identical = true;
        for (int sample = 0; sample < 5000; ++sample)
            if (seedA.tick() != seedB.tick()) { identical = false; break; }
        require(identical, "ChaosField is deterministic/reproducible from the same starting state, not opaque audio-rate randomness");

        // FREEZE/RESEED (17 ago. 2026) - performance controls added on top
        // of the prototype above, same "registrador/memória do CAOS" item
        // from TAREFAS.md, scoped down to freeze+reseed for this round.
        antitotem::ChaosField freezeTest;
        freezeTest.prepare(44100.0); freezeTest.setRate(20.0f); freezeTest.setDrive(0.8f); freezeTest.setDamping(0.15f);
        for (int sample = 0; sample < 2000; ++sample) (void)freezeTest.tick();
        freezeTest.setFrozen(true);
        const auto frozenValue = freezeTest.tick();
        bool stayedFrozen = true;
        for (int sample = 0; sample < 8000; ++sample)
            if (freezeTest.tick() != frozenValue) { stayedFrozen = false; break; }
        require(stayedFrozen, "ChaosField FREEZE holds the trajectory exactly still, not just approximately");
        freezeTest.setFrozen(false);
        bool resumedMoving = false;
        for (int sample = 0; sample < 8000; ++sample)
            if (freezeTest.tick() != frozenValue) { resumedMoving = true; break; }
        require(resumedMoving, "ChaosField resumes moving once FREEZE is released");

        antitotem::ChaosField reseedA, reseedB;
        reseedA.prepare(44100.0); reseedA.setRate(20.0f); reseedA.setDrive(0.8f); reseedA.setDamping(0.15f);
        reseedB.prepare(44100.0); reseedB.setRate(20.0f); reseedB.setDrive(0.8f); reseedB.setDamping(0.15f);
        for (int sample = 0; sample < 1000; ++sample) { (void)reseedA.tick(); (void)reseedB.tick(); }
        reseedB.reseed();
        float reseedDifference = 0.0f;
        for (int sample = 0; sample < 200; ++sample) reseedDifference += std::abs(reseedA.tick() - reseedB.tick());
        require(reseedDifference > 0.1f, "ChaosField RESEED immediately diverges the trajectory rather than waiting for the next periodic kick");
        require(std::isfinite(reseedB.tick()), "ChaosField stays finite immediately after RESEED");
    }

    // WanderSource (src/core/ChaosSources.h) - slow mean-reverting random
    // walk, same prototype-stage gate as the two modules above.
    {
        for (float depth : { 0.0f, 0.5f, 1.0f })
        for (float rate : { 0.01f, 2.0f, 20.0f })
        {
            antitotem::WanderSource wander;
            wander.prepare(44100.0);
            wander.setRate(rate); wander.setDepth(depth);
            for (int sample = 0; sample < 4000; ++sample)
            {
                const auto out = wander.tick();
                require(std::isfinite(out) && std::abs(out) <= 1.0f,
                        "WanderSource stays a bounded, finite CV across its full rate/depth range");
            }
        }

        antitotem::WanderSource moving;
        moving.prepare(44100.0); moving.setRate(15.0f); moving.setDepth(1.0f);
        float minSeen = 1.0f, maxSeen = -1.0f;
        for (int sample = 0; sample < 20000; ++sample)
        {
            const auto out = moving.tick();
            minSeen = std::min(minSeen, out); maxSeen = std::max(maxSeen, out);
        }
        require(maxSeen - minSeen > 0.2f, "WanderSource actually wanders rather than sitting at a fixed point");

        antitotem::WanderSource settling;
        settling.prepare(44100.0); settling.setRate(15.0f); settling.setDepth(0.0f);
        float lastAbs = 1.0f;
        for (int sample = 0; sample < 20000; ++sample) lastAbs = std::abs(settling.tick());
        require(lastAbs < 0.05f, "WanderSource at zero DEPTH settles toward centre via its mean-reversion pull");

        // FREEZE/RESEED (17 ago. 2026) - same performance controls as
        // ChaosField's own, see that block's comment above.
        antitotem::WanderSource freezeTest;
        freezeTest.prepare(44100.0); freezeTest.setRate(15.0f); freezeTest.setDepth(1.0f);
        for (int sample = 0; sample < 2000; ++sample) (void)freezeTest.tick();
        freezeTest.setFrozen(true);
        const auto frozenValue = freezeTest.tick();
        bool stayedFrozen = true;
        for (int sample = 0; sample < 8000; ++sample)
            if (freezeTest.tick() != frozenValue) { stayedFrozen = false; break; }
        require(stayedFrozen, "WanderSource FREEZE holds the walk exactly still");
        freezeTest.setFrozen(false);
        bool resumedMoving = false;
        for (int sample = 0; sample < 8000; ++sample)
            if (freezeTest.tick() != frozenValue) { resumedMoving = true; break; }
        require(resumedMoving, "WanderSource resumes wandering once FREEZE is released");

        antitotem::WanderSource reseedTest;
        reseedTest.prepare(44100.0); reseedTest.setRate(0.05f); reseedTest.setDepth(1.0f);
        const auto beforeReseed = reseedTest.tick();
        reseedTest.reseed();
        const auto afterReseed = reseedTest.tick();
        require(std::abs(afterReseed - beforeReseed) > 1e-6f, "WanderSource RESEED jumps immediately rather than waiting for the next RATE-paced retarget");
        require(std::isfinite(afterReseed) && std::abs(afterReseed) <= 1.0f, "WanderSource stays bounded and finite immediately after RESEED");
    }

    // MaterialFilter wired into SimpleSequencer, in series right after
    // CmosVcf (17 ago. 2026). No separate on/off - a single MIX, same as
    // REVERB/PHASER/FLANGER (author, live: "não é só ligar/desligar, é
    // ligar/escalonar/desligar"). MIX=0's own transparency is already
    // proven at the unit level just above (MaterialFilter's own
    // process() returning the input unchanged); this block only needs to
    // confirm the integrated chain stays safe and that MIX visibly does
    // something end to end.
    {
        auto renderHeldTone = [] (float mix) {
            antitotem::SimpleSequencer seq;
            seq.prepare(44100.0);
            seq.setRunning(true);
            seq.setOscillatorLevel(0, 1.0f);
            seq.setFilterCutoff(0.6f); seq.setFilterResonance(0.5f); seq.setFilterCvDepth(0.0f);
            seq.setMaterialFilterCutoff(0.5f); seq.setMaterialFilterResonance(0.8f);
            seq.setMaterialFilterDrive(0.9f); seq.setMaterialFilterAsymmetry(0.9f);
            seq.setMaterialFilterMix(mix);
            seq.setEnvelopeAttack(0.0f); seq.setEnvelopeDecay(0.0f); seq.setEnvelopeSustain(1.0f); seq.setEnvelopeRelease(0.0f);
            for (std::size_t s = 0; s < antitotem::SimpleSequencer::stepCount; ++s)
            { seq.setStepVoltage(s, 0.5f); seq.setStepLevel(s, 1.0f); seq.setStepEffectSend(s, 0.5f); }
            antitotem::MutableMixer::Channel filterCh; filterCh.enabled = true;
            seq.setMixChannel(0, filterCh);
            std::vector<float> audio(8000);
            seq.render(audio.data(), nullptr, audio.size());
            return audio;
        };

        const auto zeroMix = renderHeldTone(0.0f);
        const auto fullMix = renderHeldTone(1.0f);

        double difference = 0.0;
        for (std::size_t i = 0; i < fullMix.size(); ++i)
        {
            require(std::isfinite(fullMix[i]) && std::abs(fullMix[i]) <= 0.851f,
                    "MaterialFilter at MIX=1 stays finite and output-safe inside the full SimpleSequencer chain");
            difference += std::abs(fullMix[i] - zeroMix[i]);
        }
        require(difference > 1.0, "MaterialFilter MIX audibly scales the signal from 0 to 1");
    }

    // NOISE MIX (MODULAÇÃO) vs the mixer's own NOISE channel (18 ago.
    // 2026) - the pre-RING noise injection used to run purely off
    // noiseMix, ignoring the NOISE channel's ON/M/S and gain entirely.
    // Found live: author left NOISE MIX high, turned the mixer's NOISE
    // channel OFF (or its fader to 0), and still heard it. Fixed by
    // gating that injection (and the NOISE channel's own stereo-width
    // reinjection) behind mixer.isChannelActive(2)/its own gain - this
    // guards the fix, not just the pre-fix symptom.
    {
        auto renderNoise = [] (bool noiseEnabled, float noiseGain, float noiseMixValue) {
            antitotem::SimpleSequencer seq;
            seq.prepare(44100.0);
            seq.setRunning(true);
            seq.setOscillatorLevel(0, 1.0f);
            seq.setFilterCutoff(0.6f); seq.setFilterResonance(0.5f); seq.setFilterCvDepth(0.0f);
            seq.setEnvelopeAttack(0.0f); seq.setEnvelopeDecay(0.0f); seq.setEnvelopeSustain(1.0f); seq.setEnvelopeRelease(0.0f);
            seq.setNoiseColour(antitotem::NoisePalette::Colour::violet);
            seq.setNoiseMix(noiseMixValue);
            for (std::size_t s = 0; s < antitotem::SimpleSequencer::stepCount; ++s)
            { seq.setStepVoltage(s, 0.5f); seq.setStepLevel(s, 1.0f); seq.setStepEffectSend(s, 0.5f); }
            antitotem::MutableMixer::Channel filterCh; filterCh.enabled = true;
            antitotem::MutableMixer::Channel ringCh; ringCh.enabled = true;
            antitotem::MutableMixer::Channel noiseCh; noiseCh.enabled = noiseEnabled; noiseCh.gain = noiseGain;
            seq.setMixChannel(0, filterCh);
            seq.setMixChannel(1, ringCh);
            seq.setMixChannel(2, noiseCh);
            std::vector<float> audio(8000);
            seq.render(audio.data(), nullptr, audio.size());
            return audio;
        };

        const auto offNoMix = renderNoise(false, 1.0f, 0.0f);
        const auto offHighMix = renderNoise(false, 1.0f, 1.0f);
        const auto onZeroGainHighMix = renderNoise(true, 0.0f, 1.0f);
        const auto onFullHighMix = renderNoise(true, 1.0f, 1.0f);

        double offDifference = 0.0, zeroGainDifference = 0.0, onDifference = 0.0;
        for (std::size_t i = 0; i < offNoMix.size(); ++i)
        {
            require(std::isfinite(offHighMix[i]) && std::abs(offHighMix[i]) <= 0.851f,
                    "NOISE channel OFF stays finite and output-safe even with NOISE MIX at max");
            offDifference += std::abs(offHighMix[i] - offNoMix[i]);
            zeroGainDifference += std::abs(onZeroGainHighMix[i] - offNoMix[i]);
            onDifference += std::abs(onFullHighMix[i] - offNoMix[i]);
        }
        require(offDifference < 1e-6, "mixer NOISE channel OFF is a true master switch - NOISE MIX has zero effect regardless of its value");
        require(zeroGainDifference < 1e-6, "mixer NOISE channel's own gain fader at 0 fully silences it too, not just its ON/OFF state");
        require(onDifference > 1.0, "NOISE channel ON with gain restored still lets NOISE MIX audibly reach the output");
    }

    // The four visible rails must reach an audible DSP process.  Effects are
    // intentionally gated by each step's FX send; these direct checks keep
    // their individual algorithms from silently becoming bypasses.
    antitotem::LfoSource slowLfo, fastLfo;
    slowLfo.prepare(48000.0); fastLfo.prepare(48000.0);
    slowLfo.setRate(0.15f); fastLfo.setRate(8.0f);
    float lfoDifference = 0.0f;
    for (int sample = 0; sample < 4096; ++sample)
        lfoDifference += std::abs(slowLfo.tick() - fastLfo.tick());
    require(lfoDifference > 1.0f, "the LFO rail changes the modulation trajectory");

    antitotem::LfoSource sineLfo, triangleLfo, pulseLfo;
    sineLfo.prepare(48000.0); triangleLfo.prepare(48000.0); pulseLfo.prepare(48000.0);
    sineLfo.setRate(240.0f); triangleLfo.setRate(240.0f); pulseLfo.setRate(240.0f);
    triangleLfo.setShape(antitotem::LfoSource::Shape::triangle);
    pulseLfo.setShape(antitotem::LfoSource::Shape::square);
    float shapeDifference = 0.0f;
    for (int sample = 0; sample < 480; ++sample)
        shapeDifference += std::abs(sineLfo.tick() - triangleLfo.tick())
                         + std::abs(triangleLfo.tick() - pulseLfo.tick());
    require(shapeDifference > 1.0f, "sine, triangle and pulse LFO selections create distinct modulation fields");

    // CHAOS/WANDER LFO shapes (17 ago. 2026) - reuse LfoSource's existing
    // RING destination instead of a new CV-routing matrix (see
    // ModulationSources.h's own comment). Confirms the same contract as
    // the phase-based shapes above: bounded, genuinely distinct from sine,
    // and RATE still reaches both underlying modules through the one knob.
    antitotem::LfoSource chaosLfo, wanderLfo;
    chaosLfo.prepare(48000.0); wanderLfo.prepare(48000.0);
    chaosLfo.setRate(240.0f); wanderLfo.setRate(240.0f);
    chaosLfo.setShape(antitotem::LfoSource::Shape::chaos);
    wanderLfo.setShape(antitotem::LfoSource::Shape::wander);
    float chaosVsSineDifference = 0.0f, wanderVsSineDifference = 0.0f;
    for (int sample = 0; sample < 480; ++sample)
    {
        const auto chaosSample = chaosLfo.tick();
        const auto wanderSample = wanderLfo.tick();
        require(std::isfinite(chaosSample) && std::abs(chaosSample) <= 1.0f, "CHAOS LFO stays a bounded, finite modulator");
        require(std::isfinite(wanderSample) && std::abs(wanderSample) <= 1.0f, "WANDER LFO stays a bounded, finite modulator");
    }
    antitotem::LfoSource sineReference;
    sineReference.prepare(48000.0); sineReference.setRate(240.0f);
    chaosLfo.prepare(48000.0); wanderLfo.prepare(48000.0);
    chaosLfo.setRate(240.0f); wanderLfo.setRate(240.0f);
    chaosLfo.setShape(antitotem::LfoSource::Shape::chaos);
    wanderLfo.setShape(antitotem::LfoSource::Shape::wander);
    for (int sample = 0; sample < 480; ++sample)
    {
        chaosVsSineDifference += std::abs(chaosLfo.tick() - sineReference.tick());
        wanderVsSineDifference += std::abs(wanderLfo.tick() - sineReference.tick());
    }
    require(chaosVsSineDifference > 1.0f, "CHAOS produces a modulation field distinct from sine");
    require(wanderVsSineDifference > 1.0f, "WANDER produces a modulation field distinct from sine");

    // A single RATE still reaches CHAOS/WANDER's own internal modules, the
    // same one-knob contract as the phase-based shapes' own rate test above.
    antitotem::LfoSource slowChaos, fastChaos;
    slowChaos.prepare(48000.0); fastChaos.prepare(48000.0);
    slowChaos.setShape(antitotem::LfoSource::Shape::chaos); fastChaos.setShape(antitotem::LfoSource::Shape::chaos);
    slowChaos.setRate(0.1f); fastChaos.setRate(300.0f);
    float chaosRateDifference = 0.0f;
    for (int sample = 0; sample < 2000; ++sample)
        chaosRateDifference += std::abs(slowChaos.tick() - fastChaos.tick());
    require(chaosRateDifference > 1.0f, "RATE reaches CHAOS's own internal rate, not just the unused phase accumulator");

    // STEP LFO shape (17 ago. 2026) - sample & hold, added to close the
    // FORMA LFO layout at 3+3. Bounded/finite like the other shapes, but
    // its defining trait is holding a flat value between draws instead of
    // moving every sample - confirmed by counting how many consecutive
    // sample-to-sample deltas are exactly zero (sine/triangle/chaos/wander
    // essentially never repeat two samples in a row; STEP does, for most
    // of each RATE cycle).
    antitotem::LfoSource stepLfo;
    stepLfo.prepare(48000.0);
    stepLfo.setRate(240.0f);
    stepLfo.setShape(antitotem::LfoSource::Shape::step);
    int stepHeldSamples = 0;
    float stepPrevious = stepLfo.tick();
    for (int sample = 0; sample < 480; ++sample)
    {
        const auto stepSample = stepLfo.tick();
        require(std::isfinite(stepSample) && std::abs(stepSample) <= 1.0f, "STEP LFO stays a bounded, finite modulator");
        if (stepSample == stepPrevious) ++stepHeldSamples;
        stepPrevious = stepSample;
    }
    require(stepHeldSamples > 400, "STEP holds a flat value between draws instead of moving every sample");

    antitotem::LfoSource stepVsSineReference;
    stepVsSineReference.prepare(48000.0); stepVsSineReference.setRate(240.0f);
    stepLfo.prepare(48000.0); stepLfo.setRate(240.0f); stepLfo.setShape(antitotem::LfoSource::Shape::step);
    float stepVsSineDifference = 0.0f;
    for (int sample = 0; sample < 480; ++sample)
        stepVsSineDifference += std::abs(stepLfo.tick() - stepVsSineReference.tick());
    require(stepVsSineDifference > 1.0f, "STEP produces a modulation field distinct from sine");

    antitotem::MaterialReverb testReverb;
    testReverb.prepare(48000.0); testReverb.setMix(0.62f);
    bool reverbTailFound = false;
    for (int sample = 0; sample < 6000; ++sample)
    {
        const auto output = testReverb.process(sample == 0 ? 0.9f : 0.0f, 1.0f);
        reverbTailFound = reverbTailFound || (sample > 100 && std::abs(output) > 0.0001f);
    }
    require(reverbTailFound,
            "the reverb rail returns delayed material when FX send is open");

    antitotem::PhaseField dryPhase, wetPhase;
    dryPhase.prepare(48000.0); wetPhase.prepare(48000.0); wetPhase.setMix(0.75f);
    float phaseDifference = 0.0f;
    for (int sample = 0; sample < 512; ++sample)
        phaseDifference += std::abs(wetPhase.process(0.37f, 1.0f) - dryPhase.process(0.37f, 1.0f));
    require(phaseDifference > 0.01f, "the phaser rail changes its swept all-pass output");

    antitotem::FlangerField dryFlange, wetFlange;
    dryFlange.prepare(48000.0); wetFlange.prepare(48000.0); wetFlange.setMix(0.70f);
    float flangeDifference = 0.0f;
    for (int sample = 0; sample < 2048; ++sample)
    {
        const auto input = std::sin(static_cast<float>(sample) * 0.052f);
        flangeDifference += std::abs(wetFlange.process(input, 1.0f) - dryFlange.process(input, 1.0f));
    }
    require(flangeDifference > 0.01f, "the flanger rail changes its moving-delay output");

    sequencer.setRunning(false);
    sequencer.render(audio.data(), nullptr, audio.size());
    bool stoppedIsSilent = true;
    for (const auto sample : audio) stoppedIsSilent = stoppedIsSilent && std::abs(sample) < 0.0001f;
    require(stoppedIsSilent, "STOP fades the output to silence while retaining the selected channel");

    antitotem::OutputStage peakProtection;
    peakProtection.prepare(48000.0);
    for (int sample = 0; sample < 512; ++sample)
    {
        float left = 0.0f, right = 0.0f;
        peakProtection.process(sample % 2 == 0 ? 4.0f : -4.0f, 3.5f, left, right);
        require(std::isfinite(left) && std::isfinite(right)
                    && std::abs(left) < 0.85f && std::abs(right) < 0.85f,
                "the final soft ceiling contains abrupt noise and route-change peaks");
    }

    // OSC4 (4093/4020 study): silent by default so it cannot surprise a
    // patch that only knows about three oscillators, but audibly present -
    // and still output-safe at its widest sub-clock ratio - once raised.
    antitotem::SimpleSequencer osc4Silent;
    osc4Silent.prepare(4000.0); osc4Silent.setRunning(true); osc4Silent.setClockRate(4.0);
    std::vector<float> osc4SilentAudio(2048);
    osc4Silent.render(osc4SilentAudio.data(), nullptr, osc4SilentAudio.size());

    antitotem::SimpleSequencer osc4Loud;
    osc4Loud.prepare(4000.0); osc4Loud.setRunning(true); osc4Loud.setClockRate(4.0);
    osc4Loud.setOscillatorLevel(3, 1.0f);
    osc4Loud.setOscillatorRatio(3, 0.03125f);
    osc4Loud.setOscillatorPan(3, 1.0f);
    std::vector<float> osc4LoudAudio(2048);
    osc4Loud.render(osc4LoudAudio.data(), nullptr, osc4LoudAudio.size());
    double osc4Difference = 0.0;
    for (std::size_t i = 0; i < osc4LoudAudio.size(); ++i)
    {
        require(std::isfinite(osc4LoudAudio[i]) && std::abs(osc4LoudAudio[i]) <= 0.851f,
                "OSC4 at its deepest division and full level stays finite and output-safe");
        osc4Difference += std::abs(osc4LoudAudio[i] - osc4SilentAudio[i]);
    }
    require(osc4Difference > 0.01, "raising OSC4's level actually changes the rendered output");

    // OSC5 (4046 PLL + LM13600 ring-mod study): silent by default, and its
    // phase-detector correction plus ring modulation against OSC A must
    // still stay finite/output-safe even at the widest ratio and full level.
    antitotem::SimpleSequencer osc5Silent;
    osc5Silent.prepare(4000.0); osc5Silent.setRunning(true); osc5Silent.setClockRate(4.0);
    std::vector<float> osc5SilentAudio(2048);
    osc5Silent.render(osc5SilentAudio.data(), nullptr, osc5SilentAudio.size());

    antitotem::SimpleSequencer osc5Loud;
    osc5Loud.prepare(4000.0); osc5Loud.setRunning(true); osc5Loud.setClockRate(4.0);
    osc5Loud.setOscillatorLevel(4, 1.0f);
    osc5Loud.setOscillatorRatio(4, 4.0f);
    osc5Loud.setOscillatorPan(4, -1.0f);
    std::vector<float> osc5LoudAudio(2048);
    osc5Loud.render(osc5LoudAudio.data(), nullptr, osc5LoudAudio.size());
    double osc5Difference = 0.0;
    for (std::size_t i = 0; i < osc5LoudAudio.size(); ++i)
    {
        require(std::isfinite(osc5LoudAudio[i]) && std::abs(osc5LoudAudio[i]) <= 0.851f,
                "OSC5 at its widest ratio and full level stays finite and output-safe");
        osc5Difference += std::abs(osc5LoudAudio[i] - osc5SilentAudio[i]);
    }
    require(osc5Difference > 0.01, "raising OSC5's level actually changes the rendered output");

    antitotem::SimpleSequencer stressed;
    stressed.prepare(48000.0);
    stressed.setRunning(true);
    stressed.setClockRate(17.0);
    stressed.setMasterGain(1.0f);
    stressed.setEnergy(1.0f);
    stressed.setFeedbackAmount(0.72f);
    stressed.setFeedbackConnections(0x3fU);
    stressed.setFilterCutoff(0.86f); stressed.setFilterResonance(0.88f); stressed.setFilterCvDepth(1.0f);
    stressed.setNoiseColour(antitotem::NoisePalette::Colour::violet); stressed.setNoiseMix(0.42f);
    stressed.setSampleHoldRate(120.0f); stressed.setSampleHoldMix(1.0f);
    stressed.setRingMix(1.0f); stressed.setLfoRate(1600.0f);
    stressed.setReverbMix(0.62f); stressed.setReverbFeedback(0.78f);
    stressed.setPhaserMix(0.75f); stressed.setPhaserRate(16.0f); stressed.setPhaserDepth(1.0f);
    stressed.setFlangerMix(0.70f); stressed.setFlangerRate(8.0f); stressed.setFlangerDepth(1.0f);
    stressed.setOscillatorLevel(3, 1.0f); stressed.setOscillatorRatio(3, 4.0f); stressed.setOscillatorPan(3, -1.0f);
    stressed.setOscillatorLevel(4, 1.0f); stressed.setOscillatorRatio(4, 4.0f); stressed.setOscillatorPan(4, 1.0f);
    for (std::size_t step = 0; step < antitotem::SimpleSequencer::stepCount; ++step)
    {
        stressed.setStepVoltage(step, static_cast<float>(step) / 15.0f);
        stressed.setStepLevel(step, 1.0f); stressed.setStepEffectSend(step, 1.0f);
    }
    std::vector<float> stressAudio(96000);
    stressed.render(stressAudio.data(), nullptr, stressAudio.size());
    for (const auto sample : stressAudio)
        require(std::isfinite(sample) && std::abs(sample) <= 0.851f,
                "all enabled sound modules remain finite and output-safe under stress");

    antitotem::DualObjectEngine objects;
    objects.prepare(48000.0);
    objects.setRunning(true);
    objects.setObjectConnection(0.5f, 0.4f);
    objects.setConnectionRoutes(antitotem::DualObjectEngine::direct | antitotem::DualObjectEngine::capacitor,
                                antitotem::DualObjectEngine::diode | antitotem::DualObjectEngine::pulse);
    objects.setAuxiliaryMix(0.25f, 0.2f);
    objects.render(audio.data(), audio.data(), audio.size());
    for (const auto sample : audio)
        require(std::isfinite(sample) && std::abs(sample) <= 0.851f,
                "two connected objects remain output-safe");
}
