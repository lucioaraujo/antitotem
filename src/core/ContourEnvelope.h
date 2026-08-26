#pragma once

#include <algorithm>
#include <cmath>

namespace antitotem
{
// A safe, original digital contour. Its language is ADSR, not a clone of a
// particular 4093/4066/TL084 circuit.
class ContourEnvelope
{
public:
    void prepare(double value) noexcept { sampleRate = std::max(100.0, value); reset(); }
    void reset() noexcept { stage = Stage::idle; level = 0.0f; gateWasHigh = false; }
    void setAttack(float value) noexcept { attack = seconds(value); }
    void setDecay(float value) noexcept { decay = seconds(value); }
    void setSustain(float value) noexcept { sustain = std::clamp(value, 0.0f, 1.0f); }
    void setRelease(float value) noexcept { release = seconds(value); }
    void trigger() noexcept { stage = Stage::attack; }

    [[nodiscard]] float process(bool gate) noexcept
    {
        if (gate && !gateWasHigh) trigger();
        if (!gate && gateWasHigh && stage != Stage::idle) stage = Stage::release;
        gateWasHigh = gate;
        switch (stage)
        {
            case Stage::idle: break;
            case Stage::attack:
                level += increment(1.0f, attack);
                if (level >= 1.0f) { level = 1.0f; stage = Stage::decay; }
                break;
            case Stage::decay:
                level -= increment(1.0f - sustain, decay);
                if (level <= sustain) { level = sustain; stage = gate ? Stage::sustain : Stage::release; }
                break;
            case Stage::sustain:
                level = sustain;
                if (!gate) stage = Stage::release;
                break;
            case Stage::release:
                // Exponential release reaches the inaudible floor at the selected
                // duration instead of asymptotically leaving a ghost value alive.
                level *= std::exp(std::log(0.0001f) / std::max(1.0f, release * static_cast<float>(sampleRate)));
                if (level <= 0.0001f) { level = 0.0f; stage = Stage::idle; }
                break;
        }
        return std::clamp(level, 0.0f, 1.0f);
    }

private:
    enum class Stage : unsigned char { idle, attack, decay, sustain, release };
    [[nodiscard]] static float seconds(float normalized) noexcept
    {
        return 0.0015f * std::pow(2.0f, std::clamp(normalized, 0.0f, 1.0f) * 10.4f);
    }
    [[nodiscard]] float increment(float distance, float duration) const noexcept
    {
        return distance / std::max(1.0f, duration * static_cast<float>(sampleRate));
    }
    double sampleRate = 48000.0;
    float attack = 0.012f, decay = 0.18f, sustain = 0.62f, release = 0.25f, level = 0.0f;
    bool gateWasHigh = false;
    Stage stage = Stage::idle;
};
} // namespace antitotem
