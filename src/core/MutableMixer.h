#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace antitotem
{
class MutableMixer
{
public:
    static constexpr std::size_t channelCount = 4;
    static constexpr std::size_t memoryCount = 8;
    struct Channel { float gain = 1.0f, pan = 0.0f, reflux = 0.0f; bool enabled = false, mute = false, solo = false; };

    // No-op: channels holds user configuration (ON/M/S, gain, pan, reflux),
    // not transient DSP state - there is nothing else in this class RESET
    // should legitimately clear. It used to force channels back to a
    // hardcoded default (FILTER/RING/NOISE off, SPACE on) here, silently
    // overriding whatever the ON toggles showed, with no UI resync - a
    // real bug the author found live: a channel showing ON with volume
    // going silent after RESET (or SPACE staying audible after RESET even
    // with everything set to off), fixed only by re-toggling ON, which
    // happens to call syncMixer() and re-push the UI's real state.
    void reset() noexcept {}
    void setChannel(std::size_t index, Channel value) noexcept
    {
        if (index >= channelCount) return;
        value.gain = std::clamp(value.gain, 0.0f, 1.5f);
        value.pan = std::clamp(value.pan, -1.0f, 1.0f);
        value.reflux = std::clamp(value.reflux, 0.0f, 0.72f);
        channels[index] = value;
    }
    [[nodiscard]] Channel getChannel(std::size_t index) const noexcept { return index < channelCount ? channels[index] : Channel {}; }
    // Same gate process() applies per-channel (enabled, not muted, and
    // either nothing is soloed or this channel is the solo) - exposed so
    // callers adding a signal derived from one channel's own source
    // outside of process()'s own sources array (e.g. a stereo-differential
    // term computed from the same signal that feeds channel `index`) can
    // respect that channel's ON/M/S instead of always leaking through.
    [[nodiscard]] bool isChannelActive(std::size_t index) const noexcept
    {
        if (index >= channelCount) return false;
        const bool anySolo = std::any_of(channels.begin(), channels.end(), [] (const Channel& c) { return c.solo; });
        const auto& c = channels[index];
        return c.enabled && !c.mute && (!anySolo || c.solo);
    }
    void capture(std::size_t slot) noexcept { if (slot < memoryCount) memories[slot] = channels; }
    void recall(std::size_t slot) noexcept { if (slot < memoryCount) channels = memories[slot]; }
    void process(const std::array<float, channelCount>& sources, float& left, float& right, float& feedback) const noexcept
    {
        const bool anySolo = std::any_of(channels.begin(), channels.end(), [] (const Channel& c) { return c.solo; });
        left = right = feedback = 0.0f;
        for (std::size_t i = 0; i < channelCount; ++i)
        {
            const auto& c = channels[i];
            if (!c.enabled || c.mute || (anySolo && !c.solo)) continue;
            const auto signal = sources[i] * c.gain;
            left += signal * std::sqrt(0.5f * (1.0f - c.pan));
            right += signal * std::sqrt(0.5f * (1.0f + c.pan));
            feedback += signal * c.reflux;
        }
        feedback = std::clamp(feedback * 0.35f, -0.72f, 0.72f);
    }

private:
    std::array<Channel, channelCount> channels { Channel {}, Channel {}, Channel {}, [] { Channel c; c.enabled = true; return c; }() };
    std::array<std::array<Channel, channelCount>, memoryCount> memories {};
};
} // namespace antitotem
