// Offline CPU benchmark for DualObjectEngine (PRINCIPAL + CLONE together,
// exactly what the app renders every callback). Not a substitute for a
// real-device xrun/deadline test - see CPU_BASELINE.md for how to read
// these numbers and their limits.
#include "core/DualObjectEngine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace antitotem;

namespace
{
// Both objects fully patched - every mixer channel on, every effect
// engaged with a non-zero send - so the benchmark measures the DSP the
// engine can actually be asked to run, not an idle/bypassed path.
void configureVoice(SimpleSequencer& seq, float panBias)
{
    seq.setRunning(true);
    for (std::size_t i = 0; i < 5; ++i)
    {
        seq.setOscillatorLevel(i, i < 3 ? 0.6f : 0.0f);
        seq.setOscillatorPan(i, panBias * (i % 2 == 0 ? 1.0f : -1.0f));
    }
    seq.setFilterCutoff(0.6f); seq.setFilterResonance(0.35f); seq.setFilterCvDepth(0.4f);
    seq.setEnvelopeAttack(0.05f); seq.setEnvelopeDecay(0.2f); seq.setEnvelopeSustain(0.6f); seq.setEnvelopeRelease(0.3f);
    seq.setRingMix(0.4f); seq.setNoiseMix(0.3f); seq.setSampleHoldMix(0.5f);
    seq.setReverbMix(0.4f); seq.setReverbFeedback(0.5f);
    seq.setPhaserMix(0.4f); seq.setPhaserRate(0.6f); seq.setPhaserDepth(0.6f);
    seq.setFlangerMix(0.4f); seq.setFlangerRate(0.4f); seq.setFlangerDepth(0.5f);
    seq.setResonatorMix(0.4f); seq.setResonatorPitch(0.5f); seq.setResonatorDamping(0.5f);
    for (std::size_t c = 0; c < 4; ++c)
    {
        MutableMixer::Channel channel;
        channel.enabled = true;
        channel.gain = 0.8f;
        seq.setMixChannel(c, channel);
    }
    for (std::size_t s = 0; s < SimpleSequencer::stepCount; ++s)
    {
        seq.setStepVoltage(s, 0.2f + 0.05f * static_cast<float>(s % 8));
        seq.setStepLevel(s, 1.0f);
        seq.setStepEffectSend(s, 0.7f);
    }
}

double renderSeconds(double sampleRate, std::size_t blockSize, double seconds)
{
    DualObjectEngine engine;
    engine.prepare(sampleRate);
    engine.setRunning(true);
    configureVoice(engine.object1(), 0.6f);
    configureVoice(engine.object5(), -0.6f);
    engine.setObjectConnection(0.3f, 0.25f);
    engine.setConnectionRoutes(DualObjectEngine::direct | DualObjectEngine::capacitor,
                                DualObjectEngine::diode | DualObjectEngine::pulse);

    std::vector<float> left(blockSize), right(blockSize);

    // Warm up (fill delay lines, envelopes, filter state) before timing.
    for (std::size_t done = 0; done < static_cast<std::size_t>(0.25 * sampleRate); done += blockSize)
        engine.render(left.data(), right.data(), blockSize);

    double checksum = 0.0;
    const auto totalSamples = static_cast<std::size_t>(seconds * sampleRate);
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t done = 0; done < totalSamples; done += blockSize)
    {
        engine.render(left.data(), right.data(), blockSize);
        checksum += left[0] + right[0];
    }
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    // A finite-checksum requirement guards against ever measuring an
    // accidentally silent/optimized-away render path.
    if (!std::isfinite(checksum))
    {
        std::fprintf(stderr, "non-finite checksum - render path is not producing real audio\n");
        std::exit(1);
    }
    return elapsed;
}
} // namespace

int main()
{
    struct Case { double sampleRate; std::size_t blockSize; };
    const Case cases[] = {
        { 44100.0, 32 }, { 44100.0, 128 }, { 44100.0, 256 }, { 44100.0, 512 },
        { 48000.0, 128 }, { 96000.0, 128 }, { 96000.0, 512 },
    };
    const double secondsPerWindow = 2.0;
    const int runsPerCase = 5;

    std::printf("sample_rate_hz,block_size,cpu_percent,realtime_factor\n");
    for (const auto& c : cases)
    {
        std::vector<double> elapsedTimes;
        elapsedTimes.reserve(static_cast<std::size_t>(runsPerCase));
        for (int r = 0; r < runsPerCase; ++r)
            elapsedTimes.push_back(renderSeconds(c.sampleRate, c.blockSize, secondsPerWindow));
        std::sort(elapsedTimes.begin(), elapsedTimes.end());
        const double median = elapsedTimes[elapsedTimes.size() / 2];
        const double cpuPercent = 100.0 * median / secondsPerWindow;
        const double realtimeFactor = secondsPerWindow / median;
        std::printf("%.0f,%zu,%.2f,%.2f\n", c.sampleRate, c.blockSize, cpuPercent, realtimeFactor);
    }
    return 0;
}
