#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace antitotem
{
// Original digital double-well oscillator - critically inspired by the
// Double-Well Chaos principle studied in
// docs/PESQUISA_CMOS_LUNETTA.md (Ian Fritz, 2007), not a port of that
// circuit. Two integrators (x, y) chase a nonlinear restoring force
// (x - x^3) that pulls toward two stable wells at x = ~-1/~+1; DRIVE and
// DAMPING decide whether the system settles into one well, oscillates
// between them, or wanders unpredictably. Unlike CmosVcf's linear
// instability (found earlier this session), a chaotic source is *meant* to
// be sensitive/unpredictable - the states are still explicitly clamped and
// the output is a bounded CV in [-1, 1], not raw audio, so an unstable
// integration step cannot escape to a level dangerous for downstream
// modules. See ChaosSourceTests in tests/SimpleSequencerTests.cpp for the
// bounded-output sweep.
class ChaosField
{
public:
    void prepare(double value) noexcept { sampleRate = std::max(100.0, value); reset(); }
    // A small nonzero seed, not zero: x=0 is the system's own unstable
    // equilibrium (the peak between the two wells) - starting exactly there
    // would leave it motionless until something nudges it.
    void reset() noexcept { x = 0.15f; y = 0.0f; kickCounter = 0; noiseState = 0x9e3779b9U; }
    // RATE scales how fast the system evolves - from a slow CV wander up to
    // an audio-rate texture, the same way ENERGIA scales the voice's own
    // clock elsewhere in the instrument. It also paces the random kick
    // below, so a faster RATE both moves and hops faster together.
    void setRate(float hertz) noexcept { rate = std::clamp(hertz, 0.02f, 400.0f); }
    // DRIVE: strength of the pull toward the two wells, and (see tick())
    // the size of the periodic kick that lets the system actually cross
    // between them - low values barely move from the seed; high values
    // swing between wells rapidly and unpredictably.
    void setDrive(float value) noexcept { drive = std::clamp(value, 0.0f, 1.0f); }
    // DAMPING: energy lost per step - low damping lets the system swing
    // wide and unpredictably; high damping settles it into a single well.
    void setDamping(float value) noexcept { damping = std::clamp(value, 0.05f, 1.0f); }
    // FREEZE (17 ago. 2026): holds the trajectory at its current point -
    // tick() below skips the integration step entirely while true,
    // including the periodic kick's own cadence, so nothing moves at all
    // until released, and resuming continues exactly where it left off.
    void setFrozen(bool value) noexcept { frozen = value; }
    // RESEED: jumps straight to a fresh random point, not a velocity kick
    // like the periodic one above - found live (author: "ainda não
    // percebi no audio o reseed") that a kick to y alone is far too slow
    // to read as an event at typical CAOS rates: dt is deliberately tiny
    // (rate/sampleRate) so a slow CV source doesn't drift at audio speed,
    // which means a y-only nudge takes thousands of samples to become a
    // visible x movement - the same timescale the periodic kick already
    // uses, not an instant jump. Setting x directly changes the output
    // the very next sample, which is what "reseed" should feel like.
    void reseed() noexcept
    {
        noiseState ^= noiseState << 13U;
        noiseState ^= noiseState >> 17U;
        noiseState ^= noiseState << 5U;
        const auto noiseX = static_cast<float>(noiseState) * (1.0f / 2147483648.0f) - 1.0f;
        noiseState ^= noiseState << 13U;
        noiseState ^= noiseState >> 17U;
        noiseState ^= noiseState << 5U;
        const auto noiseY = static_cast<float>(noiseState) * (1.0f / 2147483648.0f) - 1.0f;
        x = std::clamp(noiseX * 1.6f, -2.2f, 2.2f);
        y = std::clamp(noiseY * 3.5f, -3.5f, 3.5f);
        kickCounter = 0;
    }

    [[nodiscard]] float tick() noexcept
    {
        if (frozen) return std::clamp(x, -1.0f, 1.0f);
        const auto dt = std::clamp(static_cast<float>(rate / sampleRate), 0.0f, 0.02f);
        const auto pull = drive * 2.4f;
        const auto xNext = x + dt * y * 6.0f;
        auto yNext = y + dt * (pull * (x - x * x * x) - damping * 3.0f * y);
        // Without forcing, this is a deterministic unforced ODE: once it
        // settles into orbiting one well it can never reach the other, no
        // matter how DRIVE/DAMPING are tuned (found live: a full parameter
        // sweep gave an identical positive-only output range at every
        // combination). A periodic random velocity kick, once per RATE
        // cycle, is what makes this genuinely chaotic rather than a single
        // fixed oscillation - it is what lets the trajectory occasionally
        // cross x=0 into the other well, which is the "alteração sonora"
        // the author reported as missing (17 ago. 2026).
        const auto samplesPerCycle = static_cast<std::uint32_t>(
            std::max(1.0f, static_cast<float>(sampleRate) / std::max(rate, 0.02f)));
        if (++kickCounter >= samplesPerCycle)
        {
            kickCounter = 0;
            noiseState ^= noiseState << 13U;
            noiseState ^= noiseState >> 17U;
            noiseState ^= noiseState << 5U;
            const auto noise = static_cast<float>(noiseState) * (1.0f / 2147483648.0f) - 1.0f;
            yNext += noise * drive * 2.5f;
        }
        x = std::clamp(xNext, -2.2f, 2.2f);
        y = std::clamp(yNext, -3.5f, 3.5f);
        if (!std::isfinite(x) || !std::isfinite(y)) reset();
        return std::clamp(x, -1.0f, 1.0f);
    }

private:
    double sampleRate = 48000.0;
    float x = 0.15f, y = 0.0f;
    float rate = 4.0f, drive = 0.6f, damping = 0.3f;
    std::uint32_t kickCounter = 0;
    std::uint32_t noiseState = 0x9e3779b9U;
    bool frozen = false;
};

// Original slow wandering CV source - a smoothed random walk, distinct from
// LfoSource (periodic) and from NoisePalette+SampleHold (stepped, no
// interpolation between held values). RATE picks a fresh random target the
// same way SampleHold's own rate does (see NoiseFields.h); a fixed, sample-
// rate-independent slew constant then draws a continuous, non-repeating
// path toward that target, the way a hand-drawn or breathing gesture would
// move, rather than a cycle or a staircase.
//
// An earlier version of this class integrated noise directly into the
// state each sample, scaled by rate/sampleRate; at any musically sensible
// RATE that step was too small to accumulate into a noticeable excursion
// within a normal listening window (found by the module's own stability
// test - see ChaosSourceTests/WanderSource block in
// tests/SimpleSequencerTests.cpp - failing the "actually wanders" check).
// Picking a discrete target and slewing toward it decouples "how often it
// moves" (RATE) from "how far it can go" (DEPTH), which is also easier to
// reason about than a diffusion process tuned by feel.
class WanderSource
{
public:
    void prepare(double value) noexcept { sampleRate = std::max(100.0, value); reset(); }
    void reset() noexcept { state = 0.0f; target = 0.0f; elapsed = 0.0; randomState = 0x2545f491U; }
    void setRate(float hertz) noexcept { rate = std::clamp(hertz, 0.01f, 20.0f); }
    // DEPTH: how far a fresh target can land from centre. 0 always targets
    // centre, so the walk settles there; 1 allows the full [-1, 1] range.
    void setDepth(float value) noexcept { depth = std::clamp(value, 0.0f, 1.0f); }
    // FREEZE: holds the walk at its current point - tick() below skips
    // both the retarget countdown and the slew step while true, same
    // reasoning as ChaosField's own FREEZE.
    void setFrozen(bool value) noexcept { frozen = value; }
    // RESEED: an immediate jump to a fresh random point, rather than
    // waiting for the next RATE-paced retarget - snaps state and target
    // together so it settles right there instead of sliding further.
    void reseed() noexcept
    {
        randomState ^= randomState << 13U;
        randomState ^= randomState >> 17U;
        randomState ^= randomState << 5U;
        const auto noiseSample = static_cast<float>(randomState) * (1.0f / 2147483648.0f) - 1.0f;
        state = std::clamp(noiseSample * depth, -1.0f, 1.0f);
        target = state;
        elapsed = 0.0;
    }

    [[nodiscard]] float tick() noexcept
    {
        if (frozen) return std::clamp(state, -1.0f, 1.0f);
        if (++elapsed >= sampleRate / rate)
        {
            elapsed = 0.0;
            randomState ^= randomState << 13U;
            randomState ^= randomState >> 17U;
            randomState ^= randomState << 5U;
            const auto noiseSample = static_cast<float>(randomState) * (1.0f / 2147483648.0f) - 1.0f;
            target = noiseSample * depth;
        }
        // A fixed ~150ms time constant, independent of RATE/sample rate, so
        // motion always reads as a continuous glide rather than a jump.
        const auto slewCoefficient = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * 0.15)));
        state += (target - state) * slewCoefficient;
        if (!std::isfinite(state)) reset();
        return std::clamp(state, -1.0f, 1.0f);
    }

private:
    double sampleRate = 48000.0, elapsed = 0.0;
    float state = 0.0f, target = 0.0f;
    std::uint32_t randomState = 0x2545f491U;
    float rate = 0.3f, depth = 0.6f;
    bool frozen = false;
};
} // namespace antitotem
