// Saída técnica do Antitotem (16/11/2026, nivelamento pela saida-ref 1.0,
// RASGO/REFERENCIAS/SAIDA_TECNICA). Prova: abaixo do joelho nada muda; o
// TRUE peak fica no teto; NaN/Inf não envenenam o estágio; o detector
// passa nos sinais 15-23 da EBU Tech 3341; o dither TPDF chega intacto ao
// WAV de 24 bits do JUCE.
#include "core/Dither.h"
#include "core/OutputStage.h"
#include "core/TruePeakDetector.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace
{
constexpr double pi = 3.14159265358979323846;
int failures = 0;
void check(const std::string& name, bool ok)
{
    std::cout << (ok ? "ok    " : "FAIL  ") << name << '\n';
    if (!ok) ++failures;
}

void belowTheKneeNothingChanges()
{
    antitotem::OutputStage stage;
    stage.prepare(48000.0);
    stage.setMasterGain(1.0f);
    double in = 0.0, out = 0.0;
    float worst = 0.0f;
    for (int n = 0; n < 48000; ++n)
    {
        const auto x = static_cast<float>(0.5 * std::sin(2.0 * pi * 440.0 * n / 48000.0));
        float l = 0, r = 0;
        stage.process(x, x, l, r);
        if (n > 4800) { in += static_cast<double>(x) * x; out += static_cast<double>(l) * l; }
        worst = std::max(worst, stage.telemetry().maxReductionDb);
    }
    const auto db = 10.0 * std::log10(out / in);
    std::cout << "      0.5 FS, 440 Hz: level " << db << " dB, reduction " << worst << " dB\n";
    check("below the knee: level unchanged (< 0.01 dB) and no reduction", std::abs(db) < 0.01 && worst == 0.0f);
}

void truePeakAtTheCeiling()
{
    // signals whose samples sit under their real peak (fs/4 at 45 degrees), hot
    const std::vector<float (*)(int)> signals {
        [](int n) { return 2.0f * static_cast<float>(std::sin(pi / 2.0 * n + pi / 4.0)); },
        [](int n) { return ((n / 4800) % 2 ? 2.0f : 0.05f) * static_cast<float>(std::sin(pi / 2.0 * n + pi / 4.0)); },
        [](int n) { return 3.0f * static_cast<float>(std::sin(2.0 * pi * 11025.0 * n / 48000.0 + 0.3)); } };
    float worst = 0.0f;
    for (const auto& signal : signals)
    {
        antitotem::OutputStage stage;
        stage.prepare(48000.0);
        stage.setMasterGain(1.0f);
        antitotem::TruePeakDetector meter;
        meter.prepare(48000.0);
        for (int n = 0; n < 96000; ++n)
        {
            const auto x = signal(n);
            float l = 0, r = 0;
            stage.process(x, x, l, r);
            const auto tp = meter.processSample(l);
            if (n > 24000) worst = std::max(worst, tp);
        }
    }
    std::cout << "      worst true peak out: " << 20.0 * std::log10(worst) << " dBTP (ceiling 0.85 = "
              << 20.0 * std::log10(0.85) << ")\n";
    check("true peak stays at the ceiling (<= 0.85 + 1%)", worst <= 0.85f * 1.01f);
}

void invalidSamplesDoNotPoison()
{
    antitotem::OutputStage stage;
    stage.prepare(48000.0);
    float l = 0, r = 0;
    stage.process(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), l, r);
    bool finite = true, sound = false;
    for (int n = 0; n < 4800; ++n)
    {
        const auto x = static_cast<float>(0.3 * std::sin(2.0 * pi * 220.0 * n / 48000.0));
        stage.process(x, x, l, r);
        finite = finite && std::isfinite(l) && std::isfinite(r);
        sound = sound || std::abs(l) > 0.1f;
    }
    check("NaN/Inf: counted, output stays finite and the sound comes back",
          finite && sound && stage.telemetry().invalidSamples == 2u);
}

// EBU Tech 3341 V4, table 1, signals 15-19 (tones; 20-23 are in the
// reference's EbuTruePeakTests): expected +0.2/-0.4 dBTP
void ebuTruePeak()
{
    struct Case { const char* name; double frequency, amplitude, phase, expected; };
    const Case cases[] { { "#15 fs/4 0.50 0deg", 12000.0, 0.5, 0.0, -6.0 }, { "#16 fs/4 0.50 45deg", 12000.0, 0.5, 45.0, -6.0 },
                         { "#17 fs/6 0.50 60deg", 8000.0, 0.5, 60.0, -6.0 }, { "#18 fs/8 0.50 67.5deg", 6000.0, 0.5, 67.5, -6.0 },
                         { "#19 fs/4 1.41 45deg", 12000.0, 1.41, 45.0, 3.0 } };
    for (const auto& c : cases)
    {
        antitotem::TruePeakDetector detector;
        detector.prepare(48000.0);
        float peak = 0.0f;
        for (int n = 0; n < 48000; ++n)
        {
            const auto fade = std::min({ 1.0, n / 480.0, (47999 - n) / 480.0 });
            peak = std::max(peak, detector.processSample(static_cast<float>(
                fade * c.amplitude * std::sin(2.0 * pi * c.frequency * n / 48000.0 + c.phase * pi / 180.0))));
        }
        const auto db = 20.0 * std::log10(peak);
        check(std::string("EBU 3341 ") + c.name + ": " + std::to_string(db) + " dBTP",
              db <= c.expected + 0.2 && db >= c.expected - 0.4);
    }
}

// JUCE 9.0.3 writes 24-bit WAV as round(x * (2^31 - 1)) >> 8: every
// integer must come out exactly as the dither chose it
void ditherReachesTheFileIntact()
{
    const auto juce24 = [](float x) {
        const double s = x;
        const auto i32 = s <= -1.0 ? std::numeric_limits<int>::min()
                       : s >= 1.0 ? std::numeric_limits<int>::max()
                       : static_cast<int>(std::lround(std::numeric_limits<int>::max() * s));
        return i32 >> 8;
    };
    bool exact = true;
    for (std::int32_t k = -8388608; k <= 8388607 && exact; ++k)
        exact = juce24(antitotem::floatForJuceInt24(k)) == k;
    check("every 24-bit integer survives JUCE's conversion", exact);

    antitotem::TpdfQuantiser q(24);
    double sum = 0.0, worst = 0.0;
    for (int n = 0; n < 200000; ++n)
    {
        const auto x = static_cast<float>(0.3 * std::sin(2.0 * pi * 997.0 * n / 48000.0));
        const auto e = static_cast<double>(juce24(antitotem::floatForJuceInt24(q.quantise(x)))) - x * 8388607.0;
        sum += e; worst = std::max(worst, std::abs(e));
    }
    std::cout << "      TPDF 24-bit: mean error " << sum / 200000.0 << " LSB, worst " << worst << " LSB\n";
    check("TPDF: error within 1.5 LSB, no mean", worst <= 1.5 + 1.0e-6 && std::abs(sum / 200000.0) < 0.01);
}
}

int main()
{
    belowTheKneeNothingChanges();
    truePeakAtTheCeiling();
    invalidSamplesDoNotPoison();
    ebuTruePeak();
    ditherReachesTheFileIntact();
    std::cout << (failures == 0 ? "Antitotem output quality passed\n" : "Antitotem output quality FAILED\n");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
