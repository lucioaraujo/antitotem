#pragma once

#include "core/ChaosSources.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace antitotem
{
// Original digital signal sources. They preserve the ideas of slow control,
// continuous multiplication and transistor-noise instability without emulating
// a particular physical circuit.
class LfoSource
{
public:
    // CHAOS and WANDER reuse this same source and its existing destination
    // (RING modulator) instead of a new CV-routing matrix - author, live,
    // 17 ago. 2026: chose this over building new destinations for
    // ChaosField/WanderSource (src/core/ChaosSources.h). No new knobs this
    // round (same reasoning as MaterialFilter's own CUTOFF/RESONANCE/
    // DRIVE/ASYMMETRY) - but DRIVE/DAMPING/DEPTH are set explicitly below
    // rather than left at each module's own generic class defaults.
    // ChaosSources.h's defaults (chaos drive 0.6/damping 0.3, wander depth
    // 0.6) read as noticeably subtler once embedded as an LFO shape than
    // they did in the module's own standalone listening proof, where the
    // "actively swinging" demo take used drive 0.9/damping 0.15 (author,
    // live: "caos e vaga são menos presentes na alteração sonora, vaga o
    // menos deles" - confirming the isolated proof's approval didn't
    // carry over the specific parameters that made it read as present).
    // STEP (17 ago. 2026): a classic sample & hold, added to close the
    // FORMA LFO layout at 3+3 instead of 3+2 with an empty slot - draws a
    // fresh random value once per RATE cycle and holds it flat until the
    // next draw (a stair-step, not a slide), pairing with WANDER (same
    // cadence, but slewed) and CHAOS (same unpredictability, but a
    // continuous trajectory) as the third "unpredictable" shape.
    enum class Shape : unsigned char { sine, triangle, square, chaos, wander, step };

    void prepare(double value) noexcept
    {
        sampleRate = std::max(100.0, value);
        chaosField.prepare(sampleRate);
        // Pushed well past ChaosSources.h's own tame defaults, toward the
        // demo's "actively swinging between wells" take rather than its
        // "mostly settled" one - an LFO shape should read as clearly in
        // motion, not as a rare accent.
        chaosField.setDrive(0.85f);
        chaosField.setDamping(0.18f);
        wanderSource.prepare(sampleRate);
        // Full range, not ChaosSources.h's own 0.6 default - WANDER was
        // the weaker of the two even after CHAOS's own retune, so it gets
        // pushed furthest.
        wanderSource.setDepth(1.0f);
        reset();
    }
    void reset() noexcept
    {
        phase = 0.0f;
        chaosField.reset();
        wanderSource.reset();
        stepCounter = 0;
        stepValue = 0.0f;
    }
    void setRate(float value) noexcept
    {
        rate = std::clamp(value, 0.02f, 1600.0f);
        // Both modules clamp internally to their own musical range, so a
        // single RATE knob stays meaningful for every shape rather than
        // needing a shape-specific control.
        chaosField.setRate(rate);
        wanderSource.setRate(rate);
    }
    void setShape(Shape value) noexcept { shape = value; }
    // CUTOFF/RESONANCE/DRIVE/ASYMMETRY exposure (MaterialFilter, 17 ago.
    // 2026) set the precedent: internal parameters that started fixed at
    // a retuned default (see prepare() above) get real panel control once
    // there's room, instead of staying permanently baked in. Only
    // meaningful for CHAOS/WANDER respectively - see setFrozen()'s own
    // comment on why that's fine to leave always-present rather than
    // conditionally shown.
    void setChaosDrive(float value) noexcept { chaosField.setDrive(value); }
    void setChaosDamping(float value) noexcept { chaosField.setDamping(value); }
    void setWanderDepth(float value) noexcept { wanderSource.setDepth(value); }
    // FREEZE/RESEED (17 ago. 2026): only meaningful for CHAOS/WANDER -
    // applied to both regardless of which of the two is currently
    // selected, same as RATE above, so switching between them keeps a
    // held FREEZE (or a just-triggered RESEED) in effect either way.
    // Inert for the phase-based shapes (SEN/TRI/PUL/STEP), since tick()
    // never reads chaosField/wanderSource for those.
    void setFrozen(bool value) noexcept { chaosField.setFrozen(value); wanderSource.setFrozen(value); }
    void reseed() noexcept { chaosField.reseed(); wanderSource.reseed(); }
    [[nodiscard]] float tick() noexcept
    {
        const auto current = phase;
        phase += rate / static_cast<float>(sampleRate);
        phase -= std::floor(phase);
        if (shape == Shape::sine) return std::sin(current * twoPi);
        if (shape == Shape::triangle) return 1.0f - 4.0f * std::abs(current - 0.5f);
        if (shape == Shape::square) return current < 0.5f ? 1.0f : -1.0f;
        if (shape == Shape::chaos) return chaosField.tick();
        if (shape == Shape::wander) return wanderSource.tick();
        const auto samplesPerCycle = static_cast<std::uint32_t>(
            std::max(1.0f, static_cast<float>(sampleRate) / std::max(rate, 0.02f)));
        if (++stepCounter >= samplesPerCycle)
        {
            stepCounter = 0;
            stepNoiseState ^= stepNoiseState << 13U;
            stepNoiseState ^= stepNoiseState >> 17U;
            stepNoiseState ^= stepNoiseState << 5U;
            stepValue = static_cast<float>(stepNoiseState) * (1.0f / 2147483648.0f) - 1.0f;
        }
        return stepValue;
    }

private:
    static constexpr float twoPi = 6.28318530717958647692f;
    double sampleRate = 48000.0;
    float phase = 0.0f, rate = 2.0f;
    Shape shape = Shape::sine;
    ChaosField chaosField;
    WanderSource wanderSource;
    std::uint32_t stepCounter = 0;
    std::uint32_t stepNoiseState = 0x1f83d9abU;
    float stepValue = 0.0f;
};

class WhiteNoise
{
public:
    void reset() noexcept { state = 0x6d2b79f5U; }
    [[nodiscard]] float tick() noexcept
    {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        return static_cast<float>(state) * (1.0f / 2147483648.0f) - 1.0f;
    }

private:
    std::uint32_t state = 0x6d2b79f5U;
};

class RingModulator
{
public:
    void setMix(float value) noexcept { mix = std::clamp(value, 0.0f, 1.0f); }
    [[nodiscard]] float process(float carrier, float modulator) const noexcept
    {
        const auto multiplied = carrier * std::clamp(modulator, -1.0f, 1.0f);
        return carrier + (multiplied - carrier) * mix;
    }

private:
    float mix = 0.0f;
};
} // namespace antitotem
