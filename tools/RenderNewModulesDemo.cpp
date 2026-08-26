// Listening-validation render for the two new prototype-stage modules
// (MaterialFilter, ChaosField/WanderSource - see docs/TAREFAS.md, "Retomar
// a engine musical", 17 ago. 2026). Neither module is wired into
// SimpleSequencer/the UI yet - this exercises them directly with simple
// hand-rolled test oscillators, the same isolation gate already used for
// OSC4/OSC5 before they were judged ready to integrate.
#include "core/MaterialFilter.h"
#include "core/ChaosSources.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace
{
void writeWav24(const std::string& path, const std::vector<float>& left,
                 const std::vector<float>& right, double sampleRate)
{
    const std::uint32_t frames = static_cast<std::uint32_t>(left.size());
    const std::uint16_t channels = 2, bitsPerSample = 24, blockAlign = static_cast<std::uint16_t>(channels * bitsPerSample / 8);
    const std::uint32_t byteRate = static_cast<std::uint32_t>(sampleRate) * blockAlign;
    const std::uint32_t dataBytes = frames * blockAlign;

    std::ofstream out(path, std::ios::binary);
    auto writeTag = [&] (const char* tag) { out.write(tag, 4); };
    auto writeU32 = [&] (std::uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto writeU16 = [&] (std::uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };

    writeTag("RIFF"); writeU32(36U + dataBytes); writeTag("WAVE");
    writeTag("fmt "); writeU32(16U); writeU16(1U); writeU16(channels);
    writeU32(static_cast<std::uint32_t>(sampleRate)); writeU32(byteRate);
    writeU16(blockAlign); writeU16(bitsPerSample);
    writeTag("data"); writeU32(dataBytes);

    for (std::uint32_t i = 0; i < frames; ++i)
        for (const auto sample : { left[i], right[i] })
        {
            const auto clamped = std::max(-1.0f, std::min(1.0f, sample));
            const auto value = static_cast<std::int32_t>(clamped * 8388607.0f);
            const unsigned char bytes[3] = {
                static_cast<unsigned char>(value & 0xff),
                static_cast<unsigned char>((value >> 8) & 0xff),
                static_cast<unsigned char>((value >> 16) & 0xff)
            };
            out.write(reinterpret_cast<const char*>(bytes), 3);
        }
}

// A plain sawtooth test tone - deliberately harmonically rich (not a bare
// sine) so a filter's character actually has something to act on.
class SawTone
{
public:
    void setFrequency(float hz, double sampleRate) { increment = hz / static_cast<float>(sampleRate); }
    float tick()
    {
        phase += increment;
        phase -= std::floor(phase);
        return 2.0f * phase - 1.0f;
    }
private:
    float phase = 0.0f, increment = 0.01f;
};
} // namespace

int main()
{
    const double sr = 44100.0;
    std::vector<float> left, right;
    auto appendMono = [&] (float sample) { left.push_back(sample); right.push_back(sample); };

    // --- Segment 1: MaterialFilter, sweeping RESONANCE/DRIVE/ASYMMETRY ---
    // Four 4s sub-segments on a held sawtooth: clean -> resonant -> driven
    // -> driven+asymmetric, so the "matéria carregada" character builds up
    // step by step instead of arriving all at once.
    {
        SawTone tone;
        tone.setFrequency(110.0f, sr);
        antitotem::MaterialFilter filter;
        filter.prepare(sr);
        filter.setCutoff(0.45f);
        struct Stage { float resonance, drive, asymmetry; };
        const Stage stages[] = {
            { 0.2f, 0.0f, 0.0f },
            { 0.85f, 0.0f, 0.0f },
            { 0.85f, 0.7f, 0.0f },
            { 0.85f, 0.7f, 1.0f },
        };
        for (const auto& stage : stages)
        {
            filter.setResonance(stage.resonance);
            filter.setDrive(stage.drive);
            filter.setAsymmetry(stage.asymmetry);
            for (int i = 0; i < static_cast<int>(4.0 * sr); ++i)
                appendMono(filter.process(tone.tick(), 0.5f) * 0.6f);
        }
    }

    // --- Segment 2: ChaosField driving MaterialFilter's cutoff ---
    // A held tone with fixed resonance/drive; only the chaotic double-well
    // source moves the cutoff, so its own character is what is heard
    // moving. Two 8s takes: settled-in-a-well vs. actively swinging.
    {
        SawTone tone;
        tone.setFrequency(146.0f, sr);
        antitotem::MaterialFilter filter;
        filter.prepare(sr);
        filter.setResonance(0.6f); filter.setDrive(0.3f); filter.setAsymmetry(0.3f);
        antitotem::ChaosField chaos;
        chaos.prepare(sr);
        struct Take { float rate, drive, damping; };
        const Take takes[] = {
            { 0.3f, 0.3f, 0.8f },  // low drive/high damping: mostly settled in one well
            { 6.0f, 0.9f, 0.15f }, // high drive/low damping: actively swings between wells
        };
        for (const auto& take : takes)
        {
            chaos.setRate(take.rate); chaos.setDrive(take.drive); chaos.setDamping(take.damping);
            for (int i = 0; i < static_cast<int>(8.0 * sr); ++i)
            {
                const auto cv = 0.5f + chaos.tick() * 0.45f;
                filter.setCutoff(cv);
                appendMono(filter.process(tone.tick(), 0.5f) * 0.55f);
            }
        }
    }

    // --- Segment 3: WanderSource driving pitch, slow organic drift ---
    // 10s at a moderate rate/full depth - should read as a smooth, non-
    // repeating glide, not steps or a cycle.
    {
        SawTone tone;
        antitotem::WanderSource wander;
        wander.prepare(sr);
        wander.setRate(0.6f); wander.setDepth(0.8f);
        for (int i = 0; i < static_cast<int>(10.0 * sr); ++i)
        {
            const auto semis = wander.tick() * 12.0f; // +-12 semitones
            const auto hz = 130.0f * std::pow(2.0f, semis / 12.0f);
            tone.setFrequency(hz, sr);
            appendMono(tone.tick() * 0.25f);
        }
    }

    const std::string path = std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/Downloads/antitotem_novos_modulos_validacao.wav";
    writeWav24(path, left, right, sr);
    std::fprintf(stderr, "escrito: %s (%.1f s)\n", path.c_str(), static_cast<double>(left.size()) / sr);
    return 0;
}
