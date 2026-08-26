#pragma once

#include <algorithm>
#include <cmath>

namespace antitotem
{
// Original digital study of a CMOS-inverter VCF principle: gain inside a
// voltage-shaped RC network. It is not an emulation of a 4069 circuit.
class CmosVcf
{
public:
    // Four audible projections of the same stable state-variable core - not
    // an emulation of a specific integrated circuit. Bitmask, not a single
    // enum value: low/band/high are already computed every sample
    // regardless of which projection(s) get read, so combining more than
    // one (LPF+BPF, say) costs nothing extra and is a real, coherent
    // filter response, not a trick (author, live, 17 ago. 2026: "dois ou
    // mais" - decided against the trade-off of a mutually-exclusive radio
    // group here specifically because this core already had every piece
    // needed for a real blend). NOTCH standing for low+high is itself
    // exactly what LPF+HPF selected together already sum to; it stays its
    // own bit purely as a one-click shortcut, not a distinct signal path.
    enum Mode : unsigned char { lowpass = 1, bandpass = 2, highpass = 4, notch = 8 };

    void prepare(double value) noexcept { sampleRate = std::max(100.0, value); reset(); }
    void reset() noexcept { low = band = 0.0f; }
    void setCutoff(float value) noexcept { cutoff = std::clamp(value, 0.0f, 1.0f); }
    void setResonance(float value) noexcept { resonance = std::clamp(value, 0.0f, 0.88f); }
    void setCvDepth(float value) noexcept { cvDepth = std::clamp(value, -1.0f, 1.0f); }
    // mask is any OR-combination of the Mode bits above; 0 is treated as
    // lowpass (the original single-mode default) rather than silence.
    void setModeMask(unsigned char mask) noexcept { modeMask = mask; }

    [[nodiscard]] float process(float input, float cv) noexcept
    {
        const auto shapedCv = std::clamp(cutoff + (cv - 0.5f) * cvDepth, 0.0f, 1.0f);
        const auto hertz = std::clamp(25.0f * std::pow(2.0f, shapedCv * 8.2f),
                                      8.0f, static_cast<float>(sampleRate * 0.18));
        const auto damping = 1.72f - resonance * 1.42f;
        // This naive (non-TPT) state-variable topology is only stable while
        // coefficient * damping stays under 1 - above that the pole leaves
        // the unit circle and low/band diverge exponentially. Float can
        // hold the divergence for thousands of samples before it finally
        // overflows to inf, so the isfinite() safety net below doesn't
        // catch it until long after it has already swamped the real
        // signal (found live: at resonance 0 and cutoff at/near max, low/
        // band reached ~1e37 well before going non-finite - see
        // TAREFAS.md, "PAN dos osciladores" investigation, 17 ago. 2026).
        // The old fixed 0.18*sampleRate hertz cap was only safe at high
        // resonance/low damping; cap the coefficient itself, scaled to the
        // current damping, so every resonance setting stays inside the
        // stability boundary instead of just the high-resonance ones.
        const auto maxStableCoefficient = 0.95f / damping;
        const auto coefficient = std::min(2.0f * std::sin(static_cast<float>(pi * hertz / sampleRate)), maxStableCoefficient);
        low += coefficient * band;
        const auto high = input - low - damping * band;
        band += coefficient * high;
        if (!std::isfinite(low) || !std::isfinite(band)) reset();
        // NOTCH wants both low and high (that IS its definition), so this
        // is phrased as "which of the three underlying signals does the
        // mask want" rather than "sum one term per selected bit" - LPF+
        // NOTCH together still only counts low once, and NOTCH alone
        // lands exactly on the same normalised low+high blend that
        // selecting LPF+HPF together would, instead of the two reading as
        // different loudness for what is the same combination either way.
        const bool wantLow = (modeMask & (Mode::lowpass | Mode::notch)) != 0;
        const bool wantHigh = (modeMask & (Mode::highpass | Mode::notch)) != 0;
        const bool wantBand = (modeMask & Mode::bandpass) != 0;
        float sum = 0.0f;
        int active = 0;
        if (wantLow) { sum += low; ++active; }
        if (wantHigh) { sum += high; ++active; }
        if (wantBand) { sum += band; ++active; }
        if (active == 0) return low;
        return sum / static_cast<float>(active);
    }

private:
    static constexpr double pi = 3.14159265358979323846;
    double sampleRate = 48000.0;
    float low = 0.0f, band = 0.0f;
    float cutoff = 0.58f, resonance = 0.24f, cvDepth = 0.52f;
    unsigned char modeMask = Mode::lowpass;
};
} // namespace antitotem
