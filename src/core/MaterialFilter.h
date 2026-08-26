#pragma once

#include <algorithm>
#include <cmath>

namespace antitotem
{
// Original digital study of a "loaded"/asymmetric filter character -
// critically inspired by Polivoks and OTA-driven filter behaviour, not an
// emulation of either. Where CmosVcf (see CmosVcf.h) is a clean linear
// state-variable filter, this one puts an asymmetric saturator directly
// inside the resonance feedback path: positive and negative excursions see
// different drive, so the filter's own material starts to colour and
// destabilise the sound at high RESONANCE/DRIVE, the way a real OTA or
// diode-loaded stage would. Crucially, that same saturator is also what
// keeps the filter numerically stable: tanh() cannot diverge, unlike a
// purely linear feedback coefficient (see CmosVcf's own instability, found
// and fixed earlier in the same session - TAREFAS.md, 17 ago. 2026). Full
// resonance/drive/asymmetry sweep verified bounded; see
// MaterialFilterTests in tests/SimpleSequencerTests.cpp.
class MaterialFilter
{
public:
    void prepare(double value) noexcept { sampleRate = std::max(100.0, value); reset(); }
    void reset() noexcept { stage1 = stage2 = 0.0f; }
    void setCutoff(float value) noexcept { cutoff = std::clamp(value, 0.0f, 1.0f); }
    void setResonance(float value) noexcept { resonance = std::clamp(value, 0.0f, 1.0f); }
    // DRIVE: how hard the asymmetric saturator is pushed. 0 is nearly clean;
    // higher values bring in more of the "carregada" (loaded) character.
    void setDrive(float value) noexcept { drive = std::clamp(value, 0.0f, 1.0f); }
    // ASYMMETRY: 0 saturates positive/negative excursions identically
    // (a symmetric soft clip); 1 makes the negative side noticeably less
    // driven than the positive side, the source of the filter's "matéria
    // assimétrica" - a lopsided, uneven response, not a decorative curve.
    void setAsymmetry(float value) noexcept { asymmetry = std::clamp(value, 0.0f, 1.0f); }
    // MIX: dry/wet against the raw `input`, same idiom as every other
    // effect in the instrument (MaterialReverb, PhaseField, FlangerField,
    // CombResonator - see MaterialEffects.h). At 0 this stage is
    // transparent; at 1 the output is fully the driven/asymmetric signal.
    void setMix(float value) noexcept { mix = std::clamp(value, 0.0f, 1.0f); }

    [[nodiscard]] float process(float input, float cv, float mixScale = 1.0f) noexcept
    {
        const auto shapedCutoff = std::clamp(cutoff + (cv - 0.5f) * 0.4f, 0.0f, 1.0f);
        const auto hertz = std::clamp(30.0f * std::pow(2.0f, shapedCutoff * 7.5f),
                                      20.0f, static_cast<float>(sampleRate * 0.45));
        // A conservative one-pole coefficient (well inside its own stable
        // range on its own) - the nonlinearity below is what carries the
        // filter's character and resonance, not a high linear coefficient.
        const auto g = std::clamp(static_cast<float>(hertz / sampleRate) * 3.0f, 0.001f, 0.85f);
        const auto feedback = shapeAsymmetric(stage2) * resonance * 3.6f;
        const auto driven = shapeAsymmetric((input - feedback) * (1.0f + drive * 2.0f));
        stage1 += g * (driven - stage1);
        stage2 += g * (stage1 - stage2);
        if (!std::isfinite(stage1) || !std::isfinite(stage2)) reset();
        return input + (stage2 - input) * mix * std::clamp(mixScale, 0.0f, 1.0f);
    }

private:
    [[nodiscard]] float shapeAsymmetric(float x) const noexcept
    {
        const auto driveAmount = 1.0f + drive * 3.0f;
        if (x >= 0.0f) return std::tanh(x * driveAmount);
        return std::tanh(x * driveAmount * (1.0f - asymmetry * 0.55f));
    }

    double sampleRate = 48000.0;
    float stage1 = 0.0f, stage2 = 0.0f;
    float cutoff = 0.5f, resonance = 0.3f, drive = 0.3f, asymmetry = 0.4f, mix = 1.0f;
};
} // namespace antitotem
