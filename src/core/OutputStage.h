#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace antitotem
{
// Technical output protection. It is intentionally separate from the voice:
// creative saturation belongs to the patch; DC removal and peak containment do not.
class OutputStage
{
public:
    void prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        dcCoefficient = static_cast<float>(std::exp(-2.0 * pi * 18.0 / sampleRate));
        const auto delaySamples = std::max<std::size_t>(1, static_cast<std::size_t>(sampleRate * 0.002));
        delayLeft.assign(delaySamples, 0.0f);
        delayRight.assign(delaySamples, 0.0f);
        attack = coefficient(0.35); release = coefficient(90.0);
        reset();
    }

    void reset()
    {
        std::fill(delayLeft.begin(), delayLeft.end(), 0.0f);
        std::fill(delayRight.begin(), delayRight.end(), 0.0f);
        delayPosition = 0; previousInLeft = previousInRight = 0.0f;
        previousOutLeft = previousOutRight = 0.0f; envelope = 0.0f; gain = 1.0f;
    }

    void setMasterGain(float value) noexcept { masterGain = std::clamp(value, 0.0f, 1.0f); }

    void process(float inputLeft, float inputRight, float& outputLeft, float& outputRight) noexcept
    {
        const auto dcFreeLeft = dcBlock(inputLeft, previousInLeft, previousOutLeft) * masterGain;
        const auto dcFreeRight = dcBlock(inputRight, previousInRight, previousOutRight) * masterGain;
        const auto peak = std::max(std::abs(dcFreeLeft), std::abs(dcFreeRight));
        envelope += (peak - envelope) * (peak > envelope ? attack : release);
        const auto targetGain = envelope > ceiling ? ceiling / envelope : 1.0f;
        gain += (targetGain - gain) * (targetGain < gain ? attack : release);

        // Look-ahead handles sustained energy; this final soft knee catches
        // single-sample events from noise, S&H and route changes without a
        // discontinuous hard clip.
        outputLeft = softCeiling(delayLeft[delayPosition] * gain);
        outputRight = softCeiling(delayRight[delayPosition] * gain);
        delayLeft[delayPosition] = dcFreeLeft;
        delayRight[delayPosition] = dcFreeRight;
        delayPosition = (delayPosition + 1) % delayLeft.size();
    }

private:
    [[nodiscard]] float coefficient(double milliseconds) const noexcept
    {
        return static_cast<float>(1.0 - std::exp(-1.0 / std::max(1.0, milliseconds * sampleRate / 1000.0)));
    }
    [[nodiscard]] float dcBlock(float value, float& previousInput, float& previousOutput) noexcept
    {
        const auto result = value - previousInput + dcCoefficient * previousOutput;
        previousInput = value; previousOutput = result;
        return result;
    }
    [[nodiscard]] static float softCeiling(float value) noexcept
    {
        const auto magnitude = std::abs(value);
        if (magnitude <= kneeStart) return value;
        const auto curved = kneeStart + (ceiling - kneeStart)
            * (1.0f - std::exp(-(magnitude - kneeStart) / (ceiling - kneeStart)));
        return std::copysign(curved, value);
    }

    static constexpr double pi = 3.14159265358979323846;
    static constexpr float ceiling = 0.85f; // about -1.4 dBFS, before external conversion.
    static constexpr float kneeStart = 0.72f;
    double sampleRate = 48000.0;
    float dcCoefficient = 0.997f, attack = 0.5f, release = 0.01f;
    float previousInLeft = 0.0f, previousInRight = 0.0f;
    float previousOutLeft = 0.0f, previousOutRight = 0.0f;
    float envelope = 0.0f, gain = 1.0f, masterGain = 0.72f;
    std::vector<float> delayLeft, delayRight;
    std::size_t delayPosition = 0;
};
} // namespace antitotem
