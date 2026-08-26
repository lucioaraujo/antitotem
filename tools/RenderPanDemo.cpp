// Offline listening-validation render for the PAN dos osciladores fix (see
// docs/TAREFAS.md, 17 ago. 2026). No JUCE/GUI/audio-device dependency - this
// renders straight from the same core DSP the app uses, so it can run
// headless. It does NOT replace human listening: it only produces the WAV
// for that listening to happen on, mirroring the author's own earlier test
// method (ANTITOTEM_2026-08-16_02-48-03.wav, OSC A only, pan changing every
// 15-20s) but walking through the specific channel combinations that used to
// dilute the pan, so the fix can be heard against each one directly.
#include "core/SimpleSequencer.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using namespace antitotem;

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
    {
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
}

// Only OSC A, matching the author's original test exactly. Configures a
// fresh segment's channel combination; pan itself is driven per-block by
// the caller so it can step through extremes within the segment.
void configureSegment(SimpleSequencer& seq, bool ring, bool noise, bool space)
{
    seq.reset();
    seq.setRunning(true);
    for (std::size_t i = 0; i < 5; ++i) seq.setOscillatorLevel(i, i == 0 ? 1.0f : 0.0f);
    seq.setOscillatorShape(0, 1.0f); // triangle-ish core, more harmonic content to judge stereo image by ear than a bare sine
    seq.setFilterCutoff(0.6f); seq.setFilterResonance(0.3f); seq.setFilterCvDepth(0.3f);
    seq.setEnvelopeAttack(0.02f); seq.setEnvelopeDecay(0.1f); seq.setEnvelopeSustain(0.8f); seq.setEnvelopeRelease(0.1f);
    seq.setRingMix(0.5f);
    seq.setNoiseMix(0.3f); seq.setSampleHoldMix(0.0f);
    seq.setReverbMix(0.45f); seq.setReverbFeedback(0.5f);
    seq.setPhaserMix(0.35f); seq.setPhaserRate(0.5f); seq.setPhaserDepth(0.6f);
    seq.setFlangerMix(0.35f); seq.setFlangerRate(0.3f); seq.setFlangerDepth(0.5f);
    seq.setResonatorMix(0.45f); seq.setResonatorPitch(0.4f); seq.setResonatorDamping(0.6f);
    for (std::size_t s = 0; s < SimpleSequencer::stepCount; ++s)
    {
        seq.setStepVoltage(s, 0.5f);
        seq.setStepLevel(s, 1.0f);
        seq.setStepEffectSend(s, 0.6f);
    }
    MutableMixer::Channel filterCh; filterCh.enabled = true; filterCh.gain = 1.0f;
    seq.setMixChannel(0, filterCh);
    MutableMixer::Channel ringCh; ringCh.enabled = ring; ringCh.gain = 1.0f;
    seq.setMixChannel(1, ringCh);
    MutableMixer::Channel noiseCh; noiseCh.enabled = noise; noiseCh.gain = 1.0f;
    seq.setMixChannel(2, noiseCh);
    MutableMixer::Channel spaceCh; spaceCh.enabled = space; spaceCh.gain = 1.0f;
    seq.setMixChannel(3, spaceCh);
}
} // namespace

int main()
{
    const double sampleRate = 44100.0;
    const double stepSeconds = 4.0;   // one pan position (hard left / centre / hard right)
    const int stepsPerSegment = 3;
    struct Segment { const char* label; bool ring, noise, space; };
    const Segment segments[] = {
        { "FILTER apenas (referencia, ja era correto antes)", false, false, false },
        { "FILTER + ESPACO (reverb/phaser/flanger/resonator - o que colapsava)", false, false, true },
        { "FILTER + RING", true, false, false },
        { "FILTER + RING + NOISE + ESPACO (combinacao completa)", true, true, true },
    };

    std::vector<float> left, right;
    SimpleSequencer seq;
    seq.prepare(sampleRate);

    for (const auto& segment : segments)
    {
        configureSegment(seq, segment.ring, segment.noise, segment.space);
        std::fprintf(stderr, "segmento: %s\n", segment.label);
        const float pans[stepsPerSegment] = { -1.0f, 0.0f, 1.0f };
        for (int step = 0; step < stepsPerSegment; ++step)
        {
            seq.setOscillatorPan(0, pans[step]);
            const auto samples = static_cast<std::size_t>(stepSeconds * sampleRate);
            for (std::size_t i = 0; i < samples; ++i)
            {
                float l = 0.0f, r = 0.0f;
                seq.renderSample(l, r);
                left.push_back(l);
                right.push_back(r);
            }
        }
    }

    const std::string path = std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/Downloads/antitotem_pan_fix_validacao.wav";
    writeWav24(path, left, right, sampleRate);
    std::fprintf(stderr, "escrito: %s (%.1f s)\n", path.c_str(), static_cast<double>(left.size()) / sampleRate);
    return 0;
}
