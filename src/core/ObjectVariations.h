#pragma once

#include "core/SimpleSequencer.h"

#include <array>

namespace antitotem::variations
{
inline void pulseAndGates(SimpleSequencer& object) noexcept
{
    object.setLoopEnd(8); object.setClockRate(3.6); object.setEnergy(0.78f);
    object.setOscillatorCore(CmosVoice::OscillatorCore::schmittPulse);
    object.setFeedbackConnections(static_cast<unsigned char>(CmosVoice::FeedbackSignal::direct) | static_cast<unsigned char>(CmosVoice::FeedbackSignal::pulse));
    object.setNoiseMix(0.0f); object.setRingMix(0.12f); object.setReverbMix(0.0f); object.setPhaserMix(0.0f); object.setFlangerMix(0.0f);
    for (std::size_t i = 0; i < SimpleSequencer::stepCount; ++i) { object.setStepLevel(i, i < 8 ? 0.88f : 0.0f); object.setStepEffectSend(i, 0.0f); }
}

inline void porousMemory(SimpleSequencer& object) noexcept
{
    object.setLoopEnd(16); object.setClockRate(1.25); object.setEnergy(0.52f);
    object.setOscillatorCore(CmosVoice::OscillatorCore::unbufferedDrift);
    object.setFeedbackConnections(static_cast<unsigned char>(CmosVoice::FeedbackSignal::capacitive) | static_cast<unsigned char>(CmosVoice::FeedbackSignal::reflux));
    object.setNoiseColour(NoisePalette::Colour::pink); object.setNoiseMix(0.15f); object.setSampleHoldRate(3.1f); object.setSampleHoldMix(0.9f);
    object.setReverbMix(0.38f); object.setPhaserMix(0.16f); object.setFlangerMix(0.08f);
    for (std::size_t i = 0; i < SimpleSequencer::stepCount; ++i) { object.setStepLevel(i, i % 4 == 0 ? 0.78f : 0.38f); object.setStepEffectSend(i, i % 3 == 0 ? 0.88f : 0.25f); }
}

inline void heterodyneField(SimpleSequencer& object) noexcept
{
    object.setLoopEnd(16); object.setClockRate(4.8); object.setEnergy(0.86f);
    object.setOscillatorCore(CmosVoice::OscillatorCore::functionForms);
    object.setOscillatorRatio(0, 0.98f); object.setOscillatorRatio(1, 1.02f); object.setOscillatorRatio(2, 1.49f);
    object.setFeedbackConnections(static_cast<unsigned char>(CmosVoice::FeedbackSignal::transistor) | static_cast<unsigned char>(CmosVoice::FeedbackSignal::rectified) | static_cast<unsigned char>(CmosVoice::FeedbackSignal::pulse));
    object.setLfoRate(27.0f); object.setRingMix(0.36f); object.setNoiseColour(NoisePalette::Colour::bit); object.setNoiseMix(0.08f); object.setSampleHoldMix(0.32f);
    object.setReverbMix(0.12f); object.setPhaserMix(0.34f); object.setFlangerMix(0.22f);
    for (std::size_t i = 0; i < SimpleSequencer::stepCount; ++i) { object.setStepLevel(i, 0.6f + static_cast<float>(i % 3) * 0.12f); object.setStepEffectSend(i, 0.48f + static_cast<float>(i % 4) * 0.12f); }
}

// First variation to actually engage EIXO Y/Z (proximity/orbit) - slow,
// spacious, breathing, the opposite of PULSO. Varied per-oscillator values
// (not one flat setting across all 5) so the drift/circulation reads as
// organic rather than mechanical.
inline void orbitAndDrift(SimpleSequencer& object) noexcept
{
    object.setLoopEnd(16); object.setClockRate(0.9); object.setEnergy(0.58f);
    object.setOscillatorCore(CmosVoice::OscillatorCore::functionForms);
    constexpr std::array<float, 5> proximity { 0.35f, 0.55f, 0.45f, 0.65f, 0.5f };
    constexpr std::array<float, 5> orbit { 0.6f, 0.4f, 0.7f, 0.5f, 0.65f };
    for (std::size_t i = 0; i < proximity.size(); ++i)
    {
        object.setOscillatorProximity(i, proximity[i]);
        object.setOscillatorOrbit(i, orbit[i]);
    }
    object.setFeedbackConnections(static_cast<unsigned char>(CmosVoice::FeedbackSignal::direct));
    object.setRingMix(0.0f); object.setNoiseMix(0.0f);
    object.setReverbMix(0.42f); object.setPhaserMix(0.1f); object.setFlangerMix(0.0f);
    for (std::size_t i = 0; i < SimpleSequencer::stepCount; ++i) { object.setStepLevel(i, i % 4 < 2 ? 0.6f : 0.36f); object.setStepEffectSend(i, 0.5f); }
}

// First variation to put the comb/resonator (RES MIX/ALTURA/CORPO) at the
// centre instead of as an accessory - a struck, ringing body. PERCURSO set
// to pendulum on purpose: the name isn't decorative, it echoes the actual
// scanner behaviour this preset uses.
inline void pendulumResonance(SimpleSequencer& object) noexcept
{
    object.setLoopEnd(16); object.setClockRate(2.4); object.setEnergy(0.7f);
    object.setOscillatorCore(CmosVoice::OscillatorCore::schmittPulse);
    object.setScannerDirection(SimpleSequencer::ScannerDirection::pendulum);
    object.setFeedbackConnections(static_cast<unsigned char>(CmosVoice::FeedbackSignal::pulse));
    object.setRingMix(0.0f); object.setNoiseMix(0.0f);
    object.setReverbMix(0.08f); object.setPhaserMix(0.05f); object.setFlangerMix(0.0f);
    object.setResonatorMix(0.68f); object.setResonatorPitch(0.62f); object.setResonatorDamping(0.75f);
    // Alternating strike/silence per step - the pendulum's own swing, not a
    // constant drone the resonator would otherwise just smear together.
    for (std::size_t i = 0; i < SimpleSequencer::stepCount; ++i) { object.setStepLevel(i, i % 2 == 0 ? 0.82f : 0.0f); object.setStepEffectSend(i, 0.7f); }
}
} // namespace antitotem::variations
