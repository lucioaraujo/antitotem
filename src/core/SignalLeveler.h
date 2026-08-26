#pragma once
#include <cmath>
#include <algorithm>

namespace antitotem
{
// Ported from RASGO_SYNTH's rasgo::SignalLeveler (technique, not schematic -
// same reuse policy as the rest of this engine's circuit-inspired modules).
// Automatic gain unit: tracks a signal's average loudness and corrects toward
// a target level, instead of only reacting to peaks. Antitotem's mixed bus
// (filtered + ringed + noise + resonated, see SimpleSequencer::renderSample)
// has no per-bus gain budget before OutputStage's ceiling, so quiet patches
// sit far below the 0.85 ceiling ("áudios fracos") while heavy multi-bus
// patches force OutputStage's fast envelope-follower into large, sudden
// corrections that read as pumping/"clipando". This sits upstream of
// OutputStage to flatten that swing before the safety ceiling has to react.
class SignalLeveler
{
public:
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        followCoeff_ = coeffFor(followMs_);
        riseCoeff_ = coeffFor(riseMs_);
        fallCoeff_ = coeffFor(fallMs_);
    }

    // Times in milliseconds: how fast the level-follower tracks the input
    // (followMs), and how fast the gain itself moves up vs. down once it
    // decides a correction is needed (riseMs/fallMs - slower fall than rise
    // keeps it from visibly "pumping" on transients).
    void setTimes(double followMs, double riseMs, double fallMs)
    {
        followMs_ = followMs; riseMs_ = riseMs; fallMs_ = fallMs;
        followCoeff_ = coeffFor(followMs_);
        riseCoeff_ = coeffFor(riseMs_);
        fallCoeff_ = coeffFor(fallMs_);
    }

    void setTargetLevel(float target) { target_ = target; }
    void setGainRange(float minGain, float maxGain) { minGain_ = minGain; maxGain_ = maxGain; }

    void reset() { envelope_ = 0.0f; gain_ = 1.0f; }

    // Feed a mono sum so the gain decision reacts to overall loudness; read
    // currentGain() back and apply it to both true stereo channels rather
    // than calling process() twice, which would let the two channels drift
    // to independent gains and narrow/wobble the stereo image.
    float process(float monoInput)
    {
        const auto rectified = std::abs(monoInput);
        envelope_ += (rectified - envelope_) * static_cast<float>(followCoeff_);
        const auto desiredGain = envelope_ > 1e-5f
            ? std::clamp(target_ / envelope_, minGain_, maxGain_)
            : maxGain_;
        const auto coeff = desiredGain > gain_ ? riseCoeff_ : fallCoeff_;
        gain_ += (desiredGain - gain_) * static_cast<float>(coeff);
        return gain_;
    }

    [[nodiscard]] float currentGain() const noexcept { return gain_; }

private:
    [[nodiscard]] double coeffFor(double ms) const
    {
        const auto samples = std::max(1.0, (ms / 1000.0) * sampleRate_);
        return 1.0 - std::exp(-1.0 / samples);
    }

    double sampleRate_ = 44100.0;
    double followMs_ = 60.0, riseMs_ = 120.0, fallMs_ = 400.0;
    double followCoeff_ = 0.01, riseCoeff_ = 0.01, fallCoeff_ = 0.01;
    // Target/range tuned for Antitotem's 0.85 ceiling (see OutputStage.h),
    // not RASGO_SYNTH's 1.0 scale. Raised from 0.42 to 0.52 on 14 ago.
    // 2026 after measuring real takes (analysing recorded .wav peak/RMS,
    // see docs/TAREFAS.md): 0.42 left loudest-window peaks around
    // 55-66% of the 0.851 ceiling (author, live, after hearing it: "um
    // pouco mais perto do teto mas com margem de segurança"). 0.52 moves
    // loudest peaks up toward ~75-80% of ceiling, close enough to nudge
    // LinkedOverloadProtector's own knee (threshold_ 0.62) into gently
    // doing its job on the hottest moments instead of sitting idle, while
    // OutputStage's knee (0.72-0.85) and hard ceiling stay clearly out of
    // reach - the safety margin the author asked for.
    float target_ = 0.52f;
    float minGain_ = 0.5f, maxGain_ = 4.0f;
    float envelope_ = 0.0f;
    float gain_ = 1.0f;
};
} // namespace antitotem
