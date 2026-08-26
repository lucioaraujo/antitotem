#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace antitotem
{
// Ported from RASGO_SYNTH's rasgo::LinkedOverloadProtector (technique, not
// schematic - same reuse policy as SignalLeveler.h). A matched stereo
// overload knee for the summed mixer bus. Below the threshold this is
// mathematically an identity operation. Above it, both channels use the
// same continuous tanh curve, so dozens of individually-safe buses (FILTRO/
// RING/NOISE/ESPAÇO) summing hot don't force OutputStage's fast
// envelope-follower into large, sudden gain reductions on its own - that
// forced-alone behaviour is what reads as pumping/"clipando". It is overload
// protection, not loudness normalization or an always-on tone.
class LinkedOverloadProtector
{
public:
    void setThresholdAndLimit(float threshold, float limit)
    {
        threshold_ = std::clamp(threshold, 0.05f, 1.5f);
        limit_ = std::clamp(limit, threshold_ + 0.01f, 2.0f);
    }

    void process(float inL, float inR, float& outL, float& outR)
    {
        if (!std::isfinite(inL)) inL = 0.0f;
        if (!std::isfinite(inR)) inR = 0.0f;
        const auto peak = std::max(std::abs(inL), std::abs(inR));
        if (peak > threshold_)
        {
            ++overloadFrames_;
            const auto protectedPeak = protectMagnitude(peak);
            const auto reduction = 20.0f * std::log10(peak / std::max(protectedPeak, 1.0e-9f));
            maxReductionDb_ = std::max(maxReductionDb_, reduction);
        }
        outL = protectSample(inL);
        outR = protectSample(inR);
    }

    [[nodiscard]] std::size_t overloadFrames() const noexcept { return overloadFrames_; }
    [[nodiscard]] float maxReductionDb() const noexcept { return maxReductionDb_; }

private:
    [[nodiscard]] float protectMagnitude(float magnitude) const
    {
        if (magnitude <= threshold_) return magnitude;
        const auto width = limit_ - threshold_;
        return threshold_ + width * std::tanh((magnitude - threshold_) / width);
    }

    [[nodiscard]] float protectSample(float sample) const
    {
        return std::copysign(protectMagnitude(std::abs(sample)), sample);
    }

    // Tuned for Antitotem's 0.85 ceiling (OutputStage.h), not RASGO_SYNTH's
    // 1.0 scale: threshold sits just below OutputStage's own soft-knee start
    // (0.72), so this catches multi-bus overs first, before that final knee
    // has to react at all. Pending human listening validation, same as the
    // rest of this gain-staging work (see docs/TAREFAS.md).
    float threshold_ = 0.62f;
    float limit_ = 1.0f;
    std::size_t overloadFrames_ = 0;
    float maxReductionDb_ = 0.0f;
};
} // namespace antitotem
