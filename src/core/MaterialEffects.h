#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace antitotem
{
// Original multi-memory ambience: inspired by clocked/tapped delay principles,
// not an emulation of a CD4006 circuit.
class MaterialReverb
{
public:
    void prepare(double value)
    {
        sampleRate = std::max(100.0, value);
        const auto size = static_cast<std::size_t>(sampleRate * 0.19) + 2U;
        memory.assign(size, 0.0f);
        reset();
    }
    void reset() noexcept { std::fill(memory.begin(), memory.end(), 0.0f); write = 0; }
    void setMix(float value) noexcept { mix = std::clamp(value, 0.0f, 0.62f); }
    void setFeedback(float value) noexcept { feedback = std::clamp(value, 0.0f, 0.78f); }
    [[nodiscard]] float process(float input, float mixScale = 1.0f) noexcept
    {
        if (memory.empty()) return input;
        const auto tap = [&] (float seconds) {
            const auto distance = static_cast<std::size_t>(seconds * sampleRate);
            return memory[(write + memory.size() - std::min(distance, memory.size() - 1U)) % memory.size()];
        };
        const auto shortTap = tap(0.031f);
        const auto midTap = tap(0.071f);
        const auto longTap = tap(0.149f);
        const auto wet = (shortTap * 0.44f + midTap * 0.33f + longTap * 0.23f);
        memory[write] = std::tanh(input + wet * feedback);
        write = (write + 1U) % memory.size();
        return input + (wet - input) * mix * std::clamp(mixScale, 0.0f, 1.0f);
    }

private:
    double sampleRate = 48000.0;
    std::vector<float> memory;
    std::size_t write = 0;
    float mix = 0.0f, feedback = 0.36f;
};

// Original swept all-pass network. It is a phaser behaviour, not a transistor
// stage or a clone of a commercial pedal.
class PhaseField
{
public:
    void prepare(double value) noexcept { sampleRate = std::max(100.0, value); reset(); }
    void reset() noexcept { states.fill(0.0f); phase = 0.0f; }
    void setMix(float value) noexcept { mix = std::clamp(value, 0.0f, 0.75f); }
    void setRate(float value) noexcept { rate = std::clamp(value, 0.02f, 16.0f); }
    void setDepth(float value) noexcept { depth = std::clamp(value, 0.0f, 1.0f); }
    [[nodiscard]] float process(float input, float mixScale = 1.0f) noexcept
    {
        phase += rate / static_cast<float>(sampleRate);
        phase -= std::floor(phase);
        const auto motion = 0.5f + 0.5f * std::sin(phase * twoPi);
        const auto coefficient = std::clamp(0.12f + (0.74f * motion * depth), 0.05f, 0.91f);
        auto signal = input;
        for (auto& state : states)
        {
            const auto output = -coefficient * signal + state;
            state = signal + coefficient * output;
            signal = output;
        }
        return input + (signal - input) * mix * std::clamp(mixScale, 0.0f, 1.0f);
    }

private:
    static constexpr float twoPi = 6.28318530717958647692f;
    double sampleRate = 48000.0;
    std::array<float, 4> states {};
    float phase = 0.0f, mix = 0.0f, rate = 0.18f, depth = 0.62f;
};

// Short moving delay, kept separate from the longer material memories above.
class FlangerField
{
public:
    void prepare(double value)
    {
        sampleRate = std::max(100.0, value);
        memory.assign(static_cast<std::size_t>(sampleRate * 0.028) + 3U, 0.0f);
        reset();
    }
    void reset() noexcept { std::fill(memory.begin(), memory.end(), 0.0f); write = 0; phase = 0.0f; }
    void setMix(float value) noexcept { mix = std::clamp(value, 0.0f, 0.7f); }
    void setRate(float value) noexcept { rate = std::clamp(value, 0.02f, 8.0f); }
    void setDepth(float value) noexcept { depth = std::clamp(value, 0.0f, 1.0f); }
    [[nodiscard]] float process(float input, float mixScale = 1.0f) noexcept
    {
        if (memory.empty()) return input;
        phase += rate / static_cast<float>(sampleRate);
        phase -= std::floor(phase);
        const auto motion = 0.5f + 0.5f * std::sin(phase * twoPi);
        const auto delaySamples = (0.0015f + motion * depth * 0.015f) * static_cast<float>(sampleRate);
        const auto read = static_cast<float>(write) - delaySamples + static_cast<float>(memory.size());
        const auto base = static_cast<std::size_t>(read) % memory.size();
        const auto next = (base + 1U) % memory.size();
        const auto fraction = read - std::floor(read);
        const auto delayed = memory[base] + (memory[next] - memory[base]) * fraction;
        memory[write] = std::tanh(input + delayed * 0.32f);
        write = (write + 1U) % memory.size();
        return input + (delayed - input) * mix * std::clamp(mixScale, 0.0f, 1.0f);
    }

private:
    static constexpr float twoPi = 6.28318530717958647692f;
    double sampleRate = 48000.0;
    std::vector<float> memory;
    std::size_t write = 0;
    float phase = 0.0f, mix = 0.0f, rate = 0.16f, depth = 0.65f;
};

// A comb/resonator, deliberately distinct from MaterialReverb above: one
// short delay line with strong feedback, whose length sets an audible pitch
// rather than diffusing into ambience. Turns a short delay into cavity,
// height (the resonant pitch) and body (how long it rings before decaying).
class CombResonator
{
public:
    void prepare(double value)
    {
        sampleRate = std::max(100.0, value);
        // Up to ~50ms of delay - long enough for the lowest resonant pitch
        // to read as a body/cavity tone rather than a metallic buzz.
        memory.assign(static_cast<std::size_t>(sampleRate * 0.05) + 3U, 0.0f);
        reset();
    }
    void reset() noexcept { std::fill(memory.begin(), memory.end(), 0.0f); write = 0; }
    void setMix(float value) noexcept { mix = std::clamp(value, 0.0f, 0.8f); }
    // 0 is the longest delay (lowest resonant pitch/cavity), 1 the shortest
    // (highest pitch) - "altura" in the design language.
    void setPitch(float value) noexcept { pitch = std::clamp(value, 0.0f, 1.0f); }
    // How much of the delayed signal feeds back into itself - "corpo": low
    // damping decays fast, high damping rings on closer to a sustained tone.
    void setDamping(float value) noexcept { damping = std::clamp(value, 0.0f, 0.97f); }
    [[nodiscard]] float process(float input, float mixScale = 1.0f) noexcept
    {
        if (memory.empty()) return input;
        const auto minSamples = 4.0f;
        const auto maxSamples = static_cast<float>(memory.size()) - 2.0f;
        const auto delaySamples = maxSamples - pitch * (maxSamples - minSamples);
        const auto read = static_cast<float>(write) - delaySamples + static_cast<float>(memory.size());
        const auto base = static_cast<std::size_t>(read) % memory.size();
        const auto next = (base + 1U) % memory.size();
        const auto fraction = read - std::floor(read);
        const auto delayed = memory[base] + (memory[next] - memory[base]) * fraction;
        memory[write] = std::tanh(input + delayed * damping);
        write = (write + 1U) % memory.size();
        return input + (delayed - input) * mix * std::clamp(mixScale, 0.0f, 1.0f);
    }

private:
    double sampleRate = 48000.0;
    std::vector<float> memory;
    std::size_t write = 0;
    float mix = 0.0f, pitch = 0.5f, damping = 0.7f;
};
} // namespace antitotem
