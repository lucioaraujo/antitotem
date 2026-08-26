#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_data_structures/juce_data_structures.h>
#if ANTITOTEM_HAS_LOGO
#include <BinaryData.h>
#endif
#include "core/DualObjectEngine.h"
#include "core/ObjectVariations.h"
#include "core/SimpleSequencer.h"
#include "UiLanguage.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <optional>
#include <vector>

namespace
{
namespace material
{
// Palette sampled conceptually from the Antitotem flower-field header:
// black ground, vermilion mark, pink matter, foliage, and the wooden object.
const juce::Colour shadow { 0xff110d0e };
const juce::Colour board { 0xffc98a2b };
const juce::Colour wood { 0xff9a5a27 };
const juce::Colour metal { 0xffffd4bc };
const juce::Colour clock { 0xff3e7440 };
const juce::Colour memory { 0xffe96c9c };
const juce::Colour returnPath { 0xffdd2f37 };
const juce::Colour voice { 0xfff18aae };
// The small cobalt knobs in the photographed object: a physical-control
// accent, deliberately rare so it never competes with red REC or pink memory.
const juce::Colour controlBlue { 0xff2477b8 };
const juce::Colour historicalLogoRed { 0xffff1c12 };
// REVERB/PHASER/FLANGER/NOISE SEND (18 ago. 2026) - these used to borrow
// memory/clock/voice by array index, which happened to collide with ROTAS
// ATIVAS (memory) and CAOS/LFO (clock) with no functional reason (author,
// live: "o reverb e o ring são rosa, tem alguma relação com o rotas
// ativas?" -> "acertaremos todas as cores"). RING keeps using controlBlue
// above instead of a new colour of its own - already free, already meant
// to be a rare accent. REVERB/PHASER/FLANGER and the ESPAÇO/FASE title all
// share this single `phaser` colour now (author: "deixa tudo com a cor
// atual do phaser") - one uniform colour for the whole column, same
// pattern as CAOS/MATÉRIA, rather than 3 different accents within it.
const juce::Colour phaser { 0xffd4874a };
const juce::Colour noiseSend { 0xff9b6fc9 };
// VCF/ADSR knobs used to render with the LookAndFeel's plain default
// rotary fill - no accent of their own, unlike every other knob group in
// the panel (ENERGIA/CLOCK/MASTER/RING/LFO/NOISE SEND) - so they read as
// visually anonymous next to those (18 ago. 2026, author: "agora
// precisamos mudar as cores dos knobs do vcf e do adsr (pra
// diferenciarmos dos demais), sugira algo" - both hues chosen from
// candidates offered, picked to sit apart from every colour already
// claimed above: `vcf` is a cool cyan/petrol, unlike anything else in
// this palette; `adsr` is a terracotta, warm but noticeably more muted/
// dusty than `phaser`'s orange so the two don't read as the same colour
// at a glance.
const juce::Colour vcf { 0xff2f9e94 };
const juce::Colour adsr { 0xffc9714a };
}

// CLONE's own accents - identical structure/roles to `material` above
// (green=clock, pink=memory, red=feedback, amber=board...), every hue
// rotated the same fixed amount so the whole tab reads at a different
// colour temperature at a glance, without touching what any colour
// actually signals or hand-picking a second palette from scratch
// (author, live: "layout aprovado, agora passar para etapa de mudar as
// cores entre o principal e clone" - "deslocar o matiz" was the chosen
// approach over swapping roles or a from-scratch palette). shadow stays
// unrotated - a near-black doesn't read as a colour shift either way.
namespace cloneMaterial
{
constexpr float hueShift = 0.10f;
const juce::Colour shadow = material::shadow;
const juce::Colour board = material::board.withRotatedHue(hueShift);
const juce::Colour wood = material::wood.withRotatedHue(hueShift);
const juce::Colour metal = material::metal.withRotatedHue(hueShift);
const juce::Colour clock = material::clock.withRotatedHue(hueShift);
const juce::Colour memory = material::memory.withRotatedHue(hueShift);
const juce::Colour returnPath = material::returnPath.withRotatedHue(hueShift);
const juce::Colour voice = material::voice.withRotatedHue(hueShift);
// Extra .brighter() on top of the same rotation every other colour
// here gets - blue/violet reads noticeably darker than the other
// shifted hues at the same lightness value, and the plain rotation
// read as low-contrast "azul escuro, meio violeta" on real buttons
// (author, live).
const juce::Colour controlBlue = material::controlBlue.withRotatedHue(hueShift).brighter(0.22f);
// CLONE's own body-background tint colour - a specific carmine, not a
// rotation of any existing accent (author, live: "vamos fazer um teste
// com uma cor de fundo do clone mais avermelhada" then "tente um
// vermelho carmim" - tried cloneMaterial::board then ::returnPath
// first, both superseded by this literal).
const juce::Colour cloneBodyTint { 0xff960018 };
const juce::Colour phaser = material::phaser.withRotatedHue(hueShift);
const juce::Colour noiseSend = material::noiseSend.withRotatedHue(hueShift);
const juce::Colour vcf = material::vcf.withRotatedHue(hueShift);
const juce::Colour adsr = material::adsr.withRotatedHue(hueShift);
}

// JUCE treats a bare const char* as ASCII. Every user-facing UTF-8 literal
// must cross this explicit boundary; otherwise accents trigger juce_String's
// ASCII assertion and may be rendered through a fallback incorrectly.
inline juce::String utf8(const char* text) { return juce::String::fromUTF8(text); }

// The font archive remains with the instrument, while the runtime interface
// follows the same explicit Monospace policy validated in Rasgo Synth. UTF-8
// conversion still happens at the boundary above before text reaches JUCE.
inline juce::Typeface::Ptr uiTypeface(bool bold)
{
    static auto regular = juce::Typeface::createSystemTypefaceFor(
        BinaryData::IBMPlexMonoRegular_ttf, BinaryData::IBMPlexMonoRegular_ttfSize);
    static auto heavy = juce::Typeface::createSystemTypefaceFor(
        BinaryData::IBMPlexMonoBold_ttf, BinaryData::IBMPlexMonoBold_ttfSize);
    return bold ? heavy : regular;
}

inline juce::Font uiFont(float size, bool bold = false)
{
    // Same policy already proved in Rasgo Synth: let fontconfig resolve the
    // installed Monospace family (DejaVu Sans Mono on this system). It has a
    // stable, readable x-height at the original UI sizes and complete Latin
    // coverage. UTF-8 conversion is handled separately by utf8(), so glyphs
    // and text encoding cannot contaminate one another.
    // Antitotem retains a readable physical-panel scale even when a window
    // manager presents it below the 1920x1080 reference canvas.
    constexpr float interfaceScale = 1.1f;
    return juce::Font(juce::FontOptions("Monospace", size * interfaceScale,
                                        bold ? juce::Font::bold : juce::Font::plain));
}

// Free-function twin of MainComponent's own private static configureLabel(),
// for components declared outside MainComponent (ObjectFiveComponent) that
// still want the same left-aligned Monospace label styling.
inline void configureLabel(juce::Label& label, const juce::String& text, float size, juce::Colour colour)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(uiFont(size));
    label.setColour(juce::Label::textColourId, colour);
    label.setJustificationType(juce::Justification::centredLeft);
}

// One application-wide policy closes the remaining gap left by per-component
// fonts: JUCE-owned dialogs, menus and future controls use the same readable
// Monospace family as the panel.
class AntitotemLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton&, int height) override
    {
        return uiFont(std::clamp(static_cast<float>(height) * 0.40f, 10.0f, 16.0f), true);
    }
    juce::Font getLabelFont(juce::Label& label) override
    {
        return uiFont(std::max(8.0f, label.getFont().getHeight()), label.getFont().isBold());
    }
};

AntitotemLookAndFeel& antitotemLookAndFeel()
{
    static AntitotemLookAndFeel instance;
    return instance;
}

class PatchToggleLook final : public juce::LookAndFeel_V4
{
public:
    // True only for the CLONE-only singleton instance (see
    // patchToggleLookClone() below) - every accent colour this look
    // draws shifts to cloneMaterial's hue-rotated equivalent instead of
    // material's own, so CLONE's toggle buttons read at a different
    // colour temperature at a glance (author, live: "layout aprovado,
    // agora passar para etapa de mudar as cores entre o principal e
    // clone").
    bool cloneAccent = false;
    // FREEZE (17 ago. 2026, author's own idea, live: "destacar de alguma
    // maneira que o freeze... interfere" in CAOS/VAGA) - pushed in from
    // outside on every lfoFreeze click, same mechanism already used for
    // REC/PLAY's own live state above. Highlights CAOS/VAGA whenever
    // FREEZE is engaged, regardless of which shape is currently selected
    // - a visible, at-a-glance answer to "which button does this other
    // button affect", not just a tooltip explaining it in words.
    bool lfoFrozen = false;
    // RESEED is momentary, not a persistent state like FREEZE - flipped
    // true when DERIVA fires (see deriveFromMemory(), which reseeds
    // CAOS/VAGA automatically as part of its own drift instead of a
    // dedicated button - author, live: "não achei tão interessante esse
    // botão do reseed... talvez ele funcione melhor como elemento que
    // atue automaticamente quando acionada algum deriva") and back false
    // after a short delay, giving the same ring a brief flash instead of
    // a held highlight.
    bool reseedFlash = false;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool highlighted, bool pressed) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const bool active = button.getToggleState();
        const auto id = button.getComponentID();
        const auto memoryColour = cloneAccent ? cloneMaterial::memory : material::memory;
        const auto returnPathColour = cloneAccent ? cloneMaterial::returnPath : material::returnPath;
        const auto controlBlueColour = cloneAccent ? cloneMaterial::controlBlue : material::controlBlue;
        const auto boardColour = cloneAccent ? cloneMaterial::board : material::board;
        const auto clockColour = cloneAccent ? cloneMaterial::clock : material::clock;
        const auto signalColour = id == "loop" ? memoryColour
                                : (id == "feedback" || id == "mute" || id == "derive") ? returnPathColour
                                : id == "core" ? controlBlueColour
                                : boardColour;
        // DERIVA sits in the mixer column's variation-button row next to
        // CLONE, PULSO, POROSA etc - CLONE always shows its own accent
        // (PanelButtonLook's isDecorativeTransport) so it reads as a
        // different kind of button at a glance; DERIVA had no equivalent,
        // so at rest it looked identical to every other idle toggle in the
        // panel (PULSO's 3:2, MÉTRICA's 3/4...) instead of standing out.
        // Green at rest, red once engaged (ready -> recording-memory, the
        // same language as a record light) rather than red the whole time,
        // which read as an alarm/error state even when idle. Matched by
        // componentID, not button text: a literal "DERIVA" text match
        // broke the moment the language switch translated this button's
        // caption to "DRIFT"/"DÉRIVE" (author, live, 15 ago. 2026: "botão
        // deriva com problema em ingles e frances" - PT/ES both still say
        // "DERIVA", which is exactly why only EN/FR looked broken).
        const bool isDeriva = id == "derive";
        // CAOS/VAGA text is never translated (see the constexpr shape-name
        // arrays), so a literal match is safe across all 4 languages,
        // unlike DERIVA's own text-match pitfall noted above.
        const auto text = button.getButtonText();
        const bool isFrozenTarget = (lfoFrozen || reseedFlash) && (text == "CAOS" || text == "VAGA");
        // Disabled toggles (currently only FRZ, when CAOS/VAGA isn't the
        // selected shape - 17 ago. 2026) previously looked fully normal
        // despite not responding to clicks, since this custom paint never
        // checked isEnabled() - JUCE's own default LookAndFeel dims
        // disabled buttons automatically, but a from-scratch override
        // like this one has to do it explicitly. Dimming alpha alone
        // wouldn't read against the panel's own near-black background -
        // darkening the border and the text (the two most visible
        // elements of the idle state) instead.
        const bool enabled = button.isEnabled();
        const auto base = active ? signalColour
                         : isDeriva ? clockColour.darker(0.35f)
                         : juce::Colour(0xff201d18);
        const auto edge = (isFrozenTarget ? boardColour.brighter(0.3f)
                         : active ? signalColour.brighter(0.24f)
                         : isDeriva ? clockColour.brighter(0.1f)
                         : juce::Colour(0xff746853)).withMultipliedBrightness(enabled ? 1.0f : 0.4f);
        g.setColour(highlighted ? base.brighter(0.16f) : base);
        g.fillRoundedRectangle(bounds, 3.5f);
        g.setColour(pressed ? edge.brighter(0.25f) : edge);
        g.drawRoundedRectangle(bounds, 3.5f, isFrozenTarget ? 2.2f : (active ? 1.6f : 1.0f));
        g.setColour((active ? material::shadow : material::metal).withMultipliedBrightness(enabled ? 1.0f : 0.4f));
        // DERIVA now spans the full transport column width (see
        // layoutTransportColumn()) - its own text can afford to grow with
        // it (author, live: "o texto dentro do botão deriva pode ficar um
        // pouco maior").
        g.setFont(uiFont(isDeriva ? 13.0f : 10.0f, true));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(3, 1), juce::Justification::centred, 1);
    }
};

class PanelButtonLook final : public juce::LookAndFeel_V4
{
public:
    // REC's own visual phase, driven by MainComponent's timerCallback() -
    // three states the performer needs to tell apart at a glance, not just
    // toggled-on/off like every other button here: 0 = idle (normal
    // toggle-state colouring), 1 = armed (waiting for the PRINCIPAL
    // sequencer's step 1, steady amber - not recording yet, so not red),
    // 2 = actually recording (blinks red/dark via recordBlinkOn, toggled
    // roughly twice a second).
    int recordPhase = 0;
    bool recordBlinkOn = true;
    // True only for the CLONE-only singleton instance (see
    // panelButtonLookClone() below) - shifts every accent to
    // cloneMaterial's hue-rotated equivalent (author, live: "layout
    // aprovado, agora passar para etapa de mudar as cores entre o
    // principal e clone"). CLONE never shows REC/duration buttons (no
    // recording controls of its own), so recordPhase/recordBlinkOn above
    // stay irrelevant to this instance - only PRINCIPAL's REC drives
    // those.
    bool cloneAccent = false;
    // Leve oscilação de brilho no PLAY enquanto o sequenciador toca (autor:
    // "o play também pode ter uma ligeira variação (oscilação) enquanto o
    // instrumento está tocando") - fase contínua avançada a cada frame por
    // MainComponent::timerCallback(), deliberadamente lenta e contínua
    // (uma "respiração", não um blink liga/desliga como o REC acima).
    bool playRunning = false;
    float playPulsePhase = 0.0f;
    juce::Font getTextButtonFont(juce::TextButton&, int height) override
    {
        // 13, not 14: "RESET" was the one four-across transport label just
        // wide enough to clip into "RES..." at the old max.
        return uiFont(std::clamp(static_cast<float>(height) * 0.34f, 9.0f, 13.0f), true);
    }
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool hover, bool down) override
    {
        // Same language as the transport buttons in Navalha 2 JUCE: a
        // vertical gradient rather than a flat fill, brightening upward when
        // active/pressed, with a role-coloured accent border (REC red,
        // STOP neutral wood, everything else the panel's amber).
        const auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const auto text = button.getButtonText();
        // The VCF mode buttons (LPF/BPF/HPF/NCH) keep a distinct accent
        // colour each so they read at a glance even when off, but (17
        // ago. 2026, multi-select mode mask) they are real independent
        // toggles now, not one persistent always-on label - only the ones
        // actually selected should look lit, same as any other toggle.
        // PLAY/STOP/RESET are different: still no real "on" state of
        // their own, always show their colour. REC genuinely toggles
        // (recording or not), so it uses its own toggle state like any
        // normal button - red only while actually recording, neutral
        // otherwise.
        // CLONE sits in the same row as PULSO/POROSA/HETERÓDINA/RND 16 but
        // is not a variation preset - it always shows its own accent so it
        // reads as a different kind of button at a glance, not just another
        // item in that row.
        const bool isDecorativeTransport = text == "PLAY" || text == "STOP" || text == "RESET" || text == "CLONE";
        const bool isRecordButton = text.containsIgnoreCase("REC");
        // The 1/2/3/5 MIN duration buttons arm/record exactly like REC
        // itself (record.onClick and recordDurations[i].onClick share the
        // same recordingArmed/recordingActive state) - but only the one
        // the user actually pressed should light up, so this also checks
        // getToggleState() rather than applying to all four at once.
        const bool isArmedDurationButton = text.containsIgnoreCase("MIN") && button.getToggleState();
        const bool followsRecordPhase = isRecordButton || isArmedDurationButton;
        bool active = button.getToggleState() || down || isDecorativeTransport;
        auto accent = text == "PLAY" ? material::clock
                     : isRecordButton ? material::returnPath
                     // Same green as PLAY (author, live: "mudar as cores
                     // dos botoes stop e reset, utilisar o mesmo verde do
                     // play") - the whole transport row reads as one
                     // family of controls now instead of three unrelated
                     // colours.
                     : text == "STOP" ? material::clock
                     : text == "RESET" ? material::clock
                     // Same amber/gold as the NOISE hub (author, live: "o
                     // botão clone deixar em amarelo dourado, mesma cor
                     // dos botoes do noise").
                     : text == "CLONE" ? material::board
                     : text == "LPF" ? material::clock
                     : text == "BPF" ? material::controlBlue
                     : text == "HPF" ? material::memory
                     : text == "NCH" ? material::board // same wood/amber as the noise selector hub
                     : material::board;
        // Single rotation instead of rewriting every branch above with a
        // cloneAccent conditional of its own - CLONE never shows REC/
        // duration buttons, so this only ever touches filterMode/CLONE
        // itself/the decorative transport colours, none of which apply
        // to the real PRINCIPAL-only singleton (cloneAccent stays false
        // there).
        if (cloneAccent) accent = accent.withRotatedHue(cloneMaterial::hueShift);
        // Armed: steady amber, not recording yet, so deliberately not the
        // same red the actually-recording phase uses - a different colour
        // reads faster than a same-colour blink/no-blink distinction would.
        if (followsRecordPhase && recordPhase == 1) { accent = material::board; active = true; }
        // Recording: blinks by riding the same active/inactive gradient
        // this LookAndFeel already draws for any toggled-off button,
        // rather than a second colour - "on" is bright red, "off" is the
        // ordinary dark idle look, alternating.
        else if (followsRecordPhase && recordPhase == 2) active = recordBlinkOn;
        // PLAY/STOP/RESET/CLONE stay `active` at
        // all times by design (see isDecorativeTransport
        // above) - but that also meant hover/press never changed how
        // they looked at all, unlike every PatchToggleLook button
        // (PULSO, DERIVA, PORTAS DE FEEDBACK...), which brightens on
        // hover and again on press. Layering the same brightening on top
        // of the resting accent, instead of only varying colour while
        // NOT active, gives these persistent buttons the same felt
        // interactivity (author, live: "faça uma auditoria dos padrões
        // dos botões e encontre soluções para melhorar as animações" -
        // PLAY/STOP/etc looked identical whether idle, hovered or
        // pressed).
        // PLAY's own idle state (not hovered/pressed) oscillates between
        // resting and the same brighter(0.16f) hover already uses, instead
        // of a separate/subtler brightness scale - the author confirmed
        // hover and press are clearly visible but an earlier
        // withMultipliedBrightness(0.9-1.0) pulse was not (live, 15 ago.
        // 2026: "com o play ou parado é a mesma cor... só quando passa o
        // mouse ou clica"; then asked directly for "a oscilação entre as
        // duas variações do mouse sobre e do clique"). Hover/press still
        // take priority when they actually happen, same as any other
        // button here.
        const auto playPulse = (text == "PLAY" && playRunning)
            ? accent.interpolatedWith(accent.brighter(0.16f), 0.5f + 0.5f * std::sin(playPulsePhase))
            : accent;
        const auto interactiveAccent = down ? accent.brighter(0.34f) : hover ? accent.brighter(0.16f) : playPulse;
        const auto top = active ? interactiveAccent.brighter(0.30f)
                                 : juce::Colour(hover ? 0xff241f18 : 0xff1a1712);
        const auto bottom = active ? interactiveAccent
                                    : juce::Colour(hover ? 0xff17140f : 0xff100e0a);
        juce::ColourGradient gradient(top, bounds.getTopLeft(), bottom, bounds.getBottomLeft(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(active ? interactiveAccent.brighter(0.24f)
                           : (cloneAccent ? cloneMaterial::wood : material::wood).brighter(0.30f));
        g.drawRoundedRectangle(bounds, 4.0f, active ? (down ? 2.0f : 1.5f) : 1.0f);
    }
};

class LogPanelLook final : public juce::LookAndFeel_V4
{
public:
    // Matches PanelButtonLook's own radius/fill/border language so the log
    // box reads as one family with the buttons around it, not a plain
    // rectangular text field dropped into a rounded interface.
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
    {
        g.setColour(editor.findColour(juce::TextEditor::backgroundColourId));
        g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 4.0f);
    }
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
    {
        g.setColour(editor.findColour(juce::TextEditor::outlineColourId));
        g.drawRoundedRectangle(juce::Rectangle<float>(0.0f, 0.0f,
            static_cast<float>(width), static_cast<float>(height)).reduced(0.5f), 4.0f, 1.0f);
    }
};

LogPanelLook& logPanelLook()
{
    static LogPanelLook instance;
    return instance;
}

PatchToggleLook& patchToggleLook()
{
    // This must be created after JUCE has initialised its colour statics. A
    // function-local instance keeps the visual language alive for every panel
    // without relying on unsafe process-start construction order.
    static PatchToggleLook instance;
    return instance;
}

// CLONE's own instance of the same look, hue-shifted (cloneAccent=true) -
// a second singleton rather than a runtime-toggled flag on the shared one,
// since PRINCIPAL and CLONE widgets can repaint in the same frame and a
// shared instance would race between the two accent sets.
PatchToggleLook& patchToggleLookClone()
{
    static PatchToggleLook instance;
    instance.cloneAccent = true;
    return instance;
}

PanelButtonLook& panelButtonLook()
{
    static PanelButtonLook instance;
    return instance;
}

// Same reasoning as patchToggleLookClone() above.
PanelButtonLook& panelButtonLookClone()
{
    static PanelButtonLook instance;
    instance.cloneAccent = true;
    return instance;
}

class NoiseSelector final : public juce::Component
{
public:
    // cloneAccent: CLONE's own instance passes true, so its hexagon needle
    // and the 6 colour buttons follow cloneMaterial's hue shift too - same
    // gap StepControl closed earlier (this class was left out of that pass,
    // still hardcoded to the PRINCIPAL-only patchToggleLook()/material::
    // singletons regardless of which tab actually owned the instance).
    explicit NoiseSelector(bool cloneAccentValue = false) : cloneAccent(cloneAccentValue)
    {
        constexpr std::array<const char*, 6> shortNames { "BRC", "ROS", "MAR", "AZL", "VIO", "BIT" };
        for (std::size_t i = 0; i < choices.size(); ++i)
        {
            auto& choice = choices[i];
            choice.setButtonText(shortNames[i]);
            choice.setComponentID("noise");
            choice.setRadioGroupId(281);
            choice.setLookAndFeel(cloneAccent ? &patchToggleLookClone() : &patchToggleLook());
            choice.onClick = [this, i] { select(static_cast<int>(i), true); };
            addAndMakeVisible(choice);
        }

        sampleHold.setButtonText("S&H");
        sampleHold.setComponentID("core");
        sampleHold.setClickingTogglesState(true);
        sampleHold.setLookAndFeel(cloneAccent ? &patchToggleLookClone() : &patchToggleLook());
        sampleHold.onClick = [this] { if (onSampleHoldChange) onSampleHoldChange(sampleHold.getToggleState()); };
        addAndMakeVisible(sampleHold);
        // BREATH (docs/PESQUISA_RUIDO_GENERATIVO.md item #2, author, 19
        // ago. 2026, asked directly where to place it: "botão liga/
        // desliga (como o SWG)... perto do seletor de NOISE COR" - author
        // confirmed with "ok"). A fixed depth when on (see the tab-level
        // onBreathChange wiring), not a continuous slider - same
        // one-button-not-a-dial precedent as SWG itself. Shares this
        // widget's own centre with S&H (side by side, not stacked) rather
        // than a whole new standalone component - the six colour petals
        // only occupy y = ±0.40*radiusY and ±0.80*radiusY bands (see
        // resized() below), so a wider row sitting at y=0 has real
        // clearance regardless of how wide it is, unlike the petals
        // themselves.
        breath.setButtonText("BR");
        breath.setComponentID("core");
        breath.setClickingTogglesState(true);
        breath.setLookAndFeel(cloneAccent ? &patchToggleLookClone() : &patchToggleLook());
        breath.onClick = [this] { if (onBreathChange) onBreathChange(breath.getToggleState()); };
        addAndMakeVisible(breath);
        select(0, false);
        setSampleHold(true, false);
        // Off by default (docs/PESQUISA_RUIDO_GENERATIVO.md's own 0=off
        // convention) - unlike S&H just above, which already defaulted on
        // before this control existed and stays that way.
        setBreath(false, false);
        refreshTooltips();
    }

    ~NoiseSelector() override
    {
        for (auto& choice : choices) choice.setLookAndFeel(nullptr);
        sampleHold.setLookAndFeel(nullptr);
        breath.setLookAndFeel(nullptr);
    }

    // Called by whichever tab owns this instance when the shared language
    // switch (in MainComponent's header) changes - see docs/TAREFAS.md,
    // this class was hardcoded to Portuguese-only tooltips until 15 ago.
    // 2026.
    void setLanguage(antitotem::ui::Language newLanguage)
    {
        language = newLanguage;
        refreshTooltips();
    }

    void select(int requested, bool notify)
    {
        selection = std::clamp(requested, 0, static_cast<int>(choices.size()) - 1);
        for (std::size_t i = 0; i < choices.size(); ++i)
            choices[i].setToggleState(static_cast<int>(i) == selection, juce::dontSendNotification);
        repaint();
        if (notify && onSelection) onSelection(selection);
    }

    void setSampleHold(bool enabled, bool notify)
    {
        sampleHold.setToggleState(enabled, juce::dontSendNotification);
        if (notify && onSampleHoldChange) onSampleHoldChange(enabled);
    }

    void setBreath(bool enabled, bool notify)
    {
        breath.setToggleState(enabled, juce::dontSendNotification);
        if (notify && onBreathChange) onBreathChange(enabled);
    }

    // Genuinely elliptical now (independent X/Y radii) instead of a circle
    // forced into min(width,height) - the space this reuses (below ADSR,
    // level with VCF's 3rd knob) is wider than it is tall, and a real oval
    // uses that shape instead of centring a small circle inside it.
    void resized() override
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto centreX = bounds.getWidth() * 0.5f;
        const auto centreY = bounds.getHeight() * 0.5f;
        const auto radiusX = bounds.getWidth() * 0.5f - 6.0f;
        const auto radiusY = bounds.getHeight() * 0.5f - 6.0f;
        // Same hexagonal arrangement as before, expressed as -1..1 offsets
        // from centre so it can scale independently in X and Y.
        constexpr std::array<juce::Point<float>, 6> positions {{ { 0.0f, -0.80f }, { 0.66f, -0.40f }, { 0.66f, 0.40f },
                                                                   { 0.0f, 0.80f }, { -0.66f, 0.40f }, { -0.66f, -0.40f } }};
        for (std::size_t i = 0; i < choices.size(); ++i)
        {
            constexpr auto buttonWidth = 38;
            constexpr auto buttonHeight = 17;
            const auto x = centreX + positions[i].x * radiusX - buttonWidth / 2;
            const auto y = centreY + positions[i].y * radiusY - buttonHeight / 2;
            choices[i].setBounds(static_cast<int>(x), static_cast<int>(y), buttonWidth, buttonHeight);
        }
        // "NOISE" is now a real title above this whole widget (like MASTER
        // above the master knob), not text painted inside it, so the button
        // goes back to dead-centre. Split into two now (S&H left, BREATH
        // right, 19 ago. 2026) instead of one full-width button. First cut
        // shrank both to 34px to be safe - too conservative (author, live:
        // "agora ele está fraquinho" about S&H's new smaller size) - the
        // widget is actually 190px wide (~182px usable), radiusX alone is
        // ~85px, and the six colour petals sit at y=±0.40*radiusY/
        // ±0.80*radiusY, never y=0, so the full original 40px S&H width
        // was never really at risk of colliding with anything. Both keep
        // their original 40px now, S&H's own size completely unchanged
        // from before BREATH existed.
        constexpr auto pairGap = 6;
        constexpr auto pairButtonWidth = 40;
        sampleHold.setBounds(static_cast<int>(centreX) - pairButtonWidth - pairGap / 2, static_cast<int>(centreY) - 9, pairButtonWidth, 18);
        breath.setBounds(static_cast<int>(centreX) + pairGap / 2, static_cast<int>(centreY) - 9, pairButtonWidth, 18);
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(8.0f, 4.0f);
        const auto inner = bounds.reduced(bounds.getWidth() * 0.24f, bounds.getHeight() * 0.20f);
        // No fill here - the outer brown ellipse was already removed
        // earlier; this inner one read as a leftover black disc behind the
        // needle instead of matching the panel background around it.
        g.setColour((cloneAccent ? cloneMaterial::memory : material::memory).withAlpha(0.72f));
        // 6.0f, not 5.0f: six hex positions spaced 60 degrees apart around
        // the full circle, not five. The old divisor made selection 5
        // (BIT) land on the exact same angle as selection 0 (BRC), and
        // every selection in between pointed increasingly off from its
        // actual button.
        const auto angle = juce::jmap(static_cast<float>(selection), 0.0f, 6.0f, -juce::MathConstants<float>::halfPi,
                                      juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi);
        const auto centre = inner.getCentre();
        const auto radiusX = inner.getWidth() * 0.42f;
        const auto radiusY = inner.getHeight() * 0.42f;
        g.drawLine(centre.x, centre.y, centre.x + std::cos(angle) * radiusX, centre.y + std::sin(angle) * radiusY, 2.0f);
    }

    std::function<void(int)> onSelection;
    std::function<void(bool)> onSampleHoldChange;
    std::function<void(bool)> onBreathChange;

private:
    void refreshTooltips()
    {
        for (std::size_t i = 0; i < choices.size(); ++i)
            choices[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseColourNames[i], language));
        sampleHold.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::sampleHold, language));
        breath.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseBreath, language));
    }
    std::array<juce::ToggleButton, 6> choices;
    juce::ToggleButton sampleHold;
    juce::ToggleButton breath;
    int selection = 0;
    antitotem::ui::Language language = antitotem::ui::Language::english;
    const bool cloneAccent;
};

class StereoScope final : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr std::size_t sampleCount = 256;

    // Ceiling lowered from 3.0 to 0.56 on 15 ago. 2026: that range was
    // calibrated when real output rarely approached the 0.851 safety
    // ceiling (needed a much higher gain to fill the box at all on
    // typical, then-quiet material). After tonight's gain-staging work
    // the signal itself got much louder (SignalLeveler target_=0.52,
    // hot moments near 0.85) - the old 3.0 ceiling meant most of the
    // slider's own travel immediately clipped the trace against
    // waveformBounds (author, live: "quando se sobe no slider, as ondas
    // ficam cortadas... teste com o slider 100%"). 0.56 keeps even the
    // most extreme observed peak (~0.85, from deliberate worst-case
    // takes the same night) inside the box at full gain, with a small
    // safety margin: 0.85*0.56 ≈ 0.476, under the ~0.5 (half of
    // waveformBounds' height) a sample can reach before paint()'s clip
    // region crops it. (First attempt used 0.55/default 0.4 - author
    // then said the wave read as "muito discreto" at that default, so
    // both ends moved up slightly together.)
    void setGain(float value) noexcept { gain = std::clamp(value, 0.15f, 0.56f); }

    void push(float left, float right) noexcept
    {
        // Fast buffer: waveform. Slow buffer: material movement across about
        // 5 seconds at 48 kHz, so LFO, step dynamics and feedback become visible.
        if (++fastDecimation >= 24)
        {
            fastDecimation = 0;
            const auto next = (writeIndex.load(std::memory_order_relaxed) + 1U) % sampleCount;
            leftSamples[next].store(left, std::memory_order_relaxed);
            rightSamples[next].store(right, std::memory_order_relaxed);
            writeIndex.store(next, std::memory_order_release);
        }
        movementLeft += std::abs(left); movementRight += std::abs(right);
        if (++movementDecimation >= 1024)
        {
            const auto next = (movementWriteIndex.load(std::memory_order_relaxed) + 1U) % sampleCount;
            const auto scale = 1.75f / static_cast<float>(movementDecimation);
            movementSamplesLeft[next].store(std::clamp(std::sqrt(movementLeft * scale), 0.0f, 1.0f), std::memory_order_relaxed);
            movementSamplesRight[next].store(std::clamp(std::sqrt(movementRight * scale), 0.0f, 1.0f), std::memory_order_relaxed);
            movementWriteIndex.store(next, std::memory_order_release);
            movementLeft = movementRight = 0.0f; movementDecimation = 0;
        }
    }
    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(material::shadow.brighter(0.13f)); g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(material::wood.brighter(0.14f)); g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        const auto waveformBounds = bounds.withTrimmedTop(19.0f).withTrimmedBottom(bounds.getHeight() * 0.31f);
        const auto movementBounds = bounds.withTrimmedTop(bounds.getHeight() * 0.72f).reduced(6.0f, 0.0f);
        const auto mid = waveformBounds.getCentreY();
        g.setColour(material::metal.withAlpha(0.22f)); g.drawHorizontalLine(static_cast<int>(mid), waveformBounds.getX() + 3.0f, waveformBounds.getRight() - 3.0f);
        const auto latest = writeIndex.load(std::memory_order_acquire);
        const auto drawTrace = [&] (const auto& source, juce::Colour colour, float offset)
        {
            juce::Path trace;
            for (std::size_t i = 0; i < sampleCount; ++i)
            {
                const auto index = (latest + 1U + i) % sampleCount;
                const auto x = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(sampleCount - 1U), waveformBounds.getX() + 3.0f, waveformBounds.getRight() - 3.0f);
                const auto y = mid + offset - source[index].load(std::memory_order_relaxed) * waveformBounds.getHeight() * gain;
                i == 0 ? trace.startNewSubPath(x, y) : trace.lineTo(x, y);
            }
            g.setColour(colour); g.strokePath(trace, juce::PathStrokeType(1.35f));
        };
        {
            // High gain settings can send a loud transient past the box's
            // edge - clip to the waveform area instead of letting it draw
            // over the header text or outside the panel.
            juce::Graphics::ScopedSaveState clipGuard(g);
            g.reduceClipRegion(waveformBounds.getSmallestIntegerContainer());
            drawTrace(leftSamples, material::clock, -waveformBounds.getHeight() * 0.11f);
            drawTrace(rightSamples, material::voice, waveformBounds.getHeight() * 0.11f);
        }
        const auto latestMovement = movementWriteIndex.load(std::memory_order_acquire);
        // Same GANHO Y control the fast waveform trace already uses (gain),
        // not a fixed 0.86 - quiet/typical material rarely pushes the raw
        // sqrt-averaged movement value anywhere close to 1.0, so it used to
        // read as a nearly flat line regardless of how loud the material
        // actually was. Clipped to movementBounds the same way the fast
        // trace is clipped to waveformBounds, so a high gain setting still
        // can't draw over ESPAÇO/FASE below it.
        juce::Graphics::ScopedSaveState movementClipGuard(g);
        g.reduceClipRegion(movementBounds.getSmallestIntegerContainer());
        const auto movementGain = std::min(gain, 2.2f);
        const auto drawMovement = [&] (const auto& source, juce::Colour colour)
        {
            juce::Path trace;
            for (std::size_t i = 0; i < sampleCount; ++i)
            {
                const auto index = (latestMovement + 1U + i) % sampleCount;
                const auto x = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(sampleCount - 1U), movementBounds.getX(), movementBounds.getRight());
                const auto y = movementBounds.getBottom() - source[index].load(std::memory_order_relaxed) * movementBounds.getHeight() * 0.86f * movementGain;
                i == 0 ? trace.startNewSubPath(x, y) : trace.lineTo(x, y);
            }
            g.setColour(colour.withAlpha(0.82f)); g.strokePath(trace, juce::PathStrokeType(1.15f));
        };
        drawMovement(movementSamplesLeft, material::clock);
        drawMovement(movementSamplesRight, material::voice);
        g.setFont(uiFont(10.0f, true));
        g.setColour(material::clock); g.drawText("L", getLocalBounds().reduced(8, 5), juce::Justification::topLeft);
        g.setColour(material::voice); g.drawText("R", getLocalBounds().reduced(8, 5), juce::Justification::topRight);
        g.setColour(material::metal); g.drawText(utf8("ONDA L/R · MOVIMENTO / MODULAÇÃO"), getLocalBounds().reduced(28, 5), juce::Justification::centredTop);
    }
private:
    std::array<std::atomic<float>, sampleCount> leftSamples {}, rightSamples {}, movementSamplesLeft {}, movementSamplesRight {};
    std::atomic<std::size_t> writeIndex { 0 }, movementWriteIndex { 0 };
    unsigned int fastDecimation = 0, movementDecimation = 0;
    float movementLeft = 0.0f, movementRight = 0.0f;
    // Overridden immediately by MainComponent's scopeGain slider default
    // (0.48, see its own setValue() comment) - kept in step here only so
    // this class's own default isn't stale/misleading on its own.
    float gain = 0.48f;
};

class WavRecorder final
{
public:
    WavRecorder() : writerThread("Antitotem WAV writer") { writerThread.startThread(); }
    ~WavRecorder() { stop(); writerThread.stopThread(3000); }
    bool start(double sampleRate, double maximumSeconds = 300.0)
    {
        const juce::ScopedLock guard(lock);
        active = nullptr; writer.reset();
        const auto configuredDirectory = juce::SystemStats::getEnvironmentVariable("ANTITOTEM_RECORDINGS_DIR", {});
        auto directory = configuredDirectory.isNotEmpty() ? juce::File(configuredDirectory)
                                                          : juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("Antitotem Objeto Sonoro");
        directory.createDirectory();
        const auto stamp = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
        file = directory.getChildFile("ANTITOTEM_" + stamp + ".wav");
        auto stream = file.createOutputStream();
        if (stream == nullptr) return false;
        juce::WavAudioFormat wav;
        juce::StringPairArray metadata;
        metadata.set("INAM", "Antitotem - Objeto Sonoro");
        metadata.set("IART", utf8("Antitotem / Lúcio Araújo"));
        metadata.set("ICMT", utf8("Live take — 24-bit stereo WAV"));
        metadata.set("ICRD", juce::Time::getCurrentTime().formatted("%Y-%m-%d"));
        auto* formatWriter = wav.createWriterFor(stream.release(), sampleRate, 2, 24, metadata, 0);
        if (formatWriter == nullptr) return false;
        writer = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(formatWriter, writerThread, 32768);
        writer->setFlushInterval(static_cast<int>(sampleRate));
        active = writer.get();
        recordedSamples = 0;
        maxSamples = static_cast<juce::int64>(sampleRate * std::clamp(maximumSeconds, 1.0, 300.0));
        reachedLimit = false;
        return true;
    }
    void stop()
    {
        const juce::ScopedLock guard(lock);
        active = nullptr; writer.reset();
    }
    void write(const float* left, const float* right, int samples)
    {
        const juce::ScopedLock guard(lock);
        if (active == nullptr || left == nullptr || right == nullptr) return;
        // The duration buttons (1/2/3/5 min) are a target, not a hard
        // cutoff: REC is quantized to the PRINCIPAL sequencer's loop (see
        // MainComponent::timerCallback()'s recordingStopPending/atLoopStart),
        // which is meant to keep writing real audio past maxSamples until
        // the loop actually finishes. Truncating here instead would
        // silently discard that tail and always produce an exact
        // 60/120/180/300s file regardless of loop length - reported by the
        // author as "os áudios gravados sempre têm o tempo exato".
        const float* channels[] { left, right };
        active->write(channels, samples);
        recordedSamples += samples;
        if (recordedSamples >= maxSamples) reachedLimit = true;
    }
    [[nodiscard]] bool isRecording() const noexcept { return active != nullptr; }
    [[nodiscard]] bool hasReachedLimit() const noexcept { return reachedLimit; }
    [[nodiscard]] float progress() const noexcept
    {
        const juce::ScopedLock guard(lock);
        return maxSamples > 0 ? std::clamp(static_cast<float>(recordedSamples) / static_cast<float>(maxSamples), 0.0f, 1.0f) : 0.0f;
    }
private:
    juce::TimeSliceThread writerThread;
    mutable juce::CriticalSection lock;
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> writer;
    juce::AudioFormatWriter::ThreadedWriter* active = nullptr;
    juce::File file;
    juce::int64 recordedSamples = 0, maxSamples = 0;
    bool reachedLimit = false;
};

// MidiCapture (docs/PESQUISA_COMPASSO_E_METRICA_REAL.md, seção 5 -
// "serve de ponte pra MIDI/partitura"), 20 ago. 2026, autor: "é possível
// extrair a partitura da melodia? ou a partitura rítmica, ou completa" -
// mesma vida útil que WavRecorder (start/stop atrelado ao mesmo REC),
// gravando eventos de NOTA em vez de amostras de áudio. Três trilhas
// (PRINCIPAL/CLONE/EXCITAÇÃO) sempre juntas no mesmo arquivo MIDI - o
// usuário decide DEPOIS, na notação (MuseScore e afins conseguem abrir
// um MIDI multi-trilha e mostrar/esconder cada uma), se quer só a
// melodia, só o ritmo, ou a partitura completa; não precisamos decidir
// isso aqui, exportar tudo é estritamente mais útil que escolher por
// ele. Vozes MONOFÔNICAS por trilha (`noteOn` fecha qualquer nota
// pendente na mesma trilha antes de abrir a próxima) - PRINCIPAL/CLONE/
// EXCITAÇÃO são cada uma uma linha só, nunca duas notas simultâneas na
// mesma trilha, então isso é exatamente correto, não uma simplificação.
class MidiCapture final
{
public:
    struct NoteEvent { double startSeconds = 0.0; double endSeconds = 0.0; int midiNote = 60; int velocity = 100; int track = 0; };
    // Andamento variável (20 ago. 2026, autor: "se houver variação de
    // clock durante a gravação o registro midi vai entender como?" ->
    // "isso, fazer a mudança de andamento") - até aqui só um BPM único,
    // calculado no FIM da tomada, era aplicado a toda a gravação; se
    // CLOCK/ENERGIA/SUBDIVISÃO mudassem ao vivo durante o take, o
    // resultado media errado do início ao fim, não só no trecho que
    // mudou (os timestamps em segundos sempre foram certos - só a
    // conversão segundos->ticks que era uniforme). Cada `TempoEvent` é
    // um ponto onde o BPM efetivo mudou de verdade.
    struct TempoEvent { double atSeconds = 0.0; double bpm = 120.0; };
    static constexpr int trackCount = 3;

    void start()
    {
        const juce::ScopedLock guard(lock);
        events.clear();
        events.reserve(8192);
        // Real-time-safety fix (24 ago. 2026): unlike `events`, this never
        // got a `reserve()` - `recordTempo()` runs `push_back()` on the
        // audio thread every callback a tempo change crosses the 0.5 BPM
        // threshold, so an unreserved vector could reallocate on the heap
        // from inside the audio callback (the exact thing AGENTS.md's
        // real-time rule forbids). 1024 is generous headroom: tempo
        // changes are far rarer than notes (which reserve 8192) even in a
        // take with heavy live CLOCK/ENERGIA/SUBDIVISÃO modulation.
        tempoEvents.clear();
        tempoEvents.reserve(1024);
        for (auto& t : openStart) t = -1.0;
        elapsedSeconds = 0.0;
        active = true;
    }
    void stop() { const juce::ScopedLock guard(lock); active = false; }
    // Real-time-safety redesign (24 ago. 2026, checkpoint antes disso no
    // commit "checkpoint: antes do redesenho lock-free do MidiCapture" -
    // ver TAREFAS.md). Os três métodos abaixo rodam na thread de áudio, a
    // cada callback, enquanto gravando - antes seguravam `lock` de forma
    // bloqueante (`ScopedLock`), violando a regra do AGENTS.md de zero
    // lock no callback de áudio: se `start()`/`stop()`/`finish()` (thread
    // de UI) estivesse segurando o lock no instante errado, a thread de
    // áudio ficava parada esperando - exatamente o tipo de travamento
    // ocasional, dependente de timing, que a meta "nunca travar em
    // qualquer máquina" pede pra eliminar.
    //
    // Troca: `tryEnter()` (não-bloqueante) em vez de bloquear. A thread de
    // áudio NUNCA espera - se o lock estiver ocupado (só pode ser
    // start()/stop()/finish() da UI, cada um durando microssegundos, só
    // no início/fim de uma gravação), o método simplesmente não grava
    // aquele evento e retorna, em vez de arriscar um xrun de verdade.
    // Trade-off aceito conscientemente: numa janela de contenção
    // realmente rara (por baixo de 1ms, só em start/stop), um evento de
    // tempo/nota isolado pode ficar de fora do MIDI exportado - um dado
    // perdido na exportação, não um glitch audível, e um risco muito
    // menor que travar o áudio real. Preferido a uma fila lock-free
    // própria com atomics (mais código novo, mais superfície pra um bug
    // de concorrência sutil - risco maior do que o problema que resolve).
    //
    // Nota: como cada chamada tenta o lock independentemente, é possível
    // (só nessa mesma janela rara) `advance()` falhar mas `noteOn()` do
    // mesmo callback ter sucesso - o evento fica com o `elapsedSeconds`
    // do callback anterior, um desvio de no máximo um bloco de áudio
    // (~5-20ms). Aceitável: é um evento entre milhares, na exportação de
    // partitura, não no áudio ouvido.
    //
    // Granularidade de POLLING (uma vez por callback de áudio, ver
    // SimpleSequencer::didStepSoundSincePoll()) - blockSeconds é a
    // duração real do callback, usada só pra avançar o relógio interno
    // de eventos. Suficiente pra exportação de partitura, mais fino que
    // qualquer subdivisão real do sequenciador.
    void advance(double blockSeconds)
    {
        const juce::CriticalSection::ScopedTryLockType guard(lock);
        if (!guard.isLocked()) return;
        if (active) elapsedSeconds += blockSeconds;
    }
    // Chamado uma vez por callback (junto de `advance()`) com o BPM
    // instantâneo (ver `MainComponent::computeCurrentBpm()`). Só grava
    // um novo ponto quando o BPM realmente se moveu (limiar de 0.5 BPM)
    // - sem isso, flutuações de ponto-flutuante de callback em callback
    // encheriam a trilha de tempo com centenas de eventos redundantes.
    void recordTempo(double bpm)
    {
        const juce::CriticalSection::ScopedTryLockType guard(lock);
        if (!guard.isLocked()) return;
        if (!active || bpm <= 0.0) return;
        if (tempoEvents.empty() || std::abs(tempoEvents.back().bpm - bpm) >= 0.5)
            tempoEvents.push_back({ elapsedSeconds, bpm });
    }
    void noteOn(int track, int midiNote, int velocity)
    {
        const juce::CriticalSection::ScopedTryLockType guard(lock);
        if (!guard.isLocked()) return;
        if (!active || track < 0 || track >= trackCount) return;
        closeNote(track);
        openStart[static_cast<std::size_t>(track)] = elapsedSeconds;
        openPitch[static_cast<std::size_t>(track)] = midiNote;
        openVelocity[static_cast<std::size_t>(track)] = velocity;
    }
    struct Capture { std::vector<NoteEvent> notes; std::vector<TempoEvent> tempos; };
    // Fecha qualquer nota ainda soando (a última de cada trilha nunca
    // recebe um noteOn seguinte pra fechá-la) e devolve tudo -
    // deliberadamente esvazia `events`/`tempoEvents` (move), então só
    // pode ser chamado uma vez por gravação, no fim.
    [[nodiscard]] Capture finish()
    {
        const juce::ScopedLock guard(lock);
        for (int t = 0; t < trackCount; ++t) closeNote(t);
        active = false;
        return { std::move(events), std::move(tempoEvents) };
    }

private:
    void closeNote(int track) // chamador já segura `lock`
    {
        auto& start = openStart[static_cast<std::size_t>(track)];
        if (start < 0.0) return;
        events.push_back({ start, elapsedSeconds, openPitch[static_cast<std::size_t>(track)], openVelocity[static_cast<std::size_t>(track)], track });
        start = -1.0;
    }
    mutable juce::CriticalSection lock;
    bool active = false;
    double elapsedSeconds = 0.0;
    std::vector<NoteEvent> events;
    std::vector<TempoEvent> tempoEvents;
    std::array<double, trackCount> openStart { -1.0, -1.0, -1.0 };
    std::array<int, trackCount> openPitch {}, openVelocity {};
};

// pitch01 (0..1) -> nota MIDI. Mesmo mapeamento "1 semitom = 1/51.6" já
// usado internamente por CmosVoice::tickStereo (4.3 oitavas de alcance
// = 51.6 semitons) - ancorado em C2 (nota MIDI 36) por escolha
// arbitrária, já que uma voz de theremin livre não tem calibração fixa
// de concerto. Calibração pendente, mesmo espírito de toda constante
// desta sessão - nunca comparado por escuta contra uma afinação real.
[[nodiscard]] inline int pitch01ToMidiNote(float pitch01) noexcept
{
    return juce::jlimit(0, 127, static_cast<int>(std::lround(36.0f + pitch01 * 51.6f)));
}
[[nodiscard]] inline int velocityFromLevel(float level) noexcept
{
    return juce::jlimit(1, 127, static_cast<int>(std::lround(level * 127.0f)));
}

// Segmentos de tempo (andamento variável, 20 ago. 2026, ver o
// comentário de `MidiCapture::TempoEvent`) - compartilhados entre a
// exportação MIDI e a MusicXML (ver `writeMusicXmlCaptureToFile`
// abaixo, docs/PESQUISA_REPRESENTACAO_MUSICAL_RASGO.md), já que as
// duas convertem os MESMOS eventos capturados por `MidiCapture`, só
// pra unidades diferentes no fim (ticks pro MIDI, "steps" quantizados
// pra MusicXML). Prefixo cumulativo de ticks por segmento, cada um
// começando onde o anterior terminou - a mesma lógica de "quantos
// ticks se passam entre dois pontos, dado o BPM que valia NAQUELE
// trecho" que uma DAW aplica ao mover marcadores de tempo.
struct TempoSegment { double startSeconds; double startTicks; double bpm; };
[[nodiscard]] inline std::vector<TempoSegment> buildTempoSegments(const std::vector<MidiCapture::TempoEvent>& tempos, int ticksPerQuarterNote)
{
    std::vector<TempoSegment> segments;
    segments.reserve(tempos.size());
    double cumulativeTicks = 0.0;
    for (std::size_t i = 0; i < tempos.size(); ++i)
    {
        if (i > 0)
        {
            const auto& previous = tempos[i - 1];
            const auto deltaSeconds = tempos[i].atSeconds - previous.atSeconds;
            cumulativeTicks += deltaSeconds * (previous.bpm / 60.0) * static_cast<double>(ticksPerQuarterNote);
        }
        segments.push_back({ tempos[i].atSeconds, cumulativeTicks, tempos[i].bpm });
    }
    return segments;
}
[[nodiscard]] inline double secondsToTicksPiecewise(const std::vector<TempoSegment>& segments, double seconds, int ticksPerQuarterNote)
{
    // Acha o último segmento que começa em ou antes de `seconds`
    // (poucos segmentos esperados por tomada - busca linear é
    // suficiente, não vale a pena uma busca binária aqui).
    std::size_t index = 0;
    for (std::size_t i = 0; i < segments.size(); ++i)
        if (segments[i].startSeconds <= seconds) index = i; else break;
    const auto& segment = segments[index];
    return segment.startTicks + (seconds - segment.startSeconds) * (segment.bpm / 60.0) * static_cast<double>(ticksPerQuarterNote);
}

// Escreve um arquivo .mid real (juce::MidiFile, já parte de
// juce_audio_basics - sem dependência nova) a partir dos eventos que
// MidiCapture::finish() devolveu. `beatsPerMeasure`/`beatUnit` vêm de
// TimeSignature (docs/PESQUISA_COMPASSO_E_METRICA_REAL.md) - o
// primeiro consumidor real desses valores desde que a infraestrutura
// foi implementada (20 ago. 2026), sem precisar de UI nova pra isso.
// `tempos` é a lista de pontos onde o BPM mudou de verdade durante a
// tomada (20 ago. 2026, andamento variável - ver o comentário de
// `MidiCapture::TempoEvent`) - um segmento por ponto, cada um com seu
// próprio `set_tempo` meta-event na posição de tick correta, exatamente
// como DAWs/software de notação representam automação de tempo em
// arquivos SMF. `tempos` nunca vem vazia enquanto houve alguma nota
// (o primeiro callback gravado já grava um ponto em segundo 0).
inline bool writeMidiCaptureToFile(const std::vector<MidiCapture::NoteEvent>& events,
                                    const std::vector<MidiCapture::TempoEvent>& tempos, const juce::File& file,
                                    unsigned int beatsPerMeasure, unsigned int beatUnit)
{
    if (events.empty() || tempos.empty()) return false;
    constexpr int ticksPerQuarterNote = 960;
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(static_cast<short>(ticksPerQuarterNote));
    const auto segments = buildTempoSegments(tempos, ticksPerQuarterNote);
    const auto secondsToTicks = [&] (double seconds) { return secondsToTicksPiecewise(segments, seconds, ticksPerQuarterNote); };

    juce::MidiMessageSequence tempoTrack;
    for (const auto& segment : segments)
        tempoTrack.addEvent(juce::MidiMessage::tempoMetaEvent(static_cast<int>(60000000.0 / segment.bpm)), segment.startTicks);
    tempoTrack.addEvent(juce::MidiMessage::timeSignatureMetaEvent(static_cast<int>(beatsPerMeasure), static_cast<int>(beatUnit)), 0.0);
    midiFile.addTrack(tempoTrack);

    constexpr std::array<const char*, MidiCapture::trackCount> trackNames { "PRINCIPAL", "CLONE", "EXCITACAO" };
    for (int t = 0; t < MidiCapture::trackCount; ++t)
    {
        juce::MidiMessageSequence sequence;
        sequence.addEvent(juce::MidiMessage::textMetaEvent(3, trackNames[static_cast<std::size_t>(t)]), 0.0);
        for (const auto& event : events)
        {
            if (event.track != t) continue;
            sequence.addEvent(juce::MidiMessage::noteOn(t + 1, event.midiNote, static_cast<juce::uint8>(event.velocity)), secondsToTicks(event.startSeconds));
            sequence.addEvent(juce::MidiMessage::noteOff(t + 1, event.midiNote), secondsToTicks(event.endSeconds));
        }
        sequence.updateMatchedPairs();
        midiFile.addTrack(sequence);
    }

    auto stream = file.createOutputStream();
    if (stream == nullptr) return false;
    return midiFile.writeTo(*stream);
}

// MIDI nota (0-127) -> nome/oitava/alteração MusicXML (sempre sustenido,
// nunca bemol - escolha arbitrária, mesmo espírito de nunca calibrado
// por afinação real que já se aplica a `pitch01ToMidiNote`). Fórmula
// de oitava padrão MIDI (nota 60 = C4).
struct XmlPitch { juce::String step; int alter; int octave; };
[[nodiscard]] inline XmlPitch midiNoteToXmlPitch(int midiNote)
{
    static constexpr std::array<const char*, 12> steps { "C","C","D","D","E","F","F","G","G","A","A","B" };
    static constexpr std::array<int, 12> alters { 0,1,0,1,0,0,1,0,1,0,1,0 };
    const auto pitchClass = ((midiNote % 12) + 12) % 12;
    return { juce::String(steps[static_cast<std::size_t>(pitchClass)]), alters[static_cast<std::size_t>(pitchClass)], midiNote / 12 - 1 };
}
[[nodiscard]] inline const char* dynamicMarkFromVelocity(int velocity) noexcept
{
    if (velocity < 32) return "pp";
    if (velocity < 48) return "p";
    if (velocity < 64) return "mp";
    if (velocity < 80) return "mf";
    if (velocity < 96) return "f";
    if (velocity < 112) return "ff";
    return "fff";
}
// `baseSteps` já é uma potência de dois por construção (ver o loop de
// decomposição em `writeMusicXmlCaptureToFile`) - só resta nomear a
// figura de nota correspondente. Só correto quando TimeSignature.
// beatUnit==4 (tempo = semínima) - o único caso possível hoje, sem UI
// pra mudar (mesma limitação de CRI-CMP-001/CRI-MID-001).
[[nodiscard]] inline const char* noteTypeForBaseSteps(int baseSteps, int stepsPerBeat) noexcept
{
    if (baseSteps >= stepsPerBeat)
    {
        const auto quarters = baseSteps / juce::jmax(1, stepsPerBeat);
        if (quarters >= 4) return "whole";
        if (quarters >= 2) return "half";
        return "quarter";
    }
    const auto divisor = stepsPerBeat / juce::jmax(1, baseSteps);
    if (divisor >= 16) return "64th";
    if (divisor >= 8) return "32nd";
    if (divisor >= 4) return "16th";
    return "eighth";
}
[[nodiscard]] inline int secondsToStep(const std::vector<TempoSegment>& segments, double seconds, int ticksPerQuarterNote, int stepsPerBeat)
{
    const auto ticks = secondsToTicksPiecewise(segments, seconds, ticksPerQuarterNote);
    const auto stepTicks = static_cast<double>(ticksPerQuarterNote) / static_cast<double>(stepsPerBeat);
    return juce::jmax(0, static_cast<int>(std::lround(ticks / stepTicks)));
}

// Exporta o mesmo take capturado por MidiCapture como MusicXML real -
// partitura QUANTIZADA (figuras de nota reais, compassos, dinâmica),
// em vez de ticks brutos (docs/PESQUISA_REPRESENTACAO_MUSICAL_RASGO.md,
// `CRI-SCR-001`, autor: "ótimo, vamos implementar o musicxml", 20 ago.
// 2026). Reaproveita EXATAMENTE os mesmos `NoteEvent`/`TempoEvent` já
// capturados pro MIDI (`buildTempoSegments`/`secondsToTicksPiecewise`
// acima, compartilhados) - a diferença real não é a fonte de dados, é
// que aqui o tempo em segundos é QUANTIZADO pra uma grade de "steps"
// (mesma unidade de `TimeSignature`/`stepsPerBeat`, CRI-CMP-001) e
// escrito como figura de nota (semínima/colcheia/etc., não ticks) -
// exatamente o vocabulário que só um formato de partitura de verdade
// tem. Como o ANTITOTEM já CONHECE o grid que gerou cada nota (não
// precisa inferir de áudio genérico - o mesmo argumento já registrado
// em CRI-MID-001/CRI-SCR-001), essa quantização é uma conversão
// determinística, não um palpite de transcrição.
//
// Simplificações deliberadas desta primeira versão (ver
// PESQUISA_REPRESENTACAO_MUSICAL_RASGO.md pra limitações completas):
// só pitch/ritmo/dinâmica saem no arquivo - articulação real
// (`GestureType`) ainda não chega até aqui porque `MidiCapture::
// NoteEvent` ainda não carrega esse campo (extensão natural, não
// decidida - ver `CRI-SCR-001`, "RASGO Musical Event"); clave de sol
// fixa pras três partes; só correto quando `TimeSignature.beatUnit`==4
// (o único caso possível hoje, sem UI pra mudar).
inline bool writeMusicXmlCaptureToFile(const std::vector<MidiCapture::NoteEvent>& events,
                                        const std::vector<MidiCapture::TempoEvent>& tempos, const juce::File& file,
                                        unsigned int beatsPerMeasure, unsigned int beatUnit, unsigned int stepsPerBeatIn)
{
    if (events.empty() || tempos.empty()) return false;
    constexpr int ticksPerQuarterNote = 960;
    const auto segments = buildTempoSegments(tempos, ticksPerQuarterNote);
    const int stepsPerBeat = static_cast<int>(juce::jmax(1U, stepsPerBeatIn));
    const int stepsPerMeasureVal = stepsPerBeat * static_cast<int>(juce::jmax(1U, beatsPerMeasure));

    struct QuantNote { int startStep; int endStep; int midiNote; int velocity; };
    struct XmlItem { bool isRest; int steps; int midiNote; int velocity; };

    // Passo 1: por trilha, quantiza pra steps e resolve sobreposições/
    // lacunas (a mesma responsabilidade que `MidiCapture::closeNote()`
    // já tem em segundos contínuos, aqui na grade discreta).
    std::array<std::vector<XmlItem>, MidiCapture::trackCount> tracksItems;
    std::array<int, MidiCapture::trackCount> trackEndSteps { 0, 0, 0 };
    for (int t = 0; t < MidiCapture::trackCount; ++t)
    {
        std::vector<QuantNote> quantized;
        for (const auto& event : events)
        {
            if (event.track != t) continue;
            auto startStep = secondsToStep(segments, event.startSeconds, ticksPerQuarterNote, stepsPerBeat);
            auto endStep = secondsToStep(segments, event.endSeconds, ticksPerQuarterNote, stepsPerBeat);
            if (endStep <= startStep) endStep = startStep + 1; // piso de 1 step - nunca duração zero
            quantized.push_back({ startStep, endStep, event.midiNote, event.velocity });
        }
        std::stable_sort(quantized.begin(), quantized.end(), [] (const QuantNote& a, const QuantNote& b) { return a.startStep < b.startStep; });
        int cursor = 0;
        for (auto& note : quantized)
        {
            if (note.startStep < cursor) note.startStep = cursor; // sobreposição por arredondamento - encosta na anterior
            if (note.endStep <= note.startStep) note.endStep = note.startStep + 1;
            if (note.startStep > cursor)
                tracksItems[static_cast<std::size_t>(t)].push_back({ true, note.startStep - cursor, 0, 0 });
            tracksItems[static_cast<std::size_t>(t)].push_back({ false, note.endStep - note.startStep, note.midiNote, note.velocity });
            cursor = note.endStep;
        }
        trackEndSteps[static_cast<std::size_t>(t)] = cursor;
    }

    // MusicXML exige o MESMO número de compassos em todas as partes -
    // usa o maior entre as três trilhas, completa as mais curtas (ou
    // silenciosas o take inteiro) com silêncio.
    int totalSteps = 0;
    for (const auto endStep : trackEndSteps) totalSteps = juce::jmax(totalSteps, endStep);
    const int totalMeasures = juce::jmax(1, (totalSteps + stepsPerMeasureVal - 1) / stepsPerMeasureVal);
    const int paddedTotalSteps = totalMeasures * stepsPerMeasureVal;
    for (int t = 0; t < MidiCapture::trackCount; ++t)
    {
        const auto remaining = paddedTotalSteps - trackEndSteps[static_cast<std::size_t>(t)];
        if (remaining > 0) tracksItems[static_cast<std::size_t>(t)].push_back({ true, remaining, 0, 0 });
    }

    // Compassos onde o andamento muda de verdade (mesmos `segments` já
    // usados pro MIDI), convertidos de tick pra número de compasso -
    // vira uma <direction><sound tempo=.../></direction> no início do
    // compasso certo, em vez de um único metaevento no tick 0.
    struct TempoAtMeasure { int measure; double bpm; };
    std::vector<TempoAtMeasure> tempoAtMeasure;
    for (const auto& segment : segments)
    {
        const auto step = juce::jmax(0, static_cast<int>(std::lround(segment.startTicks / (static_cast<double>(ticksPerQuarterNote) / stepsPerBeat))));
        const auto measureIndex = step / stepsPerMeasureVal + 1;
        if (tempoAtMeasure.empty() || tempoAtMeasure.back().measure != measureIndex)
            tempoAtMeasure.push_back({ measureIndex, segment.bpm });
    }

    constexpr std::array<const char*, MidiCapture::trackCount> trackNames { "PRINCIPAL", "CLONE", "EXCITACAO" };
    juce::String xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
        << "<score-partwise version=\"4.0\">\n<part-list>\n";
    for (int t = 0; t < MidiCapture::trackCount; ++t)
        xml << "<score-part id=\"P" << (t + 1) << "\"><part-name>" << trackNames[static_cast<std::size_t>(t)] << "</part-name></score-part>\n";
    xml << "</part-list>\n";

    for (int t = 0; t < MidiCapture::trackCount; ++t)
    {
        xml << "<part id=\"P" << (t + 1) << "\">\n";
        int stepsIntoMeasure = 0;
        int measureNumber = 1;
        std::size_t tempoCursor = 0;
        juce::String lastDynamic;
        auto openMeasure = [&]
        {
            xml << "<measure number=\"" << measureNumber << "\">\n";
            if (measureNumber == 1)
                xml << "<attributes><divisions>" << stepsPerBeat << "</divisions><key><fifths>0</fifths></key>"
                    << "<time><beats>" << static_cast<int>(beatsPerMeasure) << "</beats><beat-type>" << static_cast<int>(beatUnit) << "</beat-type></time>"
                    << "<clef><sign>G</sign><line>2</line></clef></attributes>\n";
            while (tempoCursor < tempoAtMeasure.size() && tempoAtMeasure[tempoCursor].measure == measureNumber)
            {
                const auto bpmText = juce::String(tempoAtMeasure[tempoCursor].bpm, 1);
                xml << "<direction placement=\"above\"><direction-type><metronome><beat-unit>quarter</beat-unit><per-minute>"
                    << bpmText << "</per-minute></metronome></direction-type><sound tempo=\"" << bpmText << "\"/></direction>\n";
                ++tempoCursor;
            }
        };
        auto closeMeasure = [&]
        {
            xml << "</measure>\n";
            stepsIntoMeasure = 0;
            ++measureNumber;
            if (measureNumber <= totalMeasures) openMeasure();
        };
        openMeasure();

        for (const auto& item : tracksItems[static_cast<std::size_t>(t)])
        {
            int remaining = item.steps;
            const juce::String dynamicMark = item.isRest ? juce::String() : juce::String(dynamicMarkFromVelocity(item.velocity));
            bool firstChunkOfItem = true;
            while (remaining > 0)
            {
                const auto roomInMeasure = stepsPerMeasureVal - stepsIntoMeasure;
                const auto cap = juce::jmin(remaining, roomInMeasure);
                int base = 1;
                while (base * 2 <= cap) base *= 2;
                int chunkSteps = base;
                bool dotted = false;
                if (base >= 2 && base + base / 2 <= cap) { chunkSteps = base + base / 2; dotted = true; }
                const auto isLastChunk = (remaining - chunkSteps == 0);

                if (! item.isRest && firstChunkOfItem && dynamicMark != lastDynamic)
                {
                    xml << "<direction placement=\"below\"><direction-type><dynamics><" << dynamicMark << "/></dynamics></direction-type></direction>\n";
                    lastDynamic = dynamicMark;
                }
                xml << "<note>\n";
                if (item.isRest) xml << "<rest/>\n";
                else
                {
                    const auto xmlPitch = midiNoteToXmlPitch(item.midiNote);
                    xml << "<pitch><step>" << xmlPitch.step << "</step>";
                    if (xmlPitch.alter != 0) xml << "<alter>" << xmlPitch.alter << "</alter>";
                    xml << "<octave>" << xmlPitch.octave << "</octave></pitch>\n";
                }
                xml << "<duration>" << chunkSteps << "</duration><voice>1</voice>"
                    << "<type>" << noteTypeForBaseSteps(base, stepsPerBeat) << "</type>";
                if (dotted) xml << "<dot/>";
                if (! item.isRest)
                {
                    if (! firstChunkOfItem) xml << "<tie type=\"stop\"/>";
                    if (! isLastChunk) xml << "<tie type=\"start\"/>";
                    juce::String tiedNotation;
                    if (! firstChunkOfItem) tiedNotation << "<tied type=\"stop\"/>";
                    if (! isLastChunk) tiedNotation << "<tied type=\"start\"/>";
                    if (tiedNotation.isNotEmpty()) xml << "<notations>" << tiedNotation << "</notations>";
                }
                xml << "</note>\n";

                stepsIntoMeasure += chunkSteps;
                remaining -= chunkSteps;
                firstChunkOfItem = false;
                if (stepsIntoMeasure >= stepsPerMeasureVal) closeMeasure();
            }
        }
        xml << "</part>\n";
    }
    xml << "</score-partwise>\n";

    auto stream = file.createOutputStream();
    if (stream == nullptr) return false;
    return stream->writeText(xml, false, false, nullptr);
}

class StepControl final : public juce::Component
{
public:
    // cloneAccent: CLONE's own 16 steps pass true, so their CV/AMP/FX
    // colours and mute button follow cloneMaterial's hue shift too
    // (author, live: "layout aprovado, agora passar para etapa de mudar
    // as cores entre o principal e clone") - this shared class used to
    // hardcode `material::` regardless of which tab's steps[] array
    // owned the instance.
    explicit StepControl(int stepNumber, bool cloneAccent = false) : number(stepNumber)
    {
        addAndMakeVisible(cv); addAndMakeVisible(level); addAndMakeVisible(send); addAndMakeVisible(mute);
        cv.setSliderStyle(juce::Slider::LinearVertical);
        // NoTextBox, not TextBoxBelow (18 ago. 2026, author: "remova as
        // caixas de numeros dos steps do sequencer, e alongue os sliders
        // vertigacais (verdes)") - same fix already used for CLOCK/
        // ENERGIA/MASTER: TextBoxBelow reserves its own height INSIDE the
        // slider component's bounds, so removing it also lengthens the
        // green track automatically (the whole point of the "alongue" half
        // of the request), not just hiding the "0.12"-style readout.
        cv.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        cv.setRange(0.0, 1.0, 0.01);
        cv.setValue(0.12 + 0.10 * static_cast<double>(stepNumber % 8));
        const auto clockColour = cloneAccent ? cloneMaterial::clock : material::clock;
        const auto voiceColour = cloneAccent ? cloneMaterial::voice : material::voice;
        const auto returnPathColour = cloneAccent ? cloneMaterial::returnPath : material::returnPath;
        cv.setColour(juce::Slider::thumbColourId, clockColour);
        cv.setColour(juce::Slider::trackColourId, clockColour.darker(0.72f));
        for (auto* slider : { &level, &send })
        {
            slider->setSliderStyle(juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider->setRange(0.0, 1.0, 0.01);
        }
        level.setValue(1.0); send.setValue(0.35);
        level.setColour(juce::Slider::thumbColourId, voiceColour);
        level.setColour(juce::Slider::trackColourId, voiceColour.darker(0.72f));
        send.setColour(juce::Slider::thumbColourId, returnPathColour);
        send.setColour(juce::Slider::trackColourId, returnPathColour.darker(0.72f));
        mute.setComponentID("mute");
        mute.setButtonText("M");
        mute.setLookAndFeel(cloneAccent ? &patchToggleLookClone() : &patchToggleLook());
        setLanguage(antitotem::ui::Language::english);
    }
    void setActive(bool value) { active = value; repaint(); }
    // Same text for all 16 steps, in both tabs - called once by whichever
    // component owns the steps[] array when the shared language switch
    // changes.
    void setLanguage(antitotem::ui::Language language)
    {
        level.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::stepAmp, language));
        send.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::stepFx, language));
        mute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::stepMute, language));
    }
    void resized() override
    {
        auto controls = getLocalBounds().withTrimmedTop(30).reduced(3, 0);
        // CV now runs the component's full height, top-aligned with the
        // "CV n" title itself (author, live: "os slider verdes sejam mais
        // longos, passem a se alinhar com o top do titulo") - longer travel
        // for the same control, while AMP/FX/M keep their own start point
        // unchanged below the active-step dot.
        auto cvArea = getLocalBounds().reduced(3, 0);
        cv.setBounds(cvArea.removeFromLeft(static_cast<int>(cvArea.getWidth() * 0.54f)).reduced(0, 2));
        controls.removeFromLeft(static_cast<int>(controls.getWidth() * 0.54f));
        auto secondary = controls.reduced(3, 0);
        // AMP/FX are real label rects now, stored and used directly by
        // paint() below - they used to be painted at their own hardcoded
        // offset while the sliders moved independently in here, so any
        // spacing change desynced the two and the label ended up
        // overlapping its own slider.
        levelLabelBounds = secondary.removeFromTop(11);
        level.setBounds(secondary.removeFromTop(18));
        sendLabelBounds = secondary.removeFromTop(11);
        send.setBounds(secondary.removeFromTop(18));
        // Compact, like the mixer's own M button - not a bar stretched to
        // the column's full width.
        mute.setBounds(secondary.removeFromTop(16).withSizeKeepingCentre(28, 14));
    }
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xffded4be));
        g.setFont(uiFont(13.0f, true));
        g.drawFittedText("CV " + juce::String(number + 1), getLocalBounds().removeFromTop(18), juce::Justification::centred, 1);
        g.setColour(active ? juce::Colour(0xffff5538) : juce::Colour(0xff38332b));
        g.fillEllipse(static_cast<float>(getWidth() / 2 - 5), 19.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0xffbdb199));
        g.setFont(uiFont(9.0f));
        g.drawText("AMP", levelLabelBounds, juce::Justification::centred);
        g.drawText("FX", sendLabelBounds, juce::Justification::centred);
    }
    juce::Slider cv, level, send;
    juce::ToggleButton mute;
private:
    juce::Rectangle<int> levelLabelBounds, sendLabelBounds;
    int number = 0;
    bool active = false;
};

// A word is a control/module name if it has no lowercase letter and at
// least one uppercase letter or digit - matches the ALL-CAPS convention
// already used throughout tutorialChapters/UiLanguage.h for every
// parameter and module name (FREQ, OSC A, VCF, 16 STEPS...), so no
// content change is needed anywhere - this is purely a rendering pass.
inline bool isCapsWord(const juce::String& word)
{
    if (word.isEmpty()) return false;
    bool hasCapsLetterOrDigit = false;
    for (auto c : word)
    {
        if (juce::CharacterFunctions::isLowerCase(c)) return false;
        if (juce::CharacterFunctions::isUpperCase(c) || juce::CharacterFunctions::isDigit(c))
            hasCapsLetterOrDigit = true;
    }
    return hasCapsLetterOrDigit;
}

// Writes `text` into `editor` with every ALL-CAPS run (author, live: "no
// tutorial para cada item... destaque em outra cor, assim será mais
// fácil de identificar no texto") coloured differently from the prose
// around it, so control/module names read as scannable landmarks.
// Adjacent caps words (e.g. "OSC A") merge into one highlighted span
// instead of two, since the whitespace between them just extends
// whichever run is already open.
inline void setHighlightedBody(juce::TextEditor& editor, const juce::String& text,
                                juce::Colour normalColour, juce::Colour highlightColour)
{
    struct Run { juce::String text; bool highlighted; };
    std::vector<Run> runs;
    juce::String current;
    bool currentHighlighted = false;
    bool haveCurrent = false;
    auto pushCurrent = [&]
    {
        if (haveCurrent && current.isNotEmpty()) runs.push_back({ current, currentHighlighted });
        current.clear();
        haveCurrent = false;
    };
    int i = 0;
    const int len = text.length();
    while (i < len)
    {
        const bool isSpace = juce::CharacterFunctions::isWhitespace(text[i]);
        const int start = i;
        while (i < len && juce::CharacterFunctions::isWhitespace(text[i]) == isSpace) ++i;
        const auto piece = text.substring(start, i);
        if (isSpace)
        {
            if (haveCurrent) current += piece;
            else { current = piece; currentHighlighted = false; haveCurrent = true; }
        }
        else
        {
            const bool highlighted = isCapsWord(piece);
            if (haveCurrent && currentHighlighted == highlighted) current += piece;
            else { pushCurrent(); current = piece; currentHighlighted = highlighted; haveCurrent = true; }
        }
    }
    pushCurrent();
    editor.clear();
    for (auto& run : runs)
    {
        editor.setColour(juce::TextEditor::textColourId, run.highlighted ? highlightColour : normalColour);
        editor.insertTextAtCaret(run.text);
    }
}

// Chapter-based, localized TUTORIAL panel - same architecture already
// proven in NAVALHA2_JUCE/src/app/Main.cpp (TutorialComponent/Window):
// a sidebar of chapters, a scrollable body, and previous/next navigation,
// all driven by antitotem::ui::tutorialChapters and a switchable Language.
class TutorialComponent final : public juce::Component
{
public:
    explicit TutorialComponent(antitotem::ui::Language initialLanguage) : language(initialLanguage)
    {
        heading.setJustificationType(juce::Justification::centredLeft);
        heading.setFont(uiFont(19.0f, true));
        heading.setColour(juce::Label::textColourId, material::board);
        addAndMakeVisible(heading);

        // Three levels (18 ago. 2026, author: "sim, em todo o instrumento",
        // confirming a recommendation given the same day) - not new content
        // invented for this, three things that already existed separately
        // now each given a place: BÁSICO is the original 8 chapters below;
        // INTERMEDIÁRIO ports docs/TUTORIAIS.md's own gesture-and-listen
        // exercises; AVANÇADO explains relationships between sections
        // (docs/FLUXO_DE_SINAL.md's own "topologia de roteamento" third),
        // not yet covering every object - see TAREFAS.md.
        const antitotem::ui::LocalizedText levelNames[3] {
            { "BASIC", "BÁSICO", "DE BASE", "BÁSICO" },
            { "MEDIUM", "MÉDIO", "MOYEN", "MEDIO" },
            { "ADVANCED", "AVANÇADO", "AVANCÉ", "AVANZADO" }
        };
        for (std::size_t index = 0; index < levelButtons.size(); ++index)
        {
            auto& button = levelButtons[index];
            button.setButtonText(antitotem::ui::text(levelNames[index], language));
            button.setClickingTogglesState(false);
            button.setComponentID("core");
            button.setLookAndFeel(&panelButtonLook());
            button.onClick = [this, index] { selectLevel(static_cast<int>(index)); };
            addAndMakeVisible(button);
        }

        contentsHeading.setJustificationType(juce::Justification::centredLeft);
        contentsHeading.setFont(uiFont(11.0f, true));
        contentsHeading.setColour(juce::Label::textColourId, material::board);
        addAndMakeVisible(contentsHeading);
        for (std::size_t index = 0; index < contentsButtons.size(); ++index)
        {
            auto& button = contentsButtons[index];
            button.setClickingTogglesState(false);
            button.setComponentID("core");
            button.setLookAndFeel(&panelButtonLook());
            button.onClick = [this, index] { selectChapter(static_cast<int>(index)); };
            addAndMakeVisible(button);
        }

        body.setMultiLine(true);
        body.setReadOnly(true);
        body.setCaretVisible(false);
        body.setScrollbarsShown(true);
        body.setFont(uiFont(13.5f));
        body.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12110e));
        body.setColour(juce::TextEditor::outlineColourId, material::wood.brighter(0.35f));
        body.setColour(juce::TextEditor::textColourId, material::metal);
        addAndMakeVisible(body);

        // Tip callout - a chapter's own practical tip (author: "avance nos
        // tutoriais, tudo bem organizado, com layout simpático e
        // didático") used to sit inline inside the body text as a plain
        // "Tip:"/"Dica:" paragraph, indistinguishable from the rest by
        // anything but the word itself. Its own small heading, styled the
        // same way as the sidebar's own CONTENTS/SUMÁRIO heading (same
        // font/colour recipe), and a highlighted background drawn in
        // paint() below, so it reads as "the one thing to try first" at a
        // glance instead of one more paragraph to read in full.
        tipHeading.setJustificationType(juce::Justification::centredLeft);
        tipHeading.setFont(uiFont(12.5f, true));
        tipHeading.setColour(juce::Label::textColourId, material::shadow);
        addAndMakeVisible(tipHeading);
        tipBody.setJustificationType(juce::Justification::topLeft);
        // 14, not 13 - a highlighted callout reading smaller than the
        // 13.5 body text it's supposed to stand out from defeated the
        // point (author: "algumas dicas as letras estão pequenas").
        tipBody.setFont(uiFont(14.0f));
        tipBody.setColour(juce::Label::textColourId, material::shadow);
        addAndMakeVisible(tipBody);

        previous.setLookAndFeel(&panelButtonLook());
        previous.onClick = [this] { selectChapter(currentChapter - 1); };
        addAndMakeVisible(previous);
        next.setLookAndFeel(&panelButtonLook());
        next.onClick = [this] { selectChapter(currentChapter + 1); };
        addAndMakeVisible(next);
        refresh();
        setSize(980, 620);
    }
    ~TutorialComponent() override
    {
        for (auto& button : levelButtons) button.setLookAndFeel(nullptr);
        for (auto& button : contentsButtons) button.setLookAndFeel(nullptr);
        previous.setLookAndFeel(nullptr);
        next.setLookAndFeel(nullptr);
    }

    void setLanguage(antitotem::ui::Language newLanguage) { language = newLanguage; refresh(); }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(material::shadow);
        g.setColour(material::board);
        g.fillRect(0, 0, 6, getHeight());
        g.setColour(material::wood.brighter(0.2f));
        g.drawRect(getLocalBounds().reduced(10), 1);
        // Tip callout background - drawn here, not as a Component of its
        // own, so it sits behind tipHeading/tipBody like the sidebar's own
        // accent stripe above sits behind everything else in this
        // paint(). Only non-empty when the current chapter actually has a
        // tip (see hasTip()/resized()).
        if (! tipBounds.isEmpty())
        {
            g.setColour(material::board);
            g.fillRoundedRectangle(tipBounds.toFloat(), 6.0f);
        }
    }
    void resized() override
    {
        auto area = getLocalBounds().reduced(24);
        auto navigation = area.removeFromBottom(46);
        auto contents = area.removeFromLeft(240);
        area.removeFromLeft(12);
        auto contentsTop = contents.removeFromTop(34);
        // Full width now - no language_ button sharing this row anymore
        // (author: "o tutorial não precisa de botão de língua, irá abrir
        // na língua que está configurado o instrumento na aba principal"
        // - language already follows PRINCIPAL via setLanguage(), a
        // second control here was redundant, not a missing feature).
        contentsHeading.setBounds(contentsTop.reduced(4, 0));

        // Level row - own strip above the chapter list, same width split
        // 3 ways as the panel's other equal-button rows.
        auto levelRow = contents.removeFromTop(26);
        const auto levelButtonWidth = levelRow.getWidth() / static_cast<int>(levelButtons.size());
        for (auto& button : levelButtons) button.setBounds(levelRow.removeFromLeft(levelButtonWidth).reduced(2));
        contents.removeFromTop(6);

        const auto chapterCount = levelChapterCount(currentLevel);
        const auto contentsButtonHeight = contents.getHeight() / chapterCount;
        for (int index = 0; index < static_cast<int>(contentsButtons.size()); ++index)
        {
            auto& button = contentsButtons[static_cast<std::size_t>(index)];
            const bool inThisLevel = index < chapterCount;
            button.setVisible(inThisLevel);
            if (inThisLevel)
                button.setBounds(contents.removeFromTop(contentsButtonHeight).reduced(2));
        }

        heading.setBounds(area.removeFromTop(44));
        area.removeFromTop(10);
        navigation.removeFromLeft(252);
        previous.setBounds(navigation.removeFromLeft(150).reduced(2));
        next.setBounds(navigation.removeFromRight(150).reduced(2));

        // Tip box only claims space when the current chapter actually has
        // one - most chapters still don't (see TAREFAS.md), and an empty
        // highlighted box would read as a mistake, not a feature.
        const bool showTip = hasTip();
        tipHeading.setVisible(showTip);
        tipBody.setVisible(showTip);
        if (showTip)
        {
            // Was 76 - too tight for the longer tips (author: "o quadro
            // da dica está pequeno, algumas dicas as letras estão
            // pequenas") - a Label's own drawFittedText() shrinks the
            // font to squeeze wrapped text into whatever height it's
            // given rather than clipping it, so a cramped box didn't
            // just look tight, it silently shrank the text too. 130
            // comfortably fits the longest tip written so far (AVANÇADO's
            // own AUDIO FLOW/OBJECT ROUTING chapters) at the font size
            // set in the constructor without the label having to shrink
            // it further.
            tipBounds = area.removeFromBottom(130);
            area.removeFromBottom(10);
            auto tipArea = tipBounds.reduced(16, 12);
            tipHeading.setBounds(tipArea.removeFromTop(18));
            tipBody.setBounds(tipArea);
        }
        else
        {
            tipBounds = {};
        }
        body.setBounds(area.reduced(2));
    }

private:
    // 0 = BÁSICO, 1 = INTERMEDIÁRIO, 2 = AVANÇADO - three different
    // std::array<TutorialChapter, N> (different N each), so lookups by
    // level go through these two helpers instead of templating every
    // call site.
    static int levelChapterCount(int level)
    {
        if (level == 1) return static_cast<int>(antitotem::ui::tutorialChaptersIntermediate.size());
        if (level == 2) return static_cast<int>(antitotem::ui::tutorialChaptersAdvanced.size());
        return static_cast<int>(antitotem::ui::tutorialChapters.size());
    }
    static const antitotem::ui::TutorialChapter& chapterAt(int level, int index)
    {
        const auto position = static_cast<std::size_t>(index);
        if (level == 1) return antitotem::ui::tutorialChaptersIntermediate[position];
        if (level == 2) return antitotem::ui::tutorialChaptersAdvanced[position];
        return antitotem::ui::tutorialChapters[position];
    }
    const antitotem::ui::TutorialChapter& currentChapterData() const { return chapterAt(currentLevel, currentChapter); }
    bool hasTip() const { return antitotem::ui::text(currentChapterData().tip, language).isNotEmpty(); }
    void selectLevel(int level)
    {
        currentLevel = juce::jlimit(0, 2, level);
        currentChapter = 0;
        refresh();
    }
    void selectChapter(int index)
    {
        currentChapter = juce::jlimit(0, levelChapterCount(currentLevel) - 1, index);
        refresh();
    }
    void refresh()
    {
        const auto& value = currentChapterData();
        const auto chapterCount = levelChapterCount(currentLevel);
        heading.setText(antitotem::ui::text(value.title, language), juce::dontSendNotification);
        const antitotem::ui::LocalizedText contentsText { "CONTENTS", "SUMÁRIO", "SOMMAIRE", "SUMARIO" };
        contentsHeading.setText(antitotem::ui::text(contentsText, language), juce::dontSendNotification);
        const antitotem::ui::LocalizedText tipHeadingText { "TIP", "DICA", "ASTUCE", "CONSEJO" };
        tipHeading.setText(antitotem::ui::text(tipHeadingText, language), juce::dontSendNotification);
        tipBody.setText(antitotem::ui::text(value.tip, language), juce::dontSendNotification);
        const antitotem::ui::LocalizedText levelNames[3] {
            { "BASIC", "BÁSICO", "DE BASE", "BÁSICO" },
            { "MEDIUM", "MÉDIO", "MOYEN", "MEDIO" },
            { "ADVANCED", "AVANÇADO", "AVANCÉ", "AVANZADO" }
        };
        for (std::size_t index = 0; index < levelButtons.size(); ++index)
        {
            levelButtons[index].setButtonText(antitotem::ui::text(levelNames[index], language));
            levelButtons[index].setToggleState(static_cast<int>(index) == currentLevel, juce::dontSendNotification);
        }
        for (int index = 0; index < chapterCount; ++index)
        {
            const auto position = static_cast<std::size_t>(index);
            contentsButtons[position].setButtonText(
                antitotem::ui::text(chapterAt(currentLevel, index).title, language));
            contentsButtons[position].setToggleState(index == currentChapter, juce::dontSendNotification);
        }
        setHighlightedBody(body, antitotem::ui::text(value.body, language), material::metal, juce::Colour(0xffffca5c));
        previous.setEnabled(currentChapter > 0);
        next.setEnabled(currentChapter + 1 < chapterCount);
        const std::array<antitotem::ui::LocalizedText, 2> buttonText {{
            { "< PREVIOUS", "< ANTERIOR", "< PRECEDENT", "< ANTERIOR" },
            { "NEXT >", "PRÓXIMO >", "SUIVANT >", "SIGUIENTE >" } }};
        previous.setButtonText(antitotem::ui::text(buttonText[0], language));
        next.setButtonText(antitotem::ui::text(buttonText[1], language));
        // Whether the tip box shows at all (hasTip()) can change with the
        // chapter/level/language just set above - resized() is what
        // actually decides that and gives body the reclaimed space back
        // when there isn't one, and also what hides contentsButtons past
        // the current level's own chapter count.
        resized();
    }

    antitotem::ui::Language language;
    int currentLevel = 0;
    int currentChapter = 0;
    juce::Label heading, contentsHeading;
    std::array<juce::TextButton, 3> levelButtons;
    // Sized to BÁSICO's own 8 - the largest of the three levels; fewer
    // chapters at another level just hides the extras (see resized()).
    std::array<juce::TextButton, antitotem::ui::tutorialChapters.size()> contentsButtons;
    juce::TextEditor body;
    juce::Label tipHeading, tipBody;
    juce::Rectangle<int> tipBounds;
    juce::TextButton previous { "< PREVIOUS" }, next { "NEXT >" };
};

class TutorialWindow final : public juce::DocumentWindow
{
public:
    explicit TutorialWindow(antitotem::ui::Language language)
        : DocumentWindow("Antitotem - TUTORIAL", material::shadow, DocumentWindow::allButtons)
    {
        tutorial = new TutorialComponent(language);
        setUsingNativeTitleBar(true);
        setContentOwned(tutorial, true);
        setResizable(true, true);
        setResizeLimits(700, 460, 1300, 900);
        centreWithSize(980, 620);
        setVisible(true);
    }
    void setLanguage(antitotem::ui::Language language) { if (tutorial != nullptr) tutorial->setLanguage(language); }
    void closeButtonPressed() override { setVisible(false); }
private:
    TutorialComponent* tutorial = nullptr;
};

// SOBRE panel: same localized-window pattern as TUTORIAL, but a single
// scrollable body instead of chapters - the content (v1, licença AGPLv3,
// créditos) is short enough not to need pagination.
class AppInfoComponent final : public juce::Component
{
public:
    explicit AppInfoComponent(antitotem::ui::Language initialLanguage) : language(initialLanguage)
    {
        title.setJustificationType(juce::Justification::centredLeft);
        title.setFont(uiFont(22.0f, true));
        title.setColour(juce::Label::textColourId, material::board);
        addAndMakeVisible(title);

        body.setMultiLine(true);
        body.setReadOnly(true);
        body.setCaretVisible(false);
        body.setScrollbarsShown(true);
        body.setFont(uiFont(13.5f));
        body.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12110e));
        body.setColour(juce::TextEditor::outlineColourId, material::wood.brighter(0.35f));
        body.setColour(juce::TextEditor::textColourId, material::metal);
        addAndMakeVisible(body);

        language_.setButtonText(antitotem::ui::languageLabel(language));
        language_.setLookAndFeel(&panelButtonLook());
        language_.onClick = [this]
        {
            language = antitotem::ui::nextLanguage(language);
            if (onLanguageChanged) onLanguageChanged(language);
            refresh();
        };
        addAndMakeVisible(language_);
        refresh();
        setSize(680, 460);
    }
    ~AppInfoComponent() override { language_.setLookAndFeel(nullptr); }

    void setLanguage(antitotem::ui::Language newLanguage) { language = newLanguage; refresh(); }
    std::function<void(antitotem::ui::Language)> onLanguageChanged;

    void paint(juce::Graphics& g) override
    {
        g.fillAll(material::shadow);
        g.setColour(material::board);
        g.fillRect(0, 0, 6, getHeight());
        g.setColour(material::wood.brighter(0.2f));
        g.drawRect(getLocalBounds().reduced(10), 1);
    }
    void resized() override
    {
        auto area = getLocalBounds().reduced(24);
        auto header = area.removeFromTop(38);
        title.setBounds(header.removeFromLeft(header.getWidth() - 60));
        language_.setBounds(header.reduced(2, 2));
        area.removeFromTop(10);
        body.setBounds(area.reduced(2));
    }

private:
    void refresh()
    {
        title.setText(antitotem::ui::text(antitotem::ui::aboutContent.title, language), juce::dontSendNotification);
        body.setText(antitotem::ui::text(antitotem::ui::aboutContent.body, language), false);
    }

    antitotem::ui::Language language;
    juce::Label title;
    juce::TextEditor body;
    juce::TextButton language_;
};

class AppInfoWindow final : public juce::DocumentWindow
{
public:
    explicit AppInfoWindow(antitotem::ui::Language language)
        : DocumentWindow("Antitotem - SOBRE", material::shadow, DocumentWindow::allButtons)
    {
        info = new AppInfoComponent(language);
        setUsingNativeTitleBar(true);
        setContentOwned(info, true);
        setResizable(true, true);
        setResizeLimits(520, 360, 1000, 800);
        centreWithSize(680, 460);
        setVisible(true);
    }
    void setLanguage(antitotem::ui::Language language) { if (info != nullptr) info->setLanguage(language); }
    std::function<void(antitotem::ui::Language)>* languageCallback() { return info != nullptr ? &info->onLanguageChanged : nullptr; }
    void closeButtonPressed() override { setVisible(false); }
private:
    AppInfoComponent* info = nullptr;
};

// Shared by both MainComponent and ObjectFiveComponent so the two tabs stay
// visually identical instead of drifting apart through separately hand-
// tuned copies - ESPAÇO/FASE/ROTAS ATIVAS lay out the same way in both,
// just with different left/right trims (the main tab's band spans the
// full width including the CLOCK/mixer columns; CLONE's `area` is
// already trimmed to the space between them, so it passes 0/0). FORMA
// LFO and MODULAÇÃO used to anchor this band too (a fixed 120px column
// plus one of the flexible ones) but moved out entirely on 18 ago. 2026
// - see layoutVoiceArea()'s own comment on the empty space below ADSR/
// NOISE they moved into. That frees this whole band to give every
// remaining column more width than it had before, not just make room
// for the new CAOS/VAGA column that took MODULAÇÃO's old slot.
inline void layoutRailsBand(juce::Rectangle<int> rails, int leftTrim, int rightTrim,
                             juce::Label& parametersLabel,
                             juce::Label& effectsLabel, juce::Label* effectLabels, juce::Slider* effectControls,
                             juce::Label& detailLabel, juce::Label* detailLabels, juce::Slider* detailControls,
                             juce::Label& materialLabel,
                             juce::Label& chaosLabel)
{
    rails.removeFromLeft(leftTrim);
    rails.removeFromRight(rightTrim);

    // Umbrella title for the whole band (18 ago. 2026) - every other
    // section of the panel already has one; this was the one gap left.
    // Spans the full band width, above the 6 columns' own smaller
    // per-column headings, not replacing them.
    parametersLabel.setBounds(rails.removeFromTop(16));

    // ESPAÇO/FASE and ROTAS ATIVAS lay out in vertical columns, each
    // stacking its own heading and 3 items top-to-bottom - ROTAS ATIVAS
    // has 9 controls, so it gets two of these column-widths instead of
    // one (still 3 items stacked in each).
    auto verticalRailGroup = [] (juce::Rectangle<int> column, juce::Label* heading,
                                  juce::Label* labels, juce::Slider* controls, int count)
    {
        // Always reserve the heading's own height, labelled or not - the
        // second ROTAS ATIVAS column has no heading text of its own, but
        // must still start its items at the same Y as the first.
        auto headingSlot = column.removeFromTop(14);
        if (heading != nullptr) heading->setBounds(headingSlot);
        const auto itemHeight = column.getHeight() / count;
        for (int i = 0; i < count; ++i)
        {
            auto cell = column.removeFromTop(itemHeight).reduced(4, 0);
            labels[i].setBounds(cell.removeFromTop(11));
            controls[i].setBounds(cell.removeFromTop(18));
        }
    };
    // Limitado como o grid de passos acima - sem isso, numa janela
    // redimensionada até 3840px de largura essa banda inteira (MODULAÇÃO/
    // EFEITOS/DETALHE) ficaria ~2.5x mais larga que o desenho original a
    // 1920px (261px de coluna). 320 dá folga real sem inflar demais; a
    // última coluna agora também usa essa mesma largura fixa em vez de
    // absorver toda a sobra, senão ela sozinha ficaria desproporcional
    // enquanto as outras quatro ficassem limitadas.
    // /6 (17 ago. 2026, still 6 as of 18 ago.): MATÉRIA (MaterialFilter's
    // CUTOFF/RESONANCE/DRIVE/ASYMMETRY) needed a 6th column when it
    // joined - same reasoning as RES MIX/ALTURA/CORPO joining ROTAS
    // ATIVAS rather than fighting for a new spot. CAOS/VAGA's own
    // DRIVE/DAMPING/DEPTH (18 ago. 2026) took over MODULAÇÃO's old slot
    // one-for-one when it moved out (see this function's own top
    // comment), so the column count didn't change again here - but
    // every column is wider now than it was with MODULAÇÃO in this same
    // slot, since FORMA LFO no longer eats a fixed 120px before this
    // division happens.
    const auto railColumnWidth = std::clamp(rails.getWidth() / 6, 130, 280);
    // Order rotated (18 ago. 2026, author: "se caos está relacionado aos
    // botões de FORMA LFO ele deve se deslocar para baixo do FORMA LFO,
    // onde está a coluna matéria, assim a coluna matéria passa a ficar
    // mais abaixo (próxima) do slider materia") - CAOS's own DRIVE/
    // DAMPING/DEPTH only mean anything with FORMA LFO's CAOS/VAGA shapes
    // selected (see FLUXO_DE_SINAL.md 3.1-adjacent reasoning), so it moved
    // into MATÉRIA's old last slot, which sits directly under FORMA LFO in
    // layoutVoiceArea(). A straight two-column swap would have put MATÉRIA
    // in the FIRST slot instead (under OSC A, further from MAT, not
    // closer) - a left-rotate keeps everything else in place and only
    // shifts MATÉRIA one slot left, landing under/near the MAT slider in
    // the VCF column above, which was the actual goal.
    verticalRailGroup(rails.removeFromLeft(railColumnWidth), &effectsLabel, effectLabels, effectControls, 3);
    verticalRailGroup(rails.removeFromLeft(railColumnWidth), &detailLabel, detailLabels, detailControls, 3);
    verticalRailGroup(rails.removeFromLeft(railColumnWidth), nullptr, detailLabels + 3, detailControls + 3, 3);
    verticalRailGroup(rails.removeFromLeft(railColumnWidth), nullptr, detailLabels + 6, detailControls + 6, 3);
    // MATÉRIA has 4 items instead of the 3 every neighbour has - the top
    // 3 (CUTOFF/RESON/DRIVE) now align exactly with the other columns'
    // own rows (author, live, 18 ago. 2026: "os 3 sliders superiores
    // fiquem alinhados com os demais das outras colunas"), using the
    // same itemHeight (column height / 3) instead of dividing by 4 for
    // this column alone. The 4th (ASYMMETRY) keeps that same spacing but
    // necessarily lands below the column's own height as a result -
    // deliberate, author: "o quarto manterá também o mesmo espaçamento
    // mas ficará abaixo (entrando um pouco na área do sequencer)".
    {
        auto materialColumn = rails.removeFromLeft(railColumnWidth);
        materialLabel.setBounds(materialColumn.removeFromTop(14));
        const auto itemHeight = materialColumn.getHeight() / 3;
        // Built from a running Y instead of chaining materialColumn.removeFromTop()
        // for all 4 items: removeFromTop() clamps its result to whatever height is
        // still left in the source rect (see juce_Rectangle.h), so by the 4th
        // iteration only originalHeight - 3*itemHeight (0-2px) remained, collapsing
        // ASYM to an invisible sliver instead of overflowing below with the same
        // itemHeight as the other 3 - caught only after a screenshot showed it
        // missing entirely (author, live, 18 ago. 2026: "onde foi parar o 4 slider
        // de matéria?").
        const auto columnX = materialColumn.getX();
        const auto columnWidth = materialColumn.getWidth();
        auto y = materialColumn.getY();
        for (int i = 0; i < 4; ++i)
        {
            auto cell = juce::Rectangle<int>(columnX, y, columnWidth, itemHeight).reduced(4, 0);
            detailLabels[9 + i].setBounds(cell.removeFromTop(11));
            detailControls[9 + i].setBounds(cell.removeFromTop(18));
            y += itemHeight;
        }
    }
    // CAOS last (see the rotation note above the other 5 columns) - lands
    // directly under FORMA LFO in layoutVoiceArea(), the same physical
    // slot MATÉRIA used to occupy.
    verticalRailGroup(rails.removeFromLeft(railColumnWidth), &chaosLabel, detailLabels + 13, detailControls + 13, 3);
}

// Shared by both MainComponent and ObjectFiveComponent, same reasoning as
// layoutRailsBand() above - CLOCK/PULSO/MÉTRICA/PERCURSO/FIM DO LOOP/
// PORTAS DE FEEDBACK/FB GAIN/DERIVA/VARIAÇÃO lay out identically in both,
// sizes baked in here (not passed in) so the two tabs can't drift apart
// through separately hand-tuned copies again. `deriveLabel` and
// `feedbackGainLabel` are nullable - CLONE's DERIVA has no separate label
// (the button text already says DERIVA) and reuses one label for both the
// doors header and the gain slider's own caption in a way MainComponent's
// naming doesn't, so the caller passes nullptr where it doesn't apply.
inline void layoutTransportColumn(juce::Rectangle<int> transport,
                                   juce::Label& modeLabel,
                                   juce::Label& clockLabel, juce::Slider& clockKnob,
                                   juce::Label& temporalLabel, juce::ToggleButton* temporalButtons, int temporalCount,
                                   // GROOVE - a general long-short modifier layered on every
                                   // SUBDIVISÃO feel, not exclusive to SWG (20 ago. 2026, "ao
                                   // invés de um knob um slider swing", then "deixa o swing
                                   // somente enquanto botão, e utilise esse slide atual do swing
                                   // para o groove"). Has its own title now ("precisa colocar o
                                   // título no slider").
                                   juce::Label& grooveLabel, juce::Slider& grooveAmount,
                                   juce::Label& metricLabel, juce::ToggleButton* metricButtons, int metricCount,
                                   juce::Label& scannerLabel, juce::ToggleButton* scannerButtons, int scannerCount,
                                   juce::Label& loopLabel, juce::ToggleButton* loopSwitches, int loopCount, int loopColumns,
                                   juce::Label& doorsLabel, juce::ToggleButton* doorButtons, int doorCount,
                                   juce::Label* gainLabel, juce::Slider& feedbackGain,
                                   juce::Label* deriveLabel, juce::Slider& deriveDepth, juce::ToggleButton& deriveButton,
                                   juce::ToggleButton* derivationLayers, int derivationLayerCount,
                                   juce::Label& variationLabel, juce::TextButton& v1, juce::TextButton& v2, juce::TextButton& v3,
                                   juce::TextButton& v4, juce::TextButton& v5, juce::TextButton& v6,
                                   // LEARN's own fixed box - nullptr for callers that don't have
                                   // one (CLONE's own transport column has no LEARN toggle yet,
                                   // see TAREFAS.md). Placed in whatever this column has left
                                   // below FIM DO LOOP, the same "leftover space" pattern LOG
                                   // already uses at the bottom of the mixer column.
                                   juce::Label* learnLabel = nullptr, juce::TextEditor* learnEditor = nullptr)
{
    // A static "MODO: PRINCIPAL"/"MODO: CLONE" caption above CLOCK, so
    // the user can tell which tab they're acting on at a glance (author,
    // live: "definir uma função que escreva na tela... para que o
    // usuário compreenda em qual aba está atuando", then "talvez ao
    // invés de deixar isso no cabeçalho, deixar na coluna da esquerda
    // acima de clock"). Genuinely static per caller, not toggled at
    // runtime - MainComponent's own left column only shows while
    // PRINCIPAL is the active body, and this tab's own transport column
    // is always CLONE's, whether embedded or in its own 2-monitor
    // window - so each caller just passes its own fixed text once.
    modeLabel.setBounds(transport.removeFromTop(16));
    transport.removeFromTop(4);
    // CLOCK's own caption moves inside the knob now too (18 ago. 2026,
    // author, extending the same treatment further: "agora o titulo
    // clock no interior do knob clock (esse pode deixar com o nome
    // completo)" - unlike the oscillator/VCF/ADSR captions capped at 4
    // letters, this one stays "CLOCK" in full). Still removes the same
    // 22px from `transport` as before (as a plain, undrawn spacer) so
    // clockHeight's own budget below is unaffected - clockLabel's real
    // bounds get set after clockKnob, once its final size/position is
    // known.
    transport.removeFromTop(22);
    // 3, not 6 (20 ago. 2026, author: "suba o pulse para mais proximo do
    // clock" - tightened further on top of the 10->6 step from 18 ago.).
    constexpr int spacerKnob = 3;
    // temporalRowH: 2 rows of 8 now, not 1 row of 4 (20 ago. 2026, author:
    // "implementamos mais uma fileira de pulse (oito no total)") - same
    // "compensate via clockCeiling below, don't push the rest of the
    // column down" treatment as MÉTRICA's own 2-row grid got (author,
    // this time pre-emptively: "suba pra cima na coluna e não pra baixo").
    constexpr int temporalLabelH = 12, temporalRowH = 48;
    // Label + slider row, right under the SUBDIVISÃO grid (20 ago. 2026,
    // "precisa colocar o título no slider") - funded by shrinking
    // doorsRowH/deriveButtonH/variationRowH just below (author: "podemos
    // diminuir alguns botões que permanecem altos como os feedbacks
    // ports" / "e os variation"), not by cutting clockCeiling further.
    // grooveRowH: 22, not 18 (20 ago. 2026, author: "deixar o slider na
    // mesma espessura que o FB GAIN" - matches gainRowH exactly, plus the
    // same .reduced(2, 1) margins at the placement site below instead of
    // (4, 2), so the rendered slider itself is pixel-identical in height).
    constexpr int grooveLabelH = 12, grooveRowH = 22;
    // metricRowH: 2 rows of 8 now, not 1 row of 4 (19-20 ago. 2026, author:
    // "em métrica gostaria de ter mais 4 possibilidades de contagem, uma
    // segunda fileira") - the extra height this adds to fixedBelowKnob is
    // compensated by clockHeight's own lowered ceiling just below (20 ago.
    // 2026), not absorbed from LEARN's box - author, live, after the first
    // pass: "voltou a mexer na altura do learn, não quero que o learn
    // mude, volte como ele era".
    constexpr int metricLabelH = 12, metricRowH = 48;
    constexpr int scannerLabelH = 12, scannerRowH = 24;
    constexpr int loopLabelH = 13, loopRowH = 36;
    // 64, not 76 (20 ago. 2026, author: "podemos diminuir alguns botões
    // que permanecem altos como os feedbacks ports" - frees room for the
    // SWING slider below without cutting clockCeiling further). Each row
    // now renders at 26px (64/2=32, reduced(4,3) -> 26), not 32.
    constexpr int doorsLabelH = 12, doorsRowH = 64;
    constexpr int gainLabelH = 12, gainRowH = 22;
    // deriveButtonH: 28, not 34 - after .reduced(2, 1) this renders at
    // 26px tall, matching CAP's own new rendered height (doorsRowH/2=32,
    // reduced(4,3) -> 26) just above - same "match CAP's own button
    // height" intent as before (author, live: "o botão deriva deixar
    // largo e do mesmo comprimento configurado atualmente no botão CAP"),
    // just recomputed against doorsRowH's own smaller number now.
    constexpr int deriveLabelH = 12, deriveDepthH = 22, deriveButtonH = 28;
    // Small gap between DERIVA's own slider and its button - they used to
    // sit flush (just the 2px from each one's own .reduced() margin),
    // which read as glued together (author, live: "o botão deriva tá
    // muito colado ao slider de cima dele").
    constexpr int deriveButtonGap = 6;
    // variationRowH: 28, not 34 - after .reduced(2, 1) this renders at
    // 26px tall, matching PORTAS DE FEEDBACK's own new (smaller) button
    // height (author, live: "deixar os botoes de variação com o mesmo
    // comprimento que os botoes portas de feedback" / "altura"; then,
    // 20 ago. 2026, "podemos diminuir alguns botões que permanecem altos
    // como os feedbacks ports" / "e os variation" - recomputed against
    // doorsRowH's own smaller number, same as deriveButtonH above).
    constexpr int variationLabelH = 15, variationRowH = 28;
    // Breathing room between each distinct group (PULSO/MÉTRICA/PERCURSO/
    // PORTAS DE FEEDBACK/FB GAIN/DERIVA/VARIAÇÃO/FIM DO LOOP - FIM DO LOOP
    // moved to last, see its own comment further down) - these used
    // to sit back-to-back with zero gap, while any leftover height below
    // VARIAÇÃO (once the CLOCK knob hit its own 190px ceiling) just went
    // unused at the column's bottom edge instead of spacing the groups
    // above it (author, live: "espaçar mais os objetos da coluna da
    // esquerda... readeque com base na ergonomia e boa visibilidade").
    // 7 gaps: temporal->metric, metric->scanner, scanner->doors,
    // doors->gain (replacing spacerB), gain->derive, derive->variation,
    // variation->loop. Plus an 8th before LEARN's own box, added later.
    // 4, not 8 (18 ago. 2026, author: "tente aumentar a altura da caixa
    // de learn, suba os objetos da coluna da esquerda em 30px" / "o
    // learn fica onde está, somente um pouco mais longo... para não
    // gerar scroll") - CLOCK sits at its own 190px ceiling on typical
    // window sizes (see the comment above), so the ~32px this frees
    // across all 8 gaps flows straight into LEARN's own leftover space
    // at the column's bottom instead of growing CLOCK further.
    constexpr int groupGap = 4;
    constexpr int fixedBelowKnob = spacerKnob
        + temporalLabelH + temporalRowH + groupGap + grooveLabelH + grooveRowH + groupGap + metricLabelH + metricRowH + groupGap + scannerLabelH + scannerRowH
        + groupGap + loopLabelH + loopRowH + groupGap + doorsLabelH + doorsRowH + groupGap
        + gainLabelH + gainRowH + groupGap + deriveLabelH + deriveDepthH + deriveButtonGap + deriveButtonH
        + groupGap + variationLabelH + variationRowH + variationRowH;
    // 141, not 145 (20 ago. 2026) - recomputed once more now that
    // grooveRowH grew 18->22 to match FB GAIN's own thickness (author:
    // "deixar o slider na mesma espessura que o FB GAIN"), a +4px the
    // doors/derive/variation shrink didn't have spare room for anymore
    // (that surplus was already spent on grooveLabelH). Same reasoning
    // chain as the comments just above it (190 -> 169 -> 145 -> 141),
    // still sized so LEARN's own leftover space at the column's bottom
    // never actually moves - every one of these grid expansions comes out
    // of CLOCK's own ceiling, not out of LEARN.
    constexpr int clockCeiling = 141;
    const auto clockHeight = std::clamp(transport.getHeight() - fixedBelowKnob, 90, clockCeiling);
    auto clockCell = transport.removeFromTop(clockHeight).reduced(16, 0);
    // Squared off (18 ago. 2026, author: "o clock também não está
    // correto... e também o alinhamento, não está centrado") - clockCell
    // itself is taller than it is wide (up to 190 tall, ~158 wide after
    // the reduce above), but the rotary LookAndFeel draws a circle using
    // the SMALLER dimension as diameter, not a true ellipse - so the
    // visible ring only filled the top ~158px of that 190px-tall
    // rectangle, and centring the caption on the full (non-square)
    // rectangle put it well below the ring's own real centre. Forcing
    // clockKnob's own bounds to be square first (min of width/height,
    // centred within clockCell) makes the label's later
    // withSizeKeepingCentre land on the same centre the ring actually
    // draws around.
    const auto clockKnobSize = std::min(clockCell.getWidth(), clockCell.getHeight());
    clockKnob.setBounds(clockCell.withSizeKeepingCentre(clockKnobSize, clockKnobSize));
    clockLabel.setBounds(clockKnob.getBounds().withSizeKeepingCentre(clockKnob.getWidth(), 16));
    clockLabel.toFront(false);
    transport.removeFromTop(spacerKnob);

    auto placeButtonRow = [&] (juce::Label& label, juce::ToggleButton* buttons, int count, int labelH, int rowH)
    {
        label.setBounds(transport.removeFromTop(labelH));
        auto row = transport.removeFromTop(rowH);
        const auto width = row.getWidth() / count;
        for (int i = 0; i < count; ++i) buttons[i].setBounds(row.removeFromLeft(width).reduced(4, 3));
    };
    // 4 columns x 2 rows, not placeButtonRow's single row - both
    // temporalCount and metricCount are 8 now (19-20 ago. 2026, see
    // temporalRowH/metricRowH's own comments above), which placeButtonRow's
    // row.getWidth()/count math would have squeezed into 8 slivers. Same
    // grid technique doorsButtons already uses below (3 columns x 2 rows).
    auto placeButtonGrid = [&] (juce::Label& label, juce::ToggleButton* buttons, int count, int labelH, int rowH)
    {
        label.setBounds(transport.removeFromTop(labelH));
        auto area = transport.removeFromTop(rowH);
        constexpr int columns = 4;
        const auto buttonWidth = area.getWidth() / columns;
        const auto buttonHeight = area.getHeight() / 2;
        for (int i = 0; i < count; ++i)
            buttons[i].setBounds(juce::Rectangle<int>(area.getX() + (i % columns) * buttonWidth,
                                                        area.getY() + (i / columns) * buttonHeight,
                                                        buttonWidth, buttonHeight).reduced(4, 2));
    };
    placeButtonGrid(temporalLabel, temporalButtons, temporalCount, temporalLabelH, temporalRowH);
    transport.removeFromTop(groupGap);
    grooveLabel.setBounds(transport.removeFromTop(grooveLabelH));
    grooveAmount.setBounds(transport.removeFromTop(grooveRowH).reduced(2, 1));
    transport.removeFromTop(groupGap);
    placeButtonGrid(metricLabel, metricButtons, metricCount, metricLabelH, metricRowH);
    transport.removeFromTop(groupGap);
    placeButtonRow(scannerLabel, scannerButtons, scannerCount, scannerLabelH, scannerRowH);
    transport.removeFromTop(groupGap);

    doorsLabel.setBounds(transport.removeFromTop(doorsLabelH));
    auto doorsArea = transport.removeFromTop(doorsRowH);
    const auto doorWidth = doorsArea.getWidth() / 3;
    for (int i = 0; i < doorCount; ++i)
        doorButtons[i].setBounds(juce::Rectangle<int>(doorsArea.getX() + (i % 3) * doorWidth,
                                                        doorsArea.getY() + (i / 3) * (doorsArea.getHeight() / 2),
                                                        doorWidth, doorsArea.getHeight() / 2).reduced(4, 3));
    transport.removeFromTop(groupGap);

    if (gainLabel != nullptr) gainLabel->setBounds(transport.removeFromTop(gainLabelH));
    else transport.removeFromTop(gainLabelH);
    feedbackGain.setBounds(transport.removeFromTop(gainRowH).reduced(2, 1));
    transport.removeFromTop(groupGap);

    if (deriveLabel != nullptr) deriveLabel->setBounds(transport.removeFromTop(deriveLabelH));
    else transport.removeFromTop(deriveLabelH);
    deriveDepth.setBounds(transport.removeFromTop(deriveDepthH).reduced(2, 1));
    transport.removeFromTop(deriveButtonGap);
    // Full column width now, not half next to empty space (author, live:
    // "o botão deriva deixar largo e do mesmo comprimento configurado
    // atualmente no botão CAP") - "mesmo comprimento" is CAP's own row
    // height (doorsRowH / 2, reduced by 3 top+bottom = 32), not its
    // width. Shares that same row now with the 3 A/B/C layer toggles
    // (19 ago. 2026, author: "os do tipo vcf, no mesmo espaço do botão
    // deriva") - DERIVA itself keeps most of the width, A/B/C sit
    // compact on the right, same row height for both.
    auto deriveRow = transport.removeFromTop(deriveButtonH);
    // 32, não mais 26 (20 ago. 2026, autor: "ainda o botão deriva pode
    // ficar menos largo, o botão azul está curto" - "azul" é A/B/C/AUTO,
    // componentID "core" pinta `controlBlueColour`; DERIVA usa
    // componentID "derive", cor diferente). DERIVA não tem largura
    // própria aqui, só recebe o que sobra depois que os 4 toggles
    // tiram a deles primeiro (`removeFromRight` abaixo) - aumentar esta
    // constante dá mais espaço aos 4 toggles E encolhe DERIVA como
    // efeito direto, sem precisar de nenhum corte separado.
    constexpr int derivationLayerWidth = 32;
    // 4º slot (20 ago. 2026, autor: "penso em algo que cada item é
    // autônomo" / "pode fazer... mas sem destruir também o que já temos
    // que é outra configuração possível") - AUTO reaproveita o mesmo
    // array/loop de A/B/C (`derivationLayers[3]`) em vez de crescer a
    // assinatura desta função de novo; ver seu próprio comentário de
    // membro sobre o que o índice 3 significa (bem diferente de 0/1/2 -
    // liga/desliga um MODO inteiro, não um grupo de Motion).
    auto derivationLayerArea = deriveRow.removeFromRight(derivationLayerWidth * derivationLayerCount);
    for (int i = 0; i < derivationLayerCount; ++i)
        derivationLayers[i].setBounds(derivationLayerArea.removeFromLeft(derivationLayerWidth).reduced(2, 1));
    deriveButton.setBounds(deriveRow.reduced(2, 1));
    transport.removeFromTop(groupGap);

    variationLabel.setBounds(transport.removeFromTop(variationLabelH));
    auto variationRow1 = transport.removeFromTop(variationRowH);
    const auto variationThird1 = variationRow1.getWidth() / 3;
    v1.setBounds(variationRow1.removeFromLeft(variationThird1).reduced(2, 1));
    v2.setBounds(variationRow1.removeFromLeft(variationThird1).reduced(2, 1));
    v3.setBounds(variationRow1.reduced(2, 1));
    auto variationRow2 = transport.removeFromTop(variationRowH);
    const auto variationThird2 = variationRow2.getWidth() / 3;
    v4.setBounds(variationRow2.removeFromLeft(variationThird2).reduced(2, 1));
    v5.setBounds(variationRow2.removeFromLeft(variationThird2).reduced(2, 1));
    v6.setBounds(variationRow2.reduced(2, 1));
    transport.removeFromTop(groupGap);

    // FIM DO LOOP moved to the end of the column (18 ago. 2026, author:
    // "o objeto FIM DO LOOP - n ativo, da coluna esquerda, passa a ser
    // último objeto da coluna, pra ficar mais próximo do sequenciador") -
    // used to sit right after PERCURSO/scanner, now sits last so it's
    // physically adjacent to CV (16 STEPS) below/beside this column,
    // which is what it actually controls (the sequencer's own loop
    // length). Same 7-gap total the comment on fixedBelowKnob above
    // already accounts for - just relabelled (scanner->doors instead of
    // scanner->loop, variation->loop instead of derive->variation being
    // the last one), the sum is unchanged.
    loopLabel.setBounds(transport.removeFromTop(loopLabelH));
    auto loopArea = transport.removeFromTop(loopRowH);
    const auto loopButtonWidth = loopArea.getWidth() / loopColumns;
    for (int i = 0; i < loopCount; ++i)
        loopSwitches[i].setBounds(loopArea.getX() + (i % loopColumns) * loopButtonWidth,
                                   loopArea.getY() + (i / loopColumns) * 18, loopButtonWidth, 18);
    // Whatever's left of `transport` below FIM DO LOOP - can be a few
    // pixels or a lot, depending on how close clockHeight's own clamp
    // (90-190) sat to its ceiling on this screen. Author, live: "vamos
    // criar uma caixa tipo no fim da coluna da esquerda, algo como o
    // terminal da coluna da direita" - same shape as LOG (label + small
    // gap + editor), just given whatever room is naturally left instead
    // of a fixed height, matching how LOG itself already gets whatever's
    // left of the mixer column.
    if (learnLabel != nullptr && learnEditor != nullptr && transport.getHeight() > 24)
    {
        transport.removeFromTop(groupGap);
        auto learnArea = transport;
        learnLabel->setBounds(learnArea.removeFromTop(14));
        learnEditor->setBounds(learnArea);
    }
}

// Shared by both tabs' MIXER column: the 4 channel strips (gain/pan/
// reflux + enable/mute/solo) plus the MEMÓRIA MIX snapshot row right
// below them. Added after auditing the two tabs' positions side by side
// and finding this block had drifted too (356px vs 408px channel height,
// 22px vs 18px channel labels, 260 vs 320 gain-fader ceiling) - kept
// PRINCIPAL's own numbers per a live A/B call ("o comprimento dos rails
// do mixer estão melhor no da aba principal"). Returns whatever's left
// of `area` below MEMÓRIA MIX so PRINCIPAL can still place CONEXÃO ENTRE
// OBJETOS and LOG there; CLONE has neither, so it just leaves that space
// unused, same as before.
inline juce::Rectangle<int> layoutMixerChannels(juce::Rectangle<int> area,
    std::array<juce::Label, 4>& mixLabels,
    std::array<juce::ToggleButton, 4>& mixEnable, std::array<juce::ToggleButton, 4>& mixMute, std::array<juce::ToggleButton, 4>& mixSolo,
    std::array<juce::Slider, 4>& mixGain, std::array<juce::Slider, 4>& mixPan, std::array<juce::Slider, 4>& mixReflux,
    juce::Label& mixMemoryLabel, std::array<juce::TextButton, 4>& mixMemorySlots, juce::TextButton& mixMemoryCapture)
{
    // 350, not 356 - REFLUX's own row shrank by the same 6px below, so
    // the strip's rendered bottom (and MEMÓRIA MIX right after it) moved
    // up to match, instead of leaving 6px of new dead space (author,
    // live: "tente aproximar um pouco mais o memória mix do mixer").
    constexpr int channelsHeight = 350;
    auto channelsArea = area.removeFromTop(channelsHeight);
    const auto stripWidth = channelsArea.getWidth() / static_cast<int>(mixGain.size());
    for (std::size_t i = 0; i < mixGain.size(); ++i)
    {
        auto strip = juce::Rectangle<int>(channelsArea.getX() + static_cast<int>(i) * stripWidth,
                                           channelsArea.getY(), stripWidth, channelsArea.getHeight()).reduced(4, 1);
        mixLabels[i].setBounds(strip.removeFromTop(22));
        auto buttons = strip.removeFromTop(24);
        const auto enableWidth = static_cast<int>(buttons.getWidth() * 0.44f);
        const auto toggleWidth = (buttons.getWidth() - enableWidth) / 2;
        mixEnable[i].setBounds(buttons.removeFromLeft(enableWidth).reduced(1));
        mixMute[i].setBounds(buttons.removeFromLeft(toggleWidth).reduced(1));
        mixSolo[i].setBounds(buttons.reduced(1));
        const auto gainHeight = std::clamp(strip.getHeight() - 64, 120, 260);
        mixGain[i].setBounds(strip.removeFromTop(gainHeight).reduced(7, 1));
        mixPan[i].setBounds(strip.removeFromTop(24).reduced(2, 1));
        mixReflux[i].setBounds(strip.removeFromTop(18).reduced(2, 1));
    }
    // Flush against the channels (0 gap): reads as one unit with the
    // channels it snapshots, not a separate module.
    auto memoryRow = area.removeFromTop(33);
    mixMemoryLabel.setBounds(memoryRow.removeFromTop(13));
    const auto memoryButtonWidth = memoryRow.getWidth() / static_cast<int>(mixMemorySlots.size() + 1);
    for (auto& slot : mixMemorySlots) slot.setBounds(memoryRow.removeFromLeft(memoryButtonWidth).reduced(2, 1));
    mixMemoryCapture.setBounds(memoryRow.reduced(2, 1));
    return area;
}

// Shared by both tabs' central "voice" block: title + CMOS core selector
// + 5 oscillators + VCF + ADSR + ENERGIA/NOISE - everything between the
// clock and mixer columns, above the FORMA LFO/MODULAÇÃO rails band.
// Added for the same reason as layoutMixerChannels()/layoutRailsBand()/
// layoutTransportColumn(): this was the block that had drifted the most
// of all - CLONE fixed it at a flat 680px-wide/360px-tall box (leaving
// ~136px of unused width between NOISE and the mixer column, never
// reaching it), while PRINCIPAL always ran fluid and flush against
// whatever's on either side. PRINCIPAL's version won (zero dead space,
// scales with the real available area) - CLONE's own fixed numbers are
// gone, not just matched.
//
// Also fixes a real alignment bug both tabs shared: ADSR's own 2-row
// grid never lined up with VCF's 3-row one next to it, despite reading
// as one continuous row of modules (author, live: "a linha de cima de
// knobs do adsr deve estar alinhada com o knob de cima do vcf, para a
// segunda fileira, alinhar com o knob do meio do vcf"). Fixed by giving
// ADSR the same 36px header VCF has (16px label + 20px mode button,
// even though ADSR has no mode button of its own - the 20px is spent as
// a plain spacer) and the same per-row pitch (filterKnobHeight) instead
// of ADSR dividing its own height into 2 independently of VCF's 3 - so
// ATT/DEC now shares FREQ's row and SUS/REL shares RES's, leaving VCF's
// third row (CV) as the only one without an ADSR counterpart. ENERGIA
// joins the same alignment (author, live: "alinhar energia com o a
// primeiro knob do vcf") via the same header-height trick. And every
// knob in this function (oscillators included) now sits a fixed
// knobLabelGap below its own caption instead of centred in whatever
// height its row happens to have - centring let VCF/ADSR's much taller
// rows read as a visibly bigger caption-to-knob gap than the
// oscillators' tighter ones even though both used "the same" 13px
// caption height (author, live: "precisa manter a mesma distancia entre
// o titulo do knob e o knob... mudar em vcf e adsr").
inline void layoutVoiceArea(juce::Rectangle<int> area,
    juce::Label& voiceLabel,
    std::array<juce::ToggleButton, 3>& coreSwitches,
    std::array<juce::Label, 5>& oscillatorLabels,
    std::array<juce::Label, 5>& rateLabels, std::array<juce::Slider, 5>& rates,
    std::array<juce::Label, 5>& levelLabels, std::array<juce::Slider, 5>& levels,
    std::array<juce::Label, 5>& shapeLabels, std::array<juce::Slider, 5>& shapeControls,
    std::array<juce::Label, 5>& panCaptions, std::array<juce::Slider, 5>& pans,
    std::array<juce::Label, 5>& proximityCaptions, std::array<juce::Slider, 5>& proximities,
    std::array<juce::Label, 5>& orbitCaptions, std::array<juce::Slider, 5>& orbits,
    juce::Label& filterLabel, std::array<juce::TextButton, 4>& filterModeButtons, std::array<juce::Label, 3>& filterControlLabels,
    juce::Slider& filterCutoff, juce::Slider& filterResonance, juce::Slider& filterDepth,
    juce::Label& materialFilterLabel, juce::Slider& materialFilterMix,
    juce::Label& envelopeLabel, std::array<juce::Label, 4>& envelopeControlLabels, std::array<juce::Slider, 4>& envelopeControls,
    juce::Label& energyLabel, juce::Slider& energy,
    juce::Label& noiseLabel, NoiseSelector& noiseSelector,
    juce::Label& modulationLabel, std::array<juce::Label, 3>& modulationLabels, std::array<juce::Slider, 3>& modulationControls,
    juce::Label& lfoShapeLabel, juce::ToggleButton* lfoShapeButtons, juce::ToggleButton& lfoFreeze,
    juce::Rectangle<int>& chaosFreezeHighlight)
{
    // Caption height and the fixed gap below it are shared by every knob
    // in this function - oscillators, VCF, ADSR - so "distance between
    // the knob's title and the knob" reads as one constant, not three
    // slightly different ones.
    constexpr int knobCaptionHeight = 13;
    constexpr int knobLabelGap = 8;
    // VCF's own label+mode-button header - reused as a plain spacer by
    // ADSR (no mode button of its own) and ENERGIA (no per-knob caption
    // of its own), so all three's first knob row starts at the exact
    // same Y as VCF's FREQ row. 16 (filterLabel) + 27 (filterMode's own
    // row, matching the core selector buttons' own height below).
    constexpr int moduleHeaderHeight = 43;
    constexpr int adsrKnobSize = 89;
    // A small helper so every knob placement below (oscillators, VCF,
    // ADSR, ENERGIA) shares the exact same caption-then-fixed-gap-then-
    // knob logic, anchored to the cell's top instead of centred in it.
    auto placeKnob = [] (juce::Rectangle<int> cell, int knobSize)
    {
        cell.removeFromTop(knobLabelGap);
        return cell.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize);
    };

    // Hoisted above the energyNoiseArea/envelopeArea/filterArea split (18
    // ago. 2026, author: "o segundo e o terceiro knobs do vcf não estão
    // alinhados com os do oscilador") - aligning just the first row's own
    // Y offset (see moduleHeaderHeight's own comment/the 24px spacer
    // below) wasn't enough, because filterKnobHeight and the oscillators'
    // own per-row height were still two DIFFERENT computed values, so
    // rows 2/3 drifted apart even with row 1 matching. area.getHeight()
    // here is identical to what the oscillator loop's own formula reads
    // much further down (area.removeFromRight() below doesn't touch
    // height, and area.removeFromTop() for voiceLabel/coreArea/
    // oscillatorLabelHeight hasn't happened yet at this point either) -
    // 157 = 22 (voiceLabel) + 27 (coreArea) + 18 (oscillatorLabelHeight)
    // + 45*2 (oscillatorPanRowHeight, EIXO Y/Z). Single source of truth
    // now instead of two formulas that happened to match only by
    // (partial) coincidence - the oscillator loop below reuses this same
    // value instead of recomputing it.
    const auto sharedKnobRowHeight = std::clamp((area.getHeight() - 157) / 3, 96, 150);

    auto energyNoiseArea = area.removeFromRight(190);
    auto envelopeArea = area.removeFromRight(190);
    auto filterArea = area.removeFromRight(140);

    filterLabel.setBounds(filterArea.removeFromTop(16));
    // 27, matching the core selector buttons' (40106/8038/4069UB) own
    // row height (author, live: "deixar o botão do vcf com mesmo tamanho
    // que o botão 8038") - the 16px horizontal reduce already lands the
    // same 108px width those get once their own 112px cap kicks in, only
    // the height (was 20) needed to change.
    // Four independent toggles now (17 ago. 2026, author: "dois ou mais"),
    // not one cycling button - same header row/height as before, just
    // divided 4 ways instead of holding a single label.
    auto filterModeRow = filterArea.removeFromTop(27).reduced(2, 1);
    const auto filterModeButtonWidth = filterModeRow.getWidth() / 4;
    for (auto& button : filterModeButtons)
        button.setBounds(filterModeRow.removeFromLeft(filterModeButtonWidth).reduced(2, 0));
    // 24px spacer so FREQ/RES/CV start at the same Y as the oscillators'
    // own FREQ/MIX/FORM row (18 ago. 2026, author: "agora podemos alinhar
    // os knobs do vcf com os dos osciladores") - filterArea and the
    // oscillator area both split off the same shared `area` before either
    // one's own header is removed, so their headers had to match exactly
    // for the knob rows below to land on the same Y, and they didn't:
    // filterLabel+filterModeRow totals 16+27=43px, but voiceLabel+coreArea
    // (5 OSC's own title+core-selector row, consumed from the oscillator
    // area before oscillatorLabels[i]'s own per-column "OSC A" header)
    // totals 22+27+18=67px - a 24px gap this spacer closes.
    filterArea.removeFromTop(24);
    // MaterialFilter's MIX row - carved from the bottom of filterArea
    // BEFORE the /3 division below, so FREQ/RES/CV (and ADSR, which reuses
    // filterKnobHeight for its own rows - see this function's own comment)
    // shrink together proportionally instead of this row displacing them.
    // Caption above, slider below - not side by side - matching every
    // other knob/slider in this function (oscillators, VCF, ADSR): title
    // on top, control below, same rhythm as the oscillators' own EIXO Y/Z
    // row this height is borrowed from (author, live, 17 ago. 2026:
    // "o titulo deve ficar acima alinhado com eixo z do oscilador").
    // 45, not a fourth full knob row: a real screenshot measured only
    // ~31px of slack per existing 141px VCF row (110px is the real
    // minimum: 13 caption + 8 gap + 89 knob), nowhere near enough for a
    // fourth adsrKnobSize knob - but plenty for this shorter row.
    // 45 matches oscillatorPanRowHeight below (declared later in this same
    // function, in the oscillator loop) - not referenced directly since
    // C++ won't let this section forward-reference it.
    auto materialFilterRow = filterArea.removeFromBottom(45);
    materialFilterLabel.setBounds(materialFilterRow.removeFromTop(knobCaptionHeight));
    // Same thickness as the oscillators' own EIXO Y/Z sliders (author,
    // live, 17 ago. 2026: "deixe ele com a mesma espessura do eixo Y dos
    // osciladores") - see proximities[i]'s own bounds a few dozen lines up
    // in this same function for the source of this exact 28/2/1 recipe.
    materialFilterMix.setBounds(materialFilterRow.withSizeKeepingCentre(materialFilterRow.getWidth(), std::min(28, materialFilterRow.getHeight())).reduced(2, 1));
    // Was filterArea.getHeight()/3 - its own independent computation,
    // which is exactly why rows 2/3 drifted from the oscillators' own
    // rows even after row 1 was aligned (see sharedKnobRowHeight's own
    // comment above). Any leftover slack between CV and MAT below is
    // deliberate now, not absorbed into the row height - the author
    // asked for this specifically without moving MAT: "faça isso sem
    // alterar a posição do slider MAT".
    const auto filterKnobHeight = sharedKnobRowHeight;
    // Test (18 ago. 2026, author: "faça um teste agora de passar os
    // titulos do knobs do vcf para o centro no interior do knob") - caption
    // moved from its own row above the knob to sitting inside the knob's
    // own hollow centre instead, so the knob keeps the cell's full height
    // (no longer shares it with a separate caption row). toFront(false) is
    // needed because these labels were added to the component tree before
    // the knobs (see the constructor a few hundred lines up) - without it
    // the knob's own painting would sit on top of the caption text, hiding
    // it entirely, since z-order here is add-order, not draw-order-by-
    // position.
    for (std::size_t i = 0; i < filterControlLabels.size(); ++i)
    {
        auto cell = filterArea.removeFromTop(filterKnobHeight);
        auto& knob = (i == 0 ? filterCutoff : (i == 1 ? filterResonance : filterDepth));
        knob.setBounds(placeKnob(cell, adsrKnobSize));
        filterControlLabels[i].setBounds(knob.getBounds().withSizeKeepingCentre(knob.getWidth(), knobCaptionHeight));
        filterControlLabels[i].toFront(false);
    }

    envelopeLabel.setBounds(envelopeArea.removeFromTop(16));
    // Matches VCF's own label+mode-button header (moduleHeaderHeight) so
    // ADSR's two rows start at the same Y as VCF's - see the function
    // comment above.
    envelopeArea.removeFromTop(moduleHeaderHeight - 16);
    // Same "caption inside the knob" treatment as VCF above (18 ago.
    // 2026, author, after seeing the VCF test: "gostei nos titulos dos
    // knobs no centro, vai ajudar a respirar melhor o painel" -> "só
    // ADSR por enquanto" when asked how far to extend it, since ADSR
    // shares VCF's own adsrKnobSize rather than the oscillators' smaller
    // knobs).
    // Knob raised 10px above its own placeKnob() position (18 ago. 2026,
    // author: "preciso que eles fiquem um pouco mais altos do que
    // estão"). First tried keeping the caption at its original,
    // un-raised position per the request's own wording ("não altere a
    // posição do titulo") - the author flagged the result as
    // decentred ("os titulos dos knobs adsr voltaram a ficar
    // descentralizados com o knob") and chose, when asked, to have the
    // caption follow the knob and stay centred instead. So both move
    // together now: knob raised first, caption computed from ITS final
    // (already-raised) bounds, same recipe as everywhere else.
    constexpr int adsrKnobRise = 10;
    for (std::size_t i = 0; i < envelopeControls.size(); ++i)
    {
        auto cell = juce::Rectangle<int>(envelopeArea.getX() + static_cast<int>(i % 2) * envelopeArea.getWidth() / 2,
                                          envelopeArea.getY() + static_cast<int>(i / 2) * filterKnobHeight,
                                          envelopeArea.getWidth() / 2, filterKnobHeight).reduced(3, 0);
        const auto knobBounds = placeKnob(cell, adsrKnobSize).translated(0, -adsrKnobRise);
        envelopeControls[i].setBounds(knobBounds);
        envelopeControlLabels[i].setBounds(knobBounds.withSizeKeepingCentre(knobBounds.getWidth(), knobCaptionHeight));
        envelopeControlLabels[i].toFront(false);
    }
    // ADSR only fills 2 of the 3 rows filterKnobHeight budgets (4 knobs,
    // 2 per row) - the loop above builds each cell manually from
    // envelopeArea's own X/Y rather than consuming it, so it's still the
    // full rect here; removing what the loop actually used leaves exactly
    // the empty third row real screenshots showed (17-18 ago. 2026).
    envelopeArea.removeFromTop(filterKnobHeight * 2);
    // MODULAÇÃO (RATE/RING/NOISE MIX) moved here from the rails band
    // (18 ago. 2026, author's own idea, live: "já tinha pensando em uma
    // possibilidade de mudar o slider do MODULAÇÃO LFO para um knob...
    // sobrará espaço para 7 colunas... passaremos a coluna FORMA LFO...
    // e o novo knob modulação de lfo para o espaço vazio que está abaixo
    // do adsr") - frees the rails band's FORMA LFO column and MODULAÇÃO
    // column entirely, both of which were fighting the same rails band
    // every other module already fights for a spot in. RATE became a
    // knob rather than staying a slim slider - author: "um knob... acho
    // esse botão funciona como algo mais performático"; RING/NOISE MIX
    // followed as vertical sliders instead of horizontal - author:
    // "deixar os sliders RING e NOISE MIX na vertical" - matching the
    // MIXER column's own vertical faders instead of ROTAS ATIVAS' slim
    // horizontal idiom, since they no longer live in that rail. Revised
    // again the same day (author, live, after seeing the first pass):
    // LFO and NOISE MIX are both knobs now, adsrKnobSize like ADSR's own
    // ("do mesmo tamanho que o knob do adsr"), sitting directly above
    // ADSR's own two columns - splitting this leftover rect's width in
    // half lands exactly on ADSR's own column X positions, since neither
    // this rect's X nor width changed since ADSR itself used it above.
    // RING went back to a horizontal slider, now below both knobs
    // ("o slider ring volta a ser horizontal e ficará abaixo"). Header
    // grew to match VCF/ADSR's own size ("titulo MODULAÇÃO fica grande,
    // do mesmo tamanho que VCF - MULTIMODO ADSR" - 16px, not the smaller
    // rail-heading height this used before it lived in the rails band).
    modulationLabel.setBounds(envelopeArea.removeFromTop(16));
    // RING now matches MaterialFilter's own MIX row exactly (18 ago.
    // 2026, author: "slider RING mesma espessura de MAT; titulo de RING
    // fica acima do slider como em MAT (mesma cor e posição -
    // centrado)") - same 45px row, same 28px slider thickness, same
    // caption-above-centred recipe (see materialFilterRow above).
    auto modulationRingRow = envelopeArea.removeFromBottom(45);
    // 8px spacer so LFO/NOISE SEND land on the same row as VCF's own
    // bottom knob, CV (18 ago. 2026, author: "alinhe os knobs da
    // modulação (lfo e noise send) com o ultimo (inferior) knob do
    // vcf") - filterArea's own header (filterLabel+filterModeRow+the
    // 24px alignment spacer above) totals 67px before CV's row-cell
    // starts, while envelopeArea's own header to reach this point
    // (envelopeLabel+moduleHeaderHeight-16 spacer+modulationLabel) totals
    // only 59px - both consume the same 2*filterKnobHeight for the rows
    // above (ADSR's two rows here, VCF's FREQ/RES rows there), so that
    // part cancels and this 8px is the entire remaining gap.
    envelopeArea.removeFromTop(8);
    {
        // Captions moved inside the knob (18 ago. 2026, author: "agora
        // insira os titulos nos knobs de modulação (LFO e NOISE SEND)") -
        // same treatment as VCF/ADSR/oscillators/ENERGIA above: the knob
        // keeps the cell's full height instead of sharing it with a
        // separate caption row, caption centred inside the knob's own
        // bounds afterwards, toFront(false) since these labels were added
        // to the tree before the knobs (see the constructor). "NOISE SEND"
        // doesn't fit on one line at this knob width, so its text carries
        // an explicit "\n" (see modulationNames below) - author: "em noise
        // send deixe em duas linhas" - and its caption box is twice
        // knobCaptionHeight tall to hold both lines.
        auto lfoCell = envelopeArea.removeFromLeft(envelopeArea.getWidth() / 2).reduced(3, 0);
        auto noiseCell = envelopeArea.reduced(3, 0);
        modulationControls[0].setBounds(placeKnob(lfoCell, adsrKnobSize));
        modulationLabels[0].setBounds(modulationControls[0].getBounds().withSizeKeepingCentre(modulationControls[0].getWidth(), knobCaptionHeight));
        modulationLabels[0].toFront(false);
        modulationControls[2].setBounds(placeKnob(noiseCell, adsrKnobSize));
        modulationLabels[2].setBounds(modulationControls[2].getBounds().withSizeKeepingCentre(modulationControls[2].getWidth(), knobCaptionHeight * 2));
        modulationLabels[2].toFront(false);
    }
    modulationLabels[1].setBounds(modulationRingRow.removeFromTop(knobCaptionHeight));
    // Same width as MAT too, not just the same thickness (18 ago. 2026,
    // author: "agora o slider ring está muito largo deixe da mesma
    // largura que MAT") - MAT's own row is exactly the VCF column's
    // width (140px, see filterArea above), while this row spans the
    // whole ADSR column (190px) it lives in; capping the slider's own
    // width to 140 and centring it in the wider row matches MAT without
    // needing to narrow the row itself.
    modulationControls[1].setBounds(modulationRingRow.withSizeKeepingCentre(std::min(140, modulationRingRow.getWidth()), std::min(28, modulationRingRow.getHeight())).reduced(2, 1));

    // No longer tied to VCF's own header (18 ago. 2026, author: "energia
    // está alinhado com o titulo do vcf, precisa remover esse vinculo") -
    // used to deliberately reuse moduleHeaderHeight so ENERGIA's title
    // lined up with VCF's own FREQ caption (see TAREFAS.md, that request
    // is still real history), but VCF's own first knob row moved 24px
    // further down since then (the sharedKnobRowHeight alignment with the
    // oscillators), and ENERGIA staying pinned to the old moduleHeaderHeight
    // value meant it silently rode along with whatever VCF's header
    // happens to be instead of being its own independent choice. Same
    // literal value (43) as before, so nothing visually moves here right
    // now - just no longer a reference to VCF's own constant, so a future
    // change to VCF's header can't drag this out of place again.
    // 23, not 43 (18 ago. 2026, author: "agora o knob energia e o objeto
    // noise devem subir um pouco algo en torno de 10px", then "suba mais
    // 10px") - ENERGIA/NOISE (and FORMA LFO below them, chained through
    // the same energyNoiseArea cursor) all move up together as a block,
    // 20px total now. FORMA LFO's own height (`lfoArea`, further down)
    // isn't anchored to anything fixed below it - it's rebuilt from
    // whatever's left of energyNoiseArea's own height at that point
    // (`.withHeight(energyNoiseArea.getHeight())`), and energyNoiseArea's
    // own TOTAL height is fixed independently of this spacer (matches
    // the voice area's own height, same as VCF/ADSR/oscillators) - so
    // shrinking this leading spacer only grows the leftover space FORMA
    // LFO's rows get, it can't clip or collide with anything below this
    // column (PARÂMETROS is a separate row below the whole voice area,
    // not chained from this column at all).
    energyNoiseArea.removeFromTop(23);
    // Caption moved inside the knob too (18 ago. 2026, author: "Knob
    // energia também inserir o título no knob... ou do knob ou da
    // quantidade de letras" when asked how to fit it). Was a hand-rolled
    // 110x110 withSizeKeepingCentre - after the caption still looked off-
    // centre at that size, temporarily dropped to adsrKnobSize (89) to
    // test against the oscillators' own exact configuration (author:
    // "não que utilize a mesma configuração de um knob do oscilador...").
    // That test isolated the real cause to setTextBoxStyle, not size (see
    // the dedicated comment on energy.setTextBoxStyle in this same
    // constructor a few dozen lines up) - once NoTextBox was applied
    // here too, size stopped mattering for centring at all, so the knob
    // came back up to a size closer to its original 110 (author: "aumente
    // o um pouco tamanho do knob energia como era antigamente") - still
    // using the same placeKnob() mechanism as the oscillators, just with
    // its own size instead of reusing adsrKnobSize literally.
    constexpr int energyKnobSize = 110;
    auto energyCell = energyNoiseArea.removeFromTop(knobCaptionHeight + knobLabelGap + energyKnobSize);
    energy.setBounds(placeKnob(energyCell, energyKnobSize));
    energyLabel.setBounds(energy.getBounds().withSizeKeepingCentre(energy.getWidth(), knobCaptionHeight));
    energyLabel.toFront(false);
    // 4, not 14 (18 ago. 2026, author: "agora somente o objeto noise
    // suba 10 px") - only this gap shrinks, so NOISE (and FORMA LFO,
    // chained after it) move up 10px on their own without dragging
    // ENERGIA along too.
    energyNoiseArea.removeFromTop(4);
    noiseLabel.setBounds(energyNoiseArea.removeFromTop(15));
    // Same title-to-control distance ENERGIA's own label/knob use
    // above (author, live: "calcule essa distancia entre titulo do
    // energia e knob energia, e aplique essa mesma distancia entre o
    // titulo do noise e knob noise") - 3, not 5: noiseSelector's own
    // .reduced(4, 2) below already eats 2px off its top, so 3+2=5
    // matches ENERGIA's own label-to-knob gap exactly.
    energyNoiseArea.removeFromTop(3);
    const auto noiseSelectorBounds = energyNoiseArea.removeFromTop(130).reduced(4, 2);
    noiseSelector.setBounds(noiseSelectorBounds);

    // FORMA LFO moved here from the rails band (18 ago. 2026, see
    // MODULAÇÃO's own comment above for the full story), then revised
    // again the same day into 3 columns instead of 2 (author, live):
    // SEN/TRI/PUL | CAOS/VAGA/STEP | FRZ - FRZ narrower ("botão menor"),
    // sitting to the right of column 2, vertically centred on the seam
    // between CAOS and VAGA ("centrado entre os dois botões CAOS/
    // VAGA") rather than spanning the full width as its own footer row.
    // Constrained to NOISE's own measured width, not the raw leftover
    // column - found live this block was overshooting NOISE's own right
    // edge by ~48px even while staying inside the column bounds (NOISE's
    // hex layout doesn't reach the column's own edges) - author: "todos
    // esse botões da FORMA LFO não devem ultrapassar a largura do
    // objeto NOISE que está acima".
    // Real bug found and fixed (18 ago. 2026, author: "quanto mais o
    // objeto noise sobe mais estica o objeto forma LFO, verifique e
    // separe-os") - this block used to take its height from whatever was
    // LEFT OVER in energyNoiseArea at this point
    // (`.withHeight(energyNoiseArea.getHeight())`), so every time a
    // spacer further up (ENERGIA's or NOISE's own) shrank, this grew
    // taller instead of just moving - not what any of those requests
    // asked for. Now a fixed height, independent of NOISE's position, so
    // FORMA LFO can never stretch again regardless of what happens above
    // it. Also a real gap now (12px) instead of sitting flush against
    // NOISE's own selector - author: "minha intenção é afastar o objeto
    // noise do forma lfo".
    energyNoiseArea.removeFromTop(12);
    constexpr int lfoAreaHeight = 113;
    auto lfoArea = noiseSelectorBounds.withY(energyNoiseArea.getY()).withHeight(lfoAreaHeight);
    lfoShapeLabel.setBounds(lfoArea.removeFromTop(14));
    constexpr int frzStripWidth = 34;
    auto frzStrip = lfoArea.removeFromRight(frzStripWidth);
    auto lfoColumn2 = lfoArea.removeFromRight(lfoArea.getWidth() / 2);
    // The extra 6px gap tried here (author: "distanciar ligeiramente os
    // botoes da coluna A da coluna B") was a fallback for the border-
    // stroked highlight looking too wide - once the border itself was
    // removed instead (see chaosFreezeHighlight's own comment below),
    // the author asked to revert this back to no extra gap: "agora que
    // tirou o contorno ficou melhor, pode voltar a mesma distancia da
    // coluna a para a coluna b".
    auto& lfoColumn1 = lfoArea;
    const auto lfoRowHeight = lfoColumn1.getHeight() / 3;
    for (int i = 0; i < 3; ++i)
        lfoShapeButtons[i].setBounds(lfoColumn1.removeFromTop(lfoRowHeight).reduced(4, 3));
    for (int i = 0; i < 3; ++i)
        lfoShapeButtons[3 + i].setBounds(lfoColumn2.removeFromTop(lfoRowHeight).reduced(4, 3));
    // Centred on the CAOS/VAGA seam: row 0 (CAOS) occupies
    // [top, top+lfoRowHeight), row 1 (VAGA) occupies the next
    // lfoRowHeight down, so their shared edge is exactly one row height
    // below frzStrip's own top.
    const auto frzHeight = std::min(28, lfoRowHeight - 6);
    lfoFreeze.setBounds(frzStrip.withY(frzStrip.getY() + lfoRowHeight - frzHeight / 2)
                                 .withHeight(frzHeight).reduced(2, 0));
    // A didactic backing panel behind CAOS/VAGA/FRZ only - not STEP
    // (author: "o fundo não precisa abarcar o STEP somente os 3
    // botões"), margin tightened to sit close to the buttons themselves
    // (author: "deixar a margem mais rente ao botões") rather than the
    // wider padding the first pass used. Built from the buttons' own
    // final bounds (CAOS/VAGA are lfoShapeButtons[3]/[4] - indices 3+i
    // for i in 0,1 of column 2 above) plus FRZ's, not the raw cells, so
    // it hugs exactly what's drawn on top of it. Actual painting happens
    // in this component's own paint() override, after its own
    // background fill (see that function's comment for why the order
    // matters).
    // Margin corrected back down (18 ago. 2026, author, after the first
    // attempt overshot): "ficou mais largo o fundo, não é isso... preciso
    // que o fundo fique mais rente do botão, menos margem" - much
    // tighter than the (10,9,9,4) asymmetric pass, closer to a thin
    // visible border than a spacious frame.
    {
        const auto core = lfoShapeButtons[3].getBounds()
                               .getUnion(lfoShapeButtons[4].getBounds())
                               .getUnion(lfoFreeze.getBounds());
        constexpr int leftPad = 3, topPad = 3, bottomPad = 3, rightPad = 3;
        chaosFreezeHighlight = core.withX(core.getX() - leftPad)
                                    .withWidth(core.getWidth() + leftPad + rightPad)
                                    .withY(core.getY() - topPad)
                                    .withHeight(core.getHeight() + topPad + bottomPad);
    }

    voiceLabel.setBounds(area.removeFromTop(22));
    auto coreArea = area.removeFromTop(27);
    const auto coreButtonWidth = std::min(112, coreArea.getWidth() / static_cast<int>(coreSwitches.size()));
    for (auto& control : coreSwitches) control.setBounds(coreArea.removeFromLeft(coreButtonWidth).reduced(2, 1));

    const auto oscillatorWidth = area.getWidth() / static_cast<int>(levels.size());
    // Single-line label now ("OSC A", or "OSC 4 - SUB/DIV" for 4/5) -
    // dropped the second "FREQ / MIX / FORMA" line (author, live: "nos
    // titulos dos osciladores vamos usar somente uma linha").
    constexpr int oscillatorLabelHeight = 18;
    // EIXO X/Y/Z are thin sliders, not knobs - a fixed, smaller row height
    // for just those two rows frees real space back to FREQ/MIX/FORMA.
    constexpr int oscillatorPanRowHeight = 45;
    // Reuses sharedKnobRowHeight (computed once, before this area was even
    // split from filterArea/envelopeArea/energyNoiseArea - see its own
    // comment) instead of recomputing the same formula here - the two used
    // to be independent expressions that only happened to agree in value,
    // which is exactly why VCF's own rows drifted from these once VCF
    // needed its own header/spacer accounted for differently.
    const auto oscillatorRowHeight = sharedKnobRowHeight;
    // Was its own smaller constant (80) - now shares VCF/ADSR's own
    // adsrKnobSize (18 ago. 2026, author, extending the "caption inside
    // the knob" treatment here too: "porém nos osciladores os knobs
    // passam a ter o mesmo tamanho que os do vcf"). Row height (96-150,
    // computed above) already comfortably clears adsrKnobSize (89) even
    // at its tightest, so this needed no other budget change.
    for (std::size_t i = 0; i < levels.size(); ++i)
    {
        auto osc = area.removeFromLeft(oscillatorWidth).reduced(3, 0);
        oscillatorLabels[i].setBounds(osc.removeFromTop(oscillatorLabelHeight));
        auto rateArea = osc.removeFromTop(oscillatorRowHeight);
        rates[i].setBounds(placeKnob(rateArea, adsrKnobSize));
        rateLabels[i].setBounds(rates[i].getBounds().withSizeKeepingCentre(rates[i].getWidth(), knobCaptionHeight));
        rateLabels[i].toFront(false);
        auto levelArea = osc.removeFromTop(oscillatorRowHeight);
        levels[i].setBounds(placeKnob(levelArea, adsrKnobSize));
        levelLabels[i].setBounds(levels[i].getBounds().withSizeKeepingCentre(levels[i].getWidth(), knobCaptionHeight));
        levelLabels[i].toFront(false);
        auto shapeArea = osc.removeFromTop(oscillatorRowHeight);
        shapeControls[i].setBounds(placeKnob(shapeArea, adsrKnobSize));
        shapeLabels[i].setBounds(shapeControls[i].getBounds().withSizeKeepingCentre(shapeControls[i].getWidth(), knobCaptionHeight));
        shapeLabels[i].toFront(false);
        auto panArea = osc.removeFromTop(oscillatorPanRowHeight);
        panCaptions[i].setBounds(panArea.removeFromTop(knobCaptionHeight));
        const auto panWidth = std::min(130, panArea.getWidth());
        pans[i].setBounds(panArea.withSizeKeepingCentre(panWidth, std::min(28, panArea.getHeight())).reduced(3, 1));
        auto yzArea = osc.removeFromTop(oscillatorPanRowHeight);
        auto yArea = yzArea.removeFromLeft(yzArea.getWidth() / 2);
        proximityCaptions[i].setBounds(yArea.removeFromTop(knobCaptionHeight));
        proximities[i].setBounds(yArea.withSizeKeepingCentre(std::min(90, yArea.getWidth()), std::min(28, yArea.getHeight())).reduced(2, 1));
        orbitCaptions[i].setBounds(yzArea.removeFromTop(knobCaptionHeight));
        orbits[i].setBounds(yzArea.withSizeKeepingCentre(std::min(90, yzArea.getWidth()), std::min(28, yzArea.getHeight())).reduced(2, 1));
    }
}

// OBJETO 5 panel: DualObjectEngine already models two full SimpleSequencer
// instances ("object1", played by the main window, and "object5" here)
// cross-connected through configurable feedback routes - this window is a
// deliberately compact voice for object5 (oscillators/filter/envelope/
// energy) plus the connection controls between the two, not a full mirror
// of every control the main window has (mixer, noise, LFO, effects and
// per-step CV editing for object5 remain future work - see TAREFAS.md).
class ObjectFiveComponent final : public juce::Component, private juce::Timer, private juce::FocusChangeListener
{
public:
    // embeddedInMainWindow: true when this is MainComponent's clonePanel
    // (setBounds() to the exact same `area` layoutUnified() itself
    // received) - in that case this component must draw nothing of its
    // own and consume zero extra inset, or its modules land a border's
    // width off from PRINCIPAL's (a real bug: this used to always draw
    // its own background/border/trace-lines and start from
    // getLocalBounds().reduced(20), stacking on top of the identical
    // treatment MainComponent::paint() already provides once for the
    // whole window - not just a duplicate frame, an actual ~20px
    // position mismatch between the two tabs' otherwise-identical
    // layouts). Standalone use (ObjectFiveWindow/ZoomableObjectFiveViewport,
    // a real independent top-level window with no outer frame of its own)
    // still wants its own background/border/heading, so this defaults
    // to false and only clonePanel's construction passes true.
    explicit ObjectFiveComponent(antitotem::DualObjectEngine& engineToUse, bool embeddedInMainWindow = false,
                                  antitotem::ui::Language initialLanguage = antitotem::ui::Language::english)
        : dualEngine(engineToUse), fifth(engineToUse.object5()), embedded(embeddedInMainWindow), language(initialLanguage)
    {
        configureLabel(heading, antitotem::ui::text(antitotem::ui::label::cloneHeading, language), 15.0f, cloneMaterial::board);
        heading.setVisible(! embedded);
        addAndMakeVisible(heading);

        // Same title the main tab shows above its own oscillators - added
        // here for true parity via layoutVoiceArea().
        configureLabel(voiceLabel, antitotem::ui::text(antitotem::ui::label::oscHeaderTitle, language), 15.0f, juce::Colour(0xffffca5c));
        voiceLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::voiceHeaderTip, language));
        addAndMakeVisible(voiceLabel);

        constexpr std::array<const char*, 3> coreNames { "40106", "8038", "4069UB" };
        for (std::size_t i = 0; i < coreSwitches.size(); ++i)
        {
            coreSwitches[i].setButtonText(coreNames[i]);
            coreSwitches[i].setRadioGroupId(9000);
            coreSwitches[i].setComponentID("core");
            coreSwitches[i].setLookAndFeel(&patchToggleLookClone());
            coreSwitches[i].onClick = [this, i]
            {
                using Mode = antitotem::CmosVoice::OscillatorCore;
                const auto mode = i == 0 ? Mode::schmittPulse : (i == 2 ? Mode::unbufferedDrift : Mode::functionForms);
                fifth.setOscillatorCore(mode);
            };
            addAndMakeVisible(coreSwitches[i]);
        }
        coreSwitches[1].setToggleState(true, juce::dontSendNotification);
        fifth.setOscillatorCore(antitotem::CmosVoice::OscillatorCore::functionForms);

        // Single line (author, live: "nos titulos dos osciladores vamos
        // usar somente uma linha... para o oscilador 4 deixe somente OSC 4
        // - SUB/DIV e oscilador 5 OSC 5 - HETERO").
        constexpr std::array<const char*, 5> names { "OSC A", "OSC B", "OSC C", "OSC 4 - SUB/DIV", "OSC 5 - HETERO" };
        constexpr std::array<double, 5> levelDefaults { 0.62, 0.38, 0.28, 0.0, 0.0 };
        constexpr std::array<double, 5> shapeDefaults { 0.0, 2.0, 3.0, 0.0, 1.0 };
        constexpr std::array<double, 5> ratioDefaults { 1.0, 0.73, 1.51, 0.25, 0.87 };
        for (std::size_t i = 0; i < oscillators.size(); ++i)
        {
            // 12.0f, matching PRINCIPAL's own size (author, live: "faça
            // uma auditoria e altere tudo que estiver diferente entre as
            // abas") - was 11.0f here only.
            configureLabel(oscillatorLabels[i], names[i], 12.0f, juce::Colour(0xffded4be));
            addAndMakeVisible(oscillatorLabels[i]);
            oscillatorRates[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            oscillatorRates[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorRates[i].setRange(i == 3 ? 0.03125 : 0.125, 4.0, 0.001);
            oscillatorRates[i].setValue(ratioDefaults[i]);
            configureLabel(oscillatorRateLabels[i], "FREQ", 9.0f, juce::Colour(0xff8f856f));
            oscillatorRateLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorRateLabels[i]);
            oscillatorRates[i].onValueChange = [this, i] { fifth.setOscillatorRatio(i, static_cast<float>(oscillatorRates[i].getValue())); };
            oscillatorRates[i].setTooltip(utf8("FREQ"));
            addAndMakeVisible(oscillatorRates[i]);
            oscillators[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            oscillators[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillators[i].setRange(0.0, 1.0, 0.01);
            oscillators[i].setValue(levelDefaults[i]);
            configureLabel(oscillatorLevelLabels[i], "MIX", 9.0f, juce::Colour(0xff8f856f));
            oscillatorLevelLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorLevelLabels[i]);
            oscillators[i].onValueChange = [this, i] { fifth.setOscillatorLevel(i, static_cast<float>(oscillators[i].getValue())); updateSilentHighlightDefault(oscillators[i]); };
            updateSilentHighlightDefault(oscillators[i]);
            oscillators[i].setTooltip(utf8("MIX"));
            addAndMakeVisible(oscillators[i]);
            oscillatorShapes[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            oscillatorShapes[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorShapes[i].setRange(0.0, 3.0, 0.01);
            oscillatorShapes[i].setValue(shapeDefaults[i]);
            configureLabel(oscillatorShapeLabels[i], antitotem::ui::text(antitotem::ui::label::shape, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorShapeLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorShapeLabels[i]);
            oscillatorShapes[i].onValueChange = [this, i] { fifth.setOscillatorShape(i, static_cast<float>(oscillatorShapes[i].getValue())); };
            oscillatorShapes[i].setTooltip(utf8("FORMA"));
            addAndMakeVisible(oscillatorShapes[i]);
            oscillatorPans[i].setSliderStyle(juce::Slider::LinearHorizontal);
            oscillatorPans[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorPans[i].setRange(-1.0, 1.0, 0.01);
            oscillatorPans[i].setValue(i == 0 ? -0.58 : (i == 2 ? 0.58 : 0.0));
            configureLabel(oscillatorPanCaptions[i], antitotem::ui::text(antitotem::ui::label::axisX, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorPanCaptions[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorPanCaptions[i]);
            oscillatorPans[i].onValueChange = [this, i] { fifth.setOscillatorPan(i, static_cast<float>(oscillatorPans[i].getValue())); };
            oscillatorPans[i].setTooltip(utf8("EIXO X"));
            addAndMakeVisible(oscillatorPans[i]);
            oscillatorProximities[i].setSliderStyle(juce::Slider::LinearHorizontal);
            oscillatorProximities[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorProximities[i].setRange(0.0, 1.0, 0.01); oscillatorProximities[i].setValue(0.0);
            configureLabel(oscillatorProximityCaptions[i], antitotem::ui::text(antitotem::ui::label::axisY, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorProximityCaptions[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorProximityCaptions[i]);
            oscillatorProximities[i].onValueChange = [this, i] { fifth.setOscillatorProximity(i, static_cast<float>(oscillatorProximities[i].getValue())); };
            oscillatorProximities[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisYShort, language));
            addAndMakeVisible(oscillatorProximities[i]);
            oscillatorOrbits[i].setSliderStyle(juce::Slider::LinearHorizontal);
            oscillatorOrbits[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorOrbits[i].setRange(0.0, 1.0, 0.01); oscillatorOrbits[i].setValue(0.0);
            configureLabel(oscillatorOrbitCaptions[i], antitotem::ui::text(antitotem::ui::label::axisZ, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorOrbitCaptions[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorOrbitCaptions[i]);
            oscillatorOrbits[i].onValueChange = [this, i] { fifth.setOscillatorOrbit(i, static_cast<float>(oscillatorOrbits[i].getValue())); };
            oscillatorOrbits[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisZShort, language));
            addAndMakeVisible(oscillatorOrbits[i]);
        }

        // Same text/size/colour as the main tab's own filterLabel.
        configureLabel(filterLabel, antitotem::ui::text(antitotem::ui::label::vcfMultimode, language), 14.0f, juce::Colour(0xffffca5c));
        filterLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::vcfHeaderTip, language));
        addAndMakeVisible(filterLabel);
        constexpr std::array<const char*, 3> filterControlNames { "FREQ", "RES", "CV" };
        for (std::size_t i = 0; i < filterControlLabels.size(); ++i)
        {
            configureLabel(filterControlLabels[i], filterControlNames[i], 9.0f, juce::Colour(0xff8f856f));
            filterControlLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(filterControlLabels[i]);
        }
        for (auto* slider : { &filterCutoff, &filterResonance, &filterDepth })
        {
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider->setRange(0.0, 1.0, 0.01);
            // Teste (18 ago. 2026, author: "teste a cor azul dos knobs do
            // vcf menos fortes (talvez com alguma porcentagem de
            // transparencia)") - withAlpha, not a different/darker hex, so
            // the dark panel background shows through and softens it
            // without changing the hue itself.
            slider->setColour(juce::Slider::rotarySliderFillColourId, cloneMaterial::vcf.withAlpha(0.6f));
            addAndMakeVisible(*slider);
        }
        filterCutoff.setValue(0.58); filterCutoff.setTooltip("FREQ");
        filterResonance.setValue(0.24); filterResonance.setTooltip("RES");
        filterDepth.setValue(0.52); filterDepth.setTooltip("CV");
        filterCutoff.onValueChange = [this] { fifth.setFilterCutoff(static_cast<float>(filterCutoff.getValue())); };
        filterResonance.onValueChange = [this] { fifth.setFilterResonance(static_cast<float>(filterResonance.getValue())); };
        filterDepth.onValueChange = [this] { fifth.setFilterCvDepth(static_cast<float>(filterDepth.getValue())); };
        // LPF/BPF/HPF/NCH: four independent toggles, not one cycling
        // button (17 ago. 2026, author: "dois ou mais" - selecting more
        // than one at once is a real, cheap filter blend, see CmosVcf.h's
        // own comment). Recomputes the whole mask from all four buttons'
        // toggle states on every click, so any combination just works.
        {
            constexpr std::array<const char*, 4> filterModeNames { "LPF", "BPF", "HPF", "NCH" };
            constexpr std::array<unsigned char, 4> filterModeBits {
                antitotem::CmosVcf::Mode::lowpass, antitotem::CmosVcf::Mode::bandpass,
                antitotem::CmosVcf::Mode::highpass, antitotem::CmosVcf::Mode::notch };
            for (std::size_t i = 0; i < filterModeButtons.size(); ++i)
            {
                filterModeButtons[i].setLookAndFeel(&panelButtonLookClone());
                filterModeButtons[i].setButtonText(filterModeNames[i]);
                filterModeButtons[i].setClickingTogglesState(true);
                filterModeButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterMode, language));
                filterModeButtons[i].onClick = [this, i]
                {
                    // At least one stays active always - clicking the
                    // last remaining ON button back off would leave the
                    // screen showing "nothing selected" while the filter
                    // kept sounding like LPF underneath anyway (mask=0
                    // already falls back to `low` in CmosVcf.h). Reverting
                    // the click that would cause that keeps what's lit
                    // always true to what's audible (author, live, 17
                    // ago. 2026, choosing this over true silence or an
                    // always-audible-but-unlit LPF).
                    bool anyActive = false;
                    for (auto& button : filterModeButtons) anyActive = anyActive || button.getToggleState();
                    if (!anyActive) filterModeButtons[i].setToggleState(true, juce::dontSendNotification);
                    unsigned char mask = 0;
                    for (std::size_t j = 0; j < filterModeButtons.size(); ++j)
                    {
                        constexpr std::array<unsigned char, 4> bits {
                            antitotem::CmosVcf::Mode::lowpass, antitotem::CmosVcf::Mode::bandpass,
                            antitotem::CmosVcf::Mode::highpass, antitotem::CmosVcf::Mode::notch };
                        if (filterModeButtons[j].getToggleState()) mask |= bits[j];
                    }
                    fifth.setFilterModeMask(mask);
                };
                addAndMakeVisible(filterModeButtons[i]);
            }
            filterModeButtons[0].setToggleState(true, juce::dontSendNotification);
            fifth.setFilterModeMask(filterModeBits[0]);
        }

        // MaterialFilter MIX - see the member declaration's own comment.
        // No on/off button: not just a switch, a continuum (author, live,
        // 17 ago. 2026: "não é só ligar/desligar, é ligar/escalonar/
        // desligar") - same as REVERB/PHASER/FLANGER's own MIX, no switch
        // there either.
        // Same caption colour as VCF's own FREQ/RES/CV (author, live, 17
        // ago. 2026: "mantenha a cor dos titulos do vcf (CV)"), not an
        // accent colour of its own.
        configureLabel(materialFilterLabel, "MAT", 9.0f, juce::Colour(0xff8f856f));
        materialFilterLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(materialFilterLabel);
        materialFilterMix.setSliderStyle(juce::Slider::LinearHorizontal);
        materialFilterMix.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        materialFilterMix.setRange(0.0, 1.0, 0.01);
        materialFilterMix.setValue(0.0);
        materialFilterMix.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialFilterMix, language));
        // Same filter colour as MATÉRIA's own CUTOFF/RESON/DRIVE/ASYM rail
        // sliders (18 ago. 2026) - MAT is their wet/dry crossfade, so it
        // should read as visually part of that same group, not a colour of
        // its own (author, live: "falta estabelecer uma relação de cores
        // entre o slider materia e os parametros materia"). Was returnPath.
        materialFilterMix.onValueChange = [this] { fifth.setMaterialFilterMix(static_cast<float>(materialFilterMix.getValue())); updateSilentHighlight(materialFilterMix, juce::Colour(0xff8f856f)); };
        updateSilentHighlight(materialFilterMix, juce::Colour(0xff8f856f));
        addAndMakeVisible(materialFilterMix);
        fifth.setMaterialFilterMix(0.0f);
        // CUTOFF/RESONANCE/DRIVE/ASYMMETRY are no longer fixed here - the
        // MATÉRIA rail sliders (detailControls[9-12] below) now own them,
        // set via updateDetails() at construction.

        configureLabel(envelopeLabel, "ADSR", 14.0f, juce::Colour(0xffffca5c));
        envelopeLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::adsrHeaderTip, language));
        addAndMakeVisible(envelopeLabel);
        constexpr std::array<const char*, 4> contourNames { "ATT", "DEC", "SUS", "REL" };
        constexpr std::array<double, 4> contourDefaults { 0.30, 0.56, 0.62, 0.60 };
        for (std::size_t i = 0; i < envelopeControls.size(); ++i)
        {
            configureLabel(envelopeLabels[i], contourNames[i], 9.0f, juce::Colour(0xff8f856f));
            // Centred, matching PRINCIPAL's own ADSR/VCF knob captions and
            // this tab's own filterControlLabels just below (author, live:
            // "titulo dos knobs do adsr precisam permanecer centralizados
            // nas duas abas") - this one was left at the default left
            // alignment.
            envelopeLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(envelopeLabels[i]);
            envelopeControls[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            envelopeControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            envelopeControls[i].setRange(0.0, 1.0, 0.01);
            envelopeControls[i].setValue(contourDefaults[i]);
            // withAlpha - softer, same treatment as VCF (18 ago. 2026,
            // author: "a cor dos knobs adsr também estão fortes, faça
            // como fez no vcf... idem para energia, lfo, noise send e
            // master").
            envelopeControls[i].setColour(juce::Slider::rotarySliderFillColourId, cloneMaterial::adsr.withAlpha(0.6f));
            addAndMakeVisible(envelopeControls[i]);
        }
        envelopeControls[0].onValueChange = [this] { fifth.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue())); };
        envelopeControls[1].onValueChange = [this] { fifth.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue())); };
        envelopeControls[2].onValueChange = [this] { fifth.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue())); };
        envelopeControls[3].onValueChange = [this] { fifth.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue())); };

        configureLabel(energyLabel, antitotem::ui::text(antitotem::ui::label::energy, language), 9.0f, juce::Colour(0xffded4be));
        // Centred, matching PRINCIPAL's own ENERGIA/NOISE titles (author,
        // live: "titulos dos objetos energia e noise devem permancer
        // centralizados nas duas abas") - was left at the default left
        // alignment here.
        energyLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(energyLabel);
        energy.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        // Was TextBoxBelow (60,20) - real root cause of the caption
        // sitting visibly low inside the knob, found after sizing alone
        // didn't fix it (18 ago. 2026, author: "não que utilize a mesma
        // configuração de um knob do oscilador para o knob de energia, e
        // teste o titulo no centro nesse knob, só mantenha as cores").
        // JUCE reserves that 20px text box INSIDE the slider's own
        // component bounds, shrinking/shifting the rotary circle itself
        // upward to make room for it below - centring the caption on the
        // full component bounds (which is what the knob code here does)
        // then landed below the circle's real, shifted-up centre.
        // Oscillators never had this problem because they were always
        // NoTextBox. Matching that removes the visible "0.78" readout -
        // flagged to the author, not silently dropped.
        energy.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        // Same fill colour as the main tab's own ENERGIA knob, withAlpha
        // for the same softening pass as VCF/ADSR/LFO/NOISE SEND/MASTER.
        energy.setColour(juce::Slider::rotarySliderFillColourId, cloneMaterial::voice.withAlpha(0.6f));
        energy.setRange(0.0, 1.0, 0.01);
        energy.setValue(0.72);
        energy.onValueChange = [this] { fifth.setEnergy(static_cast<float>(energy.getValue())); };
        addAndMakeVisible(energy);

        // "MODO: CLONE" - always true for this tab's own transport
        // column, embedded or standalone (author, live: "definir uma
        // função que escreva na tela... para que o usuário compreenda em
        // qual aba está atuando").
        configureLabel(modeLabel, antitotem::ui::text(antitotem::ui::label::modeClone, language), 11.0f, cloneMaterial::board);
        addAndMakeVisible(modeLabel);

        // CLONE has its own clock rate and temporal/scanner character - it
        // does not inherit PULSO/MÉTRICA/PERCURSO from the main window,
        // since those are per-SimpleSequencer state (each object keeps its
        // own clockFeel/metric/scannerDirection independently).
        configureLabel(clockLabel, "CLOCK", 9.0f, juce::Colour(0xffded4be));
        clockLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(clockLabel);
        clockRate.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        // Was TextBoxBelow - see energy's own detailed comment above for
        // why this was the real cause of CLOCK's caption sitting low
        // inside the knob, not a sizing/positioning bug.
        clockRate.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        // Same fill colour as the main tab's own CLOCK knob, withAlpha for
        // the same softening pass as VCF/ADSR/ENERGIA/LFO/NOISE SEND/
        // MASTER (18 ago. 2026, author: "acho que esquecemos de fazer o
        // tratamento de transparencia no clock" - correct, it was missed).
        clockRate.setColour(juce::Slider::rotarySliderFillColourId, cloneMaterial::clock.withAlpha(0.6f));
        clockRate.setRange(0.1, 20.0, 0.01);
        clockRate.setValue(2.0);
        clockRate.onValueChange = [this] { fifth.setClockRate(clockRate.getValue()); };
        clockRate.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockRateKnob, language));
        addAndMakeVisible(clockRate);

        // PULSO/MÉTRICA/PERCURSO now share the same colour as PORTAS DE
        // FEEDBACK/VARIAÇÃO/FIM DO LOOP below them (18 ago. 2026, author:
        // "na coluna da esquerda titulos: pulso, metrica, precurso também
        // ficam na mesma cor de portas de feedback") - was clock (green)/
        // memory (pink) each, now the whole left column's group of
        // titles reads as one family. Same colour used at every other
        // site that sets these 3 labels.
        configureLabel(temporalLabel, antitotem::ui::text(antitotem::ui::label::pulse, language), 10.0f, juce::Colour(0xffded4be));
        temporalLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::temporalHeaderTip, language));
        configureLabel(metricLabel, antitotem::ui::text(antitotem::ui::label::meter, language), 10.0f, juce::Colour(0xffded4be));
        metricLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricHeaderTip, language));
        configureLabel(scannerLabel, antitotem::ui::text(antitotem::ui::label::path, language), 10.0f, juce::Colour(0xffded4be));
        scannerLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerHeaderTip, language));
        addAndMakeVisible(temporalLabel); addAndMakeVisible(metricLabel); addAndMakeVisible(scannerLabel);
        auto updateTemporal = [this]
        {
            using Feel = antitotem::SimpleSequencer::ClockFeel;
            // 8, not 4 (20 ago. 2026, author: "implementamos mais uma
            // fileira de pulse (oito no total): com tercina, quintina,
            // sextina, septina, nonina, 11ina" / "além do reto e glitch") -
            // straight/glitch stay the two anchors, the other 6 are all
            // tuplet feels; see SimpleSequencer.cpp's own comment on
            // samplesPerStep() for why the 4 new ratios are (n-1)/n.
            constexpr std::array<Feel, 8> feels { Feel::straight, Feel::triplet, Feel::quintuplet, Feel::swing,
                                                   Feel::septuplet, Feel::nonuplet, Feel::undecuplet, Feel::glitch };
            // Plain beat counts, not fractions (20 ago. 2026, author: "ao
            // invés de deixar 5/4 deixe somente 5, mais justo e
            // transparente para o usuário que é musico") - the denominator
            // was only ever a subtle accent-depth nuance (author confirmed
            // keeping that design, see TAREFAS.md), never a real change of
            // feel/timing, so labelling it as a fraction implied a
            // musical distinction ("5/4" vs "5/8") that wasn't actually
            // there. unit stays a constant 4 for all 8 - the button no
            // longer claims a denominator at all.
            constexpr std::array<unsigned int, 8> beats { 2, 3, 4, 5, 6, 7, 8, 9 };
            constexpr std::array<unsigned int, 8> units { 4, 4, 4, 4, 4, 4, 4, 4 };
            fifth.setClockFeel(feels[static_cast<std::size_t>(temporalSelection)]);
            fifth.setMetric(beats[static_cast<std::size_t>(metricSelection)], units[static_cast<std::size_t>(metricSelection)]);
        };
        constexpr std::array<const char*, 8> temporalNames { "RET", "3:2", "5:4", "SWG", "7:4", "9:8", "11:8", "GLT" };
        constexpr std::array<const char*, 8> metricNames { "2", "3", "4", "5", "6", "7", "8", "9" };
        constexpr std::array<const char*, 4> scannerNames { "FWD", "REV", "ALT", "MEM" };
        for (std::size_t i = 0; i < 4; ++i)
        {
            scannerButtons[i].setButtonText(scannerNames[i]);
            // "feedback" (red/cloneMaterial::returnPath), matching the main
            // tab's own PERCURSO buttons - this tab had them on "loop"
            // (pink) instead.
            scannerButtons[i].setRadioGroupId(9003); scannerButtons[i].setComponentID("feedback");
            scannerButtons[i].setLookAndFeel(&patchToggleLookClone());
            scannerButtons[i].onClick = [this, i]
            {
                using Direction = antitotem::SimpleSequencer::ScannerDirection;
                constexpr std::array<Direction, 4> directions { Direction::forward, Direction::reverse, Direction::pendulum, Direction::memoryAddress };
                scannerSelection = static_cast<int>(i);
                fifth.setScannerDirection(directions[static_cast<std::size_t>(scannerSelection)]);
            };
            addAndMakeVisible(scannerButtons[i]);
        }
        // Split into its own loop, 19 ago. 2026 - metricButtons grew from
        // 4 to 8 (two rows of 4, see the array-size comment above), no
        // longer the same count as scannerButtons, so it can't share a
        // single combined loop with it anymore.
        for (std::size_t i = 0; i < metricButtons.size(); ++i)
        {
            metricButtons[i].setButtonText(metricNames[i]);
            // "loop" (pink/cloneMaterial::memory), matching the main tab's own
            // MÉTRICA buttons - this tab had them on "core" (blue) instead.
            metricButtons[i].setRadioGroupId(9002); metricButtons[i].setComponentID("loop");
            metricButtons[i].setLookAndFeel(&patchToggleLookClone());
            metricButtons[i].onClick = [this, i, updateTemporal] { metricSelection = static_cast<int>(i); updateTemporal(); };
            addAndMakeVisible(metricButtons[i]);
        }
        // Own loop too, 20 ago. 2026 - temporalButtons grew from 4 to 8
        // (two rows of 4), same reason metricButtons got split out above.
        for (std::size_t i = 0; i < temporalButtons.size(); ++i)
        {
            temporalButtons[i].setButtonText(temporalNames[i]);
            temporalButtons[i].setRadioGroupId(9001); temporalButtons[i].setComponentID("core");
            temporalButtons[i].setLookAndFeel(&patchToggleLookClone());
            temporalButtons[i].onClick = [this, i, updateTemporal] { temporalSelection = static_cast<int>(i); updateTemporal(); };
            addAndMakeVisible(temporalButtons[i]);
        }
        temporalButtons[0].setToggleState(true, juce::dontSendNotification);
        metricButtons[0].setToggleState(true, juce::dontSendNotification);
        scannerButtons[0].setToggleState(true, juce::dontSendNotification);
        updateTemporal();

        // GROOVE - a general long-short modifier layered on every
        // SUBDIVISÃO feel, not exclusive to SWG (20 ago. 2026, "deixa o
        // swing somente enquanto botão, e utilise esse slide atual do
        // swing para o groove"). Defaults to 0 (no effect) so anyone who
        // never touches it hears no change from before.
        configureLabel(grooveLabel, antitotem::ui::text(antitotem::ui::label::groove, language), 10.0f, juce::Colour(0xffded4be));
        grooveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, language));
        addAndMakeVisible(grooveLabel);
        grooveAmount.setSliderStyle(juce::Slider::LinearHorizontal);
        grooveAmount.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        grooveAmount.setRange(0.0, 1.0, 0.01);
        grooveAmount.setValue(0.0);
        grooveAmount.setColour(juce::Slider::thumbColourId, cloneMaterial::controlBlue);
        grooveAmount.setColour(juce::Slider::trackColourId, cloneMaterial::controlBlue.darker(0.70f));
        grooveAmount.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, language));
        grooveAmount.onValueChange = [this] { fifth.setGrooveAmount(static_cast<float>(grooveAmount.getValue())); };
        addAndMakeVisible(grooveAmount);

        // FIM DO LOOP: this object's own loop end point, independent of the
        // main tab's - matches the main tab's own text/colour/default
        // (button 16 active, full loop) even though the grid itself is
        // more compact here (8 columns, not 4 - this column has far less
        // room to spare).
        configureLabel(loopLabel, antitotem::ui::text(antitotem::ui::label::loopEndPrefix, language) + "1" + antitotem::ui::text(antitotem::ui::label::loopEndSuffix, language), 10.0f, juce::Colour(0xffded4be));
        loopLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::loopHeaderTip, language));
        addAndMakeVisible(loopLabel);
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
        {
            loopSwitches[i].setButtonText(juce::String(static_cast<int>(i + 1)));
            loopSwitches[i].setComponentID("loop");
            loopSwitches[i].setLookAndFeel(&patchToggleLookClone());
            loopSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::loopEnd, language));
            loopSwitches[i].onClick = [this, i] { setLoopEnd(i + 1); };
            addAndMakeVisible(loopSwitches[i]);
        }
        setLoopEnd(loopSwitches.size());

        // LEARN: this tab's own hover/focus listening (needed even for
        // the standalone second-monitor window, a separate top-level
        // window MainComponent's own listener can't see) but no box of
        // its own - author, live: "no segundo monitor não é necessário
        // repetir a caixa learn... fica somente na aba principal
        // (funcionando para as duas abas) e dois monitores". Explanation
        // text is handed to `onExplain`, which MainComponent wires to
        // its own single learnEditor after constructing this instance
        // (clonePanel) or the standalone ObjectFiveWindow.
        addMouseListener(this, true);
        juce::Desktop::getInstance().addFocusChangeListener(this);

        // 16-step CV: the sequenced pitch/voltage identity of the clone's
        // own voice. AMP/FX/MUTE per step remain future work (see
        // TAREFAS.md) - CV alone already lets the clone's steps differ from
        // the object it started as a copy of.
        // Same size/colour as PARÂMETROS (18 ago. 2026, author: "o Titulo
        // CV (16 steps) deixa em amarelo como em PARAMETROS e também do
        // mesmo tamanho") - was smaller and the muted board gold instead
        // of the brighter section-title yellow.
        configureLabel(stepsLabel, utf8("CV (16 STEPS)"), 14.0f, juce::Colour(0xffffca5c));
        stepsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::stepsHeaderTip, language));
        addAndMakeVisible(stepsLabel);
        for (auto& step : steps)
        {
            addAndMakeVisible(step);
            step.cv.onValueChange = [this] { syncStepControls(); };
            step.level.onValueChange = [this] { syncStepControls(); };
            step.send.onValueChange = [this] { syncStepControls(); };
            step.mute.onClick = [this] { syncStepControls(); };
        }
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            steps[i].cv.setValue(fifth.getStepVoltage(i), juce::dontSendNotification);
            steps[i].level.setValue(fifth.getStepLevel(i), juce::dontSendNotification);
            steps[i].send.setValue(fifth.getStepEffectSend(i), juce::dontSendNotification);
            steps[i].mute.setToggleState(fifth.isStepMuted(i), juce::dontSendNotification);
        }

        configureLabel(noiseLabel, "NOISE", 12.0f, juce::Colour(0xffded4be));
        noiseLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseHeaderTip, language));
        // Centred, matching PRINCIPAL's own ENERGIA/NOISE titles (author,
        // live: "titulos dos objetos energia e noise devem permancer
        // centralizados nas duas abas") - was left at the default left
        // alignment here.
        noiseLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(noiseLabel);
        constexpr std::array<antitotem::NoisePalette::Colour, 6> noiseColours {
            antitotem::NoisePalette::Colour::white, antitotem::NoisePalette::Colour::pink,
            antitotem::NoisePalette::Colour::brown, antitotem::NoisePalette::Colour::blue,
            antitotem::NoisePalette::Colour::violet, antitotem::NoisePalette::Colour::bit };
        noiseSelector.onSelection = [this, noiseColours] (int index) { fifth.setNoiseColour(noiseColours[static_cast<std::size_t>(index)]); };
        noiseSelector.onSampleHoldChange = [this] (bool enabled) { fifth.setSampleHoldMix(enabled ? 0.78f : 0.0f); };
        // Fixed depth, not a continuous slider (docs/
        // PESQUISA_RUIDO_GENERATIVO.md item #2 - author, 19 ago. 2026,
        // chose a button "como o SWG" over a dial). 0.28 - noticeable
        // breathing without dominating the mix (same 0-0.42 range
        // setNoiseBreathAmount clamps to, roughly two-thirds up it).
        noiseSelector.onBreathChange = [this] (bool enabled) { fifth.setNoiseBreathAmount(enabled ? 0.28f : 0.0f); };
        addAndMakeVisible(noiseSelector);
        noiseSelector.select(0, false);
        fifth.setNoiseColour(noiseColours[0]);

        configureLabel(modulationLabel, antitotem::ui::text(antitotem::ui::label::modulation, language), 14.0f, juce::Colour(0xffffca5c));
        modulationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::modulationHeaderTip, language));
        addAndMakeVisible(modulationLabel);
        // NOISE SEND, not NOISE MIX (18 ago. 2026) - "NOISE GATE" was
        // considered and reverted the same day (the knob itself is still a
        // continuous 0-1 blend, not a threshold/on-off control, and "gate"
        // already names a specific, unrelated audio effect). "SEND" matches
        // the real architecture instead: this knob sends an amount of noise
        // into the voice/RING chain, and the MIXER's own NOISE channel is
        // the bus that receives it, with its own master ON/gain - the same
        // send/aux-bus model as a real mixing console. See TAREFAS.md/
        // FLUXO_DE_SINAL.md for the underlying mechanism.
        constexpr std::array<const char*, 3> modulationNames { "LFO", "RING", "NOISE\nSEND" };
        for (std::size_t i = 0; i < modulationControls.size(); ++i)
        {
            configureLabel(modulationLabels[i], modulationNames[i], 9.0f, juce::Colour(0xffded4be));
            addAndMakeVisible(modulationLabels[i]);
            // Same per-index colour scheme as the main tab's own MODULAÇÃO
            // row (LFO/RING/NOISE) - this tab's sliders had no colour of
            // their own at all before, reading as visually unrelated to the
            // identical row on the main tab.
            const auto modulationColour = i == 0 ? cloneMaterial::clock : (i == 1 ? cloneMaterial::controlBlue : cloneMaterial::noiseSend);
            // Second pass the same day (18 ago. 2026, author, after seeing
            // the first result): LFO and NOISE MIX are both knobs now, RING
            // went back to horizontal - "KNOB MODULAÇÃO LFO deve ser do
            // mesmo tamanho que o knob do adsr... o MIX NOISE passa a ser
            // um knob também... o slider ring volta a ser horizontal".
            if (i == 0 || i == 2)
            {
                modulationControls[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
                // withAlpha, same softening as VCF/ADSR/ENERGIA/MASTER -
                // see filterCutoff's own comment for the full rationale.
                // RING (the `else` branch below) is untouched - only asked
                // for LFO/NOISE SEND, not RING.
                modulationControls[i].setColour(juce::Slider::rotarySliderFillColourId, modulationColour.withAlpha(0.6f));
            }
            else
            {
                modulationControls[i].setSliderStyle(juce::Slider::LinearHorizontal);
                modulationControls[i].setColour(juce::Slider::thumbColourId, modulationColour);
                modulationControls[i].setColour(juce::Slider::trackColourId, modulationColour.darker(0.72f));
            }
            modulationControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            modulationControls[i].setRange(0.0, 1.0, 0.01);
            addAndMakeVisible(modulationControls[i]);
        }
        // RING's caption matches MaterialFilter's own MAT caption now -
        // same colour, same centred justification (18 ago. 2026, author:
        // "mesma cor e posição - centrado").
        configureLabel(modulationLabels[1], modulationNames[1], 9.0f, juce::Colour(0xff8f856f));
        modulationLabels[1].setJustificationType(juce::Justification::centred);
        // LFO/NOISE MIX captions centred over their own knobs too (18
        // ago. 2026, author: "os titulos dos knobs LFO e NOISE MIX devem
        // ficar centralizados ao knob") - configureLabel's own default
        // is centredLeft, matching every other left-aligned caption in
        // the panel, but these two sit directly above round knobs now
        // rather than a left-anchored control.
        modulationLabels[0].setJustificationType(juce::Justification::centred);
        modulationLabels[2].setJustificationType(juce::Justification::centred);
        // Same colour as the oscillators' own FREQ/MIX/FORM captions (18
        // ago. 2026, author: "os titulos dos knobs lfo e noise sende
        // devem ficar na mesma cor dos titulos dos osciladores") - was
        // 0xffded4be (this loop's shared default), now the oscillators'
        // 0xff8f856f, same value RING already uses above.
        modulationLabels[0].setColour(juce::Label::textColourId, juce::Colour(0xff8f856f));
        modulationLabels[2].setColour(juce::Label::textColourId, juce::Colour(0xff8f856f));
        modulationControls[0].setValue(0.42);
        modulationControls[0].onValueChange = [this] { fifth.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f)); };
        modulationControls[1].onValueChange = [this] { fifth.setRingMix(static_cast<float>(modulationControls[1].getValue())); updateSilentHighlight(modulationControls[1], cloneMaterial::controlBlue); };
        modulationControls[2].onValueChange = [this] { fifth.setNoiseMix(static_cast<float>(modulationControls[2].getValue())); updateSilentHighlight(modulationControls[2], cloneMaterial::noiseSend); };
        updateSilentHighlight(modulationControls[1], cloneMaterial::controlBlue);
        updateSilentHighlight(modulationControls[2], cloneMaterial::noiseSend);
        // lfoShapeLabel already had bounds set in resized() but was never
        // given text/colour - it existed as an invisible empty label.
        configureLabel(lfoShapeLabel, antitotem::ui::text(antitotem::ui::label::lfoShape, language), 12.0f, cloneMaterial::clock);
        addAndMakeVisible(lfoShapeLabel);
        // CAOS/VAGA (17 ago. 2026): ChaosField/WanderSource (src/core/
        // ChaosSources.h) reusing this same LFO rail and its existing RING
        // destination, instead of a new CV-routing matrix - see
        // ModulationSources.h's own comment.
        constexpr std::array<const char*, 6> lfoShapeNames { "SEN", "TRI", "PUL", "CAOS", "VAGA", "STEP" };
        for (std::size_t i = 0; i < lfoShapeButtons.size(); ++i)
        {
            lfoShapeButtons[i].setButtonText(lfoShapeNames[i]);
            lfoShapeButtons[i].setRadioGroupId(9005); lfoShapeButtons[i].setComponentID("core");
            lfoShapeButtons[i].setLookAndFeel(&patchToggleLookClone());
            lfoShapeButtons[i].onClick = [this, i]
            {
                constexpr std::array<antitotem::LfoSource::Shape, 6> shapes {
                    antitotem::LfoSource::Shape::sine, antitotem::LfoSource::Shape::triangle, antitotem::LfoSource::Shape::square,
                    antitotem::LfoSource::Shape::chaos, antitotem::LfoSource::Shape::wander, antitotem::LfoSource::Shape::step };
                fifth.setLfoShape(shapes[i]);
                // FREEZE only ever acts on CAOS/VAGA (17 ago. 2026,
                // author, live: "não vejo sentido do botão freeze
                // funcionar quando os botões caos e vaga estão
                // desligados") - disabled rather than just silently
                // inert whenever neither is selected.
                const auto lfoIsChaosOrWander = i == 3 || i == 4;
                lfoFreeze.setEnabled(lfoIsChaosOrWander);
                // Bug found live (18 ago. 2026) - see MainComponent's own
                // copy of this same fix for the full comment: disabling
                // FRZ alone left a stale ring lit on a control that could
                // no longer be un-toggled. Leaving CAOS/VAGA now
                // force-releases FREEZE for real.
                if (!lfoIsChaosOrWander && lfoFreeze.getToggleState())
                {
                    lfoFreeze.setToggleState(false, juce::dontSendNotification);
                    fifth.setLfoFrozen(false);
                    patchToggleLookClone().lfoFrozen = false;
                    lfoShapeButtons[3].repaint(); lfoShapeButtons[4].repaint();
                }
            };
            addAndMakeVisible(lfoShapeButtons[i]);
        }
        lfoShapeButtons[0].setToggleState(true, juce::dontSendNotification);
        // FREEZE (17 ago. 2026) - only meaningful while CAOS/VAGA is
        // selected (see LfoSource's own comment), but always present in
        // the panel rather than conditionally shown/hidden. RESEED (also
        // 17 ago. 2026) had its own button here briefly, removed at the
        // author's request - see deriveFromMemory() for where it fires
        // now.
        lfoFreeze.setButtonText("FRZ");
        lfoFreeze.setComponentID("core");
        lfoFreeze.setLookAndFeel(&patchToggleLookClone());
        lfoFreeze.setEnabled(false); // SEN is the default selected shape
        lfoFreeze.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoFreeze, language));
        lfoFreeze.onClick = [this]
        {
            const auto frozen = lfoFreeze.getToggleState();
            fifth.setLfoFrozen(frozen);
            patchToggleLookClone().lfoFrozen = frozen;
            lfoShapeButtons[3].repaint(); lfoShapeButtons[4].repaint();
        };
        addAndMakeVisible(lfoFreeze);

        // Not the shared section-title gold (0xffffca5c) anymore (18 ago.
        // 2026, author: "o titulo espaço/fase não deve ficar amarelo, pois
        // o PARAMETROS é amarelo") - ESPAÇO/FASE sits directly under
        // PARÂMETROS in the rails band, so sharing that exact colour read
        // as a collision, unlike MODULAÇÃO/VCF/ADSR elsewhere in the panel
        // which aren't physically adjacent to PARÂMETROS. Uses REVERB's own
        // new colour (0xff3fb0a8) - same pattern as CAOS/MATÉRIA, where the
        // column heading matches its own content's colour.
        configureLabel(effectsLabel, antitotem::ui::text(antitotem::ui::label::spacePhase, language), 10.0f, cloneMaterial::phaser);
        effectsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::effectsHeaderTip, language));
        addAndMakeVisible(effectsLabel);
        constexpr std::array<const char*, 3> effectNames { "REVERB", "PHASER", "FLANGER" };
        for (std::size_t i = 0; i < effectControls.size(); ++i)
        {
            // Caption colour matches the slider's own colour now (18 ago.
            // 2026, author: "deixe todos os titulos dos sliders das 6
            // colunas como fez na coluna espaço/fase") - was the neutral
            // caption colour every other rails-band column had already
            // moved away from (CAOS/ROTAS ATIVAS/MATÉRIA's own detailLabels
            // already used detailColour(i) for this same reason).
            configureLabel(effectLabels[i], effectNames[i], 9.0f, cloneMaterial::phaser);
            addAndMakeVisible(effectLabels[i]);
            effectControls[i].setSliderStyle(juce::Slider::LinearHorizontal);
            effectControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            effectControls[i].setRange(0.0, 1.0, 0.01);
            // Same per-index colour scheme as the main tab's own ESPAÇO/
            // FASE row (REVERB/PHASER/FLANGER).
            const auto effectColour = cloneMaterial::phaser;
            effectControls[i].setColour(juce::Slider::thumbColourId, effectColour);
            effectControls[i].setColour(juce::Slider::trackColourId, effectColour.darker(0.72f));
            addAndMakeVisible(effectControls[i]);
        }
        effectControls[0].onValueChange = [this] { fifth.setReverbMix(static_cast<float>(effectControls[0].getValue())); updateSilentHighlight(effectControls[0], cloneMaterial::phaser); };
        effectControls[1].onValueChange = [this] { fifth.setPhaserMix(static_cast<float>(effectControls[1].getValue())); updateSilentHighlight(effectControls[1], cloneMaterial::phaser); };
        effectControls[2].onValueChange = [this] { fifth.setFlangerMix(static_cast<float>(effectControls[2].getValue())); updateSilentHighlight(effectControls[2], cloneMaterial::phaser); };
        updateSilentHighlight(effectControls[0], cloneMaterial::phaser);
        updateSilentHighlight(effectControls[1], cloneMaterial::phaser);
        updateSilentHighlight(effectControls[2], cloneMaterial::phaser);
        // Section header above the whole grid - missing before (only the
        // 9 per-control captions existed), matching the main tab's own
        // ROTAS ATIVAS.
        configureLabel(detailLabel, antitotem::ui::text(antitotem::ui::label::activeRoutes, language), 10.0f, cloneMaterial::memory);
        detailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailHeaderTip, language));
        addAndMakeVisible(detailLabel);
        // Umbrella title for the whole 6-column band (18 ago. 2026) -
        // every other panel section already had one; this was the one
        // gap left (author, live: "precisa de titulo" after being asked
        // what to call the space).
        configureLabel(parametersLabel, antitotem::ui::text(antitotem::ui::label::parametersRail, language), 14.0f, juce::Colour(0xffffca5c));
        parametersLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::parametersHeaderTip, language));
        addAndMakeVisible(parametersLabel);
        // Filter colour (0xff8f856f) - already the shared colour for
        // FREQ/MIX/SHAPE/AXIS/VCF/ADSR/MAT/RING, so reusing it here ties
        // MATÉRIA visually to the rest of the filter/voice chain it
        // belongs to (author, live, 18 ago. 2026: "preciso de variação de
        // cor segundo função... se matéria é ligado a filtro, merece
        // outra cor"). CAOS gets FORMA LFO's own clock colour for the
        // same reason - its DRIVE/DAMPING/DEPTH are LFO parameters, not
        // generic ROTAS ATIVAS ones.
        configureLabel(materialRailLabel, antitotem::ui::text(antitotem::ui::label::materialRail, language), 10.0f, juce::Colour(0xff8f856f));
        materialRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialHeaderTip, language));
        addAndMakeVisible(materialRailLabel);
        configureLabel(chaosRailLabel, antitotem::ui::text(antitotem::ui::label::chaosRail, language), 10.0f, cloneMaterial::clock);
        chaosRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::chaosHeaderTip, language));
        addAndMakeVisible(chaosRailLabel);
        // CUTOFF/RESONANCE/DRIVE/ASYMMETRY (9-12, 17 ago. 2026): MaterialFilter's
        // remaining parameters, previously fixed at an internal value with no
        // panel control (see "Proliferação de módulos" in TAREFAS.md) - join
        // ROTAS ATIVAS as a 6th column ("MATÉRIA"), same reasoning as RES MIX/
        // ALTURA/CORPO joining it instead of fighting for a new spot.
        const std::array<juce::String, 16> detailNames { "S&H RATE", "RVB RET", "PHS RATE", "PHS PROF", "FLG RATE", "FLG PROF",
                                                             "RES MIX", antitotem::ui::text(antitotem::ui::label::resPitch, language), antitotem::ui::text(antitotem::ui::label::resBody, language),
                                                             "CUTOFF", "RESON", "DRIVE", "ASYM",
                                                             "DRIVE", "DAMPING", "DEPTH" };
        // MATÉRIA (9-12) and CAOS (13-15) get their own function colour
        // (see comment above materialRailLabel); the rest keep the
        // uniform ROTAS ATIVAS colour they always had.
        const auto detailColour = [] (std::size_t index)
        {
            if (index >= 9 && index <= 12) return juce::Colour(0xff8f856f);
            if (index >= 13) return cloneMaterial::clock;
            return cloneMaterial::memory;
        };
        for (std::size_t i = 0; i < detailControls.size(); ++i)
        {
            // 9.0f, matching PRINCIPAL's own size (author, live: "titulos
            // dos sliders do objeto rotas ativas devem permanecer do mesmo
            // tamanho em ambas as abas") - was 8.0f here only.
            configureLabel(detailLabels[i], detailNames[i], 9.0f, detailColour(i));
            addAndMakeVisible(detailLabels[i]);
            detailControls[i].setSliderStyle(juce::Slider::LinearHorizontal);
            detailControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            detailControls[i].setRange(0.0, 1.0, 0.01);
            // Defaults 9-12 match the fixed values they replace (CUTOFF
            // 0.5, RESONANCE 0.6, DRIVE 0.5, ASYMMETRY 0.6 - see
            // TAREFAS.md, "Proliferação de módulos"). Defaults 13-15
            // (CAOS/VAGA's own DRIVE/DAMPING/DEPTH, 18 ago. 2026) match
            // the retuned values LfoSource::prepare() used to hardcode
            // (0.85/0.18/1.0 - see ModulationSources.h).
            constexpr std::array<double, 16> detailDefaults { 0.28, 0.28, 0.28, 0.5, 0.28, 0.5, 0.0, 0.5, 0.5, 0.5, 0.6, 0.5, 0.6, 0.85, 0.18, 1.0 };
            detailControls[i].setValue(detailDefaults[i]);
            detailControls[i].setColour(juce::Slider::thumbColourId, detailColour(i));
            detailControls[i].setColour(juce::Slider::trackColourId, detailColour(i).darker(0.70f));
            detailControls[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailControlTips[i], language));
            addAndMakeVisible(detailControls[i]);
        }
        auto updateDetails = [this]
        {
            const auto v = [this] (std::size_t index) { return static_cast<float>(detailControls[index].getValue()); };
            fifth.setSampleHoldRate(0.4f * std::pow(2.0f, v(0) * 10.0f));
            fifth.setReverbFeedback(v(1) * 0.8f);
            fifth.setPhaserRate(0.04f * std::pow(2.0f, v(2) * 8.0f)); fifth.setPhaserDepth(v(3));
            fifth.setFlangerRate(0.04f * std::pow(2.0f, v(4) * 7.0f)); fifth.setFlangerDepth(v(5));
            fifth.setResonatorMix(v(6)); fifth.setResonatorPitch(v(7)); fifth.setResonatorDamping(v(8));
            fifth.setMaterialFilterCutoff(v(9)); fifth.setMaterialFilterResonance(v(10));
            fifth.setMaterialFilterDrive(v(11)); fifth.setMaterialFilterAsymmetry(v(12));
            fifth.setLfoChaosDrive(v(13)); fifth.setLfoChaosDamping(v(14)); fifth.setLfoWanderDepth(v(15));
        };
        for (std::size_t i = 0; i < detailControls.size(); ++i)
            detailControls[i].onValueChange = [this, i, updateDetails] { updateDetails(); if (i == 6) updateSilentHighlight(detailControls[i], cloneMaterial::memory); };
        updateDetails();
        updateSilentHighlight(detailControls[6], cloneMaterial::memory);

        // Same 6 presets as the main tab's own variation row, applied to
        // this object instead - see applyVariation() below.
        // Same colour as FIM DO LOOP's own title now, not the generic
        // section-title yellow (18 ago. 2026, author: "Título de
        // variação e porta de feedback devem ficar na mesma cor que o
        // titulo de fim do loop") - all 3 sit in the same left column, so
        // grouping their titles under one colour reads as "these belong
        // together" the way ROTAS ATIVAS/CAOS/MATÉRIA already do
        // elsewhere. Same change applied at every other site that sets
        // this label's colour, and at PORTAS DE FEEDBACK's own title
        // (`feedbackLabel` here/`connectionLabel` in MainComponent).
        configureLabel(variationLabel, antitotem::ui::text(antitotem::ui::label::variation, language), 10.0f, juce::Colour(0xffded4be));
        variationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeaderTip, language));
        addAndMakeVisible(variationLabel);
        pulseVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPulse, language));
        pulseVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPulse, language));
        pulseVariation.onClick = [this] { applyVariation(Variation::pulse); };
        porousVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPorous, language));
        porousVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPorous, language));
        porousVariation.onClick = [this] { applyVariation(Variation::porous); };
        heterodyneVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationHeterodyne, language));
        heterodyneVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeterodyne, language));
        heterodyneVariation.onClick = [this] { applyVariation(Variation::heterodyne); };
        randomizeStepsButton.setButtonText("RND 16");
        randomizeStepsButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::randomizeSteps, language));
        randomizeStepsButton.onClick = [this] { fifth.randomizeSteps(); refreshStepControls(); };
        orbitVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationOrbit, language));
        orbitVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationOrbit, language));
        orbitVariation.onClick = [this] { applyVariation(Variation::orbit); };
        pendulumVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPendulum, language));
        pendulumVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPendulum, language));
        pendulumVariation.onClick = [this] { applyVariation(Variation::pendulum); };
        for (auto* button : { &pulseVariation, &porousVariation, &heterodyneVariation,
                              &randomizeStepsButton, &orbitVariation, &pendulumVariation })
            button->setLookAndFeel(&panelButtonLookClone());
        addAndMakeVisible(pulseVariation); addAndMakeVisible(porousVariation); addAndMakeVisible(heterodyneVariation);
        addAndMakeVisible(randomizeStepsButton); addAndMakeVisible(orbitVariation); addAndMakeVisible(pendulumVariation);

        // Text/colour now match PRINCIPAL's own connectionLabel (author,
        // live: "faça uma auditoria e altere tudo que estiver diferente
        // entre as abas") - this used red/no count suffix here only.
        configureLabel(feedbackLabel, antitotem::ui::text(antitotem::ui::label::feedbackPorts, language), 10.0f, juce::Colour(0xffded4be));
        feedbackLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackHeaderTip, language));
        addAndMakeVisible(feedbackLabel);
        constexpr std::array<const char*, 6> feedbackNames { "FB", "DIODE", "CAP", "PULSE", "TRANS", "REFLUX" };
        auto updateFeedbackRoutes = [this]
        {
            unsigned char routes = 0;
            for (std::size_t i = 0; i < feedbackButtons.size(); ++i)
                if (feedbackButtons[i].getToggleState()) routes |= static_cast<unsigned char>(1U << i);
            fifth.setFeedbackConnections(routes);
        };
        for (std::size_t i = 0; i < feedbackButtons.size(); ++i)
        {
            feedbackButtons[i].setButtonText(feedbackNames[i]);
            feedbackButtons[i].setComponentID("feedback");
            feedbackButtons[i].setLookAndFeel(&patchToggleLookClone());
            feedbackButtons[i].onClick = updateFeedbackRoutes;
            addAndMakeVisible(feedbackButtons[i]);
        }
        feedbackButtons[2].setToggleState(true, juce::dontSendNotification);
        updateFeedbackRoutes();
        // "FB GAIN" label + colour, matching the main tab's own feedbackGain
        // slider - this one had neither before (no label at all, default
        // slider colour instead of returnPath/red).
        configureLabel(feedbackGainLabel, "FB GAIN", 10.0f, cloneMaterial::returnPath);
        feedbackGainLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackGain, language));
        addAndMakeVisible(feedbackGainLabel);
        feedbackGain.setSliderStyle(juce::Slider::LinearHorizontal);
        // NoTextBox, not TextBoxRight (18 ago. 2026, author: "não precisa
        // mais de caixa de numero nos sliders da coluna da esquerda").
        feedbackGain.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        feedbackGain.setColour(juce::Slider::thumbColourId, cloneMaterial::returnPath);
        feedbackGain.setColour(juce::Slider::trackColourId, cloneMaterial::returnPath.darker(0.70f));
        feedbackGain.setRange(0.0, 0.72, 0.01);
        feedbackGain.setValue(0.26);
        feedbackGain.onValueChange = [this] { fifth.setFeedbackAmount(static_cast<float>(feedbackGain.getValue())); };
        addAndMakeVisible(feedbackGain);

        // DERIVA: own phrase memory for this object, separate from the main
        // tab's (own derivationCv/Amp/Fx/Ratios, own topology memory, own
        // RNG state) - CLONE has its own steps/oscillators/routes to derive,
        // sharing the main tab's copy would drift the wrong object's memory.
        // Now has the same "DERIVA · PROFUNDIDADE" caption PRINCIPAL shows
        // (author, live: "copie toda a configuração de layout dos objetos
        // da coluna da esquerda da aba principal e altere os da aba clone
        // com as mesmas configurações") - used to skip it on the
        // assumption the button's own text already said enough.
        // Title and slider now match the DERIVA button's own colour (18
        // ago. 2026, author: "Titulo e slider DERIVA-PROFUNDIDADE passam
        // a ter as mesmas cores que o botão deriva") - was memory (pink),
        // now clock (green), the button's own resting-state colour (see
        // PatchToggleLook::drawToggleButton's `isDeriva` branch - it
        // turns red only once engaged, but that's a dynamic two-state
        // paint override, not something a plain Label/Slider colour can
        // follow, so the static "at rest" colour is what's shared here).
        configureLabel(deriveLabel, antitotem::ui::text(antitotem::ui::label::driftDepthLabel, language), 10.0f, cloneMaterial::clock);
        deriveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveHeaderTip, language));
        addAndMakeVisible(deriveLabel);
        deriveButton.setButtonText(antitotem::ui::text(antitotem::ui::label::drift, language));
        deriveButton.setComponentID("derive");
        deriveButton.setClickingTogglesState(true);
        deriveButton.setLookAndFeel(&patchToggleLookClone());
        deriveButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveButton, language));
        deriveButton.onClick = [this] { if (deriveButton.getToggleState()) captureDerivationMemory(); };
        addAndMakeVisible(deriveButton);
        // Camadas de deriva (ver derivationLayers's próprio comentário de
        // membro) - multi-select real, mesmo mecanismo dos botões de
        // modo do VCF (setClickingTogglesState, SEM setRadioGroupId).
        {
            constexpr std::array<const char*, 4> derivationLayerNames { "A", "B", "C", "AUTO" };
            for (std::size_t i = 0; i < derivationLayers.size(); ++i)
            {
                derivationLayers[i].setButtonText(derivationLayerNames[i]);
                derivationLayers[i].setComponentID("core");
                derivationLayers[i].setClickingTogglesState(true);
                // AUTO (índice 3) começa desligado - a configuração
                // padrão continua sendo A/B/C, não o modo novo (autor:
                // "sem destruir também o que já temos").
                derivationLayers[i].setToggleState(i != 3, juce::dontSendNotification);
                derivationLayers[i].setLookAndFeel(&patchToggleLookClone());
                derivationLayers[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::derivationLayer, language));
                addAndMakeVisible(derivationLayers[i]);
            }
        }
        // Participação por título (20 ago. 2026) - ver o comentário
        // completo no próprio membro `participateSteps` etc.
        {
            const std::array<juce::ToggleButton*, 20> participationToggles {
                &participateSteps, &participateVoice, &participateEffects, &participateDetail,
                &participateMixer, &participateEnvelope, &participateModulation, &participateGroove,
                &participateFilter, &participateMetric, &participateTemporal, &participateNoiseColour,
                &participateLoopEnd, &participateRoutes, &participateMixMemory,
                &participateMaterial, &participateChaos, &participateMat, &participateLfoShape, &participateClock
            };
            for (auto* toggle : participationToggles)
            {
                toggle->setButtonText("");
                toggle->setComponentID("core");
                toggle->setClickingTogglesState(true);
                toggle->setToggleState(true, juce::dontSendNotification);
                toggle->setLookAndFeel(&patchToggleLookClone());
                toggle->setTooltip(antitotem::ui::text(antitotem::ui::tooltip::derivationParticipation, language));
                addAndMakeVisible(*toggle);
            }
        }
        deriveDepth.setSliderStyle(juce::Slider::LinearHorizontal);
        // NoTextBox, not TextBoxRight (18 ago. 2026, author: "não precisa
        // mais de caixa de numero nos sliders da coluna da esquerda") -
        // same treatment as the CV steps' own green sliders earlier this
        // turn; the freed width lets the track run further right too.
        deriveDepth.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        deriveDepth.setRange(0.0, 1.0, 0.01);
        deriveDepth.setValue(0.46);
        deriveDepth.setColour(juce::Slider::thumbColourId, cloneMaterial::clock);
        deriveDepth.setColour(juce::Slider::trackColourId, cloneMaterial::clock.darker(0.70f));
        deriveDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::driftDepth, language));
        addAndMakeVisible(deriveDepth);

        // Same size/colour as ADSR (18 ago. 2026, author: "o titulo MIXER
        // também merece ficar com mesmo tamanho e cor que titulo ADSR") -
        // was smaller and the muted board gold instead of the brighter
        // section-title yellow.
        configureLabel(mixerLabel, utf8("MIXER"), 14.0f, juce::Colour(0xffffca5c));
        addAndMakeVisible(mixerLabel);
        // Same MEMÓRIA MIX the main tab has (M1-M4 + CAPTURAR) - OBJETO 5
        // had 4 channel strips but no way to snapshot/recall them as a set.
        configureLabel(mixMemoryLabel, antitotem::ui::text(antitotem::ui::label::mixMemory, language), 10.0f, juce::Colour(0xff8f856f));
        mixMemoryLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixMemoryHeaderTip, language));
        addAndMakeVisible(mixMemoryLabel);
        mixMemoryCapture.setButtonText(antitotem::ui::text(antitotem::ui::button::capture, language));
        mixMemoryCapture.setClickingTogglesState(true);
        mixMemoryCapture.setComponentID("mute");
        mixMemoryCapture.setLookAndFeel(&panelButtonLookClone());
        mixMemoryCapture.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixCapture, language));
        addAndMakeVisible(mixMemoryCapture);
        for (std::size_t i = 0; i < mixMemorySlots.size(); ++i)
        {
            mixMemorySlots[i].setButtonText("M" + juce::String(i + 1));
            mixMemorySlots[i].setLookAndFeel(&panelButtonLookClone());
            mixMemorySlots[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixSlotRecall, language));
            mixMemorySlots[i].onClick = [this, i]
            {
                if (mixMemoryCapture.getToggleState())
                {
                    fifth.captureMixMemory(i);
                    mixMemoryCaptured[i] = true;
                    mixMemoryCapture.setToggleState(false, juce::dontSendNotification);
                }
                else
                {
                    fifth.recallMixMemory(i);
                    for (std::size_t channel = 0; channel < mixGain.size(); ++channel)
                    {
                        const auto recalled = fifth.getMixChannel(channel);
                        mixGain[channel].setValue(recalled.gain, juce::dontSendNotification);
                        mixPan[channel].setValue(recalled.pan, juce::dontSendNotification);
                        mixReflux[channel].setValue(recalled.reflux, juce::dontSendNotification);
                        mixEnable[channel].setToggleState(recalled.enabled, juce::dontSendNotification);
                        mixMute[channel].setToggleState(recalled.mute, juce::dontSendNotification);
                        mixSolo[channel].setToggleState(recalled.solo, juce::dontSendNotification);
                    }
                }
            };
            addAndMakeVisible(mixMemorySlots[i]);
        }
        // "NOISE", matching the term used everywhere else (NOISE title,
        // NOISE MIX rail slider, tooltips) - author, live: "em alguns
        // lugares diz noise em outros ruido, verifiar isso" / picked
        // "NOISE em tudo". Was "RUÍDO" here only.
        const std::array<juce::String, 4> mixerNames { antitotem::ui::text(antitotem::ui::label::mixerChannelNames[0], language), antitotem::ui::text(antitotem::ui::label::mixerChannelNames[1], language), antitotem::ui::text(antitotem::ui::label::mixerChannelNames[2], language), antitotem::ui::text(antitotem::ui::label::mixerChannelNames[3], language) };
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            // 11.0f, matching PRINCIPAL's own size (author, live: "rever
            // os tamanho das fontes do titulo Mixer e a abaixo do mixer...
            // há diferenças entre as abas") - was 9.0f here only.
            configureLabel(mixLabels[i], mixerNames[i], 11.0f, cloneMaterial::metal);
            mixGain[i].setSliderStyle(juce::Slider::LinearVertical);
            // TextBoxBelow, matching the main tab's own mixGain - this is
            // the one mixer slider with a numeric readout (mixPan/mixReflux
            // stay NoTextBox in both tabs), not an oversight to leave blank.
            mixGain[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
            mixGain[i].setRange(0.0, 1.5, 0.01);
            // 0.6 pro NOISE (índice 2), 1.0 pros outros três - see the
            // matching mixerGainDefaults comment in MainComponent's own
            // constructor for the full context (19 ago. 2026, "toma
            // conta do áudio").
            mixGain[i].setValue(i == 2 ? 0.6 : 1.0);
            mixGain[i].setColour(juce::Slider::thumbColourId, cloneMaterial::voice); mixGain[i].setColour(juce::Slider::trackColourId, cloneMaterial::voice.darker(0.72f));
            mixPan[i].setSliderStyle(juce::Slider::LinearHorizontal);
            mixPan[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            mixPan[i].setRange(-1.0, 1.0, 0.01);
            mixPan[i].setColour(juce::Slider::thumbColourId, cloneMaterial::clock); mixPan[i].setColour(juce::Slider::trackColourId, cloneMaterial::clock.darker(0.72f));
            mixReflux[i].setSliderStyle(juce::Slider::LinearHorizontal);
            mixReflux[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            mixReflux[i].setRange(0.0, 0.72, 0.01);
            mixReflux[i].setColour(juce::Slider::thumbColourId, cloneMaterial::returnPath); mixReflux[i].setColour(juce::Slider::trackColourId, cloneMaterial::returnPath.darker(0.72f));
            mixEnable[i].setButtonText("ON"); mixEnable[i].setComponentID("core");
            mixEnable[i].setToggleState(i == 0 || i == 3, juce::dontSendNotification);
            // FILTER/RING sit in series (RING feeds FILTER, FILTER feeds
            // ESPAÇO) - explained here since turning ON off only removes
            // that channel's own contribution, not its processing further
            // down the chain (see TAREFAS.md, 18 ago. 2026).
            if (i == 0) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterChannelSeries, language));
            else if (i == 1) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::ringChannelSeries, language));
            else if (i == 2) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseChannelSeries, language));
            else if (i == 3) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::spaceChannelSeries, language));
            mixMute[i].setButtonText("M"); mixMute[i].setComponentID("mute");
            mixMute[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixChannelMute, language));
            mixSolo[i].setButtonText("S"); mixSolo[i].setComponentID("loop");
            mixSolo[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixChannelSolo, language));
            for (auto* button : { &mixEnable[i], &mixMute[i], &mixSolo[i] }) button->setLookAndFeel(&patchToggleLookClone());
            addAndMakeVisible(mixLabels[i]); addAndMakeVisible(mixGain[i]); addAndMakeVisible(mixPan[i]); addAndMakeVisible(mixReflux[i]);
            addAndMakeVisible(mixEnable[i]); addAndMakeVisible(mixMute[i]); addAndMakeVisible(mixSolo[i]);
        }
        auto updateMixerChannel = [this] (std::size_t i)
        {
            antitotem::MutableMixer::Channel channel;
            channel.gain = static_cast<float>(mixGain[i].getValue());
            channel.pan = static_cast<float>(mixPan[i].getValue());
            channel.reflux = static_cast<float>(mixReflux[i].getValue());
            channel.enabled = mixEnable[i].getToggleState();
            channel.mute = mixMute[i].getToggleState();
            channel.solo = mixSolo[i].getToggleState();
            fifth.setMixChannel(i, channel);
        };
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            for (auto* slider : { &mixGain[i], &mixPan[i], &mixReflux[i] }) slider->onValueChange = [this, i, updateMixerChannel] { updateMixerChannel(i); };
            for (auto* button : { &mixEnable[i], &mixMute[i], &mixSolo[i] }) button->onClick = [this, i, updateMixerChannel] { updateMixerChannel(i); };
            updateMixerChannel(i);
        }

        // CONEXÃO ENTRE OBJETOS moved to MainComponent, fixed above LOG,
        // regardless of which body (PRINCIPAL/CLONE) is showing - it
        // describes the relationship between the two objects, not a
        // property of either one alone. See MainComponent's own
        // connectionLabel/gainToFifth/etc.
        // Only needed so DERIVA can see the running/step-position signal
        // (fifth.getCurrentStep()/isRunning()) to know when a phrase has
        // looped back to step 0 - this tab had no timer at all before.
        startTimerHz(30);
        setSize(1860, 924);
        // noiseSelector/steps are members, already default-constructed
        // (English) before this constructor body ran - correct them to
        // the language actually requested at construction (see
        // `language(initialLanguage)` above).
        noiseSelector.setLanguage(language);
        for (auto& step : steps) step.setLanguage(language);
    }
    ~ObjectFiveComponent() override
    {
        juce::Desktop::getInstance().removeFocusChangeListener(this);
        for (auto& button : filterModeButtons) button.setLookAndFeel(nullptr);
        for (auto* button : { &pulseVariation, &porousVariation, &heterodyneVariation,
                              &randomizeStepsButton, &orbitVariation, &pendulumVariation })
            button->setLookAndFeel(nullptr);
        for (auto& slot : mixMemorySlots) slot.setLookAndFeel(nullptr);
        mixMemoryCapture.setLookAndFeel(nullptr);
        for (auto& button : temporalButtons) button.setLookAndFeel(nullptr);
        for (auto& button : metricButtons) button.setLookAndFeel(nullptr);
        for (auto& button : scannerButtons) button.setLookAndFeel(nullptr);
        for (auto& button : loopSwitches) button.setLookAndFeel(nullptr);
        for (auto& button : lfoShapeButtons) button.setLookAndFeel(nullptr);
        lfoFreeze.setLookAndFeel(nullptr);
        for (auto& button : feedbackButtons) button.setLookAndFeel(nullptr);
        deriveButton.setLookAndFeel(nullptr);
        for (auto& button : mixEnable) button.setLookAndFeel(nullptr);
        for (auto& button : mixMute) button.setLookAndFeel(nullptr);
        for (auto& button : mixSolo) button.setLookAndFeel(nullptr);
        for (auto& button : coreSwitches) button.setLookAndFeel(nullptr);
    }

    // Was noiseSelector/steps only, on the theory that CLONE's ~40 other
    // labels/buttons/tooltips could stay "correct as of when this body was
    // last built" - wrong in practice (author, live, 15 ago. 2026: "no
    // modo clone as mudanças não estão acontecendo corretamente"). Mirrors
    // MainComponent::refreshLanguageTexts() below, same reasoning: every
    // line here is the exact call already made once during construction,
    // safe to repeat verbatim.
    // Same pointer-to-member-callback pattern as AppInfoWindow's own
    // languageCallback() - the caller assigns through the returned
    // pointer, keeping onExplain itself private.
    std::function<void(const juce::String&)>* explainCallback() { return &onExplain; }
    void setLanguage(antitotem::ui::Language newLanguage)
    {
        language = newLanguage;
        noiseSelector.setLanguage(language);
        for (auto& step : steps) step.setLanguage(language);
        configureLabel(heading, antitotem::ui::text(antitotem::ui::label::cloneHeading, language), 15.0f, cloneMaterial::board);
        configureLabel(voiceLabel, antitotem::ui::text(antitotem::ui::label::oscHeaderTitle, language), 15.0f, juce::Colour(0xffffca5c));
        voiceLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::voiceHeaderTip, language));
        configureLabel(filterLabel, antitotem::ui::text(antitotem::ui::label::vcfMultimode, language), 14.0f, juce::Colour(0xffffca5c));
        filterLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::vcfHeaderTip, language));
        configureLabel(energyLabel, antitotem::ui::text(antitotem::ui::label::energy, language), 9.0f, juce::Colour(0xffded4be));
        // configureLabel() resets justification to centredLeft every
        // call - re-applied here too, not just the constructor, same
        // fix as oscillatorShapeLabels/PanCaptions/etc above.
        energyLabel.setJustificationType(juce::Justification::centred);
        energy.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::energy, language));
        configureLabel(modeLabel, antitotem::ui::text(antitotem::ui::label::modeClone, language), 11.0f, cloneMaterial::board);
        clockRate.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockRateKnob, language));
        configureLabel(temporalLabel, antitotem::ui::text(antitotem::ui::label::pulse, language), 10.0f, juce::Colour(0xffded4be));
        temporalLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::temporalHeaderTip, language));
        configureLabel(grooveLabel, antitotem::ui::text(antitotem::ui::label::groove, language), 10.0f, juce::Colour(0xffded4be));
        grooveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, language));
        grooveAmount.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, language));
        configureLabel(metricLabel, antitotem::ui::text(antitotem::ui::label::meter, language), 10.0f, juce::Colour(0xffded4be));
        metricLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricHeaderTip, language));
        configureLabel(scannerLabel, antitotem::ui::text(antitotem::ui::label::path, language), 10.0f, juce::Colour(0xffded4be));
        scannerLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerHeaderTip, language));
        loopLabel.setText(antitotem::ui::text(antitotem::ui::label::loopEndPrefix, language) + juce::String(static_cast<int>(fifth.getLoopEnd())) + antitotem::ui::text(antitotem::ui::label::loopEndSuffix, language), juce::dontSendNotification);
        for (auto& button : loopSwitches) button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::loopEnd, language));
        for (std::size_t i = 0; i < temporalButtons.size(); ++i) temporalButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockFeelTips[i], language));
        for (auto& button : metricButtons) button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricButton, language));
        for (std::size_t i = 0; i < scannerButtons.size(); ++i) scannerButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerTips[i], language));
        configureLabel(modulationLabel, antitotem::ui::text(antitotem::ui::label::modulation, language), 14.0f, juce::Colour(0xffffca5c));
        modulationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::modulationHeaderTip, language));
        configureLabel(lfoShapeLabel, antitotem::ui::text(antitotem::ui::label::lfoShape, language), 12.0f, cloneMaterial::clock);
        for (std::size_t i = 0; i < lfoShapeButtons.size(); ++i) lfoShapeButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoShapeTips[i], language));
        lfoFreeze.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoFreeze, language));
        // Not the shared section-title gold (0xffffca5c) anymore (18 ago.
        // 2026, author: "o titulo espaço/fase não deve ficar amarelo, pois
        // o PARAMETROS é amarelo") - ESPAÇO/FASE sits directly under
        // PARÂMETROS in the rails band, so sharing that exact colour read
        // as a collision, unlike MODULAÇÃO/VCF/ADSR elsewhere in the panel
        // which aren't physically adjacent to PARÂMETROS. Uses REVERB's own
        // new colour (0xff3fb0a8) - same pattern as CAOS/MATÉRIA, where the
        // column heading matches its own content's colour.
        configureLabel(effectsLabel, antitotem::ui::text(antitotem::ui::label::spacePhase, language), 10.0f, cloneMaterial::phaser);
        effectsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::effectsHeaderTip, language));
        configureLabel(detailLabel, antitotem::ui::text(antitotem::ui::label::activeRoutes, language), 10.0f, cloneMaterial::memory);
        detailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailHeaderTip, language));
        configureLabel(parametersLabel, antitotem::ui::text(antitotem::ui::label::parametersRail, language), 14.0f, juce::Colour(0xffffca5c));
        parametersLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::parametersHeaderTip, language));
        configureLabel(materialRailLabel, antitotem::ui::text(antitotem::ui::label::materialRail, language), 10.0f, juce::Colour(0xff8f856f));
        materialRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialHeaderTip, language));
        configureLabel(chaosRailLabel, antitotem::ui::text(antitotem::ui::label::chaosRail, language), 10.0f, cloneMaterial::clock);
        chaosRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::chaosHeaderTip, language));
        configureLabel(variationLabel, antitotem::ui::text(antitotem::ui::label::variation, language), 10.0f, juce::Colour(0xffded4be));
        variationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeaderTip, language));
        pulseVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPulse, language));
        pulseVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPulse, language));
        porousVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPorous, language));
        porousVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPorous, language));
        heterodyneVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationHeterodyne, language));
        heterodyneVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeterodyne, language));
        randomizeStepsButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::randomizeSteps, language));
        orbitVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationOrbit, language));
        orbitVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationOrbit, language));
        pendulumVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPendulum, language));
        pendulumVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPendulum, language));
        configureLabel(feedbackLabel, antitotem::ui::text(antitotem::ui::label::feedbackPorts, language), 10.0f, juce::Colour(0xffded4be));
        feedbackLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackHeaderTip, language));
        for (std::size_t i = 0; i < feedbackButtons.size(); ++i)
            feedbackButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackDoorTips[i], language));
        feedbackGain.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackGain, language));
        configureLabel(deriveLabel, antitotem::ui::text(antitotem::ui::label::driftDepthLabel, language), 10.0f, cloneMaterial::clock);
        deriveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveHeaderTip, language));
        deriveButton.setButtonText(antitotem::ui::text(antitotem::ui::label::drift, language));
        deriveButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveButton, language));
        deriveButton.setLookAndFeel(&patchToggleLookClone());
        deriveDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::driftDepth, language));
        configureLabel(mixMemoryLabel, antitotem::ui::text(antitotem::ui::label::mixMemory, language), 10.0f, juce::Colour(0xff8f856f));
        mixMemoryLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixMemoryHeaderTip, language));
        mixMemoryCapture.setButtonText(antitotem::ui::text(antitotem::ui::button::capture, language));
        mixMemoryCapture.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixCapture, language));
        for (auto& slot : mixMemorySlots) slot.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixSlotRecall, language));
        for (std::size_t i = 0; i < coreSwitches.size(); ++i) coreSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::coreTips[i], language));
        filterCutoff.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterCutoff, language));
        filterResonance.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterResonance, language));
        filterDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterCvDepth, language));
        for (auto& button : filterModeButtons) button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterMode, language));
        materialFilterMix.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialFilterMix, language));
        envelopeControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeAttack, language));
        envelopeControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeDecay, language));
        envelopeControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeSustain, language));
        envelopeControls[3].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeRelease, language));
        modulationControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfo, language));
        modulationControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::ring, language));
        modulationControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseMod, language));
        effectControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::reverb, language));
        effectControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::phaser, language));
        effectControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::flanger, language));
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            mixGain[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelGain, language));
            mixPan[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelPan, language));
            mixReflux[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelReturn, language));
            if (i == 0) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterChannelSeries, language));
            else if (i == 1) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::ringChannelSeries, language));
            else if (i == 2) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseChannelSeries, language));
            else if (i == 3) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::spaceChannelSeries, language));
            mixMute[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixChannelMute, language));
            mixSolo[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixChannelSolo, language));
        }
        const std::array<juce::String, 4> mixerNamesRefresh {
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[0], language),
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[1], language),
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[2], language),
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[3], language)
        };
        for (std::size_t i = 0; i < mixLabels.size(); ++i) configureLabel(mixLabels[i], mixerNamesRefresh[i], 11.0f, cloneMaterial::metal);
        const std::array<juce::String, 16> detailNamesRefresh { "S&H RATE", "RVB RET", "PHS RATE", "PHS PROF", "FLG RATE", "FLG PROF",
            "RES MIX", antitotem::ui::text(antitotem::ui::label::resPitch, language), antitotem::ui::text(antitotem::ui::label::resBody, language),
            "CUTOFF", "RESON", "DRIVE", "ASYM",
            "DRIVE", "DAMPING", "DEPTH" };
        for (std::size_t i = 0; i < detailLabels.size(); ++i)
        {
            configureLabel(detailLabels[i], detailNamesRefresh[i], 9.0f,
                            i >= 9 && i <= 12 ? juce::Colour(0xff8f856f) : (i >= 13 ? cloneMaterial::clock : cloneMaterial::memory));
            detailControls[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailControlTips[i], language));
        }
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
        {
            oscillatorRates[i].setTooltip(i == 3 ? antitotem::ui::text(antitotem::ui::tooltip::osc4Freq, language)
                                         : i == 4 ? antitotem::ui::text(antitotem::ui::tooltip::osc5Freq, language)
                                                   : antitotem::ui::text(antitotem::ui::tooltip::oscFreqGeneric, language));
            oscillators[i].setTooltip(i == 4 ? antitotem::ui::text(antitotem::ui::tooltip::mixRingProduct, language)
                                              : antitotem::ui::text(antitotem::ui::tooltip::mixGeneric, language));
            configureLabel(oscillatorShapeLabels[i], antitotem::ui::text(antitotem::ui::label::shape, language), 9.0f, juce::Colour(0xff8f856f));
            // configureLabel() always resets justification to
            // centredLeft - re-applying centred here too, not just in
            // the constructor, or a language switch quietly knocks this
            // caption off-centre again (author, live, reported more than
            // once: "titulo form do knob form do oscilador fora do
            // centro"). rateLabels/levelLabels never had this override
            // in the first place, which is why only FORM was affected.
            oscillatorShapeLabels[i].setJustificationType(juce::Justification::centred);
            configureLabel(oscillatorPanCaptions[i], antitotem::ui::text(antitotem::ui::label::axisX, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorPanCaptions[i].setJustificationType(juce::Justification::centred);
            oscillatorProximities[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisYShort, language));
            configureLabel(oscillatorProximityCaptions[i], antitotem::ui::text(antitotem::ui::label::axisY, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorProximityCaptions[i].setJustificationType(juce::Justification::centred);
            oscillatorOrbits[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisZShort, language));
            configureLabel(oscillatorOrbitCaptions[i], antitotem::ui::text(antitotem::ui::label::axisZ, language), 9.0f, juce::Colour(0xff8f856f));
            oscillatorOrbitCaptions[i].setJustificationType(juce::Justification::centred);
        }
        // Last, after every tooltip above has already been re-localized -
        // if this tab's own window is what's currently under the mouse,
        // refresh the shared LEARN box (via onExplain) in the new
        // language instead of leaving stale-language text on screen.
        // No "else idle" branch here: MainComponent owns that box, and
        // resetting it to idle text just because nothing happens to be
        // hovered in THIS window right now would stomp on whatever
        // MainComponent's own window is legitimately showing.
        if (auto* under = getComponentAt(getMouseXYRelative()))
            explainHovered(under);
    }

    // LEARN: this tab's own always-on hover/focus listening, feeding
    // MainComponent's single shared box via `onExplain` instead of
    // keeping a box of its own (author, live: "no segundo monitor não é
    // necessário repetir a caixa learn... fica somente na aba principal
    // (funcionando para as duas abas) e dois monitores"). Listening has
    // to stay local to this instance even so: a MouseListener installed
    // on MainComponent never sees mouse activity in the standalone
    // second-monitor ObjectFiveWindow's own, completely separate
    // top-level window - each ObjectFiveComponent installing its own
    // listener on itself is what makes both the embedded (clonePanel)
    // and standalone cases work with the same code, no cross-window
    // wiring beyond the callback itself.
    void mouseEnter(const juce::MouseEvent& event) override
    {
        explainHovered(event.originalComponent);
    }
    void globalFocusChanged(juce::Component* focusedComponent) override
    {
        explainHovered(focusedComponent);
    }
    // Falls back to the idle text when nothing along the ancestor chain
    // has a tooltip, instead of silently leaving whatever was explained
    // last on screen - many controls still have no tooltip::* entry of
    // their own (author, live: "vários knobs não tem conteúdo no learn" /
    // "passo o mouse no phs rate e aparece eje Y"): stale leftover text
    // from the last successfully-explained hover read as a wrong answer
    // for the new control, not as "nothing to say here".
    void explainHovered(juce::Component* component)
    {
        while (component != nullptr && component != this)
        {
            if (auto* tooltipClient = dynamic_cast<juce::TooltipClient*>(component))
            {
                const auto text = tooltipClient->getTooltip();
                if (text.isNotEmpty())
                {
                    if (onExplain) onExplain(text);
                    return;
                }
            }
            component = component->getParentComponent();
        }
        if (onExplain) onExplain(antitotem::ui::text(antitotem::ui::tooltip::learnPanelIdle, language));
    }
    void paint(juce::Graphics& g) override
    {
        // Embedded (clonePanel): MainComponent::paint() already fills the
        // whole window, draws the outer border (colour/thickness already
        // signal PRINCIPAL vs CLONE) and the same trace lines, once, for
        // the full window - painting all of that again here would just be
        // a redundant second frame, not a visual improvement.
        if (! embedded)
        {
            // Same treatment as the main tab's own background (fill, rounded
            // border, vertical grid lines) - this used to be its own distinct
            // look (a flat amber left accent bar, sharp-cornered border), which
            // read as a different app rather than the same instrument's second
            // object.
            // Flat fill again - the gradient test (option 1) didn't land, back
            // to the plain colour the author already liked. Same reddish tint
            // the embedded single-window case uses (MainComponent::paint()) -
            // this standalone window (2 MONITORES mode) is always CLONE,
            // unconditionally, so no showingCloneBody check is needed here
            // the way MainComponent's own version has. Missing this entirely
            // was a real gap, not deliberate (author, live: "a cor não
            // funciona no modo 2 monitores, fez isso de proposito né?") - the
            // accent colours (buttons/sliders) already used cloneMaterial::
            // regardless of embedded, only this background fill had been
            // left as the old flat colour.
            g.fillAll(juce::Colour(0xff171511).interpolatedWith(cloneMaterial::cloneBodyTint, 0.06f));
            g.setColour(juce::Colour(0xff665b49)); g.drawRoundedRectangle(getLocalBounds().reduced(13).toFloat(), 9.0f, 1.2f);
            // Option 2: uneven opacity and start height per line instead of a
            // uniform row of identical hairlines - reads closer to traces on a
            // board than a mechanical ruler.
            int lineIndex = 0;
            for (int x = 185; x < getWidth() - 154; x += 92)
            {
                const auto opacity = lineIndex % 3 == 0 ? 1.0f : (lineIndex % 3 == 1 ? 0.5f : 0.75f);
                const auto topOffset = static_cast<float>(lineIndex % 2) * 16.0f;
                g.setColour(juce::Colour(0xff302b23).withAlpha(opacity));
                g.drawVerticalLine(x, 60.0f + topOffset, static_cast<float>(getHeight() - 28));
                ++lineIndex;
            }
        }
        // Didactic backing panel behind CAOS/VAGA/FRZ (18 ago. 2026,
        // author: "hávera um fundo destacado em alguma cor delimitando
        // didaticamente a atuação desses 3 botões") - drawn unconditionally
        // regardless of embedded, but after the fillAll() above (found
        // live: fillAll wipes out anything painted earlier in the same
        // call - when embedded, MainComponent's own fillAll already ran
        // before this child's paint(), so no conflict there either way).
        // Gated on the CAOS button's own visibility, not a page/embedding
        // flag by name - draws exactly when the buttons it decorates are
        // actually shown.
        // No stroked border (18 ago. 2026, author: "talvez baste eliminar
        // a borda do fundo") - a hard outline drew attention to the
        // padding itself; a soft fill alone reads as closer-fitting even
        // at the same margin.
        if (! chaosFreezeHighlight.isEmpty() && lfoShapeButtons[3].isVisible())
        {
            // Same colour as the CAOS rail column's own sliders (18 ago.
            // 2026, author: "se os sliders da coluna caos se relacionam
            // somente com os tres botoes CAOS, VAGA e FREEZE é melhor que
            // o fundo desses botões tenha a mesma cor que os sliders") -
            // was the neutral board colour, tying the highlight to those
            // 3 buttons specifically instead of to the DRIVE/DAMPING/DEPTH
            // parameters that only apply while one of them is selected.
            g.setColour(cloneMaterial::clock.withAlpha(0.28f));
            g.fillRoundedRectangle(chaosFreezeHighlight.toFloat(), 5.0f);
        }
    }
    // Same "highlight the knob that starts silent" logic as MainComponent.
    static void updateSilentHighlight(juce::Slider& slider, juce::Colour normalColour)
    {
        slider.setColour(juce::Slider::thumbColourId, slider.getValue() <= 0.0005 ? cloneMaterial::board.brighter(0.35f) : normalColour);
    }
    // Rotary knobs (the oscillator MIX knobs) carry no explicit colour of
    // their own - they use the LookAndFeel default. removeColour() reverts
    // to exactly that once raised above 0.
    static void updateSilentHighlightDefault(juce::Slider& slider)
    {
        if (slider.getValue() <= 0.0005) slider.setColour(juce::Slider::thumbColourId, cloneMaterial::board.brighter(0.35f));
        else slider.removeColour(juce::Slider::thumbColourId);
    }
    // Same pattern as MainComponent's own syncCV/syncStepDynamics: push
    // every step's current UI values into the engine at once, rather than
    // wiring a per-slider onValueChange straight to the engine.
    void syncStepControls()
    {
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            fifth.setStepVoltage(i, static_cast<float>(steps[i].cv.getValue()));
            fifth.setStepLevel(i, static_cast<float>(steps[i].level.getValue()));
            fifth.setStepEffectSend(i, static_cast<float>(steps[i].send.getValue()));
            fifth.setStepMuted(i, steps[i].mute.getToggleState());
        }
    }
    void syncRatios()
    {
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
            fifth.setOscillatorRatio(i, static_cast<float>(oscillatorRates[i].getValue()));
    }
    void syncEffects()
    {
        fifth.setReverbMix(static_cast<float>(effectControls[0].getValue())); updateSilentHighlight(effectControls[0], cloneMaterial::phaser);
        fifth.setPhaserMix(static_cast<float>(effectControls[1].getValue())); updateSilentHighlight(effectControls[1], cloneMaterial::phaser);
        fifth.setFlangerMix(static_cast<float>(effectControls[2].getValue())); updateSilentHighlight(effectControls[2], cloneMaterial::phaser);
    }
    // Pulls the engine's own step state into the UI - the opposite
    // direction from syncStepControls() - needed after applyVariation()
    // calls an antitotem::variations:: preset, which writes steps straight
    // to fifth without going through these sliders at all.
    void refreshStepControls()
    {
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            steps[i].cv.setValue(fifth.getStepVoltage(i), juce::dontSendNotification);
            steps[i].level.setValue(fifth.getStepLevel(i), juce::dontSendNotification);
            steps[i].send.setValue(fifth.getStepEffectSend(i), juce::dontSendNotification);
            steps[i].mute.setToggleState(fifth.isStepMuted(i), juce::dontSendNotification);
        }
    }
    enum class Variation { pulse, porous, heterodyne, orbit, pendulum };
    // Same 6 presets as MainComponent's own applyVariation, targeting
    // fifth instead - the antitotem::variations:: functions are already
    // generic over SimpleSequencer&, so only the UI-widget sync here is
    // CLONE-specific, not the underlying preset logic.
    void applyVariation(Variation variation)
    {
        switch (variation)
        {
            case Variation::pulse:
                antitotem::variations::pulseAndGates(fifth);
                clockRate.setValue(3.6, juce::dontSendNotification); energy.setValue(0.78, juce::dontSendNotification);
                coreSwitches[0].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(8); setFeedbackRoutes(0x09U); feedbackGain.setValue(0.30, juce::dontSendNotification);
                modulationControls[1].setValue(0.12, juce::dontSendNotification); modulationControls[2].setValue(0.0, juce::dontSendNotification);
                effectControls[0].setValue(0.0, juce::dontSendNotification); effectControls[1].setValue(0.0, juce::dontSendNotification); effectControls[2].setValue(0.0, juce::dontSendNotification);
                noiseSelector.setSampleHold(false, false); fifth.setSampleHoldMix(0.0f);
                break;
            case Variation::porous:
                antitotem::variations::porousMemory(fifth);
                clockRate.setValue(1.25, juce::dontSendNotification); energy.setValue(0.52, juce::dontSendNotification);
                coreSwitches[2].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x24U); feedbackGain.setValue(0.42, juce::dontSendNotification);
                modulationControls[1].setValue(0.0, juce::dontSendNotification); modulationControls[2].setValue(0.15, juce::dontSendNotification);
                effectControls[0].setValue(0.38, juce::dontSendNotification); effectControls[1].setValue(0.16, juce::dontSendNotification); effectControls[2].setValue(0.08, juce::dontSendNotification);
                noiseSelector.setSampleHold(true, false); fifth.setSampleHoldMix(0.78f);
                noiseSelector.select(1, false); fifth.setNoiseColour(antitotem::NoisePalette::Colour::pink);
                break;
            case Variation::heterodyne:
                antitotem::variations::heterodyneField(fifth);
                clockRate.setValue(4.8, juce::dontSendNotification); energy.setValue(0.86, juce::dontSendNotification);
                coreSwitches[1].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x1aU); feedbackGain.setValue(0.56, juce::dontSendNotification);
                modulationControls[1].setValue(0.36, juce::dontSendNotification); modulationControls[2].setValue(0.08, juce::dontSendNotification);
                effectControls[0].setValue(0.12, juce::dontSendNotification); effectControls[1].setValue(0.34, juce::dontSendNotification); effectControls[2].setValue(0.22, juce::dontSendNotification);
                noiseSelector.setSampleHold(true, false); fifth.setSampleHoldMix(0.78f);
                noiseSelector.select(5, false); fifth.setNoiseColour(antitotem::NoisePalette::Colour::bit);
                break;
            case Variation::orbit:
                antitotem::variations::orbitAndDrift(fifth);
                clockRate.setValue(0.9, juce::dontSendNotification); energy.setValue(0.58, juce::dontSendNotification);
                coreSwitches[1].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x01U); feedbackGain.setValue(0.18, juce::dontSendNotification);
                modulationControls[1].setValue(0.0, juce::dontSendNotification); modulationControls[2].setValue(0.0, juce::dontSendNotification);
                effectControls[0].setValue(0.42, juce::dontSendNotification); effectControls[1].setValue(0.1, juce::dontSendNotification); effectControls[2].setValue(0.0, juce::dontSendNotification);
                {
                    constexpr std::array<float, 5> proximity { 0.35f, 0.55f, 0.45f, 0.65f, 0.5f };
                    constexpr std::array<float, 5> orbit { 0.6f, 0.4f, 0.7f, 0.5f, 0.65f };
                    for (std::size_t i = 0; i < proximity.size(); ++i)
                    {
                        oscillatorProximities[i].setValue(proximity[i], juce::dontSendNotification);
                        oscillatorOrbits[i].setValue(orbit[i], juce::dontSendNotification);
                    }
                }
                noiseSelector.setSampleHold(false, false); fifth.setSampleHoldMix(0.0f);
                break;
            case Variation::pendulum:
                antitotem::variations::pendulumResonance(fifth);
                clockRate.setValue(2.4, juce::dontSendNotification); energy.setValue(0.7, juce::dontSendNotification);
                coreSwitches[0].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x08U); feedbackGain.setValue(0.4, juce::dontSendNotification);
                scannerButtons[2].setToggleState(true, juce::dontSendNotification);
                modulationControls[1].setValue(0.0, juce::dontSendNotification); modulationControls[2].setValue(0.0, juce::dontSendNotification);
                effectControls[0].setValue(0.08, juce::dontSendNotification); effectControls[1].setValue(0.05, juce::dontSendNotification); effectControls[2].setValue(0.0, juce::dontSendNotification);
                detailControls[6].setValue(0.68, juce::dontSendNotification); detailControls[7].setValue(0.62, juce::dontSendNotification); detailControls[8].setValue(0.75, juce::dontSendNotification);
                noiseSelector.setSampleHold(false, false); fifth.setSampleHoldMix(0.0f);
                break;
        }
        fifth.setFeedbackAmount(static_cast<float>(feedbackGain.getValue()));
        refreshStepControls();
        syncRatios();
        syncEffects();
        auto updateDetails = [this]
        {
            const auto v = [this] (std::size_t index) { return static_cast<float>(detailControls[index].getValue()); };
            fifth.setSampleHoldRate(0.4f * std::pow(2.0f, v(0) * 10.0f));
            fifth.setReverbFeedback(v(1) * 0.8f);
            fifth.setPhaserRate(0.04f * std::pow(2.0f, v(2) * 8.0f)); fifth.setPhaserDepth(v(3));
            fifth.setFlangerRate(0.04f * std::pow(2.0f, v(4) * 7.0f)); fifth.setFlangerDepth(v(5));
            fifth.setResonatorMix(v(6)); fifth.setResonatorPitch(v(7)); fifth.setResonatorDamping(v(8));
        };
        updateDetails();
        updateSilentHighlight(modulationControls[1], cloneMaterial::controlBlue); updateSilentHighlight(modulationControls[2], cloneMaterial::noiseSend);
        updateSilentHighlight(detailControls[6], cloneMaterial::memory);
    }
    [[nodiscard]] unsigned char currentFeedbackRoutes() const noexcept
    {
        unsigned char routes = 0;
        for (std::size_t i = 0; i < feedbackButtons.size(); ++i)
            if (feedbackButtons[i].getToggleState()) routes |= static_cast<unsigned char>(1U << i);
        return routes;
    }
    void setFeedbackRoutes(unsigned char routes)
    {
        for (std::size_t i = 0; i < feedbackButtons.size(); ++i)
            feedbackButtons[i].setToggleState((routes & static_cast<unsigned char>(1U << i)) != 0, juce::dontSendNotification);
        fifth.setFeedbackConnections(routes);
    }
    // Same pattern as MainComponent's own setLoopEnd - CLONE has its own
    // 16 steps and its own independent PERCURSO/CLOCK, so it needs its own
    // loop end point too, not a mirror of the main tab's.
    void setLoopEnd(std::size_t end)
    {
        const auto bounded = std::clamp(end, std::size_t { 1 }, loopSwitches.size());
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
            loopSwitches[i].setToggleState(i + 1 == bounded, juce::dontSendNotification);
        fifth.setLoopEnd(bounded);
        loopLabel.setText(antitotem::ui::text(antitotem::ui::label::loopEndPrefix, language) + juce::String(static_cast<int>(bounded)) + antitotem::ui::text(antitotem::ui::label::loopEndSuffix, language), juce::dontSendNotification);
    }
    // Same xorshift as MainComponent's own nextDerivationUnit - deterministic
    // and repeatable from derivationState, never a raw audio-rate source.
    [[nodiscard]] float nextDerivationUnit() noexcept
    {
        derivationState ^= derivationState << 13U;
        derivationState ^= derivationState >> 17U;
        derivationState ^= derivationState << 5U;
        return static_cast<float>(derivationState & 0x00ffffffU) / static_cast<float>(0x00ffffffU);
    }
    void captureDerivationMemory()
    {
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            derivationCv[i] = static_cast<float>(steps[i].cv.getValue());
            derivationAmp[i] = static_cast<float>(steps[i].level.getValue());
            derivationFx[i] = static_cast<float>(steps[i].send.getValue());
        }
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
            derivationRatios[i] = static_cast<float>(oscillatorRates[i].getValue());
        for (std::size_t i = 0; i < detailControls.size(); ++i)
            derivationDetail[i] = static_cast<float>(detailControls[i].getValue());
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            derivationMixGain[i] = static_cast<float>(mixGain[i].getValue());
            derivationMixPan[i] = static_cast<float>(mixPan[i].getValue());
            derivationMixReflux[i] = static_cast<float>(mixReflux[i].getValue());
        }
        for (std::size_t i = 0; i < effectControls.size(); ++i)
            derivationEffects[i] = static_cast<float>(effectControls[i].getValue());
        for (std::size_t i = 0; i < envelopeControls.size(); ++i)
            derivationEnvelope[i] = static_cast<float>(envelopeControls[i].getValue());
        for (std::size_t i = 0; i < oscillatorPans.size(); ++i)
            derivationPans[i] = static_cast<float>(oscillatorPans[i].getValue());
        derivationLfo = static_cast<float>(modulationControls[0].getValue());
        derivationNoiseMix = static_cast<float>(modulationControls[2].getValue());
        derivationGroove = static_cast<float>(grooveAmount.getValue());
        derivationFilterCutoff = static_cast<float>(filterCutoff.getValue());
        derivationFilterResonance = static_cast<float>(filterResonance.getValue());
        hungerCv.fill(0.0f); hungerAmp.fill(0.0f); hungerFx.fill(0.0f);
        hungerRatios.fill(0.0f); hungerEffects.fill(0.0f); hungerEnvelope.fill(0.0f);
        hungerLfo = hungerNoiseMix = hungerGroove = hungerFilterCutoff = hungerFilterResonance = hungerFilterMode = hungerCore = hungerMixMemory = 0.0f;
        hungerPans.fill(0.0f); hungerDetail.fill(0.0f);
        hungerMixGain.fill(0.0f); hungerMixPan.fill(0.0f); hungerMixReflux.fill(0.0f);
        hungerMetric = hungerTemporal = hungerNoiseColour = hungerLoopEnd = 0.0f;
        derivationRing = static_cast<float>(modulationControls[1].getValue());
        hungerRing = 0.0f;
        derivationMat = static_cast<float>(materialFilterMix.getValue());
        hungerMat = 0.0f;
        hungerLfoShape = 0.0f;
        derivationClock = static_cast<float>(clockRate.getValue());
        hungerClock = 0.0f;
        topologyMemory.fill(currentFeedbackRoutes());
        topologyWrite = 0;
        derivationPhrase = 0;
        derivationMotion = 0.0f;
        derivationMotionB = 0.0f;
        derivationMotionC = 0.0f;
        derivationAnchors = { 0, -1 };
        derivationAnchorWrite = 0;
        lastDerivationStep = fifth.getCurrentStep();
    }
    // Autonomia por item (20 ago. 2026, autor: "penso em algo que cada
    // item é autônomo") - decide por conta própria se ESTE slider muda
    // agora, sem saber nada dos outros. Fome (`hunger`) cresce toda vez
    // que o item não age, reseta quando age; tanto a chance de agir
    // quanto o tamanho do salto escalam com a própria fome - um item
    // quieto há muito tempo fica mais provável de agir E dá um salto
    // maior quando finalmente age (o mesmo espírito da recalibração de
    // CV/AMP/FX, mas decidido pelo próprio item, não por uma constante
    // escolhida à mão por bloco). sendNotificationSync (não
    // dontSendNotification) porque praticamente todo slider já tem seu
    // próprio onValueChange que sincroniza sozinho - inclusive em
    // CLONE, onde vários syncs moram em lambdas locais ao construtor
    // não acessíveis por fora (mesmo truque de NOISE COR/ROTAS ATIVAS/
    // mixer, ver PESQUISA_DERIVA_GENERATIVA.md, itens 7-8). Exceção
    // real: ADSR não tem onValueChange nenhum - quem chama isto pro
    // envelope precisa sincronizar manualmente depois (ver o bloco de
    // ADSR dentro do modo AUTO abaixo).
    void driftAutonomousItem(juce::Slider& control, float& memory, float& hunger,
                              float rangeLo, float rangeHi, float widthFactor, float activeDepthValue)
    {
        // Recalibrado (20 ago. 2026, autor: "a variação é sutil") - fome
        // cresce mais rápido (chega no teto em ~13 ciclos, não 20) e o
        // salto em si ficou maior tanto na chance quanto na mistura.
        hunger = std::min(hunger + 0.075f, 1.0f);
        const auto chance = 0.08f + hunger * 0.6f;
        if (nextDerivationUnit() >= chance) return;
        const auto current = static_cast<float>(control.getValue());
        const auto target = std::clamp(memory + (nextDerivationUnit() - 0.5f) * widthFactor * activeDepthValue, rangeLo, rangeHi);
        const auto blend = 0.14f + hunger * 0.6f;
        const auto value = current + (target - current) * blend;
        control.setValue(value, juce::sendNotificationSync);
        memory += (value - memory) * 0.18f;
        hunger = 0.0f;
    }
    // Same algorithm as MainComponent's own deriveFromMemory, applied to
    // this object's own captured memory instead - CLONE has its own steps,
    // oscillators, feedback routes and RNG state to drift, not a mirror of
    // the main tab's.
    void deriveFromMemory()
    {
        const auto userDepth = static_cast<float>(deriveDepth.getValue());
        // Ativação do meta-sequenciador (docs/PESQUISA_SEQUENCER_GENERATIVO.md,
        // seção 7.1 - `StepRule`), 20 ago. 2026, autor: "preciso que todas
        // as implementações tenham aplicações diretas" - `metaSequencerAmount`
        // tinha ficado sem NENHUM chamador (0 pra sempre, as 6 regras
        // não-normais nunca disparavam). Amarrado direto em DERIVA·
        // PROFUNDIDADE em vez de um knob novo - o meta-sequenciador já é
        // conceitualmente uma forma de mutação (disputa o MESMO event
        // budget que DERIVA usa), então reaproveitar o controle que o
        // autor já entende e usa é mais coerente que inventar uma
        // superfície nova só pra isso.
        fifth.setMetaSequencerAmount(userDepth);
        // Noise Field ↔ DERIVA (docs/PESQUISA_DERIVA_GENERATIVA.md, seção
        // 6, item 3 - "dois sistemas de 'campo lento' paralelos e
        // desconhecidos um do outro"), 19 ago. 2026, author confirmou a
        // conexão diretamente ("isso"). Lida ANTES do passo de random
        // walk - um Noise Field já agitado empresta um pouco de energia
        // extra ao próprio passo de deriva (até +0.15 no tamanho do
        // passo), não troca o mecanismo, só o deixa ciente do "clima"
        // que já existe.
        const auto sharedInstability = dualEngine.getInstabilityField();
        derivationMotion = std::clamp(derivationMotion + (nextDerivationUnit() - 0.5f) * (0.16f + userDepth * 0.24f + sharedInstability * 0.15f), -0.55f, 0.55f);
        // Atratores (docs/PESQUISA_DERIVA_GENERATIVA.md, seção 6, item 1
        // - "regiões preferenciais... o sistema pode ficar orbitando
        // esses estados"), 19 ago. 2026. Três regiões nomeadas ao longo
        // do próprio eixo que derivationMotion já percorre: limpo/tonal
        // (-0.4), neutro (0.0), denso/ruidoso (+0.4) - depois do passo
        // de random walk de sempre, um puxão SUAVE (não uma trava) rumo
        // ao atrator mais próximo, então o passeio tende a orbitar entre
        // essas três regiões em vez de vagar uniformemente por todo o
        // intervalo -0.55..0.55. 0.12 é uma força pequena o bastante pra
        // não travar o caráter aleatório que já existia - só inclina a
        // trajetória, não a substitui.
        constexpr std::array<float, 3> derivationAttractors { -0.4f, 0.0f, 0.4f };
        auto nearestAttractor = derivationAttractors[0];
        auto nearestAttractorDistance = std::abs(derivationMotion - nearestAttractor);
        for (const auto attractor : derivationAttractors)
        {
            const auto distance = std::abs(derivationMotion - attractor);
            if (distance < nearestAttractorDistance) { nearestAttractor = attractor; nearestAttractorDistance = distance; }
        }
        derivationMotion = std::clamp(derivationMotion + (nearestAttractor - derivationMotion) * 0.12f, -0.55f, 0.55f);
        // The other half of the coupling above - a DERIVA event that
        // moved a real distance from centre lends a little energy BACK
        // to the shared field (up to +0.011 per event, this only fires
        // once per loop cycle so the cumulative pull is slow, not a
        // sudden spike). Neither system is put in charge of the other -
        // each still runs its own rhythm (DERIVA once per loop, the
        // field continuously every sample with its own elastic pull to
        // 0.2), they just lean on each other a little now.
        dualEngine.nudgeInstability(std::abs(derivationMotion) * 0.02f);
        const auto activeDepth = std::clamp(userDepth * (0.62f + std::abs(derivationMotion)), 0.0f, 1.0f);
        // Instâncias paralelas B e C (ver derivationMotionB/C's próprio
        // comentário de membro) - MESMO mecanismo de A (random walk +
        // atratores), caráter deliberadamente diferente pra ler como
        // processos distintos, não três cópias do mesmo. B: mais lenta/
        // calma (2 atratores, puxão mais forte) - dirige ADSR/LFO/NOISE
        // MIX/GROOVE/filtro. C: mais rápida/inquieta (3 atratores mais
        // largos, puxão mais fraco) - dirige pans dos osciladores/
        // MÉTRICA/SUBDIVISÃO/NOISE COR.
        derivationMotionB = std::clamp(derivationMotionB + (nextDerivationUnit() - 0.5f) * (0.08f + userDepth * 0.12f + sharedInstability * 0.08f), -0.55f, 0.55f);
        {
            constexpr std::array<float, 2> attractorsB { -0.3f, 0.3f };
            auto nearest = attractorsB[0];
            auto nearestDistance = std::abs(derivationMotionB - nearest);
            for (const auto attractor : attractorsB)
            {
                const auto distance = std::abs(derivationMotionB - attractor);
                if (distance < nearestDistance) { nearest = attractor; nearestDistance = distance; }
            }
            derivationMotionB = std::clamp(derivationMotionB + (nearest - derivationMotionB) * 0.18f, -0.55f, 0.55f);
        }
        const auto activeDepthB = std::clamp(userDepth * (0.62f + std::abs(derivationMotionB)), 0.0f, 1.0f);
        derivationMotionC = std::clamp(derivationMotionC + (nextDerivationUnit() - 0.5f) * (0.24f + userDepth * 0.30f + sharedInstability * 0.20f), -0.55f, 0.55f);
        {
            constexpr std::array<float, 3> attractorsC { -0.5f, 0.0f, 0.5f };
            auto nearest = attractorsC[0];
            auto nearestDistance = std::abs(derivationMotionC - nearest);
            for (const auto attractor : attractorsC)
            {
                const auto distance = std::abs(derivationMotionC - attractor);
                if (distance < nearestDistance) { nearest = attractor; nearestDistance = distance; }
            }
            derivationMotionC = std::clamp(derivationMotionC + (nearest - derivationMotionC) * 0.06f, -0.55f, 0.55f);
        }
        const auto activeDepthC = std::clamp(userDepth * (0.62f + std::abs(derivationMotionC)), 0.0f, 1.0f);
        constexpr auto topologyMask = static_cast<unsigned char>(0x3fU);
        const auto historicalRoute = topologyMemory[(topologyWrite + 1U + static_cast<std::size_t>(nextDerivationUnit() * 7.0f)) % topologyMemory.size()];
        auto nextRoute = nextDerivationUnit() < (0.78f - activeDepth * 0.32f) ? historicalRoute : currentFeedbackRoutes();
        float routeDensity = 0.0f;
        // Event budget (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1),
        // 20 ago. 2026, autor: "event budget" - reconfigurar a topologia é
        // a mutação mais "estrutural" que existe aqui (ver a distinção
        // GLITCH×RASGO/RUPTURE já registrada: roteamento é o exemplo
        // canônico de ruptura real, não um glitch pontual), então custa
        // mais (0.35) do que as rodadas de Motion abaixo. `hasEventBudget`
        // gasta ANTES de tentar, não depois - um evento caro que não
        // "coubesse" no orçamento restante simplesmente não acontece essa
        // rodada, volta a poder no próximo compasso (`fifth.
        // getMeasureStepIndex()` envolvendo, ver SimpleSequencer.h).
        if (participateRoutes.getToggleState() && fifth.hasEventBudget(0.35f))
        {
        fifth.spendEventBudget(0.35f);
        // The topology-mutation roll just below is already "a combination
        // of settings given by chance" (its own odds scale with
        // activeDepth, itself derived from DERIVA·PROFUNDIDADE and the
        // running derivationMotion) - CAOS/VAGA reseed automatically
        // whenever this same roll actually mutates the route, rather than
        // on every drift event (17 ago. 2026, author's own idea, live:
        // "não é pra ele se ativar uma vez por loop... que ele fique num
        // modo em que ele se ative aleatoriamente quando o deriva é
        // acionado... quando uma determinada condição de combinação de
        // configurações dadas pelo aleatório aconteça" - reused this
        // existing roll instead of drawing a separate independent one, so
        // the reseed reads as tied to the same event, not a bolted-on
        // coin flip). Replaces the dedicated RSD button removed earlier
        // the same day. Inert (but harmless) unless CAOS/VAGA is the
        // selected shape, same reasoning as FREEZE.
        const auto routeMutates = nextDerivationUnit() < 0.18f + activeDepth * 0.66f;
        if (routeMutates)
        {
            nextRoute ^= static_cast<unsigned char>(1U << static_cast<unsigned int>(nextDerivationUnit() * 6.0f));
            fifth.reseedLfo();
            patchToggleLookClone().reseedFlash = true;
            lfoShapeButtons[3].repaint(); lfoShapeButtons[4].repaint();
            juce::Component::SafePointer<ObjectFiveComponent> safeThis(this);
            juce::Timer::callAfterDelay(220, [safeThis]
            {
                if (safeThis == nullptr) return;
                patchToggleLookClone().reseedFlash = false;
                safeThis->lfoShapeButtons[3].repaint(); safeThis->lfoShapeButtons[4].repaint();
            });
        }
        nextRoute &= topologyMask;
        if (nextRoute == 0) nextRoute = static_cast<unsigned char>(1U << static_cast<unsigned int>(nextDerivationUnit() * 6.0f));
        setFeedbackRoutes(nextRoute);
        topologyMemory[topologyWrite] = nextRoute;
        topologyWrite = (topologyWrite + 1U) % topologyMemory.size();

        unsigned int routeCount = 0;
        for (auto routeBits = static_cast<unsigned int>(nextRoute); routeBits != 0U; routeBits >>= 1U)
            routeCount += routeBits & 1U;
        routeDensity = static_cast<float>(routeCount) / 6.0f;
        const auto currentFeedback = static_cast<float>(feedbackGain.getValue());
        const auto feedbackTarget = 0.14f + routeDensity * (0.12f + activeDepth * 0.24f) + nextDerivationUnit() * (0.03f + activeDepth * 0.11f);
        feedbackGain.setValue(std::clamp(currentFeedback + (feedbackTarget - currentFeedback) * (0.06f + activeDepth * 0.26f), 0.08f, 0.58f), juce::dontSendNotification);
        fifth.setFeedbackAmount(static_cast<float>(feedbackGain.getValue()));
        }

        // AUTO (índice 3, 20 ago. 2026, autor: "penso em algo que cada
        // item é autônomo" / "pode fazer... mas sem destruir também o
        // que já temos que é outra configuração possível") - outra
        // configuração inteira de deriveFromMemory(), não uma variação
        // de Motion A/B/C: aqui nenhum item sabe da existência dos
        // outros, cada um decide sozinho quando age via
        // `driftAutonomousItem()` (fome própria - ver seu comentário
        // acima). A/B/C continuam intactos no ramo `else` logo abaixo -
        // ligar este botão troca de configuração inteira, não mistura
        // as duas.
        //
        // Event budget (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1) -
        // ver o comentário completo no bloco de rotas acima; uma rodada
        // inteira de Motion custa 0.25, por RODADA não por item.
        if (derivationLayers[3].getToggleState() && fifth.hasEventBudget(0.25f))
        {
        fifth.spendEventBudget(0.25f);
if (participateSteps.getToggleState())
                        for (std::size_t i = 0; i < steps.size(); ++i)
            {
                driftAutonomousItem(steps[i].cv, derivationCv[i], hungerCv[i], 0.0f, 1.0f, 0.30f, activeDepth);
                driftAutonomousItem(steps[i].level, derivationAmp[i], hungerAmp[i], 0.0f, 1.0f, 0.24f, activeDepth);
                driftAutonomousItem(steps[i].send, derivationFx[i], hungerFx[i], 0.0f, 1.0f, 0.34f, activeDepth);
            }
            // Reshape ocasional - ver o comentário completo no ramo
            // `else` acima (mesma ideia: o random walk por item aqui
            // preserva a ordem relativa entre passos quase sempre, uma
            // troca de posição embaralha o próprio desenho). Sem checar
            // âncoras - o modo AUTO não tem esse conceito.
            if (participateSteps.getToggleState() && nextDerivationUnit() < 0.03f + activeDepth * 0.55f)
            {
                const auto a = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
                const auto b = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
                if (a != b)
                {
                    std::swap(derivationCv[a], derivationCv[b]);
                    std::swap(derivationAmp[a], derivationAmp[b]);
                    std::swap(derivationFx[a], derivationFx[b]);
                    const auto cvA = static_cast<float>(steps[a].cv.getValue()), cvB = static_cast<float>(steps[b].cv.getValue());
                    const auto ampA = static_cast<float>(steps[a].level.getValue()), ampB = static_cast<float>(steps[b].level.getValue());
                    const auto fxA = static_cast<float>(steps[a].send.getValue()), fxB = static_cast<float>(steps[b].send.getValue());
                    steps[a].cv.setValue(cvB, juce::sendNotificationSync); steps[b].cv.setValue(cvA, juce::sendNotificationSync);
                    steps[a].level.setValue(ampB, juce::sendNotificationSync); steps[b].level.setValue(ampA, juce::sendNotificationSync);
                    steps[a].send.setValue(fxB, juce::sendNotificationSync); steps[b].send.setValue(fxA, juce::sendNotificationSync);
                }
            }
if (participateVoice.getToggleState())
                        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
                driftAutonomousItem(oscillatorRates[i], derivationRatios[i], hungerRatios[i],
                                     static_cast<float>(oscillatorRates[i].getMinimum()), static_cast<float>(oscillatorRates[i].getMaximum()), 0.6f, activeDepth);
            // Botões CORE dos osciladores (radio group, 40106/8038/
            // 4069UB) - salto discreto, mesma fome dos outros toggles.
            hungerCore = std::min(hungerCore + 0.05f, 1.0f);
            if (participateVoice.getToggleState() && nextDerivationUnit() < 0.04f + hungerCore * 0.5f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(coreSwitches.size())) % coreSwitches.size();
                coreSwitches[index].setToggleState(true, juce::sendNotificationSync);
                hungerCore = 0.0f;
            }
            // FORMA LFO (autor, 20 ago. 2026: "faltaram botoes em:
            // ...FORMA LFO...") - radio group de 6, salto discreto.
            hungerLfoShape = std::min(hungerLfoShape + 0.05f, 1.0f);
            if (participateLfoShape.getToggleState() && nextDerivationUnit() < 0.04f + hungerLfoShape * 0.5f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(lfoShapeButtons.size())) % lfoShapeButtons.size();
                lfoShapeButtons[index].setToggleState(true, juce::sendNotificationSync);
                hungerLfoShape = 0.0f;
            }
if (participateEffects.getToggleState())
                        for (std::size_t i = 0; i < effectControls.size(); ++i)
                driftAutonomousItem(effectControls[i], derivationEffects[i], hungerEffects[i], 0.0f, 1.0f, 0.30f, activeDepth);
for (std::size_t i = 0; i < detailControls.size(); ++i)
            {
                // ROTAS ATIVAS (0-8) / MATÉRIA (9-12) / CAOS (13-15) -
                // três títulos visuais distintos sobre o mesmo array
                // (20 ago. 2026, autor: "faltaram botoes em: MATERIA,
                // CAOS...").
                auto& toggle = i < 9 ? participateDetail : (i < 13 ? participateMaterial : participateChaos);
                if (toggle.getToggleState())
                    driftAutonomousItem(detailControls[i], derivationDetail[i], hungerDetail[i], 0.0f, 1.0f, 0.34f, activeDepth);
            }
if (participateMixer.getToggleState())
                        for (std::size_t i = 0; i < mixGain.size(); ++i)
            {
                driftAutonomousItem(mixPan[i], derivationMixPan[i], hungerMixPan[i], -1.0f, 1.0f, 0.6f, activeDepth);
                driftAutonomousItem(mixReflux[i], derivationMixReflux[i], hungerMixReflux[i], 0.0f, 0.72f, 0.3f, activeDepth);
            }
            // M1-4 RECALL (nunca CAPTURE - ver `mixMemoryCaptured`'s
            // próprio comentário de membro). Só recall se o slot
            // sorteado já foi capturado alguma vez - um slot vazio tem
            // `enabled = false` nos 4 canais por padrão e silenciaria o
            // mixer inteiro.
            hungerMixMemory = std::min(hungerMixMemory + 0.05f, 1.0f);
            if (participateMixMemory.getToggleState() && nextDerivationUnit() < 0.03f + hungerMixMemory * 0.35f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(mixMemorySlots.size())) % mixMemorySlots.size();
                if (mixMemoryCaptured[index])
                {
                    fifth.recallMixMemory(index);
                    for (std::size_t channel = 0; channel < mixGain.size(); ++channel)
                    {
                        const auto recalled = fifth.getMixChannel(channel);
                        mixGain[channel].setValue(recalled.gain, juce::dontSendNotification);
                        mixPan[channel].setValue(recalled.pan, juce::dontSendNotification);
                        mixReflux[channel].setValue(recalled.reflux, juce::dontSendNotification);
                        mixEnable[channel].setToggleState(recalled.enabled, juce::dontSendNotification);
                        mixMute[channel].setToggleState(recalled.mute, juce::dontSendNotification);
                        mixSolo[channel].setToggleState(recalled.solo, juce::dontSendNotification);
                        derivationMixGain[channel] = recalled.gain;
                        derivationMixPan[channel] = recalled.pan;
                        derivationMixReflux[channel] = recalled.reflux;
                    }
                    hungerMixMemory = 0.0f;
                }
            }
if (participateVoice.getToggleState())
                        for (std::size_t i = 0; i < oscillatorPans.size(); ++i)
                driftAutonomousItem(oscillatorPans[i], derivationPans[i], hungerPans[i], -1.0f, 1.0f, 0.4f, activeDepth);
if (participateModulation.getToggleState()) {             driftAutonomousItem(modulationControls[0], derivationLfo, hungerLfo, 0.0f, 1.0f, 0.30f, activeDepth);
            fifth.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f)); }
if (participateModulation.getToggleState())             driftAutonomousItem(modulationControls[2], derivationNoiseMix, hungerNoiseMix, 0.0f, 1.0f, 0.30f, activeDepth);
            // RING (autor, 20 ago. 2026: "faltaram botoes em: ...RING"
            // -> "creio que ring já está em modulação") - mesmo toggle
            // de MODULAÇÃO que LFO/NOISE MIX já usam, nunca tinha
            // mecanismo de deriva nenhum antes.
            if (participateModulation.getToggleState())
                driftAutonomousItem(modulationControls[1], derivationRing, hungerRing, 0.0f, 1.0f, 0.30f, activeDepth);
if (participateGroove.getToggleState())             driftAutonomousItem(grooveAmount, derivationGroove, hungerGroove, 0.0f, 1.0f, 0.30f, activeDepth);
            // MAT (autor, 20 ago. 2026: "e MAT" - diferente do rail
            // MATÉRIA, um knob só) e KNOB CLOCK, nunca tinham mecanismo
            // de deriva.
            if (participateMat.getToggleState())
                driftAutonomousItem(materialFilterMix, derivationMat, hungerMat, 0.0f, 1.0f, 0.30f, activeDepth);
            if (participateClock.getToggleState())
                driftAutonomousItem(clockRate, derivationClock, hungerClock,
                                     static_cast<float>(clockRate.getMinimum()), static_cast<float>(clockRate.getMaximum()), 0.6f, activeDepth);
if (participateFilter.getToggleState()) {             driftAutonomousItem(filterCutoff, derivationFilterCutoff, hungerFilterCutoff, 0.0f, 1.0f, 0.30f, activeDepth);
            driftAutonomousItem(filterResonance, derivationFilterResonance, hungerFilterResonance, 0.0f, 1.0f, 0.30f, activeDepth); }
            // Botões do VCF (filterModeButtons) - independentes (multi-
            // select), mesmo mecanismo de INVERTER um sorteado que
            // CONEXÕES ENTRE OBJETOS já usa pros toggles de rota.
            hungerFilterMode = std::min(hungerFilterMode + 0.05f, 1.0f);
            if (participateFilter.getToggleState() && nextDerivationUnit() < 0.04f + hungerFilterMode * 0.5f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(filterModeButtons.size())) % filterModeButtons.size();
                filterModeButtons[index].setToggleState(! filterModeButtons[index].getToggleState(), juce::sendNotificationSync);
                hungerFilterMode = 0.0f;
            }
            // ADSR não tem onValueChange - sendNotificationSync não
            // sincroniza nada sozinho aqui, precisa do passo manual
            // (mesmo motivo do ramo `else`, ver o comentário original
            // de ADSR/LFO mais abaixo).
if (participateEnvelope.getToggleState()) {             for (std::size_t i = 0; i < envelopeControls.size(); ++i)
                driftAutonomousItem(envelopeControls[i], derivationEnvelope[i], hungerEnvelope[i], 0.0f, 1.0f, 0.30f, activeDepth);
            fifth.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue()));
            fifth.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue()));
            fifth.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue()));
            fifth.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue())); }
            // Botões (MÉTRICA/SUBDIVISÃO/NOISE COR) - mesma fome, só que
            // decidindo um salto discreto em vez de um blend contínuo.
            hungerMetric = std::min(hungerMetric + 0.05f, 1.0f);
            if (participateMetric.getToggleState() && nextDerivationUnit() < 0.03f + hungerMetric * 0.4f)
            {
                metricSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
                metricButtons[static_cast<std::size_t>(metricSelection)].setToggleState(true, juce::dontSendNotification);
                hungerMetric = 0.0f;
            }
            hungerTemporal = std::min(hungerTemporal + 0.05f, 1.0f);
            if (participateTemporal.getToggleState() && nextDerivationUnit() < 0.03f + hungerTemporal * 0.4f)
            {
                temporalSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
                temporalButtons[static_cast<std::size_t>(temporalSelection)].setToggleState(true, juce::dontSendNotification);
                hungerTemporal = 0.0f;
            }
            {
                using Feel = antitotem::SimpleSequencer::ClockFeel;
                constexpr std::array<Feel, 8> feels { Feel::straight, Feel::triplet, Feel::quintuplet, Feel::swing,
                                                       Feel::septuplet, Feel::nonuplet, Feel::undecuplet, Feel::glitch };
                constexpr std::array<unsigned int, 8> beats { 2, 3, 4, 5, 6, 7, 8, 9 };
                constexpr std::array<unsigned int, 8> units { 4, 4, 4, 4, 4, 4, 4, 4 };
                fifth.setClockFeel(feels[static_cast<std::size_t>(temporalSelection)]);
                fifth.setMetric(beats[static_cast<std::size_t>(metricSelection)], units[static_cast<std::size_t>(metricSelection)]);
            }
            hungerNoiseColour = std::min(hungerNoiseColour + 0.05f, 1.0f);
            if (participateNoiseColour.getToggleState() && nextDerivationUnit() < 0.03f + hungerNoiseColour * 0.4f)
            {
                noiseSelector.select(static_cast<int>(nextDerivationUnit() * 6.0f) % 6, true);
                hungerNoiseColour = 0.0f;
            }
            // FIM DO LOOP (20 ago. 2026, autor: "sliders vermelhos do
            // mixer... FIM DO LOOP... cada um a sua maneira" - pedido
            // original, só implementado agora) - mesma fome, mesmo
            // salto discreto que MÉTRICA/SUBDIVISÃO/NOISE COR.
            hungerLoopEnd = std::min(hungerLoopEnd + 0.05f, 1.0f);
            if (participateLoopEnd.getToggleState() && nextDerivationUnit() < 0.03f + hungerLoopEnd * 0.4f)
            {
                // Nunca 1 (20 ago. 2026, autor: "travou no 1 do fim do loop, tanto
                // no clone como no principal") - com loopEnd=1 o playhead
                // nunca sai do passo 0, então a condição de disparo de
                // deriveFromMemory() (`active == 0 && lastDerivationStep
                // != 0`) nunca mais fica verdadeira - um deadlock que a
                // própria DERIVA causava em si mesma. Faixa [2,16] em vez
                // de [1,16] garante que o playhead sempre tem que sair do
                // 0 antes de poder voltar pra ele.
                setLoopEnd(static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(loopSwitches.size() - 1)) + 2);
                hungerLoopEnd = 0.0f;
            }
        }
        else
        {
        // Camada A (botão VCF "A", docs/PESQUISA_DERIVA_GENERATIVA.md,
        // seção 6, item "Camadas combináveis") - desligar o botão para
        // esta cópia (PRINCIPAL/CLONE são independentes) congela
        // passos/osciladores/FX de Motion A sem afetar B ou C.
        // Event budget (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1) -
        // A/B/C são combináveis (blocos `if` sequenciais, não else-if), e
        // por isso competem de verdade pelo MESMO orçamento se mais de uma
        // estiver ligada - exatamente "disputado entre sequenciadores".
        if (derivationLayers[0].getToggleState() && fifth.hasEventBudget(0.25f))
        {
        fifth.spendEventBudget(0.25f);
        if (participateSteps.getToggleState() && nextDerivationUnit() < 0.85f)
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            // Passo por evento recalibrado (19 ago. 2026, autor: "CV 16
            // steps muda somente o slider verde, bem pouco, o fx e amp
            // não percebo ainda alterações" - CV/AMP/FX usam exatamente
            // a mesma fórmula de `drift` e o mesmo sorteio de chance, e
            // mesmo assim só CV mostrava algo). Causa real: o valor
            // antigo (0.025 a 0.13, vezes activeDepth ~0.3-0.5) resultava
            // num passo absoluto de milésimos por evento - visível só na
            // barra vertical mais alta (CV), invisível nas barras
            // horizontais curtas (AMP/FX). Não é um bug desta rodada, é
            // uma constante antiga nunca recalibrada (ver "Itens ANTIGOS"
            // em PESQUISA_DERIVA_GENERATIVA.md, seção 7) que só ficou
            // evidente agora que o bloco inteiro já não roda em TODO
            // ciclo.
            // Recalibrado de novo (20 ago. 2026, autor: "a variação é
            // sutil", depois do lote de reshape) - mesmo raciocínio de
            // antes (ver comentário de `driftAutonomousItem`), passo
            // ainda maior.
            const auto drift = (0.22f + nextDerivationUnit() * 0.55f) * activeDepth;
            const auto cvTarget = std::clamp(derivationCv[i] + (nextDerivationUnit() - 0.5f) * 0.30f * activeDepth, 0.0f, 1.0f);
            const auto ampTarget = std::clamp(derivationAmp[i] + (nextDerivationUnit() - 0.5f) * 0.24f * activeDepth, 0.12f, 1.0f);
            const auto fxTarget = std::clamp(derivationFx[i] + (nextDerivationUnit() - 0.5f) * 0.34f * activeDepth + routeDensity * 0.08f * activeDepth, 0.0f, 1.0f);
            // Âncoras combináveis (docs/PESQUISA_DERIVA_GENERATIVA.md,
            // seção 6, item 2), 19 ago. 2026 - até 2 passos ao mesmo
            // tempo (`derivationAnchors`, ver seu próprio comentário de
            // membro) nunca derivam de CV, ficam exatamente onde foram
            // capturados, enquanto AMP/FX deles e TUDO dos outros passos
            // continua se movendo normalmente.
            const auto isAnchored = std::find(derivationAnchors.begin(), derivationAnchors.end(), static_cast<int>(i)) != derivationAnchors.end();
            const auto cv = isAnchored ? static_cast<float>(steps[i].cv.getValue())
                                    : static_cast<float>(steps[i].cv.getValue()) + (cvTarget - static_cast<float>(steps[i].cv.getValue())) * drift;
            const auto amp = static_cast<float>(steps[i].level.getValue()) + (ampTarget - static_cast<float>(steps[i].level.getValue())) * drift;
            const auto fx = static_cast<float>(steps[i].send.getValue()) + (fxTarget - static_cast<float>(steps[i].send.getValue())) * drift;
            steps[i].cv.setValue(cv, juce::dontSendNotification);
            steps[i].level.setValue(amp, juce::dontSendNotification);
            steps[i].send.setValue(fx, juce::dontSendNotification);
            derivationCv[i] += (cv - derivationCv[i]) * 0.18f;
            derivationAmp[i] += (amp - derivationAmp[i]) * 0.18f;
            derivationFx[i] += (fx - derivationFx[i]) * 0.18f;
        }
        // Reshape ocasional (20 ago. 2026, autor: "gostaria de mais
        // variação nos sliders do cv 16 steps, eles alteram mas sempre
        // com o mesmo gráfico") - o random walk independente por passo
        // acima preserva a ORDEM relativa entre os passos quase sempre
        // (dois passos raramente se cruzam com um jitter pequeno), por
        // isso os valores mudavam mas o contorno geral do gráfico
        // continuava parecendo o mesmo. Uma troca de posição entre dois
        // passos (CV+AMP+FX juntos, não só CV, pra manter a "voz" de
        // cada passo coerente) embaralha o próprio DESENHO de vez em
        // quando, não só os valores dentro dele - passos ancorados
        // ficam de fora (têm que continuar fixos).
        if (participateSteps.getToggleState() && nextDerivationUnit() < 0.03f + activeDepth * 0.55f)
        {
            const auto a = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
            const auto b = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
            const auto aAnchored = std::find(derivationAnchors.begin(), derivationAnchors.end(), static_cast<int>(a)) != derivationAnchors.end();
            const auto bAnchored = std::find(derivationAnchors.begin(), derivationAnchors.end(), static_cast<int>(b)) != derivationAnchors.end();
            if (a != b && !aAnchored && !bAnchored)
            {
                std::swap(derivationCv[a], derivationCv[b]);
                std::swap(derivationAmp[a], derivationAmp[b]);
                std::swap(derivationFx[a], derivationFx[b]);
                const auto cvA = static_cast<float>(steps[a].cv.getValue()), cvB = static_cast<float>(steps[b].cv.getValue());
                const auto ampA = static_cast<float>(steps[a].level.getValue()), ampB = static_cast<float>(steps[b].level.getValue());
                const auto fxA = static_cast<float>(steps[a].send.getValue()), fxB = static_cast<float>(steps[b].send.getValue());
                steps[a].cv.setValue(cvB, juce::dontSendNotification); steps[b].cv.setValue(cvA, juce::dontSendNotification);
                steps[a].level.setValue(ampB, juce::dontSendNotification); steps[b].level.setValue(ampA, juce::dontSendNotification);
                steps[a].send.setValue(fxB, juce::dontSendNotification); steps[b].send.setValue(fxA, juce::dontSendNotification);
            }
        }
        if (participateVoice.getToggleState() && nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
        {
            const auto exponent = (nextDerivationUnit() - 0.5f) * (0.10f + activeDepth * 0.38f) + derivationMotion * 0.18f;
            const auto target = std::clamp(derivationRatios[i] * std::pow(2.0f, exponent), 0.25f, 4.0f);
            const auto current = static_cast<float>(oscillatorRates[i].getValue());
            const auto ratio = current + (target - current) * (0.09f + activeDepth * 0.32f);
            oscillatorRates[i].setValue(ratio, juce::dontSendNotification);
            derivationRatios[i] += (ratio - derivationRatios[i]) * 0.16f;
        }
        // Botões CORE dos osciladores (20 ago. 2026, autor: "verifique
        // se os 3 botões dos osciladores se conectam a deriva") - radio
        // group (40106/8038/4069UB), nunca participava. Salto discreto,
        // mesmo espírito de MÉTRICA/SUBDIVISÃO.
        if (participateVoice.getToggleState() && nextDerivationUnit() < 0.06f + activeDepth * 0.18f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(coreSwitches.size())) % coreSwitches.size();
            coreSwitches[index].setToggleState(true, juce::sendNotificationSync);
        }
        // FORMA LFO - radio group de 6, salto discreto.
        if (participateLfoShape.getToggleState() && nextDerivationUnit() < 0.06f + activeDepth * 0.18f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(lfoShapeButtons.size())) % lfoShapeButtons.size();
            lfoShapeButtons[index].setToggleState(true, juce::sendNotificationSync);
        }
        if (participateEffects.getToggleState() && nextDerivationUnit() < 0.45f)
        for (std::size_t i = 0; i < effectControls.size(); ++i)
        {
            const auto current = static_cast<float>(effectControls[i].getValue());
            const auto target = std::clamp(routeDensity * (0.18f + 0.12f * static_cast<float>(i)) * activeDepth + nextDerivationUnit() * 0.24f * activeDepth, 0.0f, 0.56f);
            effectControls[i].setValue(current + (target - current) * (0.10f + activeDepth * 0.28f), juce::dontSendNotification);
        }
        // ROTAS ATIVAS/MATÉRIA/CAOS (autor, 19 ago. 2026: "rotas ativas
        // não percebo" / "matéria também não" / "CAOS também não") -
        // mesmo painel (`detailControls[0..15]`: 0-8 S&H/reverb/phaser/
        // flanger/resonador, 9-12 MATÉRIA, 13-15 CAOS/VAGA), nunca tinha
        // participado da DERIVA. sendNotificationSync (não
        // dontSendNotification) porque o sync real mora num lambda local
        // ao construtor (`updateDetails`, não acessível daqui) - deixar o
        // onValueChange de cada slider fazer o trabalho, mesmo truque já
        // usado por NOISE COR (ver comentário mais abaixo).
        if (nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < detailControls.size(); ++i)
        {
            auto& toggle = i < 9 ? participateDetail : (i < 13 ? participateMaterial : participateChaos);
            if (!toggle.getToggleState()) continue;
            const auto current = static_cast<float>(detailControls[i].getValue());
            const auto target = std::clamp(derivationDetail[i] + (nextDerivationUnit() - 0.5f) * 0.34f * activeDepth, 0.0f, 1.0f);
            const auto value = current + (target - current) * (0.12f + activeDepth * 0.35f);
            detailControls[i].setValue(value, juce::sendNotificationSync);
            derivationDetail[i] += (value - derivationDetail[i]) * 0.18f;
        }
        }
        // ADSR e LFO rate (docs/PESQUISA_DERIVA_GENERATIVA.md, "diversos
        // fluxos e controles onde a deriva não atua" - autor, 19 ago.
        // 2026), mesmo padrão do bloco de effectControls acima (um
        // random walk em torno do valor ATUAL, não um alvo absoluto
        // como oscillatorRates/steps têm - ADSR/LFO não têm um "estado
        // original" próprio pra puxar de volta, então derivam livremente
        // dentro de si mesmos). Sem função auxiliar nova - o próprio
        // nextDerivationUnit() já não é compartilhado entre PRINCIPAL/
        // CLONE, uma abstração nova aqui quebraria essa mesma escolha já
        // feita no arquivo.
        // Chance de agir independente por parâmetro (19 ago. 2026, autor:
        // "ainda me dá a impressão que as alterações estão acontecendo
        // todas no mesmo momento" - a velocidade diferente de cada motion
        // não bastava, já que todo bloco aplicava sua mudança em TODO
        // ciclo de deriva, só o tamanho do passo variava). Cada bloco
        // agora só age se um sorteio próprio (chance distinta por
        // parâmetro, não um valor único) permitir - lê como eventos
        // assíncronos de verdade, não um enxame sincronizado.
        // Camada B (botão VCF "B") - envolve ADSR/LFO/NOISE MIX/GROOVE/
        // filtro, o mesmo agrupamento de Motion B.
        // Event budget - ver Camada A acima pro comentário completo.
        if (derivationLayers[1].getToggleState() && fifth.hasEventBudget(0.25f))
        {
        fifth.spendEventBudget(0.25f);
        if (participateEnvelope.getToggleState() && nextDerivationUnit() < 0.55f)
        {
            for (std::size_t i = 0; i < envelopeControls.size(); ++i)
            {
                const auto current = static_cast<float>(envelopeControls[i].getValue());
                const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
                envelopeControls[i].setValue(current + (target - current) * (0.08f + activeDepthB * 0.30f), juce::dontSendNotification);
            }
            fifth.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue()));
            fifth.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue()));
            fifth.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue()));
            fifth.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue()));
        }
        if (participateModulation.getToggleState() && nextDerivationUnit() < 0.65f)
        {
            const auto current = static_cast<float>(modulationControls[0].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            modulationControls[0].setValue(current + (target - current) * (0.07f + activeDepthB * 0.25f), juce::dontSendNotification);
            fifth.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f));
        }
        // NOISE MIX, GROOVE, filtro, pans dos osciladores (autor, 19 ago.
        // 2026: "métrica, noise, groove, subdivisão" / "pans dos
        // osciladores" / "filtro") - mesmo padrão de random walk em
        // torno do valor atual já usado pro ADSR/LFO acima.
        if (participateModulation.getToggleState() && nextDerivationUnit() < 0.6f)
        {
            const auto current = static_cast<float>(modulationControls[2].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            modulationControls[2].setValue(current + (target - current) * (0.08f + activeDepthB * 0.28f), juce::dontSendNotification);
            fifth.setNoiseMix(static_cast<float>(modulationControls[2].getValue()));
        }
        // RING - mesmo toggle de MODULAÇÃO (participateModulation).
        if (participateModulation.getToggleState() && nextDerivationUnit() < 0.6f)
        {
            const auto current = static_cast<float>(modulationControls[1].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            modulationControls[1].setValue(current + (target - current) * (0.08f + activeDepthB * 0.28f), juce::sendNotificationSync);
        }
        if (participateGroove.getToggleState() && nextDerivationUnit() < 0.5f)
        {
            const auto current = static_cast<float>(grooveAmount.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            grooveAmount.setValue(current + (target - current) * (0.09f + activeDepthB * 0.30f), juce::dontSendNotification);
            fifth.setGrooveAmount(static_cast<float>(grooveAmount.getValue()));
        }
        // MAT + KNOB CLOCK (autor, 20 ago. 2026: "e MAT" / "KNOB
        // CLOCK") - nunca tinham mecanismo de deriva.
        if (participateMat.getToggleState() && nextDerivationUnit() < 0.5f)
        {
            const auto current = static_cast<float>(materialFilterMix.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            materialFilterMix.setValue(current + (target - current) * (0.09f + activeDepthB * 0.30f), juce::sendNotificationSync);
        }
        if (participateClock.getToggleState() && nextDerivationUnit() < 0.4f)
        {
            const auto current = static_cast<float>(clockRate.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (clockRate.getMaximum() - clockRate.getMinimum()) * 0.15f,
                                            clockRate.getMinimum(), clockRate.getMaximum());
            clockRate.setValue(current + (target - current) * (0.08f + activeDepthB * 0.24f), juce::sendNotificationSync);
        }
        if (participateFilter.getToggleState() && nextDerivationUnit() < 0.45f)
        {
            const auto current = static_cast<float>(filterCutoff.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            filterCutoff.setValue(current + (target - current) * (0.10f + activeDepthB * 0.32f), juce::dontSendNotification);
            fifth.setFilterCutoff(static_cast<float>(filterCutoff.getValue()));
        }
        if (participateFilter.getToggleState() && nextDerivationUnit() < 0.4f)
        {
            const auto current = static_cast<float>(filterResonance.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            filterResonance.setValue(current + (target - current) * (0.11f + activeDepthB * 0.34f), juce::dontSendNotification);
            fifth.setFilterResonance(static_cast<float>(filterResonance.getValue()));
        }
        // Botões do VCF (20 ago. 2026, autor: "os botões do vcf não
        // estão conectados a deriva") - filterModeButtons (LP/BP/HP/
        // NOTCH combináveis, multi-select) nunca participava, só
        // cutoff/resonância. Mesmo mecanismo de INVERTER um sorteado
        // que CONEXÕES ENTRE OBJETOS já usa pros toggles de rota.
        if (participateFilter.getToggleState() && nextDerivationUnit() < 0.2f + activeDepthB * 0.4f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(filterModeButtons.size())) % filterModeButtons.size();
            filterModeButtons[index].setToggleState(! filterModeButtons[index].getToggleState(), juce::sendNotificationSync);
        }
        // Mixer (sliders vermelhos, autor, 19 ago. 2026: "sliders
        // horizontais do mixer também não") - mixGain (vertical, 0..1.5),
        // mixPan/mixReflux (horizontais, -1..1 / 0..0.72) nunca
        // participavam da DERIVA. Cada faixa mantém seu próprio range de
        // clamp - sem isso mixGain estouraria em 1.0 (perderia metade do
        // seu range) e mixPan/mixReflux ficariam presos no range 0..1
        // errado. sendNotificationSync pelo mesmo motivo do painel ROTAS
        // ATIVAS acima - o sync mora num lambda local ao construtor
        // (`updateMixerChannel`).
        if (participateMixer.getToggleState() && nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            // mixGain (vertical) fica de fora - autor: "No mixer os
            // únicos itens com deriva são os sliders horizontais".
            const auto panCurrent = static_cast<float>(mixPan[i].getValue());
            const auto panTarget = std::clamp(derivationMixPan[i] + (nextDerivationUnit() - 0.5f) * 0.6f * activeDepthB, -1.0f, 1.0f);
            const auto pan = panCurrent + (panTarget - panCurrent) * (0.10f + activeDepthB * 0.30f);
            mixPan[i].setValue(pan, juce::sendNotificationSync);
            derivationMixPan[i] += (pan - derivationMixPan[i]) * 0.18f;

            const auto refluxCurrent = static_cast<float>(mixReflux[i].getValue());
            const auto refluxTarget = std::clamp(derivationMixReflux[i] + (nextDerivationUnit() - 0.5f) * 0.3f * activeDepthB, 0.0f, 0.72f);
            const auto reflux = refluxCurrent + (refluxTarget - refluxCurrent) * (0.10f + activeDepthB * 0.30f);
            mixReflux[i].setValue(reflux, juce::sendNotificationSync);
            derivationMixReflux[i] += (reflux - derivationMixReflux[i]) * 0.18f;
        }
        // M1-4 RECALL (nunca CAPTURE) - ver o comentário completo no
        // modo AUTO acima.
        if (participateMixMemory.getToggleState() && nextDerivationUnit() < 0.05f + activeDepthB * 0.25f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(mixMemorySlots.size())) % mixMemorySlots.size();
            if (mixMemoryCaptured[index])
            {
                fifth.recallMixMemory(index);
                for (std::size_t channel = 0; channel < mixGain.size(); ++channel)
                {
                    const auto recalled = fifth.getMixChannel(channel);
                    mixGain[channel].setValue(recalled.gain, juce::dontSendNotification);
                    mixPan[channel].setValue(recalled.pan, juce::dontSendNotification);
                    mixReflux[channel].setValue(recalled.reflux, juce::dontSendNotification);
                    mixEnable[channel].setToggleState(recalled.enabled, juce::dontSendNotification);
                    mixMute[channel].setToggleState(recalled.mute, juce::dontSendNotification);
                    mixSolo[channel].setToggleState(recalled.solo, juce::dontSendNotification);
                    derivationMixGain[channel] = recalled.gain;
                    derivationMixPan[channel] = recalled.pan;
                    derivationMixReflux[channel] = recalled.reflux;
                }
            }
        }
        }
        // Camada C (botão VCF "C") - pans dos osciladores + MÉTRICA/
        // SUBDIVISÃO/NOISE COR, o mesmo agrupamento de Motion C.
        // Event budget - ver Camada A acima pro comentário completo.
        if (derivationLayers[2].getToggleState() && fifth.hasEventBudget(0.25f))
        {
        fifth.spendEventBudget(0.25f);
        if (participateVoice.getToggleState() && nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < oscillatorPans.size(); ++i)
        {
            // -1..1, not 0..1 - EIXO X's own range (CmosVoice::
            // setOscillatorPan clamps -1..1), unlike every other slider
            // in this function.
            const auto current = static_cast<float>(oscillatorPans[i].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.4f + activeDepthC * 0.6f), -1.0f, 1.0f);
            const auto pan = current + (target - current) * (0.09f + activeDepth * 0.30f);
            oscillatorPans[i].setValue(pan, juce::dontSendNotification);
            fifth.setOscillatorPan(i, pan);
        }
        // MÉTRICA/SUBDIVISÃO: seleções discretas (botões em grupo de
        // rádio), não sliders contínuos - um SALTO probabilístico, mesmo
        // espírito do routeMutates da topologia acima (chance escalada
        // por activeDepth), não um blend. setToggleState(true, ...) já
        // desliga o botão anterior sozinho (mesmo radio group). Tabelas
        // de feels/beats reaproveitadas de updateTemporal() (lambda
        // local ao construtor, não acessível daqui - redeclaradas, não
        // chamadas por referência).
        if (participateMetric.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
        {
            metricSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
            metricButtons[static_cast<std::size_t>(metricSelection)].setToggleState(true, juce::dontSendNotification);
        }
        if (participateTemporal.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
        {
            temporalSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
            temporalButtons[static_cast<std::size_t>(temporalSelection)].setToggleState(true, juce::dontSendNotification);
            // GLT decide a âncora (19 ago. 2026, autor: "no step 1" - a
            // âncora não fica mais sempre no passo 1; "talvez se o
            // glitch decidir?"). Só realoca na TRANSIÇÃO pra GLT (índice
            // 7 de temporalNames), não a cada ciclo enquanto continua
            // em GLT - senão a âncora deixaria de significar algo (um
            // "ponto fixo" que nunca fica quieto não é mais um ponto
            // fixo). Uma causa real e audível pra mudança, não um
            // sorteio silencioso. Ocupa a próxima posição do pool
            // (round-robin) em vez de substituir a única âncora - até 2
            // pontos fixos coexistindo depois de 2 eventos GLT.
            if (temporalSelection == 7)
            {
                derivationAnchors[static_cast<std::size_t>(derivationAnchorWrite)] = static_cast<int>(nextDerivationUnit() * static_cast<float>(steps.size()));
                derivationAnchorWrite = (derivationAnchorWrite + 1) % static_cast<int>(derivationAnchors.size());
            }
        }
        {
            using Feel = antitotem::SimpleSequencer::ClockFeel;
            constexpr std::array<Feel, 8> feels { Feel::straight, Feel::triplet, Feel::quintuplet, Feel::swing,
                                                   Feel::septuplet, Feel::nonuplet, Feel::undecuplet, Feel::glitch };
            constexpr std::array<unsigned int, 8> beats { 2, 3, 4, 5, 6, 7, 8, 9 };
            constexpr std::array<unsigned int, 8> units { 4, 4, 4, 4, 4, 4, 4, 4 };
            fifth.setClockFeel(feels[static_cast<std::size_t>(temporalSelection)]);
            fifth.setMetric(beats[static_cast<std::size_t>(metricSelection)], units[static_cast<std::size_t>(metricSelection)]);
        }
        // NOISE COR (autor, 19 ago. 2026: "noise cor também") - mesma
        // seleção discreta que MÉTRICA/SUBDIVISÃO, mas noiseSelector's
        // própria select(index, true) já dispara onSelection sozinha
        // (chama fifth.setNoiseColour por dentro), então não precisa do
        // passo manual de sync que MÉTRICA/SUBDIVISÃO tiveram que fazer.
        if (participateNoiseColour.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
            noiseSelector.select(static_cast<int>(nextDerivationUnit() * 6.0f) % 6, true);
        // FIM DO LOOP (20 ago. 2026, autor: "sliders vermelhos do
        // mixer... FIM DO LOOP... cada um a sua maneira" - pedido
        // original, só implementado agora) - mesmo salto discreto que
        // MÉTRICA/SUBDIVISÃO/NOISE COR, mesma Motion C.
        if (participateLoopEnd.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
            // Nunca 1 (20 ago. 2026, autor: "travou no 1 do fim do loop, tanto
                // no clone como no principal") - com loopEnd=1 o playhead
                // nunca sai do passo 0, então a condição de disparo de
                // deriveFromMemory() (`active == 0 && lastDerivationStep
                // != 0`) nunca mais fica verdadeira - um deadlock que a
                // própria DERIVA causava em si mesma. Faixa [2,16] em vez
                // de [1,16] garante que o playhead sempre tem que sair do
                // 0 antes de poder voltar pra ele.
                setLoopEnd(static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(loopSwitches.size() - 1)) + 2);
        }
        }
        syncStepControls(); syncRatios(); syncEffects();
        ++derivationPhrase;
    }
    void timerCallback() override
    {
        const auto active = fifth.getCurrentStep();
        if (deriveButton.getToggleState() && fifth.isRunning() && active == 0 && lastDerivationStep != 0)
            deriveFromMemory();
        lastDerivationStep = active;
    }
    void resized() override
    {
        // Same policy as the main window: fits 1920x1080 without scrolling,
        // scroll (via ZoomableObjectFiveViewport's own scrollbars) only
        // kicks in below that - so every row here is deliberately tighter
        // than a first pass would need, not just "however much space is
        // available".
        // Embedded: zero extra inset and no heading row - this must be the
        // exact same rect layoutUnified() receives for PRINCIPAL, or every
        // module below lands a border's width off from its PRINCIPAL
        // counterpart (border colour/thickness and current-tab name are
        // already shown once, by MainComponent's own frame and the
        // PRINCIPAL/CLONE header button - repeating either here just
        // shifts this tab's own layout instead of adding information).
        auto area = embedded ? getLocalBounds() : getLocalBounds().reduced(20);
        if (! embedded)
        {
            heading.setBounds(area.removeFromTop(20));
            area.removeFromTop(4);
        }

        // Same 235/120/6 bottom budget PRINCIPAL's layoutUnified() removes
        // (stepsArea/rails/routing), in the same order, before either side
        // column is carved out - the only way both tabs' voice area below
        // ends up exactly the same fluid height instead of CLONE's own
        // fixed 360px box (see layoutVoiceArea()'s own comment for why
        // that mattered).
        // The 6px gap now sits between rails and stepsArea, not between
        // moduleArea and rails (18 ago. 2026, author: "o último slider da
        // coluna matéria... está muito colado ao CV do Sequencer... subir
        // levemente todo o objeto parametros... sem alterar o tamanho dos
        // osciladores") - MATÉRIA's own 4th slider (ASYM) deliberately
        // overflows below rails' own height into stepsArea (see
        // layoutRailsBand()'s own comment), and that overflow had zero
        // clearance before stepsArea's own content since the only gap in
        // this budget sat on the OTHER side of rails. Moving it here shifts
        // the whole rails band up by the same 6px without taking anything
        // from moduleArea/the oscillators - the total removed from `area`
        // is still exactly 235+6+120, unchanged, just reordered.
        auto stepsArea = area.removeFromBottom(235);
        area.removeFromBottom(6);
        auto rails = area.removeFromBottom(120);

        // A left column, full height, mirroring the main tab's own CLOCK
        // column: CLOCK/PULSO/MÉTRICA/PERCURSO/PORTAS DE FEEDBACK/DERIVA all
        // read as one vertical stack instead of a horizontal strip competing
        // with the oscillators for the same row. OBJETO 5 has no transport
        // of its own (it shares the main engine's clock/play/stop), so
        // unlike the main tab's column there is no PLAY/STOP/REC/RESET,
        // REC TIMERS or FIM DO LOOP here - only the controls that actually
        // exist on this object. DERIVA joined in 12 ago. 2026 - CLOCK gives
        // it the running/step-position signal it needs.
        constexpr int clockColumnWidth = 230;
        // Reconstructed at full height (area's own height plus what was
        // just removed for rails/stepsArea/the routing spacer) - matching
        // PRINCIPAL's own `transport` rectangle exactly. A real bug found
        // here: taking this straight from the already bottom-trimmed
        // `area` (as CLONE used to, back when it split columns before
        // trimming height) silently cut CLOCK's own column ~361px short,
        // clipping DERIVA/VARIAÇÃO off its own bottom.
        // .reduced(4, 2), matching PRINCIPAL's own transport rectangle
        // exactly (clockColumnInset=4, 2 top/bottom) - author, live: "há
        // uma ligeira diferença de posição na coluna da esquerda... acho
        // que há uma diferença de margem superior". Was (4, 0) here only,
        // landing every row 2px higher than PRINCIPAL's.
        auto clockColumn = juce::Rectangle<int>(area.getX(), area.getY(), clockColumnWidth,
            area.getHeight() + rails.getHeight() + stepsArea.getHeight() + 6).reduced(4, 2);
        area.removeFromLeft(clockColumnWidth);
        // 14px breathing room between the columns and the central body -
        // PRINCIPAL now matches this too (see layoutUnified()'s own
        // columnGap).
        constexpr int columnGap = 14;
        area.removeFromLeft(columnGap);
        stepsArea.removeFromLeft(clockColumnWidth + columnGap);

        // A right column, full height, mirroring the main tab's own mixer
        // column: OBJETO 5's MEMÓRIA MIX, 4 channel strips and its ENERGIA
        // knob read as one vertical stack on the right edge instead of a
        // horizontal row competing with the sequencer/field rows for the
        // same space. There is no MASTER or LOG here - OBJETO 5 has
        // neither; only the controls that actually exist on this object.
        // Same full-height reconstruction as clockColumn above.
        constexpr int mixerColumnWidth = 300;
        auto mixerColumn = juce::Rectangle<int>(area.getRight() - mixerColumnWidth, area.getY(), mixerColumnWidth,
            area.getHeight() + rails.getHeight() + stepsArea.getHeight() + 6);
        area.removeFromRight(mixerColumnWidth);
        area.removeFromRight(columnGap);
        stepsArea.removeFromRight(mixerColumnWidth + columnGap);
        mixerLabel.setBounds(mixerColumn.removeFromTop(15));
        // VARIAÇÃO moved to the left column, below DERIVA (same move the
        // main tab already made) - this column opens straight from MIXER
        // into MEMÓRIA MIX now.
        mixerColumn.removeFromTop(4);
        // Channels + MEMÓRIA MIX, shared with PRINCIPAL via
        // layoutMixerChannels() - see that function's own comment for the
        // audit that led to extracting it. CLONE has no LOG, so whatever's
        // left below MEMÓRIA MIX just stays unused, same as before.
        mixerColumn = layoutMixerChannels(mixerColumn, mixLabels, mixEnable, mixMute, mixSolo,
                                           mixGain, mixPan, mixReflux, mixMemoryLabel, mixMemorySlots, mixMemoryCapture);
        // ENERGIA sat alone at the bottom of this column with a large dead
        // gap above it (nothing else needed the column's full height once
        // the 4 channel strips fit inside their own fixed slice) - it now
        // lives beside ADSR instead, in the voice area below.

        // MODULAÇÃO/FORMA LFO/ESPAÇO-FASE/ROTAS ATIVAS moved to a
        // horizontal band above this object's own sequencer (see
        // layoutRailsBand() below, shared with the main tab) - this
        // column ends at MEMÓRIA MIX now.
        layoutTransportColumn(clockColumn, modeLabel, clockLabel, clockRate,
                               temporalLabel, temporalButtons.data(), static_cast<int>(temporalButtons.size()),
                               grooveLabel, grooveAmount,
                               metricLabel, metricButtons.data(), static_cast<int>(metricButtons.size()),
                               scannerLabel, scannerButtons.data(), static_cast<int>(scannerButtons.size()),
                               loopLabel, loopSwitches.data(), static_cast<int>(loopSwitches.size()), 8,
                               feedbackLabel, feedbackButtons.data(), static_cast<int>(feedbackButtons.size()),
                               &feedbackGainLabel, feedbackGain,
                               &deriveLabel, deriveDepth, deriveButton,
                               derivationLayers.data(), static_cast<int>(derivationLayers.size()),
                               variationLabel, pulseVariation, porousVariation, heterodyneVariation,
                               randomizeStepsButton, orbitVariation, pendulumVariation);

        // CONEXÃO ENTRE OBJETOS moved to MainComponent (fixed, above LOG,
        // regardless of which body is showing) - this column ends at
        // VARIAÇÃO now, with whatever's left below unused. MODULAÇÃO/FORMA
        // LFO/ESPAÇO-FASE/ROTAS ATIVAS moved back to the mixer column
        // (tried here, didn't read well).

        // Oscillators/VCF/ADSR/ENERGIA/NOISE, shared with PRINCIPAL via
        // layoutVoiceArea() - see that function's own comment for the
        // audit that led to extracting it (this used to be a fixed
        // 680x360 box here, leaving ~136px of unused width that never
        // reached the mixer column - PRINCIPAL's always-fluid, always-
        // flush version won).
        layoutVoiceArea(area, voiceLabel, coreSwitches,
                         oscillatorLabels, oscillatorRateLabels, oscillatorRates,
                         oscillatorLevelLabels, oscillators, oscillatorShapeLabels, oscillatorShapes,
                         oscillatorPanCaptions, oscillatorPans,
                         oscillatorProximityCaptions, oscillatorProximities,
                         oscillatorOrbitCaptions, oscillatorOrbits,
                         filterLabel, filterModeButtons, filterControlLabels, filterCutoff, filterResonance, filterDepth,
                         materialFilterLabel, materialFilterMix,
                         envelopeLabel, envelopeLabels, envelopeControls,
                         energyLabel, energy, noiseLabel, noiseSelector,
                         modulationLabel, modulationLabels, modulationControls,
                         lfoShapeLabel, lfoShapeButtons.data(), lfoFreeze,
                         chaosFreezeHighlight);

        // FORMA LFO/MODULAÇÃO/ESPAÇO-FASE/ROTAS ATIVAS, same shared
        // layoutRailsBand() the main tab uses - `rails` was carved from
        // the full-width area before the side columns were split off, so
        // it needs the same left/right trim layoutUnified() gives it, not
        // 0/0 (that was only correct while this trimmed the shared `area`
        // itself first, before rails was cut from it).
        layoutRailsBand(rails, clockColumnWidth + columnGap, mixerColumnWidth + columnGap,
                         parametersLabel,
                         effectsLabel, effectLabels.data(), effectControls.data(),
                         detailLabel, detailLabels.data(), detailControls.data(),
                         materialRailLabel, chaosRailLabel);
        // Two rows of 8, not one row of 16: the same organisation the main
        // tab uses for its CV sliders (CV1-8 on top, CV9-16 below) instead
        // of squeezing every step into a single cramped row.
        // Same .reduced(5, 2) margin PRINCIPAL's own stepGrid uses (author,
        // live: "verificar o objeto sequencer, há uma ligeira variação de
        // posição entre as duas abas") - this tab used the raw area
        // without it, landing its whole grid a few px off PRINCIPAL's.
        auto stepGrid = stepsArea.reduced(5, 2);
        stepsLabel.setBounds(stepGrid.removeFromTop(15));
        // Same grid math as the main tab's own step grid (historicalScannerSteps
        // per row, 2 rows) and the same StepControl component, so this reads
        // as the same sequencer, not a simplified stand-in.
        // Limitado (não só dividido) para o grid de 16 passos não inflar
        // indefinidamente numa janela redimensionada bem larga - 162px é o
        // valor natural na resolução alvo do app (1920x1080); 200 dá
        // alguma folga acima disso sem deixar cada célula desproporcional
        // perto de 400px (o que ocorria sem limite, no teto de 3840px).
        const auto stepWidth = std::clamp(stepGrid.getWidth() / static_cast<int>(antitotem::SimpleSequencer::historicalScannerSteps), 100, 200);
        const auto stepRowHeight = stepGrid.getHeight() / 2;
        for (std::size_t i = 0; i < steps.size(); ++i)
            steps[i].setBounds(juce::Rectangle<int>(
                stepGrid.getX() + static_cast<int>(i % antitotem::SimpleSequencer::historicalScannerSteps) * stepWidth,
                stepGrid.getY() + static_cast<int>(i / antitotem::SimpleSequencer::historicalScannerSteps) * stepRowHeight,
                stepWidth, stepRowHeight).reduced(5, 4));

        // NOISE moved next to ADSR/ENERGIA above (matching the main tab's
        // own placement, below ADSR/ENERGIA) - not laid out here anymore.

        // MODULAÇÃO/ESPAÇO-FASE/ROTAS-equivalent controls moved into the
        // right column below the mixer (see below) - not laid out here
        // anymore.

        // PORTAS DE FEEDBACK and FB GAIN are laid out in the left column
        // above, not here - kept there next to PULSO/MÉTRICA/PERCURSO,
        // matching the main tab's own CLOCK column grouping.

        // Channel strips (mixGain/mixPan/mixReflux/mixEnable/mixMute/
        // mixSolo) are laid out in the right column above, not here.

        // CONEXÃO ENTRE OBJETOS moved to MainComponent - not laid out here
        // at all anymore.

        // Participação por título (20 ago. 2026) - pequeno o bastante
        // pra não alterar o layout existente (autor: "acho que o botão
        // pode ser pequeno pra não alterar o layout"), posicionado a
        // partir dos bounds finais de cada label já calculados acima -
        // nenhuma linha de layout existente precisou mudar.
        {
            const std::array<juce::Label*, 20> participationLabels {
                &stepsLabel, &voiceLabel, &effectsLabel, &detailLabel, &mixerLabel, &envelopeLabel,
                &modulationLabel, &grooveLabel, &filterLabel, &metricLabel, &temporalLabel, &noiseLabel,
                &loopLabel, &feedbackLabel, &mixMemoryLabel,
                &materialRailLabel, &chaosRailLabel, &materialFilterLabel, &lfoShapeLabel, &clockLabel
            };
            const std::array<juce::ToggleButton*, 20> participationTogglesForLayout {
                &participateSteps, &participateVoice, &participateEffects, &participateDetail,
                &participateMixer, &participateEnvelope, &participateModulation, &participateGroove,
                &participateFilter, &participateMetric, &participateTemporal, &participateNoiseColour,
                &participateLoopEnd, &participateRoutes, &participateMixMemory,
                &participateMaterial, &participateChaos, &participateMat, &participateLfoShape, &participateClock
            };
            // Perto do título de verdade, não da borda direita do
            // Label (20 ago. 2026, autor: "preciso que os botoes
            // fiquem proximos dos titulos e não afastados") -
            // `getRight()` media a largura TOTAL do componente Label
            // (geralmente bem mais largo que o texto visível, já que
            // várias colunas reservam espaço fixo), não onde o texto
            // realmente termina. Medindo a largura real do texto com a
            // própria fonte do label e encostando logo depois dele.
            // `toFront()` também (autor: "alguns botões não consegui
            // clicar") - construídos cedo no construtor, componentes
            // adicionados DEPOIS ficavam por cima na pilha de z-order e
            // roubavam o clique.
            // Gap por item (20 ago. 2026, autor: "acho que vai ter que
            // analisar item por item" - cada título usa uma fonte/
            // tamanho diferente, um gap único não serve pra todos).
            // Índice bate com `participationLabels`/
            // `participationTogglesForLayout` acima, nessa ordem: 0
            // steps, 1 voice/osc, 2 effects/fx, 3 detail/rotas ativas,
            // 4 mixer, 5 envelope/adsr, 6 modulation, 7 groove, 8
            // filter/vcf, 9 metric, 10 temporal/subdivisão, 11 noise
            // colour, 12 loop end, 13 routes/portas feedback, 14 mix
            // memory, 15 material/matéria, 16 chaos/caos, 17 mat, 18
            // lfo shape/forma lfo, 19 clock. 17px é o padrão herdado do
            // ajuste global anterior - só sobrescrever os índices que o
            // autor reportar como errados.
            // 20 ago. 2026, autor: "conexão entre objetos, portas de
            // feedback e fim do loop (so um pouquinho)" - 12 loop end
            // +5, 13 routes/portas feedback +13 (bem mais largo que os
            // outros).
            // autor, mesmo dia: "clock, MAT e noise são os mais
            // distantes... se pudesse priximar um pouco (3px a 5 px)"
            // - 11 noise colour, 17 mat, 19 clock: 17 -> 13.
            constexpr std::array<int, 20> participationGapPx {
                17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
                17, 13, 22, 30, 17, 17, 17, 13, 17, 13
            };
            for (std::size_t i = 0; i < participationLabels.size(); ++i)
            {
                auto* label = participationLabels[i];
                auto* toggle = participationTogglesForLayout[i];
                const auto textWidth = juce::GlyphArrangement::getStringWidthInt(label->getFont(), label->getText());
                // "o clock ficou do lado esquerdo deslocado" - clockLabel
                // é o único com `Justification::centred` numa caixa BEM
                // mais larga que o texto (a largura do knob inteiro);
                // `getX() + textWidth` assumia alinhamento à esquerda
                // sempre, então acertava todos os outros títulos mas
                // errava esse. Sensível à justificação agora - texto
                // centralizado calcula onde ele de fato termina dentro
                // da caixa, não a partir da borda esquerda dela.
                const auto textEndX = label->getJustificationType().testFlags(juce::Justification::horizontallyCentred)
                                           ? label->getX() + (label->getWidth() + textWidth) / 2
                                           : label->getX() + textWidth;
                toggle->setBounds(textEndX + participationGapPx[i], label->getY() + (label->getHeight() - 10) / 2, 10, 10);
                toggle->toFront(false);
            }
        }
    }

private:
    // Noise Field ↔ DERIVA (19 ago. 2026) - deriveFromMemory() needs the
    // whole engine, not just its own fifth (SimpleSequencer only carries
    // the CURRENT pushed instability value, has no way to read it back
    // out as a getter or to nudge it - the field itself lives in
    // DualObjectEngine). Declared before fifth so initializer-list order
    // (dualEngine, fifth, ...) matches declaration order.
    antitotem::DualObjectEngine& dualEngine;
    antitotem::SimpleSequencer& fifth;
    const bool embedded = false;
    antitotem::ui::Language language = antitotem::ui::Language::english;
    juce::Label heading;
    // Same title text/size/colour as the main tab's own voiceLabel, added
    // for true parity via layoutVoiceArea() - this tab had no equivalent
    // caption above its oscillators before.
    juce::Label voiceLabel;
    std::array<juce::ToggleButton, 3> coreSwitches;
    std::array<juce::Label, 5> oscillatorLabels;
    // Per-knob captions (FREQ/MIX/FORMA/EIXO X) above each control - the
    // main tab has these (oscillatorRateLabels etc.); this tab didn't,
    // which is why the pan sliders in particular read as unlabelled.
    std::array<juce::Label, 5> oscillatorRateLabels, oscillatorLevelLabels, oscillatorShapeLabels, oscillatorPanCaptions;
    // Y (proximity) and Z (orbit) - same per-oscillator pair the main tab
    // has, this tab didn't.
    std::array<juce::Label, 5> oscillatorProximityCaptions, oscillatorOrbitCaptions;
    std::array<juce::Slider, 5> oscillatorRates, oscillators, oscillatorShapes, oscillatorPans, oscillatorProximities, oscillatorOrbits;
    juce::Label filterLabel;
    // Per-knob captions (FREQ/RES/CV) - the main tab has these
    // (filterControlLabels); this tab didn't.
    std::array<juce::Label, 3> filterControlLabels;
    juce::Slider filterCutoff, filterResonance, filterDepth;
    // LPF/BPF/HPF/NCH, independent toggles (17 ago. 2026, multi-select
    // mode mask - see CmosVcf.h's own comment); no filterModeSelection
    // int anymore, the mask is computed straight from these four buttons'
    // toggle states.
    std::array<juce::TextButton, 4> filterModeButtons;
    // MaterialFilter (src/core/MaterialFilter.h) - MIX only, no on/off
    // switch (see the constructor's own comment); CUTOFF/RESONANCE/DRIVE/
    // ASYMMETRY stay at their tested internal defaults for now, no room in
    // the VCF column for four more knobs (17 ago. 2026).
    juce::Label materialFilterLabel;
    juce::Slider materialFilterMix;
    juce::Label envelopeLabel;
    std::array<juce::Label, 4> envelopeLabels;
    std::array<juce::Slider, 4> envelopeControls;
    juce::Label energyLabel;
    juce::Slider energy;
    // "MODO: CLONE" - always this tab's own identity, see
    // layoutTransportColumn()'s own comment.
    juce::Label modeLabel;
    juce::Label clockLabel;
    juce::Slider clockRate;
    juce::Label temporalLabel, metricLabel, scannerLabel;
    std::array<juce::ToggleButton, 4> scannerButtons;
    // 8, not 4 - two rows of 4 (see the array-size comments where
    // beats/units and feels are defined for the full context).
    std::array<juce::ToggleButton, 8> temporalButtons;
    // 0-1, layered on every SUBDIVISÃO feel, not exclusive to SWG (20 ago.
    // 2026, "ao invés de um knob um slider swing", then "deixa o swing
    // somente enquanto botão, e utilise esse slide atual do swing para o
    // groove").
    juce::Label grooveLabel;
    juce::Slider grooveAmount;
    std::array<juce::ToggleButton, 8> metricButtons;
    int temporalSelection = 0, metricSelection = 0, scannerSelection = 0;
    juce::Label loopLabel;
    std::array<juce::ToggleButton, antitotem::SimpleSequencer::stepCount> loopSwitches;
    // LEARN: this instance's own hover/focus explanation is handed to
    // whoever set explainCallback() - MainComponent, into its single
    // shared box (author, live: "no segundo monitor não é necessário
    // repetir a caixa learn... fica somente na aba principal"). See the
    // mouseEnter/globalFocusChanged/explainHovered member comment.
    std::function<void(const juce::String&)> onExplain;
    juce::Label stepsLabel;
    // The same StepControl component the main tab uses (CV + AMP + FX + M
    // + an active-step dot, drawn by StepControl itself) - not a bare CV
    // slider with none of that. There was no room to justify the fuller
    // control before the sequencer only had a thin 150px strip; now it has
    // the rest of the column's height.
    // true: this tab's own steps follow cloneMaterial's hue shift (see
    // StepControl's own constructor comment).
    std::array<StepControl, antitotem::SimpleSequencer::stepCount> steps { StepControl {0, true}, StepControl {1, true}, StepControl {2, true}, StepControl {3, true},
        StepControl {4, true}, StepControl {5, true}, StepControl {6, true}, StepControl {7, true}, StepControl {8, true}, StepControl {9, true}, StepControl {10, true}, StepControl {11, true},
        StepControl {12, true}, StepControl {13, true}, StepControl {14, true}, StepControl {15, true} };
    juce::Label noiseLabel;
    // Same component the main tab uses (hex layout, S&H at the hub) - not
    // a separate plainer 6-button grid with no S&H equivalent. true: follows
    // cloneMaterial's hue shift, same as this tab's StepControl instances
    // above.
    NoiseSelector noiseSelector { true };
    juce::Label modulationLabel;
    std::array<juce::Label, 3> modulationLabels;
    std::array<juce::Slider, 3> modulationControls;
    juce::Label lfoShapeLabel;
    std::array<juce::ToggleButton, 6> lfoShapeButtons;
    juce::Label materialRailLabel;
    juce::Label chaosRailLabel;
    // Umbrella title for the whole rails band (18 ago. 2026) - see
    // layoutRailsBand()'s own comment.
    juce::Label parametersLabel;
    juce::ToggleButton lfoFreeze;
    // Didactic backing panel behind CAOS/VAGA/FRZ (18 ago. 2026) - see
    // layoutVoiceArea()'s own comment. Filled in by that function, drawn
    // in this component's own paint().
    juce::Rectangle<int> chaosFreezeHighlight;
    juce::Label effectsLabel;
    std::array<juce::Label, 3> effectLabels;
    std::array<juce::Slider, 3> effectControls;
    juce::Label detailLabel;
    std::array<juce::Label, 16> detailLabels;
    std::array<juce::Slider, 16> detailControls;
    // Same 6 presets as the main tab's own variation row, operating on
    // fifth instead of sequencer - the engine-level functions in
    // ObjectVariations.h are already generic over SimpleSequencer&.
    juce::Label variationLabel;
    juce::TextButton pulseVariation, porousVariation, heterodyneVariation, randomizeStepsButton, orbitVariation, pendulumVariation;
    juce::Label feedbackLabel;
    std::array<juce::ToggleButton, 6> feedbackButtons;
    juce::Label feedbackGainLabel;
    juce::Slider feedbackGain;
    juce::ToggleButton deriveButton;
    // Camadas de deriva (19 ago. 2026, autor: "os do tipo vcf, no mesmo
    // espaço do botão deriva (independente para principal e clone)") -
    // multi-select real como os botões de modo do VCF (LPF/BPF/HPF/NCH:
    // setClickingTogglesState, sem radio group, cada um liga/desliga
    // sozinho), não um grupo exclusivo. A/B/C correspondem às três
    // "instâncias paralelas" (derivationMotion/B/C) - desligar uma
    // impede aquele grupo de parâmetros de derivar neste ciclo, sem
    // afetar as outras duas. Todas ligadas por padrão (mesmo
    // comportamento de antes destes botões existirem).
    // AUTO (índice 3, 20 ago. 2026) liga um MODO inteiro diferente de
    // deriveFromMemory() - ver seu próprio comentário lá dentro. A/B/C
    // (0/1/2) continuam exatamente como eram, intactos - "sem destruir
    // também o que já temos que é outra configuração possível" (autor).
    std::array<juce::ToggleButton, 4> derivationLayers;
    juce::Slider deriveDepth;
    // Same "DERIVA · PROFUNDIDADE" caption PRINCIPAL shows above its own
    // deriveDepth slider - added for full parity (author, live: "copie
    // toda a configuração de layout dos objetos da coluna da esquerda da
    // aba principal e altere os da aba clone com as mesmas
    // configurações"). Used to pass nullptr here on the assumption the
    // DERIVA button's own text already said enough.
    juce::Label deriveLabel;
    std::array<float, antitotem::SimpleSequencer::stepCount> derivationCv {}, derivationAmp {}, derivationFx {};
    std::array<float, 5> derivationRatios {};
    // Alcance estendido, 19 ago. 2026, autor: "rotas ativas não percebo" /
    // "matéria também não" / "CAOS também não" / "sliders horizontais do
    // mixer também não" - ROTAS ATIVAS/MATÉRIA/CAOS são o mesmo painel
    // (`detailControls[0..15]`, ver seu próprio comentário de membro em
    // ambas as cópias) e nunca tinham participado da DERIVA; mixer
    // (`mixGain`/`mixPan`/`mixReflux`) também não - mesmo padrão de
    // memória por slider que steps/ratios já usam.
    std::array<float, 16> derivationDetail {};
    // Memória própria só pro modo AUTO (20 ago. 2026) - o modo A/B/C
    // nunca precisou de uma âncora capturada pra esses grupos (ADSR/
    // LFO/NOISE MIX/GROOVE/filtro/pans/FX derivam livremente em torno
    // do valor ATUAL, não de um valor capturado - ver o comentário
    // original acima de ADSR/LFO no ramo `else`), mas
    // `driftAutonomousItem()` sempre precisa de uma memória própria por
    // item pra funcionar.
    std::array<float, 3> derivationEffects {};
    std::array<float, 4> derivationEnvelope {};
    float derivationLfo = 0.0f, derivationNoiseMix = 0.0f, derivationGroove = 0.0f, derivationFilterCutoff = 0.0f, derivationFilterResonance = 0.0f;
    std::array<float, 5> derivationPans {};
    std::array<float, 4> derivationMixGain {}, derivationMixPan {}, derivationMixReflux {};
    // AUTO (20 ago. 2026, autor: "penso em algo que cada item é
    // autônomo") - fome por item, ver `driftAutonomousItem()`. Cresce a
    // cada ciclo em que aquele item específico NÃO age, reseta quando
    // age - sem coordenador central, cada slider se revezando sozinho
    // em vez de um Motion A/B/C decidindo por grupos inteiros.
    std::array<float, antitotem::SimpleSequencer::stepCount> hungerCv {}, hungerAmp {}, hungerFx {};
    std::array<float, 5> hungerRatios {};
    std::array<float, 3> hungerEffects {};
    std::array<float, 4> hungerEnvelope {};
    float hungerLfo = 0.0f, hungerNoiseMix = 0.0f, hungerGroove = 0.0f, hungerFilterCutoff = 0.0f, hungerFilterResonance = 0.0f;
    // Botões do VCF/filterModeButtons (20 ago. 2026, autor: "os botões
    // do vcf não estão conectados a deriva") - filterCutoff/
    // filterResonance já participavam, mas o MODO do filtro em si
    // (combinação de LP/BP/HP/NOTCH, `filterModeButtons`, multi-select
    // igual A/B/C/AUTO) nunca tinha entrado. Fome só, sem memória - são
    // toggles, não sliders (mesmo padrão de `hungerObjectRoute`).
    float hungerFilterMode = 0.0f;
    // Botões CORE dos osciladores (20 ago. 2026, autor: "verifique se
    // os 3 botões dos osciladores se conectam a deriva") -
    // `coreSwitches` (40106/8038/4069UB, radio group), nunca
    // participava. Salto discreto, mesmo padrão de MÉTRICA/SUBDIVISÃO.
    float hungerCore = 0.0f;
    // M1-4 (RECALL só, nunca CAPTURE - ver `mixMemoryCaptured`'s próprio
    // comentário de membro).
    float hungerMixMemory = 0.0f;
    std::array<float, 5> hungerPans {};
    std::array<float, 16> hungerDetail {};
    std::array<float, 4> hungerMixGain {}, hungerMixPan {}, hungerMixReflux {};
    float hungerMetric = 0.0f, hungerTemporal = 0.0f, hungerNoiseColour = 0.0f, hungerLoopEnd = 0.0f;
    // RING/MAT/FORMA LFO/KNOB CLOCK (20 ago. 2026) - memória+fome no
    // mesmo padrão de tudo mais.
    float derivationRing = 0.0f, hungerRing = 0.0f;
    float derivationMat = 0.0f, hungerMat = 0.0f;
    float hungerLfoShape = 0.0f;
    float derivationClock = 0.0f, hungerClock = 0.0f;
    float derivationMotion = 0.0f;
    // Instâncias paralelas (19 ago. 2026, autor: "as duas" - confirmando
    // tanto múltiplas âncoras quanto camadas paralelas de deriva depois
    // de brainstormar "níveis de trocas em steps diferentes... steps
    // distintos... instancias paralelas"). derivationMotion (acima)
    // continua o núcleo original (steps CV/AMP/FX, topologia/feedback,
    // razão dos osciladores). B e C são réplicas independentes do MESMO
    // mecanismo (random walk + atratores), cada uma com sua própria
    // velocidade/caráter, dirigindo grupos de parâmetros DIFERENTES -
    // ver deriveFromMemory() pra qual grupo cada uma controla. O
    // instrumento passa a evoluir como processos paralelos em vez de um
    // bloco só se movendo junto.
    float derivationMotionB = 0.0f, derivationMotionC = 0.0f;
    // Âncoras combináveis (19 ago. 2026, mesmo brainstorm - "talvez como
    // fizemos nos botoes do vcf, combinações de botões de derivas": o
    // VCF já tem toggles multi-select reais/independentes - LPF/BPF/HPF/
    // NCH, não um modo exclusivo). Pool de 2 posições, -1 = vazia.
    // Começa só com o passo 1 (índice 0); cada evento GLT ocupa a
    // próxima posição livre e, uma vez as duas ocupadas, substitui a
    // mais antiga (round-robin, `derivationAnchorWrite`) - nunca cresce
    // sem limite.
    std::array<int, 2> derivationAnchors { 0, -1 };
    int derivationAnchorWrite = 0;
    std::array<unsigned char, 8> topologyMemory {};
    std::size_t topologyWrite = 0, derivationPhrase = 0, lastDerivationStep = 0;
    unsigned int derivationState = 0xC0FFEE1U;
    juce::Label mixerLabel;
    juce::Label mixMemoryLabel;
    std::array<juce::TextButton, 4> mixMemorySlots;
    // M1-4/CAPTURAR (20 ago. 2026, autor: "e os botoes de memoria
    // captura também" -> escolheu só RECALL, nunca CAPTURE, via
    // AskUserQuestion). Rastreado aqui porque `MutableMixer` não expõe
    // se um slot já foi capturado - um slot nunca capturado tem
    // `enabled = false` nos 4 canais por padrão, então recall nele
    // silenciaria o mixer inteiro. Marcado `true` só no próprio
    // `onClick` de captura (nunca pela DERIVA).
    std::array<bool, 4> mixMemoryCaptured {};
    // Participação por título (20 ago. 2026, autor: "tive uma ideia
    // para as seleções dos conteúdos a fazerem parte dos item a
    // partiticar da deriva, ao lado de cada título um pequeno botão
    // toogle" - confirmado via AskUserQuestion: vale pros dois modos
    // [A/B/C e AUTO], todos os títulos de uma vez, exceto a barra de
    // transporte no topo). Um bool por título já existente na tela -
    // desligar um congela só aquele bloco, sem depender de A/B/C/AUTO.
    // Todos começam ligados (mesmo padrão de "sem destruir o que já
    // temos"). Sem fome/memória própria - são só um portão extra em
    // cima do que já existe, checado com `&&` nas condições que já
    // tinham (não uma reestruturação).
    juce::ToggleButton participateSteps, participateVoice, participateEffects, participateDetail,
                        participateMixer, participateEnvelope, participateModulation, participateGroove,
                        participateFilter, participateMetric, participateTemporal, participateNoiseColour,
                        participateLoopEnd, participateRoutes, participateMixMemory;
    // Faltavam na primeira leva (20 ago. 2026, autor: "faltaram botoes
    // em: MATERIA, CAOS, FORMA LFO, RING, KNOB CLOCK" / "e MAT") -
    // MATÉRIA (rail CUTOFF/RESON/DRIVE/ASYM, detailControls[9..12]) e
    // CAOS (rail DRIVE/DAMPING/DEPTH, detailControls[13..15]) tinham
    // título visual próprio mas caíam dentro do `participateDetail`
    // único (ROTAS ATIVAS, detailControls[0..8]) - separados aqui.
    // MAT (`materialFilterMix`, um knob só) é uma coisa DIFERENTE do
    // rail MATÉRIA (mesmo nome curto, controles diferentes). RING
    // (`modulationControls[1]`) nunca teve mecanismo de deriva nenhum -
    // entra no MESMO toggle de MODULAÇÃO que LFO/NOISE MIX já usam
    // (autor: "creio que ring já está em modulação"), sem toggle novo.
    juce::ToggleButton participateMaterial, participateChaos, participateMat, participateLfoShape, participateClock;
    juce::TextButton mixMemoryCapture;
    std::array<juce::Label, 4> mixLabels;
    std::array<juce::Slider, 4> mixGain, mixPan, mixReflux;
    std::array<juce::ToggleButton, 4> mixEnable, mixMute, mixSolo;
};

// CLONE's own copy of ZoomableViewport (see that class's own comment for
// the full rationale) - the 2-monitor CLONE window is a separate
// DocumentWindow from PRINCIPAL's, so it needs its own zoomable wrapper
// and its own KeyListener below, not just a shared flag (18 ago. 2026,
// author: "ctrl + - só funciona na aba principal" / "no clone ainda não
// (2monitor)" - single-window embedded CLONE already worked, since it's
// just another child inside MainWindow's own ZoomableViewport; only this
// separate window was missing the mechanism entirely). Also this window's
// ONLY content class now - it used to share the job with a plain
// non-zoomable ObjectFiveViewport, picked by a pixel-width threshold that
// turned out impossible to get right (the window's own default size, and
// its resized-to-88%-of-a-second-monitor size, could each independently
// land under whatever fixed number was tried - see
// ObjectFiveWindow::applyContentForCurrentSize() for the full story).
// This class already has its own scrollbars and its own sane minimum
// size, so it never needed a separate smaller-window fallback at all.
class ZoomableObjectFiveViewport final : public juce::Viewport
{
public:
    explicit ZoomableObjectFiveViewport(antitotem::DualObjectEngine& engine,
                                         antitotem::ui::Language initialLanguage = antitotem::ui::Language::english)
        : panel(engine, false, initialLanguage)
    {
        setScrollBarsShown(true, true, true, true);
        panel.setTopLeftPosition(0, 0);
        sizer.addAndMakeVisible(panel);
        setViewedComponent(&sizer, false);
        applyZoom();
    }
    void setLanguage(antitotem::ui::Language language) { panel.setLanguage(language); }
    std::function<void(const juce::String&)>* explainCallback() { return panel.explainCallback(); }
    // Real regression found live (18 ago. 2026, author: "funcionou mas os
    // os objetos sairam do lugar" / "bagunçou a aba clone") - resized()
    // used to feed the Viewport's own current pixel size straight into
    // `panel` (std::max(1850, getWidth()), std::max(500, getHeight())),
    // so `panel`'s logical size changed with whatever this window
    // happened to be at the time - its default 1860x950, or 88% of
    // whichever second monitor positionObjectFiveWindow() found. Content
    // components elsewhere in this app (MainComponent) are DESIGNED to
    // relayout responsively at arbitrary sizes; ObjectFiveComponent's own
    // ungrouped layout was only ever exercised at a fixed 1860x924 before
    // today (the window's own direct-content size at its own default,
    // pre-zoom), so feeding it other heights broke its own internal
    // pixel math. Fixed logical size now (matching that known-good
    // 1860x924) - the Viewport's own scrollbars are exactly what's
    // supposed to reconcile "the window is some other size" with "the
    // content has one true size", not a resize of the content itself,
    // same as how zoom (a scale, not a resize) already works.
    void resized() override { juce::Viewport::resized(); }
    void setZoom(float newZoom)
    {
        const auto clamped = std::clamp(newZoom, 0.7f, 1.5f);
        if (std::abs(clamped - zoom) < 0.001f)
            return;
        zoom = clamped;
        applyZoom();
    }
    float getZoom() const noexcept { return zoom; }
private:
    void applyZoom()
    {
        panel.setSize(logicalWidth, logicalHeight);
        panel.setTransform(juce::AffineTransform::scale(zoom));
        sizer.setSize(juce::roundToInt(static_cast<float>(logicalWidth) * zoom),
                      juce::roundToInt(static_cast<float>(logicalHeight) * zoom));
    }
    juce::Component sizer;
    ObjectFiveComponent panel;
    // Fixed - see resized()'s own comment for why this must NOT track the
    // Viewport's actual current size. 1860x924: this window's own default
    // client size (centreWithSize(1860, 950), minus native title bar).
    static constexpr int logicalWidth = 1860, logicalHeight = 924;
    float zoom = 1.0f;
};

class ObjectFiveWindow final : public juce::DocumentWindow, public juce::KeyListener
{
public:
    explicit ObjectFiveWindow(antitotem::DualObjectEngine& engine,
                               antitotem::ui::Language initialLanguage = antitotem::ui::Language::english)
        : DocumentWindow("Antitotem - CLONE", cloneMaterial::shadow, DocumentWindow::allButtons),
          dualEngine(engine), language(initialLanguage)
    {
        addKeyListener(this);
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setResizeLimits(900, 500, 2000, 1400);
        centreWithSize(1860, 950);
        applyContentForCurrentSize();
        setVisible(true);
    }
    void closeButtonPressed() override { setVisible(false); }
    void resized() override
    {
        DocumentWindow::resized();
        applyContentForCurrentSize();
    }
    // Stores the language for whichever content gets (re)built next
    // (applyContentForCurrentSize() can replace it on any resize crossing
    // the 1880/950 threshold), and pushes it live into whatever content
    // exists right now.
    void setLanguage(antitotem::ui::Language newLanguage)
    {
        language = newLanguage;
        if (auto* component = dynamic_cast<ZoomableObjectFiveViewport*>(getContentComponent()))
            component->setLanguage(language);
    }
    // LEARN: this window's own content always exists by the time this is
    // callable (applyContentForCurrentSize() runs synchronously in the
    // constructor above), so no null-content guard is needed the way
    // setLanguage() has one for the "content rebuilt on resize" case that
    // no longer applies to LEARN specifically (contentApplied latches
    // after the first build).
    std::function<void(const juce::String&)>* explainCallback()
    {
        auto* component = dynamic_cast<ZoomableObjectFiveViewport*>(getContentComponent());
        return component != nullptr ? component->explainCallback() : nullptr;
    }
    // See MainWindow's own copy for the full comment.
    using juce::Component::keyPressed;
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override
    {
        auto* zoomable = dynamic_cast<ZoomableObjectFiveViewport*>(getContentComponent());
        if (zoomable == nullptr)
            return false;
        if (key == juce::KeyPress('=', juce::ModifierKeys::commandModifier, 0)
            || key == juce::KeyPress('+', juce::ModifierKeys::commandModifier, 0))
        {
            zoomable->setZoom(zoomable->getZoom() + 0.1f);
            return true;
        }
        if (key == juce::KeyPress('-', juce::ModifierKeys::commandModifier, 0))
        {
            zoomable->setZoom(zoomable->getZoom() - 0.1f);
            return true;
        }
        if (key == juce::KeyPress('0', juce::ModifierKeys::commandModifier, 0))
        {
            zoomable->setZoom(1.0f);
            return true;
        }
        return false;
    }
private:
    void applyContentForCurrentSize()
    {
        // Two real, separate bugs found in the same investigation (18 ago.
        // 2026, author: "ctrl + - só funciona na aba principal" / "ainda
        // não funciona a aba clone" / "quando tento fazer o ctrl + - não
        // funciona") and fixed together:
        //
        // 1. This window's own default size is centreWithSize(1860, 950),
        //    but the threshold used to pick the zoomable path used to be
        //    1880 wide: 1860 < 1880, so the OLD plain ObjectFiveViewport
        //    (no KeyListener, no zoom) was selected from the very first
        //    frame, every time, regardless of anything the author did
        //    afterwards.
        // 2. Even after narrowing that threshold, positionObjectFiveWindow()
        //    resizes this window again right after construction when dual
        //    monitor mode is active, to 88% of the SECOND monitor's own
        //    area (see that method) - a smaller/narrower secondary display
        //    can easily land back under any fixed pixel threshold, right
        //    back to the same failure by a different route.
        //
        // ZoomableObjectFiveViewport already has its own scrollbars
        // (setScrollBarsShown(true, true, true, true)) and its own sane
        // minimum content size (std::max(1850, ...)/std::max(500, ...)),
        // so it's a strict superset of what the plain ObjectFiveViewport
        // offered - there was never a real reason to keep two different
        // classes for this window. Always using it removes the entire
        // category of "guess the right pixel threshold" bugs instead of
        // moving the goalposts again.
        if (contentApplied)
            return;
        contentApplied = true;
        setContentOwned(new ZoomableObjectFiveViewport(dualEngine, language), false);
    }
    antitotem::DualObjectEngine& dualEngine;
    antitotem::ui::Language language = antitotem::ui::Language::english;
    bool contentApplied = false;
};

class ChipConcept final : public juce::Component
{
public:
    ChipConcept(juce::String chipName, juce::String functionName, bool isStudy = false)
        : chip(chipName), function(functionName), study(isStudy) {}
    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colour(study ? 0xff302a20 : 0xff25231d));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(study ? 0xff927335 : 0xff665b49));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        g.setColour(juce::Colour(0xffffca5c));
        g.setFont(uiFont(15.0f, true));
        g.drawFittedText(chip, getLocalBounds().removeFromTop(22), juce::Justification::centred, 1);
        g.setColour(juce::Colour(0xffded4be));
        g.setFont(uiFont(10.5f));
        g.drawFittedText(function, getLocalBounds().withTrimmedTop(21), juce::Justification::centred, 2);
    }
private:
    juce::String chip, function;
    bool study = false;
};

class MainComponent final : public juce::AudioAppComponent, private juce::Timer, private juce::FocusChangeListener
{
public:
    // Atalho de teclado Shift+C (20 ago. 2026, autor: "pode criar um
    // atalho para o boltão clone principal" -> "shift"). MainWindow's
    // próprio `keyPressed()` chama isto pra disparar o MESMO `onClick`
    // que o botão CLONE/PRINCIPAL já tem (não duplica a lógica de
    // dual-monitor vs. troca de corpo na mesma janela).
    void toggleCloneView() { objectFive.triggerClick(); }
    MainComponent() : sequencer(dualEngine.object1())
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "Antitotem";
        options.filenameSuffix = "settings";
        options.folderName = "Antitotem";
        options.osxLibrarySubFolder = "Application Support";
        applicationProperties.setStorageParameters(options);
        if (auto* settings = applicationProperties.getUserSettings())
        {
            uiLanguage = antitotem::ui::languageFromCode(settings->getValue("uiLanguage", "en"));
            dualMonitorMode = settings->getBoolValue("dualMonitorMode", false);
        }
        // noiseSelector/steps are members, already default-constructed
        // (English) before this constructor body runs - correct them to
        // the actually persisted language now that it's known.
        noiseSelector.setLanguage(uiLanguage);
        for (auto& step : steps) step.setLanguage(uiLanguage);

        configureLabel(title, antitotem::ui::text(antitotem::ui::mainTitle, uiLanguage), 18.0f, juce::Colour(0xfffff5e5));
        configureLabel(flow, antitotem::ui::text(antitotem::ui::label::coreHeader, uiLanguage), 13.0f, juce::Colour(0xffbdb199));
        // Left-aligned (author, live: "volte o texto do rodapé alinhado
        // pela esquerda") - configureLabel's own default, no override
        // needed; was explicitly forced to centredRight here.
        configureLabel(footer, antitotem::ui::text(antitotem::ui::label::footerCredit, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
        // "MODO: PRINCIPAL" - always true for this tab's own transport
        // column, since it's only ever visible while PRINCIPAL is the
        // active body (author, live: "definir uma função que escreva na
        // tela... para que o usuário compreenda em qual aba está
        // atuando").
        configureLabel(modeLabel, antitotem::ui::text(antitotem::ui::label::modePrincipal, uiLanguage), 11.0f, juce::Colour(0xffffca5c));
        addAndMakeVisible(modeLabel);
        configureLabel(clockLabel, "CLOCK", 9.0f, juce::Colour(0xffded4be));
        clockLabel.setJustificationType(juce::Justification::centred);
        configureLabel(loopLabel, antitotem::ui::text(antitotem::ui::label::loopEndPrefix, uiLanguage) + "1" + antitotem::ui::text(antitotem::ui::label::loopEndSuffix, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        loopLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::loopHeaderTip, uiLanguage));
        configureLabel(connectionLabel, antitotem::ui::text(antitotem::ui::label::feedbackPorts, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        connectionLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackHeaderTip, uiLanguage));
        configureLabel(energyLabel, antitotem::ui::text(antitotem::ui::label::energy, uiLanguage), 9.0f, juce::Colour(0xffded4be));
        energyLabel.setJustificationType(juce::Justification::centred);
        // 9.0f, not 12.0f - same font size as ENERGIA's own caption (18
        // ago. 2026, author: "deixe o tamanho do titulo com a mesma
        // configuração de layout do titulo de energia").
        configureLabel(masterLabel, "MASTER", 9.0f, juce::Colour(0xffded4be));
        masterLabel.setJustificationType(juce::Justification::centred);
        // CLONE-only volume - separate from MASTER (which scales both
        // objects together, see DualObjectEngine::setMasterGain's own
        // comment). Registered by the author as a way to actually
        // silence CLONE from the mix without stopping its transport or
        // hand-zeroing every oscillator/mixer level.
        // PRINCIPAL/CLONE treated as a 2-channel rail (gain + M/S) right
        // above the 4-channel mixer, same vocabulary - registered by the
        // author as a way to actually silence/isolate either object from
        // the mix without stopping its transport or hand-zeroing every
        // level ("um slider de volume com botão S e M pros dois
        // instrumentos... fica mais simétrico para as trocas de abas").
        configureLabel(objectMixLabel, antitotem::ui::text(antitotem::ui::label::objectMix, uiLanguage), 12.0f, juce::Colour(0xffded4be));
        objectMixLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixHeaderTip, uiLanguage));
        configureLabel(principalVolumeLabel, "PRINC", 9.5f, juce::Colour(0xffded4be));
        configureLabel(cloneVolumeLabel, "CLONE", 9.5f, juce::Colour(0xffded4be));
        configureLabel(excitationLabel, "EXCIT", 9.5f, juce::Colour(0xffded4be));
        excitationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::excitationAmount, uiLanguage));
        configureLabel(noiseLabel, "NOISE", 12.0f, juce::Colour(0xffded4be));
        noiseLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseHeaderTip, uiLanguage));
        configureLabel(recordDurationsLabel, utf8("REC TIMERS"), 10.0f, juce::Colour(0xff8f856f));
        recordDurationsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::recordDurationsHeaderTip, uiLanguage));
        configureLabel(recordingLabel, antitotem::ui::text(antitotem::ui::label::recControls, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        addAndMakeVisible(recordingLabel);
        // configureLabel defaults to left-aligned (matching ENERGIA/MASTER,
        // which sit in their own narrow cells); NOISE spans the wider
        // remaining footer width, so it needs to be told to centre instead.
        noiseLabel.setJustificationType(juce::Justification::centred);
        configureLabel(modulationLabel, antitotem::ui::text(antitotem::ui::label::modulation, uiLanguage), 14.0f, juce::Colour(0xffffca5c));
        modulationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::modulationHeaderTip, uiLanguage));
        // Not the shared section-title gold anymore - see the matching comment
        // in ObjectFiveComponent's own constructor.
        configureLabel(effectsLabel, antitotem::ui::text(antitotem::ui::label::spacePhase, uiLanguage), 10.0f, material::phaser);
        effectsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::effectsHeaderTip, uiLanguage));
        configureLabel(detailLabel, antitotem::ui::text(antitotem::ui::label::activeRoutes, uiLanguage), 10.0f, material::memory);
        detailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailHeaderTip, uiLanguage));
        configureLabel(parametersLabel, antitotem::ui::text(antitotem::ui::label::parametersRail, uiLanguage), 14.0f, juce::Colour(0xffffca5c));
        parametersLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::parametersHeaderTip, uiLanguage));
        configureLabel(materialRailLabel, antitotem::ui::text(antitotem::ui::label::materialRail, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        materialRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialHeaderTip, uiLanguage));
        configureLabel(chaosRailLabel, antitotem::ui::text(antitotem::ui::label::chaosRail, uiLanguage), 10.0f, material::clock);
        chaosRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::chaosHeaderTip, uiLanguage));
        configureLabel(filterLabel, antitotem::ui::text(antitotem::ui::label::vcfMultimode, uiLanguage), 14.0f, juce::Colour(0xffffca5c));
        filterLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::vcfHeaderTip, uiLanguage));
        configureLabel(envelopeLabel, "ADSR", 14.0f, juce::Colour(0xffffca5c));
        envelopeLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::adsrHeaderTip, uiLanguage));
        configureLabel(voiceLabel, antitotem::ui::text(antitotem::ui::label::oscHeaderTitle, uiLanguage), 15.0f, juce::Colour(0xffffca5c));
        voiceLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::voiceHeaderTip, uiLanguage));
        // Same title CLONE already shows above its own step grid (author,
        // live: "na aba clone há o titulo do sequencer, no principal está
        // faltando") - PRINCIPAL's sequencer never had one.
        // Same size/colour as PARÂMETROS - see the matching comment in
        // ObjectFiveComponent's own constructor.
        configureLabel(stepsLabel, utf8("CV (16 STEPS)"), 14.0f, juce::Colour(0xffffca5c));
        stepsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::stepsHeaderTip, uiLanguage));
        // A persistent header, not the box's own first scrolling line -
        // that line only looked like a title but was really just the
        // first appendLog()-style entry, and the log's own 1500-char
        // rolling buffer could eventually trim it away like anything else
        // (author, 15 ago. 2026: "percebi também que não há título para o
        // objeto log").
        configureLabel(logLabel, antitotem::ui::text(antitotem::ui::logText::title, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        for (auto* label : { &title, &flow, &footer, &clockLabel, &loopLabel, &connectionLabel, &energyLabel, &masterLabel, &objectMixLabel, &principalVolumeLabel, &cloneVolumeLabel, &excitationLabel, &noiseLabel, &modulationLabel, &effectsLabel, &detailLabel, &parametersLabel, &materialRailLabel, &chaosRailLabel, &filterLabel, &envelopeLabel, &voiceLabel, &recordDurationsLabel, &stepsLabel, &logLabel }) addAndMakeVisible(*label);
        log.setMultiLine(true); log.setReadOnly(true); log.setScrollbarsShown(true); log.setCaretVisible(false);
        log.setLookAndFeel(&logPanelLook());
        log.setFont(uiFont(12.0f));
        log.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12110e));
        log.setColour(juce::TextEditor::textColourId, material::metal);
        log.setColour(juce::TextEditor::outlineColourId, material::wood.brighter(0.35f));
        log.setText(antitotem::ui::text(antitotem::ui::logText::diagnosticLines, uiLanguage), false);
        log.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::activityLog, uiLanguage));
        addAndMakeVisible(log);

        // CONEXÃO ENTRE OBJETOS - moved here from CLONE's own left column,
        // fixed above LOG (see the mixer column layout below).
        configureLabel(objectConnectionLabel, antitotem::ui::text(antitotem::ui::label::objectConnection, uiLanguage), 13.0f, material::returnPath);
        objectConnectionLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectConnectionHeaderTip, uiLanguage));
        addAndMakeVisible(objectConnectionLabel);
        configureLabel(gainToFifthLabel, utf8("PRINCIPAL → CLONE"), 10.0f, juce::Colour(0xffded4be));
        configureLabel(gainToFirstLabel, utf8("CLONE → PRINCIPAL"), 10.0f, juce::Colour(0xffded4be));
        configureLabel(auxToFirstLabel, utf8("AUX → PRINCIPAL"), 10.0f, juce::Colour(0xffded4be));
        configureLabel(auxToFifthLabel, utf8("AUX → CLONE"), 10.0f, juce::Colour(0xffded4be));
        for (auto* label : { &gainToFifthLabel, &gainToFirstLabel, &auxToFirstLabel, &auxToFifthLabel }) addAndMakeVisible(*label);
        for (auto* slider : { &gainToFifth, &gainToFirst, &auxToFirst, &auxToFifth })
        {
            slider->setSliderStyle(juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);
            slider->setRange(0.0, 0.72, 0.01);
            slider->setValue(0.0);
            addAndMakeVisible(*slider);
        }
        gainToFifth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::gainToClone, uiLanguage));
        gainToFirst.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::gainToPrincipal, uiLanguage));
        auxToFirst.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::auxToPrincipal, uiLanguage));
        auxToFifth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::auxToCloneObject, uiLanguage));
        auto updateObjectConnection = [this]
        {
            dualEngine.setObjectConnection(static_cast<float>(gainToFifth.getValue()), static_cast<float>(gainToFirst.getValue()));
            dualEngine.setAuxiliaryMix(static_cast<float>(auxToFirst.getValue()), static_cast<float>(auxToFifth.getValue()));
        };
        for (auto* slider : { &gainToFifth, &gainToFirst, &auxToFirst, &auxToFifth }) slider->onValueChange = updateObjectConnection;

        const std::array<juce::String, 4> objectRouteNames {
            antitotem::ui::text(antitotem::ui::button::routeDirect, uiLanguage),
            antitotem::ui::text(antitotem::ui::button::routeDiode, uiLanguage),
            "CAP",
            antitotem::ui::text(antitotem::ui::button::routePulse, uiLanguage)
        };
        constexpr std::array<antitotem::DualObjectEngine::ConnectionRoute, 4> objectRouteFlags {
            antitotem::DualObjectEngine::direct, antitotem::DualObjectEngine::diode,
            antitotem::DualObjectEngine::capacitor, antitotem::DualObjectEngine::pulse };
        configureLabel(routesToFifthLabel, antitotem::ui::text(antitotem::ui::label::routeToClone, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        configureLabel(routesToFirstLabel, antitotem::ui::text(antitotem::ui::label::routeToPrincipal, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        addAndMakeVisible(routesToFifthLabel); addAndMakeVisible(routesToFirstLabel);
        auto updateObjectRoutes = [this, objectRouteFlags]
        {
            unsigned char toFifth = 0, toFirst = 0;
            for (std::size_t i = 0; i < 4; ++i)
            {
                if (routesToFifth[i].getToggleState()) toFifth |= static_cast<unsigned char>(objectRouteFlags[i]);
                if (routesToFirst[i].getToggleState()) toFirst |= static_cast<unsigned char>(objectRouteFlags[i]);
            }
            dualEngine.setConnectionRoutes(toFifth, toFirst);
        };
        for (std::size_t i = 0; i < 4; ++i)
        {
            routesToFifth[i].setButtonText(objectRouteNames[i]);
            routesToFifth[i].setComponentID("loop");
            routesToFifth[i].setLookAndFeel(&patchToggleLook());
            routesToFifth[i].setToggleState(i == 2, juce::dontSendNotification);
            routesToFifth[i].onClick = updateObjectRoutes;
            routesToFifth[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectRouteTips[i], uiLanguage));
            addAndMakeVisible(routesToFifth[i]);
            routesToFirst[i].setButtonText(objectRouteNames[i]);
            routesToFirst[i].setComponentID("loop");
            routesToFirst[i].setLookAndFeel(&patchToggleLook());
            routesToFirst[i].setToggleState(i == 2, juce::dontSendNotification);
            routesToFirst[i].onClick = updateObjectRoutes;
            routesToFirst[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectRouteTips[i], uiLanguage));
            addAndMakeVisible(routesToFirst[i]);
        }
        updateObjectConnection();
        updateObjectRoutes();

        soundPage.setButtonText(antitotem::ui::text(antitotem::ui::button::soundPage, uiLanguage));
        sequencePage.setButtonText(antitotem::ui::text(antitotem::ui::button::sequencePage, uiLanguage));
        mixPage.setButtonText("MIX");
        soundPage.setRadioGroupId(601); sequencePage.setRadioGroupId(601); mixPage.setRadioGroupId(601);
        soundPage.setToggleState(true, juce::dontSendNotification);
        soundPage.onClick = [this] { setPage(Page::sound); };
        sequencePage.onClick = [this] { setPage(Page::sequence); };
        mixPage.onClick = [this] { setPage(Page::mix); };
        addAndMakeVisible(soundPage); addAndMakeVisible(sequencePage); addAndMakeVisible(mixPage);
        // Hidden by default, not shown-then-hidden: setSize() below still
        // reports a width under the unified-layout threshold (1320, then
        // whatever this component's initial placeholder size is) until
        // MainWindow imposes its own real size moments later, so setPage()
        // running before that lands would otherwise show these for one
        // visible frame, overlapping the oscillator area - a real bug a
        // live test caught (SOM/SEQUÊNCIA/MIX briefly visible over "5 OSC
        // — FREQ/MIX/FORMA/EIXO X" on launch). setPage() still turns them
        // back on correctly if the window ever is genuinely non-unified.
        soundPage.setVisible(false); sequencePage.setVisible(false); mixPage.setVisible(false);
        // Same size/colour as ADSR - see the matching comment in
        // ObjectFiveComponent's own constructor.
        configureLabel(mixerLabel, utf8("MIXER"), 14.0f, juce::Colour(0xffffca5c));
        addAndMakeVisible(mixerLabel);
        configureLabel(variationLabel, antitotem::ui::text(antitotem::ui::label::variation, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        variationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeaderTip, uiLanguage));
        addAndMakeVisible(variationLabel);
        pulseVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPulse, uiLanguage));
        pulseVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPulse, uiLanguage));
        pulseVariation.onClick = [this] { applyVariation(Variation::pulse); };
        porousVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPorous, uiLanguage));
        porousVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPorous, uiLanguage));
        porousVariation.onClick = [this] { applyVariation(Variation::porous); };
        heterodyneVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationHeterodyne, uiLanguage));
        heterodyneVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeterodyne, uiLanguage));
        heterodyneVariation.onClick = [this] { applyVariation(Variation::heterodyne); };
        randomizeStepsButton.setButtonText("RND 16");
        randomizeStepsButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::randomizeSteps, uiLanguage));
        randomizeStepsButton.onClick = [this]
        {
            sequencer.randomizeSteps();
            refreshStepControls();
            appendLog(antitotem::ui::text(antitotem::ui::logText::rnd16, uiLanguage));
        };
        orbitVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationOrbit, uiLanguage));
        orbitVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationOrbit, uiLanguage));
        orbitVariation.onClick = [this] { applyVariation(Variation::orbit); };
        pendulumVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPendulum, uiLanguage));
        pendulumVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPendulum, uiLanguage));
        pendulumVariation.onClick = [this] { applyVariation(Variation::pendulum); };
        deriveButton.setButtonText(antitotem::ui::text(antitotem::ui::label::drift, uiLanguage));
        deriveButton.setComponentID("derive");
        deriveButton.setClickingTogglesState(true);
        deriveButton.setLookAndFeel(&patchToggleLook());
        deriveButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveButton, uiLanguage));
        deriveButton.onClick = [this]
        {
            if (deriveButton.getToggleState())
            {
                captureDerivationMemory();
                appendLog(antitotem::ui::text(antitotem::ui::logText::driftCaptured, uiLanguage));
            }
            else configureLabel(flow, antitotem::ui::text(antitotem::ui::label::coreHeader, uiLanguage), 13.0f, juce::Colour(0xffbdb199));
            if (! deriveButton.getToggleState()) appendLog(antitotem::ui::text(antitotem::ui::logText::driftResting, uiLanguage));
        };
        addAndMakeVisible(pulseVariation); addAndMakeVisible(porousVariation);
        addAndMakeVisible(heterodyneVariation); addAndMakeVisible(randomizeStepsButton); addAndMakeVisible(deriveButton);
        addAndMakeVisible(orbitVariation); addAndMakeVisible(pendulumVariation);
        // Camadas de deriva - see ObjectFiveComponent's own copy for the
        // full comment (docs/PESQUISA_DERIVA_GENERATIVA.md).
        {
            constexpr std::array<const char*, 4> derivationLayerNames { "A", "B", "C", "AUTO" };
            for (std::size_t i = 0; i < derivationLayers.size(); ++i)
            {
                derivationLayers[i].setButtonText(derivationLayerNames[i]);
                derivationLayers[i].setComponentID("core");
                derivationLayers[i].setClickingTogglesState(true);
                // AUTO (índice 3) começa desligado - a configuração
                // padrão continua sendo A/B/C, não o modo novo (autor:
                // "sem destruir também o que já temos").
                derivationLayers[i].setToggleState(i != 3, juce::dontSendNotification);
                derivationLayers[i].setLookAndFeel(&patchToggleLook());
                derivationLayers[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::derivationLayer, uiLanguage));
                addAndMakeVisible(derivationLayers[i]);
            }
        }
        // Participação por título (20 ago. 2026) - ver o comentário
        // completo no próprio membro `participateSteps` etc, e o
        // membro `participateConnections` (só aqui, CONEXÕES ENTRE
        // OBJETOS não existe em CLONE). Autor: "não quero deriva no
        // master, nem no osciloscopio e nem no mixer objetos" - os três
        // nunca participavam mesmo, fora do escopo por padrão.
        {
            const std::array<juce::ToggleButton*, 21> participationToggles {
                &participateSteps, &participateVoice, &participateEffects, &participateDetail,
                &participateMixer, &participateEnvelope, &participateModulation, &participateGroove,
                &participateFilter, &participateMetric, &participateTemporal, &participateNoiseColour,
                &participateLoopEnd, &participateRoutes, &participateMixMemory, &participateConnections,
                &participateMaterial, &participateChaos, &participateMat, &participateLfoShape, &participateClock
            };
            for (auto* toggle : participationToggles)
            {
                toggle->setButtonText("");
                toggle->setComponentID("core");
                toggle->setClickingTogglesState(true);
                toggle->setToggleState(true, juce::dontSendNotification);
                toggle->setLookAndFeel(&patchToggleLook());
                toggle->setTooltip(antitotem::ui::text(antitotem::ui::tooltip::derivationParticipation, uiLanguage));
                addAndMakeVisible(*toggle);
            }
        }
        tutorial.setButtonText("TUTORIAL");
        tutorial.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::openTutorial, uiLanguage));
        tutorial.onClick = [this]
        {
            if (appInfoWindow != nullptr) appInfoWindow->setVisible(false);
            if (tutorialWindow == nullptr)
                tutorialWindow = std::make_unique<TutorialWindow>(uiLanguage);
            tutorialWindow->setVisible(true);
            tutorialWindow->toFront(true);
        };
        about.setButtonText(antitotem::ui::text(antitotem::ui::button::about, uiLanguage));
        about.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::openAbout, uiLanguage));
        about.onClick = [this]
        {
            if (tutorialWindow != nullptr) tutorialWindow->setVisible(false);
            if (appInfoWindow == nullptr)
            {
                appInfoWindow = std::make_unique<AppInfoWindow>(uiLanguage);
                *appInfoWindow->languageCallback() = [this] (antitotem::ui::Language language) { setUiLanguage(language); };
            }
            appInfoWindow->setVisible(true);
            appInfoWindow->toFront(true);
        };
        // LEARN: no toggle - always on, same category of persistent
        // object as LOG (author, live: "talvez o botão learn nem seja
        // necessário se a caixa sempre é visível" / "sempre funcionará o
        // learn"). Reuses whatever tooltip text each control already has
        // (juce::TooltipClient::getTooltip()) instead of a parallel
        // key->text table - ANTITOTEM already had tooltip::* on nearly
        // every control before LEARN existed, so this is the "same
        // method, not the same code" reuse of NAVALHA2_JUCE's own
        // LEARNING MODE precedent (GOVERNANCA_E_TRANSVERSALIDADE.md
        // Section 7.1).
        configureLabel(learnLabel, "LEARN", 10.0f, juce::Colour(0xff8f856f));
        addAndMakeVisible(learnLabel);
        learnEditor.setMultiLine(true); learnEditor.setReadOnly(true); learnEditor.setScrollbarsShown(true); learnEditor.setCaretVisible(false);
        learnEditor.setLookAndFeel(&logPanelLook());
        learnEditor.setFont(uiFont(12.0f));
        learnEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12110e));
        learnEditor.setColour(juce::TextEditor::textColourId, material::metal);
        learnEditor.setColour(juce::TextEditor::outlineColourId, material::wood.brighter(0.35f));
        learnEditor.setText(antitotem::ui::text(antitotem::ui::tooltip::learnPanelIdle, uiLanguage), false);
        addAndMakeVisible(learnEditor);
        addMouseListener(this, true);
        juce::Desktop::getInstance().addFocusChangeListener(this);
        languageSwitch.setButtonText(antitotem::ui::languageLabel(uiLanguage));
        languageSwitch.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::languageSwitch, uiLanguage));
        languageSwitch.onClick = [this] { setUiLanguage(antitotem::ui::nextLanguage(uiLanguage)); };
        addAndMakeVisible(languageSwitch);
        // Only offered when a second display actually exists - never
        // shown to single-monitor users. Persisted like uiLanguage.
        monitorModeToggle.setButtonText(dualMonitorMode ? antitotem::ui::text(antitotem::ui::button::twoMonitors, uiLanguage) : antitotem::ui::text(antitotem::ui::button::oneMonitor, uiLanguage));
        monitorModeToggle.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::monitorModeToggle, uiLanguage));
        monitorModeToggle.setVisible(hasSecondMonitor());
        // Disabled until CLONE has actually been opened once - before
        // that there is no window for this to reposition, so letting the
        // user operate it reads as a control that does nothing.
        monitorModeToggle.setEnabled(false);
        monitorModeToggle.onClick = [this]
        {
            dualMonitorMode = ! dualMonitorMode;
            monitorModeToggle.setButtonText(dualMonitorMode ? antitotem::ui::text(antitotem::ui::button::twoMonitors, uiLanguage) : antitotem::ui::text(antitotem::ui::button::oneMonitor, uiLanguage));
            if (auto* settings = applicationProperties.getUserSettings())
                settings->setValue("dualMonitorMode", dualMonitorMode);
            // CLONE only ever shows in one place at a time - switching
            // modes carries it from the in-window body to its own window
            // (or back), instead of leaving the old spot visible too (a
            // real bug found in testing: toggling to 2 monitores while
            // the in-window body was showing CLONE left it visible there
            // *and* opened a second copy on the other monitor).
            if (dualMonitorMode)
            {
                const bool wasShowingClone = showingCloneBody;
                if (showingCloneBody) setShowingCloneBody(false);
                if (wasShowingClone)
                {
                    if (objectFiveWindow == nullptr) objectFiveWindow = std::make_unique<ObjectFiveWindow>(dualEngine, uiLanguage);
                    // LEARN: this window's own hover/focus feeds MainComponent's
                    // single shared box - see ObjectFiveComponent's own onExplain
                    // member comment. Reassigning on every open is harmless.
                    *objectFiveWindow->explainCallback() = [this] (const juce::String& text) { learnEditor.setText(text, false); };
                    positionObjectFiveWindow();
                    objectFiveWindow->setVisible(true);
                    objectFiveWindow->toFront(true);
                }
            }
            else if (objectFiveWindow != nullptr && objectFiveWindow->isVisible())
            {
                objectFiveWindow->setVisible(false);
                setShowingCloneBody(true);
            }
        };
        addAndMakeVisible(monitorModeToggle);
        objectFive.setButtonText(utf8("CLONE"));
        objectFive.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::cloneToggle, uiLanguage));
        objectFive.onClick = [this]
        {
            // Dual monitor mode only actually applies with a second
            // display present - otherwise this always toggles the body
            // in this same window, regardless of what dualMonitorMode
            // was last persisted as.
            if (dualMonitorMode && hasSecondMonitor())
            {
                const bool wasOpen = objectFiveWindow != nullptr;
                if (objectFiveWindow == nullptr) objectFiveWindow = std::make_unique<ObjectFiveWindow>(dualEngine, uiLanguage);
                *objectFiveWindow->explainCallback() = [this] (const juce::String& text) { learnEditor.setText(text, false); };
                if (! wasOpen) positionObjectFiveWindow();
                objectFiveWindow->setVisible(true);
                objectFiveWindow->toFront(true);
            }
            else
            {
                setShowingCloneBody(! showingCloneBody);
            }
            monitorModeToggle.setEnabled(true);
        };
        addAndMakeVisible(objectFive);
        addAndMakeVisible(tutorial); addAndMakeVisible(about);
        for (auto& chipCard : concepts) addAndMakeVisible(chipCard);
        clock.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        // Was TextBoxBelow - see energy's own detailed comment (this
        // component's shared layoutVoiceArea/CLONE constructor) for why
        // this was the real cause of CLOCK's caption sitting low inside
        // the knob, not a sizing/positioning bug.
        clock.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        clock.setRange(0.1, 20.0, 0.01); clock.setValue(2.0);
        // withAlpha - see ObjectFiveComponent's own copy (clockRate) for
        // the full comment.
        clock.setColour(juce::Slider::rotarySliderFillColourId, material::clock.withAlpha(0.6f));
        clock.onValueChange = [this] { sequencer.setClockRate(clock.getValue()); };
        clock.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockRateKnob, uiLanguage));
        addAndMakeVisible(clock);
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
        {
            loopSwitches[i].setButtonText(juce::String(static_cast<int>(i + 1)));
            loopSwitches[i].setComponentID("loop");
            loopSwitches[i].setLookAndFeel(&patchToggleLook());
            loopSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::loopEnd, uiLanguage));
            loopSwitches[i].onClick = [this, i] { setLoopEnd(i + 1); };
            addAndMakeVisible(loopSwitches[i]);
        }
        setLoopEnd(loopSwitches.size());
        configureLabel(temporalLabel, antitotem::ui::text(antitotem::ui::label::pulse, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        temporalLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::temporalHeaderTip, uiLanguage));
        configureLabel(metricLabel, antitotem::ui::text(antitotem::ui::label::meter, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        metricLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricHeaderTip, uiLanguage));
        configureLabel(scannerLabel, antitotem::ui::text(antitotem::ui::label::path, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        scannerLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerHeaderTip, uiLanguage));
        addAndMakeVisible(temporalLabel); addAndMakeVisible(metricLabel); addAndMakeVisible(scannerLabel);
        constexpr std::array<const char*, 8> clockFeelNames { "RET", "3:2", "5:4", "SWG", "7:4", "9:8", "11:8", "GLT" };
        constexpr std::array<const char*, 8> metricNames { "2", "3", "4", "5", "6", "7", "8", "9" };
        for (std::size_t i = 0; i < temporalButtons.size(); ++i)
        {
            temporalButtons[i].setButtonText(clockFeelNames[i]); temporalButtons[i].setRadioGroupId(730);
            temporalButtons[i].setComponentID("core"); temporalButtons[i].setLookAndFeel(&patchToggleLook()); temporalButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockFeelTips[i], uiLanguage));
            temporalButtons[i].onClick = [this, i] { temporalSelection = static_cast<int>(i); syncTemporal(); };
            addAndMakeVisible(temporalButtons[i]);
        }
        temporalButtons[0].setToggleState(true, juce::dontSendNotification);
        // GROOVE - a general long-short modifier layered on every
        // SUBDIVISÃO feel, not exclusive to SWG (20 ago. 2026, "deixa o
        // swing somente enquanto botão, e utilise esse slide atual do
        // swing para o groove"). Defaults to 0 (no effect) so anyone who
        // never touches it hears no change from before.
        configureLabel(grooveLabel, antitotem::ui::text(antitotem::ui::label::groove, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        grooveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, uiLanguage));
        addAndMakeVisible(grooveLabel);
        grooveAmount.setSliderStyle(juce::Slider::LinearHorizontal);
        grooveAmount.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        grooveAmount.setRange(0.0, 1.0, 0.01);
        grooveAmount.setValue(0.0);
        grooveAmount.setColour(juce::Slider::thumbColourId, material::controlBlue);
        grooveAmount.setColour(juce::Slider::trackColourId, material::controlBlue.darker(0.70f));
        grooveAmount.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, uiLanguage));
        grooveAmount.onValueChange = [this] { sequencer.setGrooveAmount(static_cast<float>(grooveAmount.getValue())); };
        addAndMakeVisible(grooveAmount);
        // Split from temporalButtons' own loop, 19 ago. 2026 - metricButtons
        // grew from 4 to 8 (two rows), no longer the same count.
        for (std::size_t i = 0; i < metricButtons.size(); ++i)
        {
            metricButtons[i].setButtonText(metricNames[i]); metricButtons[i].setRadioGroupId(731);
            metricButtons[i].setComponentID("loop"); metricButtons[i].setLookAndFeel(&patchToggleLook()); metricButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricButton, uiLanguage));
            metricButtons[i].onClick = [this, i] { metricSelection = static_cast<int>(i); syncTemporal(); };
            addAndMakeVisible(metricButtons[i]);
        }
        metricButtons[0].setToggleState(true, juce::dontSendNotification);
        constexpr std::array<const char*, 4> scannerNames { "FWD", "REV", "ALT", "MEM" };
        for (std::size_t i = 0; i < scannerButtons.size(); ++i)
        {
            scannerButtons[i].setButtonText(scannerNames[i]); scannerButtons[i].setRadioGroupId(732);
            scannerButtons[i].setComponentID("feedback"); scannerButtons[i].setLookAndFeel(&patchToggleLook()); scannerButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerTips[i], uiLanguage));
            scannerButtons[i].onClick = [this, i] { scannerSelection = static_cast<int>(i); syncScanner(); };
            addAndMakeVisible(scannerButtons[i]);
        }
        scannerButtons[0].setToggleState(true, juce::dontSendNotification);
        constexpr std::array<const char*, 6> connectionNames { "FB", "DIODE", "CAP", "PULSE", "TRANS", "REFLUX" };
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
        {
            connectionSwitches[i].setButtonText(connectionNames[i]);
            connectionSwitches[i].setComponentID("feedback");
            connectionSwitches[i].setLookAndFeel(&patchToggleLook());
            connectionSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackDoorTips[i], uiLanguage));
            connectionSwitches[i].onClick = [this] { syncFeedbackConnections(); };
            addAndMakeVisible(connectionSwitches[i]);
        }
        connectionSwitches[2].setToggleState(true, juce::dontSendNotification);
        configureLabel(feedbackLabel, "FB GAIN", 10.0f, material::returnPath);
        feedbackLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackGain, uiLanguage));
        addAndMakeVisible(feedbackLabel);
        feedbackGain.setSliderStyle(juce::Slider::LinearHorizontal);
        // NoTextBox, not TextBoxRight (18 ago. 2026, author: "não precisa
        // mais de caixa de numero nos sliders da coluna da esquerda").
        feedbackGain.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        feedbackGain.setRange(0.0, 0.72, 0.01); feedbackGain.setValue(0.26);
        feedbackGain.setColour(juce::Slider::thumbColourId, material::returnPath);
        feedbackGain.setColour(juce::Slider::trackColourId, material::returnPath.darker(0.70f));
        feedbackGain.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackGain, uiLanguage));
        feedbackGain.onValueChange = [this] { sequencer.setFeedbackAmount(static_cast<float>(feedbackGain.getValue())); };
        addAndMakeVisible(feedbackGain);
        // Title and slider match the DERIVA button's own resting colour
        // now - see ObjectFiveComponent's own copy for the full comment.
        configureLabel(deriveLabel, antitotem::ui::text(antitotem::ui::label::driftDepthLabel, uiLanguage), 10.0f, material::clock);
        deriveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveHeaderTip, uiLanguage));
        deriveDepth.setSliderStyle(juce::Slider::LinearHorizontal);
        deriveDepth.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        deriveDepth.setRange(0.0, 1.0, 0.01); deriveDepth.setValue(0.46);
        deriveDepth.setColour(juce::Slider::thumbColourId, material::clock);
        deriveDepth.setColour(juce::Slider::trackColourId, material::clock.darker(0.70f));
        deriveDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::driftDepthPhrase, uiLanguage));
        addAndMakeVisible(deriveLabel); addAndMakeVisible(deriveDepth);
        master.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        // NoTextBox, not TextBoxBelow (18 ago. 2026, author: "faça o mesmo
        // procedimento do energia no knob master no cabeçalho") - same root
        // cause as ENERGIA/CLOCK before their own fix: TextBoxBelow reserves
        // its height INSIDE the slider component's own bounds, shifting the
        // rotary circle up within it, so a caption centred on the full
        // component bounds lands below the circle's true centre. See
        // TAREFAS.md for the original diagnosis.
        master.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        master.setRange(0.0, 1.0, 0.01); master.setValue(0.72);
        // controlBlue, not the mixer's own material::voice - MASTER is the
        // one truly single/shared output stage (author, live: "criar cores
        // específica para ele"), so it reads as its own kind of control,
        // not one more fader sharing the channel strips' own colour.
        // withAlpha - softer, same pass as VCF/ADSR/ENERGIA/LFO/NOISE
        // SEND.
        master.setColour(juce::Slider::rotarySliderFillColourId, material::controlBlue.withAlpha(0.6f));
        // dualEngine.setMasterGain, not sequencer.setMasterGain: MASTER is
        // meant to be the one truly single/shared gain in the whole engine
        // (applied after PRINCIPAL and CLONE are already summed), not a
        // per-object pre-mix stage - it was wired to the wrong one before.
        master.onValueChange = [this] { dualEngine.setMasterGain(static_cast<float>(master.getValue())); };
        addAndMakeVisible(master);
        auto configureObjectMixSlider = [this] (juce::Slider& slider, juce::Colour colour)
        {
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider.setRange(0.0, 1.0, 0.01); slider.setValue(1.0);
            slider.setColour(juce::Slider::thumbColourId, colour);
            slider.setColour(juce::Slider::trackColourId, colour.darker(0.72f));
            slider.onValueChange = [this] { syncObjectMix(); };
            addAndMakeVisible(slider);
        };
        configureObjectMixSlider(principalVolume, material::voice);
        configureObjectMixSlider(cloneVolume, cloneMaterial::voice);
        principalVolume.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixPrincipal, uiLanguage));
        cloneVolume.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixClone, uiLanguage));
        // EXCITAÇÃO's own slider - same visual family as PRINC/CLONE, but
        // starts at 0 (off) rather than 1.0, and drives the generative
        // engine directly instead of going through syncObjectMix()/
        // objectChannels (it isn't a channel of an existing object, it's
        // its own generative source - see DualObjectEngine.h).
        excitationAmount.setSliderStyle(juce::Slider::LinearHorizontal);
        excitationAmount.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        excitationAmount.setRange(0.0, 1.0, 0.01);
        excitationAmount.setValue(0.0);
        // Green, same as DERIVA's own slider (20 ago. 2026, author:
        // "altere a cor do slider excit para verde (mesmo verde do botão
        // deriva)") - material::clock, the same colour deriveDepth uses.
        excitationAmount.setColour(juce::Slider::thumbColourId, material::clock);
        excitationAmount.setColour(juce::Slider::trackColourId, material::clock.darker(0.72f));
        excitationAmount.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::excitationAmount, uiLanguage));
        excitationAmount.onValueChange = [this] { dualEngine.setExcitationAmount(static_cast<float>(excitationAmount.getValue())); };
        addAndMakeVisible(excitationAmount);
        // Tooltips reuse each own volume slider's own text (author, live,
        // 20 ago. 2026: "os botoes m dos instumentos object mixer ainda
        // não constam no learn") - none of the three M buttons had a
        // tooltip at all, not just EXCITAÇÃO's new one; the slider's own
        // tooltip already explains what M does ("M/S work like the
        // mixer's own..."), so reusing it avoids a near-duplicate string.
        principalMute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixPrincipal, uiLanguage));
        cloneMute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixClone, uiLanguage));
        for (auto* button : { &principalMute, &cloneMute })
        {
            button->setButtonText("M"); button->setComponentID("mute");
            button->setLookAndFeel(&patchToggleLook());
            button->onClick = [this] { syncObjectMix(); };
            addAndMakeVisible(*button);
        }
        // EXCITAÇÃO's own M button (20 ago. 2026, author: "crie o botão de
        // mute para o excit no object mixer") - same visual family as
        // PRINC/CLONE's, but drives dualEngine.setExcitationMute()
        // directly instead of syncObjectMix()/objectChannels, since
        // EXCITAÇÃO isn't a channel of an existing object.
        excitationMute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::excitationAmount, uiLanguage));
        excitationMute.setButtonText("M"); excitationMute.setComponentID("mute");
        excitationMute.setLookAndFeel(&patchToggleLook());
        excitationMute.onClick = [this] { dualEngine.setExcitationMute(excitationMute.getToggleState()); };
        addAndMakeVisible(excitationMute);
        energy.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        // Was TextBoxBelow - see ObjectFiveComponent's own constructor
        // (this file, energy's matching block) for the detailed root
        // cause: JUCE reserves that text box INSIDE the slider's own
        // bounds, shifting the rotary circle itself up to make room,
        // which broke the caption centring done against the full bounds.
        energy.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        energy.setRange(0.0, 1.0, 0.01); energy.setValue(0.72);
        // withAlpha - see ObjectFiveComponent's own copy for the full
        // comment.
        energy.setColour(juce::Slider::rotarySliderFillColourId, material::voice.withAlpha(0.6f));
        energy.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::energy, uiLanguage));
        energy.onValueChange = [this] { sequencer.setEnergy(static_cast<float>(energy.getValue())); };
        addAndMakeVisible(energy);
        // "NOISE MIX", matching CLONE's own text (author, live:
        // "padronizar para noise mix") - was plain "NOISE" here only.
        // NOISE SEND, not NOISE MIX (18 ago. 2026) - "NOISE GATE" was
        // considered and reverted the same day (the knob itself is still a
        // continuous 0-1 blend, not a threshold/on-off control, and "gate"
        // already names a specific, unrelated audio effect). "SEND" matches
        // the real architecture instead: this knob sends an amount of noise
        // into the voice/RING chain, and the MIXER's own NOISE channel is
        // the bus that receives it, with its own master ON/gain - the same
        // send/aux-bus model as a real mixing console. See TAREFAS.md/
        // FLUXO_DE_SINAL.md for the underlying mechanism.
        constexpr std::array<const char*, 3> modulationNames { "LFO", "RING", "NOISE\nSEND" };
        constexpr std::array<const char*, 3> effectNames { "REVERB", "PHASER", "FLANGER" };
        for (std::size_t i = 0; i < modulationControls.size(); ++i)
        {
            configureLabel(modulationControlLabels[i], modulationNames[i], 9.0f, juce::Colour(0xffded4be));
            // Caption colour matches the slider's own colour - see the
            // matching comment in ObjectFiveComponent's own constructor.
            configureLabel(effectControlLabels[i], effectNames[i], 9.0f, material::phaser);
            addAndMakeVisible(modulationControlLabels[i]); addAndMakeVisible(effectControlLabels[i]);
            // LFO and NOISE MIX are both knobs, RING is horizontal again
            // (18 ago. 2026, second pass) - see ObjectFiveComponent's own
            // copy of this loop for the full comment.
            const auto modulationColour = i == 0 ? material::clock : (i == 1 ? material::controlBlue : material::noiseSend);
            if (i == 0 || i == 2)
            {
                modulationControls[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
                // withAlpha - see ObjectFiveComponent's own copy for the
                // full comment.
                modulationControls[i].setColour(juce::Slider::rotarySliderFillColourId, modulationColour.withAlpha(0.6f));
            }
            else
            {
                modulationControls[i].setSliderStyle(juce::Slider::LinearHorizontal);
                modulationControls[i].setColour(juce::Slider::thumbColourId, modulationColour);
                modulationControls[i].setColour(juce::Slider::trackColourId, modulationColour.darker(0.72f));
            }
            modulationControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            modulationControls[i].setRange(0.0, 1.0, 0.01);
            effectControls[i].setSliderStyle(juce::Slider::LinearHorizontal);
            effectControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            effectControls[i].setRange(0.0, 1.0, 0.01);
            const auto effectColour = material::phaser;
            effectControls[i].setColour(juce::Slider::thumbColourId, effectColour);
            effectControls[i].setColour(juce::Slider::trackColourId, effectColour.darker(0.72f));
            addAndMakeVisible(modulationControls[i]); addAndMakeVisible(effectControls[i]);
        }
        // RING's caption matches MaterialFilter's own MAT caption now -
        // same colour, same centred justification (18 ago. 2026, author:
        // "mesma cor e posição - centrado").
        configureLabel(modulationControlLabels[1], modulationNames[1], 9.0f, juce::Colour(0xff8f856f));
        modulationControlLabels[1].setJustificationType(juce::Justification::centred);
        // LFO/NOISE MIX captions centred over their own knobs too (18
        // ago. 2026) - see ObjectFiveComponent's own copy for the full
        // comment.
        modulationControlLabels[0].setJustificationType(juce::Justification::centred);
        modulationControlLabels[2].setJustificationType(juce::Justification::centred);
        // Same colour as the oscillators' own FREQ/MIX/FORM captions - see
        // ObjectFiveComponent's own copy for the full comment.
        modulationControlLabels[0].setColour(juce::Label::textColourId, juce::Colour(0xff8f856f));
        modulationControlLabels[2].setColour(juce::Label::textColourId, juce::Colour(0xff8f856f));
        modulationControls[0].setValue(0.42); modulationControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfo, uiLanguage));
        modulationControls[1].setValue(0.0); modulationControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::ring, uiLanguage));
        modulationControls[2].setValue(0.0); modulationControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseMod, uiLanguage));
        effectControls[0].setValue(0.0); effectControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::reverb, uiLanguage));
        effectControls[1].setValue(0.0); effectControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::phaser, uiLanguage));
        effectControls[2].setValue(0.0); effectControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::flanger, uiLanguage));
        modulationControls[0].onValueChange = [this] { sequencer.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f)); };
        modulationControls[1].onValueChange = [this] { sequencer.setRingMix(static_cast<float>(modulationControls[1].getValue())); updateSilentHighlight(modulationControls[1], material::controlBlue); };
        modulationControls[2].onValueChange = [this] { sequencer.setNoiseMix(static_cast<float>(modulationControls[2].getValue())); updateSilentHighlight(modulationControls[2], material::noiseSend); };
        updateSilentHighlight(modulationControls[1], material::controlBlue);
        updateSilentHighlight(modulationControls[2], material::noiseSend);
        configureLabel(lfoShapeLabel, antitotem::ui::text(antitotem::ui::label::lfoShape, uiLanguage), 12.0f, material::clock);
        addAndMakeVisible(lfoShapeLabel);
        // CAOS/VAGA (17 ago. 2026) - see ObjectFiveComponent's own comment
        // on this same rail.
        constexpr std::array<const char*, 6> lfoShapeNames { "SEN", "TRI", "PUL", "CAOS", "VAGA", "STEP" };
        for (std::size_t i = 0; i < lfoShapeButtons.size(); ++i)
        {
            lfoShapeButtons[i].setButtonText(lfoShapeNames[i]);
            lfoShapeButtons[i].setRadioGroupId(842);
            lfoShapeButtons[i].setComponentID("core");
            lfoShapeButtons[i].setLookAndFeel(&patchToggleLook());
            lfoShapeButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoShapeTips[i], uiLanguage));
            lfoShapeButtons[i].onClick = [this, i] { lfoShapeSelection = static_cast<int>(i); syncLfoShape(); };
            addAndMakeVisible(lfoShapeButtons[i]);
        }
        lfoShapeButtons[0].setToggleState(true, juce::dontSendNotification);
        // FREEZE (17 ago. 2026) - only meaningful while CAOS/VAGA is
        // selected (see LfoSource's own comment), but always present in
        // the panel rather than conditionally shown/hidden. RESEED (also
        // 17 ago. 2026) had its own button here briefly, removed at the
        // author's request - see deriveFromMemory() for where it fires
        // now.
        lfoFreeze.setButtonText("FRZ");
        lfoFreeze.setComponentID("core");
        lfoFreeze.setLookAndFeel(&patchToggleLook());
        lfoFreeze.setEnabled(false); // SEN is the default selected shape
        lfoFreeze.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoFreeze, uiLanguage));
        lfoFreeze.onClick = [this]
        {
            const auto frozen = lfoFreeze.getToggleState();
            sequencer.setLfoFrozen(frozen);
            patchToggleLook().lfoFrozen = frozen;
            lfoShapeButtons[3].repaint(); lfoShapeButtons[4].repaint();
        };
        addAndMakeVisible(lfoFreeze);
        noiseSelector.onSelection = [this] (int index) { setNoiseColour(index); };
        noiseSelector.onSampleHoldChange = [this] (bool enabled) { sequencer.setSampleHoldMix(enabled ? 0.78f : 0.0f); };
        // Same fixed depth as CLONE's own wiring above - see that
        // instance's comment.
        noiseSelector.onBreathChange = [this] (bool enabled) { sequencer.setNoiseBreathAmount(enabled ? 0.28f : 0.0f); };
        addAndMakeVisible(noiseSelector);
        effectControls[0].onValueChange = [this] { sequencer.setReverbMix(static_cast<float>(effectControls[0].getValue())); updateSilentHighlight(effectControls[0], material::phaser); };
        effectControls[1].onValueChange = [this] { sequencer.setPhaserMix(static_cast<float>(effectControls[1].getValue())); updateSilentHighlight(effectControls[1], material::phaser); };
        effectControls[2].onValueChange = [this] { sequencer.setFlangerMix(static_cast<float>(effectControls[2].getValue())); updateSilentHighlight(effectControls[2], material::phaser); };
        updateSilentHighlight(effectControls[0], material::phaser);
        updateSilentHighlight(effectControls[1], material::phaser);
        updateSilentHighlight(effectControls[2], material::phaser);
        // RES MIX/ALTURA/CORPO (indices 6-8) are the CombResonator's own
        // controls - ROTAS ATIVAS was already a home for "extra parameters
        // of an existing effect" (S&H/reverb/phaser/flanger), so the
        // resonator's join the same list rather than fighting for a new
        // column of its own in an already-full rails band.
        // CUTOFF/RESONANCE/DRIVE/ASYMMETRY (9-12, 17 ago. 2026): MaterialFilter's
        // remaining parameters, previously fixed at an internal value with no
        // panel control - join ROTAS ATIVAS as a 6th column ("MATÉRIA"), same
        // reasoning as RES MIX/ALTURA/CORPO above.
        const std::array<juce::String, 16> detailNames { "S&H RATE", "RVB RET", "PHS RATE", "PHS PROF", "FLG RATE", "FLG PROF",
                                                             "RES MIX", antitotem::ui::text(antitotem::ui::label::resPitch, uiLanguage), antitotem::ui::text(antitotem::ui::label::resBody, uiLanguage),
                                                             "CUTOFF", "RESON", "DRIVE", "ASYM",
                                                             "DRIVE", "DAMPING", "DEPTH" };
        // Defaults 9-12 match the fixed values they replace (CUTOFF 0.5,
        // RESONANCE 0.6, DRIVE 0.5, ASYMMETRY 0.6 - see TAREFAS.md,
        // "Proliferação de módulos"). Defaults 13-15 (CAOS/VAGA's own
        // DRIVE/DAMPING/DEPTH, 18 ago. 2026) match the retuned values
        // LfoSource::prepare() used to hardcode (0.85/0.18/1.0).
        const std::array<double, 16> detailDefaults { 0.28, 0.28, 0.28, 0.5, 0.28, 0.5, 0.0, 0.5, 0.5, 0.5, 0.6, 0.5, 0.6, 0.85, 0.18, 1.0 };
        // MATÉRIA (9-12) and CAOS (13-15) get their own function colour -
        // see the comment above materialRailLabel/chaosRailLabel earlier
        // in this constructor.
        const auto detailColour = [] (std::size_t index)
        {
            if (index >= 9 && index <= 12) return juce::Colour(0xff8f856f);
            if (index >= 13) return material::clock;
            return material::memory;
        };
        for (std::size_t i = 0; i < detailControls.size(); ++i)
        {
            configureLabel(detailControlLabels[i], detailNames[i], 9.0f, detailColour(i));
            detailControls[i].setSliderStyle(juce::Slider::LinearHorizontal);
            detailControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            detailControls[i].setRange(0.0, 1.0, 0.01);
            // RES MIX (6) starts silent, like every other effect mix; RES
            // ALTURA/CORPO (7,8) start at a neutral middle, like PHS/FLG
            // PROF, so the resonator already sounds reasonable the moment
            // its mix is raised.
            detailControls[i].setValue(detailDefaults[i]);
            detailControls[i].setColour(juce::Slider::thumbColourId, detailColour(i));
            detailControls[i].setColour(juce::Slider::trackColourId, detailColour(i).darker(0.70f));
            detailControls[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailControlTips[i], uiLanguage));
            detailControls[i].onValueChange = [this, i] { syncDetails(); if (i == 6) updateSilentHighlight(detailControls[i], material::memory); };
            if (i == 6) updateSilentHighlight(detailControls[i], material::memory);
            addAndMakeVisible(detailControlLabels[i]); addAndMakeVisible(detailControls[i]);
        }
        // "NOISE", matching the term used everywhere else (NOISE title,
        // NOISE MIX rail slider, tooltips) - author, live: "em alguns
        // lugares diz noise em outros ruido, verifiar isso" / picked
        // "NOISE em tudo". Was "RUÍDO" here only.
        const std::array<juce::String, 4> mixerNames { antitotem::ui::text(antitotem::ui::label::mixerChannelNames[0], uiLanguage), antitotem::ui::text(antitotem::ui::label::mixerChannelNames[1], uiLanguage), antitotem::ui::text(antitotem::ui::label::mixerChannelNames[2], uiLanguage), antitotem::ui::text(antitotem::ui::label::mixerChannelNames[3], uiLanguage) };
        // Author, 15 ago. 2026: "os sliders do mixer - Filtro, ring, noise,
        // espaço devem iniciar no valor 1.00" - REVOGADO só pro NOISE (19
        // ago. 2026, autor: "toma conta do áudio", confirmado via
        // AskUserQuestion "Pode mudar o padrão do fader (revogar o pedido
        // antigo)"). Achado real ao investigar: cortar noiseTotal na DSP
        // (*0.42f, tentado e revertido antes) afeta S&H igualmente (mesmo
        // sinal) - o fader do canal é uma alavanca DIFERENTE, escala só o
        // resultado final, sem mexer no caráter S&H/cru entre si. FILTER/
        // RING/SPACE continuam em 1.00 como sempre foi pedido.
        constexpr std::array<double, 4> mixerGainDefaults { 1.0, 1.0, 0.6, 1.0 };
        constexpr std::array<double, 4> mixerPanDefaults { 0.0, -0.24, 0.18, 0.0 };
        constexpr std::array<double, 4> mixerRefluxDefaults { 0.08, 0.18, 0.0, 0.28 };
        constexpr std::array<bool, 4> mixerEnabledDefaults { true, false, false, true };
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            configureLabel(mixLabels[i], mixerNames[i], 11.0f, material::metal);
            mixGain[i].setSliderStyle(juce::Slider::LinearVertical); mixGain[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
            mixGain[i].setRange(0.0, 1.5, 0.01); mixGain[i].setValue(mixerGainDefaults[i]);
            mixGain[i].setColour(juce::Slider::thumbColourId, material::voice); mixGain[i].setColour(juce::Slider::trackColourId, material::voice.darker(0.72f));
            mixPan[i].setSliderStyle(juce::Slider::LinearHorizontal); mixPan[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            mixPan[i].setRange(-1.0, 1.0, 0.01); mixPan[i].setValue(mixerPanDefaults[i]);
            mixPan[i].setColour(juce::Slider::thumbColourId, material::clock); mixPan[i].setColour(juce::Slider::trackColourId, material::clock.darker(0.72f));
            mixReflux[i].setSliderStyle(juce::Slider::LinearHorizontal); mixReflux[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            mixReflux[i].setRange(0.0, 0.72, 0.01); mixReflux[i].setValue(mixerRefluxDefaults[i]);
            mixReflux[i].setColour(juce::Slider::thumbColourId, material::returnPath); mixReflux[i].setColour(juce::Slider::trackColourId, material::returnPath.darker(0.72f));
            mixEnable[i].setButtonText("ON"); mixEnable[i].setComponentID("core"); mixEnable[i].setToggleState(mixerEnabledDefaults[i], juce::dontSendNotification);
            // FILTER/RING sit in series (RING feeds FILTER, FILTER feeds
            // ESPAÇO) - explained here since turning ON off only removes
            // that channel's own contribution, not its processing further
            // down the chain (see TAREFAS.md, 18 ago. 2026).
            if (i == 0) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterChannelSeries, uiLanguage));
            else if (i == 1) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::ringChannelSeries, uiLanguage));
            else if (i == 2) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseChannelSeries, uiLanguage));
            else if (i == 3) mixEnable[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::spaceChannelSeries, uiLanguage));
            mixMute[i].setButtonText("M"); mixMute[i].setComponentID("mute");
            mixMute[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixChannelMute, uiLanguage));
            mixSolo[i].setButtonText("S"); mixSolo[i].setComponentID("loop");
            mixSolo[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixChannelSolo, uiLanguage));
            for (auto* button : { &mixEnable[i], &mixMute[i], &mixSolo[i] }) { button->setLookAndFeel(&patchToggleLook()); button->onClick = [this] { syncMixer(); }; }
            for (auto* slider : { &mixGain[i], &mixPan[i], &mixReflux[i] }) slider->onValueChange = [this] { syncMixer(); };
            mixGain[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelGain, uiLanguage)); mixPan[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelPan, uiLanguage)); mixReflux[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelReturn, uiLanguage));
            addAndMakeVisible(mixLabels[i]); addAndMakeVisible(mixGain[i]); addAndMakeVisible(mixPan[i]); addAndMakeVisible(mixReflux[i]);
            addAndMakeVisible(mixEnable[i]); addAndMakeVisible(mixMute[i]); addAndMakeVisible(mixSolo[i]);
        }
        // Mixer memory: the engine already holds 8 full-channel snapshot
        // slots (MutableMixer::capture/recall); only 4 are exposed here to
        // keep the row compact. CAPTURAR arms the next slot click to save
        // instead of recall, then disarms itself so a stray click can't
        // silently overwrite a slot.
        configureLabel(mixMemoryLabel, antitotem::ui::text(antitotem::ui::label::mixMemory, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        mixMemoryLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixMemoryHeaderTip, uiLanguage));
        addAndMakeVisible(mixMemoryLabel);
        mixMemoryCapture.setButtonText(antitotem::ui::text(antitotem::ui::button::capture, uiLanguage));
        mixMemoryCapture.setClickingTogglesState(true);
        mixMemoryCapture.setComponentID("mute");
        mixMemoryCapture.setLookAndFeel(&panelButtonLook());
        mixMemoryCapture.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixCapture, uiLanguage));
        addAndMakeVisible(mixMemoryCapture);
        for (std::size_t i = 0; i < mixMemorySlots.size(); ++i)
        {
            mixMemorySlots[i].setButtonText("M" + juce::String(i + 1));
            mixMemorySlots[i].setLookAndFeel(&panelButtonLook());
            mixMemorySlots[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixSlotRecall, uiLanguage));
            mixMemorySlots[i].onClick = [this, i]
            {
                if (mixMemoryCapture.getToggleState())
                {
                    sequencer.captureMixMemory(i);
                    mixMemoryCaptured[i] = true;
                    mixMemoryCapture.setToggleState(false, juce::dontSendNotification);
                    appendLog(antitotem::ui::text(antitotem::ui::logText::mixMemoryPrefix, uiLanguage) + juce::String(i + 1) + antitotem::ui::text(antitotem::ui::logText::mixMemoryCapturedSuffix, uiLanguage));
                }
                else
                {
                    sequencer.recallMixMemory(i);
                    pullMixerFromEngine();
                    appendLog(antitotem::ui::text(antitotem::ui::logText::mixMemoryPrefix, uiLanguage) + juce::String(i + 1) + antitotem::ui::text(antitotem::ui::logText::mixMemoryRecalledSuffix, uiLanguage));
                }
            };
            addAndMakeVisible(mixMemorySlots[i]);
        }
        addAndMakeVisible(stereoScope);
        scopeGain.setSliderStyle(juce::Slider::LinearVertical);
        scopeGain.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        // Range was 0.15-3.0 (default 2.145, 70% of it) from when the raw
        // signal rarely exceeded ~0.15-0.3, before any upstream leveling
        // existed (see SignalLeveler.h's history). Now that gain-staging
        // keeps typical peaks around 0.5-0.7 and hot passages near 0.85,
        // that same range let most of the slider's own travel - not just
        // its old default - drive the trace straight into paint()'s
        // waveformBounds clip (author, live, 15 ago. 2026, after already
        // lowering the default once: "quando se sobe no slider, as ondas
        // ficam cortadas... teste com o slider 100%"). Ceiling lowered to
        // 0.56 (see StereoScope::setGain()'s own comment for the exact
        // math) so even the most extreme peak observed that same night
        // (~0.85, deliberately maxed-out patches) stays inside the box at
        // full gain, not just at the default. Default raised to 0.48
        // (from an initial 0.4) after the author found that first pass
        // "muito discreto" - still leaves margin under the 0.56 ceiling
        // even for an extreme peak at the default position.
        scopeGain.setRange(0.15, 0.56, 0.01);
        scopeGain.setValue(0.48);
        scopeGain.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scopeGain, uiLanguage));
        scopeGain.onValueChange = [this] { stereoScope.setGain(static_cast<float>(scopeGain.getValue())); };
        addAndMakeVisible(scopeGain);
        stereoScope.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scopeTrace, uiLanguage));
        constexpr std::array<const char*, 3> coreNames { "40106", "8038", "4069UB" };
        for (std::size_t i = 0; i < coreSwitches.size(); ++i)
        {
            coreSwitches[i].setButtonText(coreNames[i]);
            coreSwitches[i].setComponentID("core");
            coreSwitches[i].setRadioGroupId(102);
            coreSwitches[i].setLookAndFeel(&patchToggleLook());
            coreSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::coreTips[i], uiLanguage));
            coreSwitches[i].onClick = [this, i] { oscillatorCoreSelection = static_cast<int>(i + 1); syncCore(); };
            addAndMakeVisible(coreSwitches[i]);
        }
        coreSwitches[1].setToggleState(true, juce::dontSendNotification);
        for (auto* slider : { &filterCutoff, &filterResonance, &filterDepth })
        {
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider->setRange(0.0, 1.0, 0.01);
            // Teste, see ObjectFiveComponent's own copy for the full
            // comment.
            slider->setColour(juce::Slider::rotarySliderFillColourId, material::vcf.withAlpha(0.6f));
            addAndMakeVisible(*slider);
        }
        filterCutoff.setValue(0.58); filterCutoff.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterCutoff, uiLanguage));
        filterResonance.setValue(0.24); filterResonance.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterResonance, uiLanguage));
        filterDepth.setValue(0.52); filterDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterCvDepth, uiLanguage));
        filterCutoff.onValueChange = [this] { sequencer.setFilterCutoff(static_cast<float>(filterCutoff.getValue())); };
        filterResonance.onValueChange = [this] { sequencer.setFilterResonance(static_cast<float>(filterResonance.getValue())); };
        filterDepth.onValueChange = [this] { sequencer.setFilterCvDepth(static_cast<float>(filterDepth.getValue())); };
        constexpr std::array<const char*, 3> filterControlNames { "FREQ", "RES", "CV" };
        for (std::size_t i = 0; i < filterControlLabels.size(); ++i)
        {
            configureLabel(filterControlLabels[i], filterControlNames[i], 9.0f, juce::Colour(0xff8f856f));
            filterControlLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(filterControlLabels[i]);
        }
        // LPF/BPF/HPF/NCH: four independent toggles, not one cycling
        // button - see ObjectFiveComponent's own comment on this same
        // control (17 ago. 2026, author: "dois ou mais").
        {
            constexpr std::array<const char*, 4> filterModeNames { "LPF", "BPF", "HPF", "NCH" };
            for (std::size_t i = 0; i < filterModeButtons.size(); ++i)
            {
                filterModeButtons[i].setButtonText(filterModeNames[i]);
                filterModeButtons[i].setClickingTogglesState(true);
                filterModeButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterMode, uiLanguage));
                filterModeButtons[i].onClick = [this, i]
                {
                    // At least one stays active always - see
                    // ObjectFiveComponent's own comment on this same
                    // handler (17 ago. 2026).
                    bool anyActive = false;
                    for (auto& button : filterModeButtons) anyActive = anyActive || button.getToggleState();
                    if (!anyActive) filterModeButtons[i].setToggleState(true, juce::dontSendNotification);
                    syncFilter();
                };
                addAndMakeVisible(filterModeButtons[i]);
            }
            filterModeButtons[0].setToggleState(true, juce::dontSendNotification);
            syncFilter();
        }

        // MaterialFilter MIX - see the member declaration's own comment.
        // No on/off button: not just a switch, a continuum (author, live,
        // 17 ago. 2026: "não é só ligar/desligar, é ligar/escalonar/
        // desligar") - same as REVERB/PHASER/FLANGER's own MIX, no switch
        // there either.
        // Same caption colour as VCF's own FREQ/RES/CV (author, live, 17
        // ago. 2026: "mantenha a cor dos titulos do vcf (CV)"), not an
        // accent colour of its own.
        configureLabel(materialFilterLabel, "MAT", 9.0f, juce::Colour(0xff8f856f));
        materialFilterLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(materialFilterLabel);
        materialFilterMix.setSliderStyle(juce::Slider::LinearHorizontal);
        materialFilterMix.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        materialFilterMix.setRange(0.0, 1.0, 0.01);
        materialFilterMix.setValue(0.0);
        materialFilterMix.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialFilterMix, uiLanguage));
        // Same filter colour as MATÉRIA's own CUTOFF/RESON/DRIVE/ASYM rail
        // sliders (18 ago. 2026) - MAT is their wet/dry crossfade, so it
        // should read as visually part of that same group. Was returnPath.
        materialFilterMix.onValueChange = [this] { sequencer.setMaterialFilterMix(static_cast<float>(materialFilterMix.getValue())); updateSilentHighlight(materialFilterMix, juce::Colour(0xff8f856f)); };
        updateSilentHighlight(materialFilterMix, juce::Colour(0xff8f856f));
        addAndMakeVisible(materialFilterMix);
        sequencer.setMaterialFilterMix(0.0f);
        // CUTOFF/RESONANCE/DRIVE/ASYMMETRY are no longer fixed here - the
        // MATÉRIA rail sliders (detailControls[9-12] below) now own them,
        // set via syncDetails() at construction.

        constexpr std::array<const char*, 4> contourNames { "ATT", "DEC", "SUS", "REL" };
        constexpr std::array<double, 4> contourDefaults { 0.30, 0.56, 0.62, 0.60 };
        for (std::size_t i = 0; i < envelopeControls.size(); ++i)
        {
            configureLabel(envelopeControlLabels[i], contourNames[i], 9.0f, juce::Colour(0xff8f856f));
            envelopeControlLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(envelopeControlLabels[i]);
            envelopeControls[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            envelopeControls[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            envelopeControls[i].setRange(0.0, 1.0, 0.01);
            envelopeControls[i].setValue(contourDefaults[i]);
            // withAlpha - see ObjectFiveComponent's own copy for the
            // full comment.
            envelopeControls[i].setColour(juce::Slider::rotarySliderFillColourId, material::adsr.withAlpha(0.6f));
            addAndMakeVisible(envelopeControls[i]);
        }
        envelopeControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeAttack, uiLanguage));
        envelopeControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeDecay, uiLanguage));
        envelopeControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeSustain, uiLanguage));
        envelopeControls[3].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeRelease, uiLanguage));
        envelopeControls[0].onValueChange = [this] { sequencer.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue())); };
        envelopeControls[1].onValueChange = [this] { sequencer.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue())); };
        envelopeControls[2].onValueChange = [this] { sequencer.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue())); };
        envelopeControls[3].onValueChange = [this] { sequencer.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue())); };
        // OSC4 (index 3) is the 4093/4020 study: a relaxation source divided
        // deep enough to read as a slow pulse, not a fourth voice tuned like
        // A/B/C. OSC5 (index 4) is the 4046/LM13600 study: a PLL-style VCO
        // ring-modulated against OSC A, so its FREQ ratio sets how far it
        // drifts from OSC A's pitch rather than a fixed voice tuning. Both
        // share the same MIX/FORMA/EIXO X controls (the engine mixes them
        // through the same generic per-oscillator loop) and start silent
        // (level 0) so existing patches are unaffected until raised.
        // Single line (author, live: "nos titulos dos osciladores vamos
        // usar somente uma linha... para o oscilador 4 deixe somente OSC 4
        // - SUB/DIV e oscilador 5 OSC 5 - HETERO") - dropped the second
        // "FREQ / MIX / FORMA" line these used to carry.
        constexpr std::array<const char*, 5> oscillatorNames { "OSC A", "OSC B", "OSC C", "OSC 4 - SUB/DIV", "OSC 5 - HETERO" };
        constexpr std::array<double, 5> oscillatorDefaults { 0.62, 0.38, 0.28, 0.0, 0.0 };
        constexpr std::array<double, 5> shapeDefaults { 0.0, 2.0, 3.0, 0.0, 1.0 };
        constexpr std::array<double, 5> ratioDefaults { 1.0, 0.73, 1.51, 0.25, 0.87 };
        for (std::size_t i = 0; i < oscillators.size(); ++i)
        {
            configureLabel(oscillatorLabels[i], utf8(oscillatorNames[i]), 12.0f, juce::Colour(0xffded4be));
            addAndMakeVisible(oscillatorLabels[i]);
            oscillatorRates[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            oscillatorRates[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            if (i == 3)
            {
                oscillatorRates[i].setRange(0.03125, 4.0, 0.001);
                oscillatorRates[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::osc4Freq, uiLanguage));
            }
            else if (i == 4)
            {
                oscillatorRates[i].setRange(0.125, 4.0, 0.01);
                oscillatorRates[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::osc5Freq, uiLanguage));
            }
            else
            {
                oscillatorRates[i].setRange(0.125, 4.0, 0.01);
                oscillatorRates[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::oscFreqGeneric, uiLanguage));
            }
            oscillatorRates[i].setValue(ratioDefaults[i]);
            oscillatorRates[i].onValueChange = [this, i] { sequencer.setOscillatorRatio(i, static_cast<float>(oscillatorRates[i].getValue())); };
            addAndMakeVisible(oscillatorRates[i]);
            configureLabel(oscillatorRateLabels[i], "FREQ", 9.0f, juce::Colour(0xff8f856f));
            oscillatorRateLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorRateLabels[i]);
            oscillators[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            oscillators[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillators[i].setRange(0.0, 1.0, 0.01); oscillators[i].setValue(oscillatorDefaults[i]);
            oscillators[i].setTooltip(i == 4 ? antitotem::ui::text(antitotem::ui::tooltip::mixRingProduct, uiLanguage)
                                              : antitotem::ui::text(antitotem::ui::tooltip::mixGeneric, uiLanguage));
            oscillators[i].onValueChange = [this, i] { sequencer.setOscillatorLevel(i, static_cast<float>(oscillators[i].getValue())); updateSilentHighlightDefault(oscillators[i]); };
            updateSilentHighlightDefault(oscillators[i]);
            addAndMakeVisible(oscillators[i]);
            configureLabel(oscillatorLevelLabels[i], "MIX", 9.0f, juce::Colour(0xff8f856f));
            oscillatorLevelLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorLevelLabels[i]);
            shapes[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            shapes[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            shapes[i].setRange(0.0, 3.0, 0.01); shapes[i].setValue(shapeDefaults[i]);
            shapes[i].setName(i == 3 ? utf8("FORMA: assimetria do pulso (largo ↔ estreito)")
                                      : utf8("FORMA: senoide → triangular → serra → quadrada"));
            shapes[i].onValueChange = [this, i] { sequencer.setOscillatorShape(i, static_cast<float>(shapes[i].getValue())); };
            addAndMakeVisible(shapes[i]);
            configureLabel(oscillatorShapeLabels[i], antitotem::ui::text(antitotem::ui::label::shape, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorShapeLabels[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorShapeLabels[i]);
            oscillatorPans[i].setSliderStyle(juce::Slider::LinearHorizontal);
            oscillatorPans[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorPans[i].setRange(-1.0, 1.0, 0.01); oscillatorPans[i].setValue(i == 0 ? -0.58 : (i == 2 ? 0.58 : (i == 4 ? -0.24 : 0.0)));
            oscillatorPans[i].setName(utf8("EIXO X: esquerda ↔ direita"));
            oscillatorPans[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisXGeneric, uiLanguage));
            oscillatorPans[i].onValueChange = [this, i] { sequencer.setOscillatorPan(i, static_cast<float>(oscillatorPans[i].getValue())); };
            addAndMakeVisible(oscillatorPans[i]);
            configureLabel(oscillatorPanCaptions[i], antitotem::ui::text(antitotem::ui::label::axisX, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorPanCaptions[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorPanCaptions[i]);
            oscillatorProximities[i].setSliderStyle(juce::Slider::LinearHorizontal);
            oscillatorProximities[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorProximities[i].setRange(0.0, 1.0, 0.01); oscillatorProximities[i].setValue(0.0);
            oscillatorProximities[i].setName(utf8("EIXO Y: proximidade/materialidade"));
            oscillatorProximities[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisYGeneric, uiLanguage));
            oscillatorProximities[i].onValueChange = [this, i] { sequencer.setOscillatorProximity(i, static_cast<float>(oscillatorProximities[i].getValue())); };
            addAndMakeVisible(oscillatorProximities[i]);
            configureLabel(oscillatorProximityCaptions[i], antitotem::ui::text(antitotem::ui::label::axisY, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorProximityCaptions[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorProximityCaptions[i]);
            oscillatorOrbits[i].setSliderStyle(juce::Slider::LinearHorizontal);
            oscillatorOrbits[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            oscillatorOrbits[i].setRange(0.0, 1.0, 0.01); oscillatorOrbits[i].setValue(0.0);
            oscillatorOrbits[i].setName(utf8("EIXO Z: órbita/altura"));
            oscillatorOrbits[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisZGeneric, uiLanguage));
            oscillatorOrbits[i].onValueChange = [this, i] { sequencer.setOscillatorOrbit(i, static_cast<float>(oscillatorOrbits[i].getValue())); };
            addAndMakeVisible(oscillatorOrbits[i]);
            configureLabel(oscillatorOrbitCaptions[i], antitotem::ui::text(antitotem::ui::label::axisZ, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorOrbitCaptions[i].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(oscillatorOrbitCaptions[i]);
        }
        run.setButtonText("PLAY");
        run.onClick = [this] { dualEngine.setRunning(true); appendLog(antitotem::ui::text(antitotem::ui::logText::playLog, uiLanguage)); };
        stop.setButtonText("STOP"); stop.onClick = [this]
        {
            dualEngine.setRunning(false);
            appendLog(antitotem::ui::text(antitotem::ui::logText::stopLog, uiLanguage));
            // REC's loop-end quantization (recordingStopPending/atLoopStart
            // in timerCallback()) needs the PRINCIPAL sequencer running to
            // ever reach that boundary again - halting the transport here
            // would otherwise leave a pending recording hanging forever
            // with its file never closed. Finalize/cancel immediately
            // instead, same as the equivalent record.onClick branches.
            if (recordingArmed)
            {
                recorder.stop();
                finishMidiRecording();
                recordingArmed = false; recordingActive = false; recordingStopPending = false;
                record.setToggleState(false, juce::dontSendNotification);
                for (auto& button : recordDurations) button.setToggleState(false, juce::dontSendNotification);
                configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recCancelledByStopShort, uiLanguage), 12.0f, material::returnPath);
                appendLog(antitotem::ui::text(antitotem::ui::logText::recCancelledByStop, uiLanguage));
            }
            else if (recordingActive || recordingStopPending)
            {
                recordingActive = false; recordingStopPending = false;
                recorder.stop();
                finishMidiRecording();
                record.setToggleState(false, juce::dontSendNotification);
                for (auto& button : recordDurations) button.setToggleState(false, juce::dontSendNotification);
                configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recFinishedByStopShort, uiLanguage), 12.0f, material::returnPath);
                appendLog(antitotem::ui::text(antitotem::ui::logText::recFinishedByStop, uiLanguage));
            }
        };
        // DualObjectEngine has no reset() of its own (object1/object5 each
        // keep independent step/phase state) - resetting only object1 would
        // silently leave object5 out of sync whenever OBJETO 5 is open.
        reset.setButtonText("RESET"); reset.onClick = [this] { sequencer.reset(); dualEngine.object5().reset(); appendLog(antitotem::ui::text(antitotem::ui::logText::resetLog, uiLanguage)); };
        record.setButtonText("REC");
        record.setClickingTogglesState(true);
        record.onClick = [this]
        {
            if (record.getToggleState())
            {
                recordingEvent = 0;
                if (!recorder.start(currentSampleRate, recordingLengthSeconds))
                {
                    record.setToggleState(false, juce::dontSendNotification);
                    for (auto& button : recordDurations) button.setToggleState(false, juce::dontSendNotification);
                }
                else
                {
                    // Armed, not recording yet - the file/writer exist
                    // already (so the WAV header/timestamp reflect when
                    // REC was pressed), but no sample is actually written
                    // until timerCallback() sees the PRINCIPAL sequencer
                    // reach step 1. If it's already sitting at step 1 (or
                    // not running at all), this simply waits for the next
                    // time it gets there.
                    recordingArmed = true; recordingActive = false; recordingStopPending = false;
                    // Captura MIDI armada junto com o WAV, mesmo stamp
                    // (ver o comentário do membro `midiRecordingStamp`).
                    midiRecordingStamp = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
                    midiCapture.start();
                    if (deriveButton.getToggleState()) captureDerivationMemory();
                    configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recWaitingStep1, uiLanguage), 12.0f, material::returnPath);
                    appendLog(antitotem::ui::text(antitotem::ui::logText::recArmedWaiting, uiLanguage));
                }
            }
            else if (recordingActive)
            {
                // Already actually recording - let the current loop cycle
                // finish (same step-1 boundary, one cycle later) instead
                // of cutting off mid-loop; timerCallback() finalizes the
                // file once that boundary is reached.
                recordingStopPending = true;
                configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recFinishingAtLoopEnd, uiLanguage), 12.0f, material::returnPath);
                appendLog(antitotem::ui::text(antitotem::ui::logText::recStopRequested, uiLanguage));
            }
            else
            {
                // Still armed, never actually started writing - nothing
                // to finish, safe to cancel outright.
                recorder.stop();
                finishMidiRecording();
                recordingArmed = false; recordingActive = false; recordingStopPending = false;
                for (auto& button : recordDurations) button.setToggleState(false, juce::dontSendNotification);
                configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recCancelledRecording, uiLanguage), 12.0f, material::returnPath);
                appendLog(antitotem::ui::text(antitotem::ui::logText::recCancelledBeforeStart, uiLanguage));
            }
        };
        // Four explicit buttons (1/2/3/5 min) instead of one button cycling
        // through hidden state - each picks its length and starts REC at
        // once, rather than making the performer guess what a repeated
        // click on a single "FAIXA" button currently means.
        constexpr std::array<double, 4> recordDurationSeconds { 60.0, 120.0, 180.0, 300.0 };
        constexpr std::array<const char*, 4> recordDurationLabels { "1 MIN", "2 MIN", "3 MIN", "5 MIN" };
        // Reuses the palette's existing per-role colours (patchToggleLook)
        // so each duration reads distinctly - blue/pink/amber/red - and,
        // crucially, only the active one is lit: these are not
        // setClickingTogglesState buttons, their toggle state is managed by
        // hand alongside REC's own, so stopping (manually or on timeout)
        // clears all four back to neutral instead of leaving one stuck lit.
        constexpr std::array<const char*, 4> recordDurationIds { "core", "loop", nullptr, "feedback" };
        for (std::size_t i = 0; i < recordDurations.size(); ++i)
        {
            auto& button = recordDurations[i];
            button.setButtonText(recordDurationLabels[i]);
            if (recordDurationIds[i] != nullptr) button.setComponentID(recordDurationIds[i]);
            button.setLookAndFeel(&patchToggleLook());
            button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::recordDurationPrefix, uiLanguage)
                + juce::String(static_cast<int>(recordDurationSeconds[i] / 60.0))
                + antitotem::ui::text(antitotem::ui::tooltip::recordDurationSuffix, uiLanguage));
            button.onClick = [this, i, recordDurationSeconds]
            {
                recordingLengthSeconds = recordDurationSeconds[i];
                for (auto& other : recordDurations) other.setToggleState(false, juce::dontSendNotification);
                recordDurations[i].setToggleState(true, juce::dontSendNotification);
                // Picking a duration while a take is already in progress
                // restarts it immediately (not quantized) - a deliberate
                // simplification: the performer just asked for a
                // different take altogether, not a graceful handoff.
                if (record.getToggleState()) { recorder.stop(); finishMidiRecording(); }
                recordingEvent = 0;
                if (!recorder.start(currentSampleRate, recordingLengthSeconds))
                {
                    record.setToggleState(false, juce::dontSendNotification);
                    recordDurations[i].setToggleState(false, juce::dontSendNotification);
                    recordingArmed = false; recordingActive = false; recordingStopPending = false;
                }
                else
                {
                    record.setToggleState(true, juce::dontSendNotification);
                    // Armed, not recording yet - see record.onClick above
                    // for why (waits for the PRINCIPAL sequencer's step 1).
                    recordingArmed = true; recordingActive = false; recordingStopPending = false;
                    // Captura MIDI armada junto com o WAV, mesmo stamp -
                    // ver record.onClick acima pro racional completo.
                    midiRecordingStamp = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
                    midiCapture.start();
                    if (deriveButton.getToggleState()) captureDerivationMemory();
                    configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recWaitingStep1, uiLanguage), 12.0f, material::returnPath);
                    appendLog(antitotem::ui::text(antitotem::ui::logText::recArmedPrefix, uiLanguage) + juce::String(static_cast<int>(recordingLengthSeconds / 60.0)) + antitotem::ui::text(antitotem::ui::logText::recArmedSuffix, uiLanguage));
                }
            };
            addAndMakeVisible(button);
        }
        for (auto* button : { &soundPage, &sequencePage, &mixPage, &tutorial, &about,
                              &languageSwitch, &objectFive, &monitorModeToggle,
                              &pulseVariation, &porousVariation, &heterodyneVariation,
                              &randomizeStepsButton, &orbitVariation, &pendulumVariation,
                              &run, &stop, &reset, &record })
            button->setLookAndFeel(&panelButtonLook());
        for (auto& button : filterModeButtons) button.setLookAndFeel(&panelButtonLook());
        addAndMakeVisible(run); addAndMakeVisible(stop); addAndMakeVisible(reset); addAndMakeVisible(record);
        for (auto& step : steps)
        {
            addAndMakeVisible(step);
            step.cv.onValueChange = [this] { syncCV(); };
            step.level.onValueChange = [this] { syncStepDynamics(); };
            step.send.onValueChange = [this] { syncStepDynamics(); };
            step.mute.onClick = [this] { syncStepDynamics(); };
        }
#if ANTITOTEM_HAS_LOGO
        logo = juce::Drawable::createFromImageData(BinaryData::logo_antitotem_novo_svg,
                                                    BinaryData::logo_antitotem_novo_svgSize);
        // The original red stays documented in the palette, but the header
        // uses the active-audio amber so red remains reserved for REC/retorno.
        if (logo != nullptr) logo->replaceColour(material::historicalLogoRed, material::historicalLogoRed);
#endif
        // Single-instance in the engine (see the audit in
        // JANELA_UNICA_E_MONITORES.md) - stay visible regardless of which
        // body (PRINCIPAL/CLONE) setShowingCloneBody() is showing.
        // soundPage/sequencePage/mixPage are the opposite case - they must
        // stay excluded from the toggle loop too, but because they should
        // stay *hidden* always in unified layout (setPage() already
        // manages their visibility based on useUnifiedLayout(), a concern
        // orthogonal to PRINCIPAL/CLONE) - without this, switching back to
        // PRINCIPAL force-showed them regardless of layout mode, a real
        // bug a live test caught (SOM/SEQUÊNCIA/MIX staying visible,
        // overlapping the oscillator area, after every toggle back).
        alwaysVisibleInBody = {
            &title, &flow, &footer, &stereoScope, &scopeGain,
            &run, &stop, &reset, &record, &recordingLabel, &recordDurationsLabel,
            &master, &masterLabel, &monitorModeToggle, &objectFive, &languageSwitch, &tutorial, &about,
            &objectMixLabel, &principalVolumeLabel, &principalVolume, &principalMute,
            &cloneVolumeLabel, &cloneVolume, &cloneMute,
            // EXCITAÇÃO is the same kind of single-instance-in-the-engine
            // control as PRINCIPAL/CLONE's own mixer row just above (one
            // shared voice, not one per object) - missing from this list
            // entirely (19 ago. 2026, author: "o excit não está visivel
            // durante a aba clone") meant setShowingCloneBody's own loop
            // hid it like any ordinary PRINCIPAL-only control the moment
            // CLONE's body was shown, the exact bug this list exists to
            // prevent for objectMixLabel/principalVolume/cloneVolume.
            &excitationLabel, &excitationAmount, &excitationMute,
            &objectConnectionLabel, &gainToFifthLabel, &gainToFirstLabel, &auxToFirstLabel, &auxToFifthLabel,
            &gainToFifth, &gainToFirst, &auxToFirst, &auxToFifth,
            &routesToFifthLabel, &routesToFirstLabel, &log, &logLabel,
            &learnLabel, &learnEditor,
            &soundPage, &sequencePage, &mixPage
        };
        for (auto& button : recordDurations) alwaysVisibleInBody.push_back(&button);
        for (auto& button : routesToFifth) alwaysVisibleInBody.push_back(&button);
        for (auto& button : routesToFirst) alwaysVisibleInBody.push_back(&button);
        setPage(Page::sound);
        setSize(1320, 860);
        // setSize can cross the unified-layout threshold, so reapply
        // visibility after the component has its final initial bounds.
        setPage(Page::sound);
        // Both setPage() calls above ran before MainWindow imposed its own
        // real size (it starts at 1600x920, itself right at the unified
        // threshold, before growing to the display's full bounds a moment
        // later) - useUnifiedLayout() legitimately read false at least
        // once in between, showing SOM/SEQUÊNCIA/MIX for a visible frame,
        // overlapping the oscillator area. Force them hidden again here;
        // resized()'s own unifiedNow/unifiedVisibilityApplied transition
        // still corrects this properly if the window is ever genuinely
        // resized below the threshold later.
        soundPage.setVisible(false); sequencePage.setVisible(false); mixPage.setVisible(false);
        // A cada reinicialização, começar de uma configuração variada em vez
        // de sempre o mesmo estado padrão (autor: "está sempre começando
        // igual"). Reaproveita as 5 VARIAÇÕES já compostas (PULSO/POROSA/
        // HETERÓDINA/ÓRBITA/PÊNDULO) em vez de uma perturbação bruta -
        // mesmo princípio de "perturbação controlada" que RND16 já segue -
        // e depois perturba os 16 steps dentro dos mesmos limites seguros
        // do próprio RND16. randomState continua com semente fixa por
        // padrão (reprodutível para os testes automatizados); só a escolha
        // de qual variação abrir, e a semente do RNG interno do PRINCIPAL,
        // usam entropia real de lançamento aqui. CLONE fica de fora desta
        // rodada - sua UI (ObjectFiveComponent) só é construída na primeira
        // vez que a janela abre, e reaplicaria seus próprios valores fixos
        // por cima de qualquer coisa aplicada a `fifth` agora; só a semente
        // do RNG dela é adiantada, para que RND16/MEM no CLONE também não
        // repitam sempre o mesmo padrão quando usados.
        {
            auto& launchRandom = juce::Random::getSystemRandom();
            sequencer.seedRandom(static_cast<unsigned int>(launchRandom.nextInt64()));
            dualEngine.object5().seedRandom(static_cast<unsigned int>(launchRandom.nextInt64()));
            applyVariation(static_cast<Variation>(launchRandom.nextInt(5)));
            sequencer.randomizeSteps();
            refreshStepControls();
            refreshSilentHighlights();
        }
        setAudioChannels(0, 2); startTimerHz(30);
    }
    ~MainComponent() override
    {
        juce::Desktop::getInstance().removeFocusChangeListener(this);
        shutdownAudio();
    }
    void prepareToPlay(int, double rate) override
    {
        loggingEnabled = false;
        currentSampleRate = rate;
        dualEngine.prepare(rate); sequencer.setClockRate(clock.getValue());
        sequencer.setLoopEnd(selectedLoopEnd());
        dualEngine.setMasterGain(static_cast<float>(master.getValue())); sequencer.setEnergy(static_cast<float>(energy.getValue()));
        sequencer.setFeedbackAmount(static_cast<float>(feedbackGain.getValue()));
        syncCV(); syncStepDynamics(); syncOscillators(); syncRatios(); syncShapes(); syncPans(); syncProximities(); syncOrbits(); syncFilter(); syncEnvelope(); syncModulation(); syncEffects(); syncDetails(); syncFeedbackConnections(); syncCore(); syncTemporal(); syncScanner(); syncMixer(); dualEngine.setRunning(true);
        loggingEnabled = true;
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
    {
        info.clearActiveBufferRegion();
        auto* left = info.buffer->getWritePointer(0, info.startSample);
        auto* right = info.buffer->getNumChannels() > 1 ? info.buffer->getWritePointer(1, info.startSample) : left;
        dualEngine.render(left, right, static_cast<std::size_t>(info.numSamples));
        for (int sample = 0; sample < info.numSamples; ++sample) stereoScope.push(left[sample], right[sample]);
        // Gated by recordingActive, not recorder.isRecording() - the
        // writer/file already exist while merely "armed" (waiting for the
        // PRINCIPAL sequencer's step 1), but no sample gets written until
        // that boundary actually arrives - see timerCallback().
        if (recordingActive) recorder.write(left, right, info.numSamples);
        // Captura MIDI (docs/PESQUISA_COMPASSO_E_METRICA_REAL.md, seção
        // 5), 20 ago. 2026, autor: "é possível extrair a partitura da
        // melodia? ou a partitura rítmica, ou completa" - mesmo gate que
        // a gravação de WAV logo acima (recordingActive), granularidade
        // de UM POLL por callback (ver SimpleSequencer::
        // didStepSoundSincePoll()/DualObjectEngine::
        // didExcitationTriggerSincePoll()). PRINCIPAL (trilha 0)/CLONE
        // (trilha 1)/EXCITAÇÃO (trilha 2) em trilhas separadas no mesmo
        // arquivo - a notação decide depois qual(is) mostrar.
        //
        // Mute do object mixer respeitado (bug real, 20 ago. 2026, autor
        // testou e reportou "quando há algum instrumento mutado no
        // object mixer ele produz notas no midi" - confirmado abrindo o
        // .mid exportado: 251 notas de PRINCIPAL + 124 de CLONE mesmo
        // com os dois mutados, só EXCITAÇÃO audível). Causa: o mute do
        // mixer só zera a SAÍDA de áudio final (`DualObjectEngine::
        // render()`/`excitationGain`) - `SimpleSequencer`/
        // `MelodicInterpreter` continuam disparando por dentro por
        // design (pra retomar exatamente de onde estavam ao desmutar,
        // ver o comentário de `objectChannels`/`excitationMuted` em
        // DualObjectEngine.h), e meus ganchos de captura viviam lá
        // dentro, sem saber do mute do mixer. `didStepSoundSincePoll()`/
        // `didExcitationTriggerSincePoll()` continuam sendo chamados
        // SEMPRE (drena o flag "desde o último poll" mesmo mutado, pra
        // não vazar um disparo fantasma quando desmutar depois) - só o
        // `noteOn()` em si fica condicionado ao mute NÃO estar ativo.
        if (recordingActive)
        {
            midiCapture.advance(static_cast<double>(info.numSamples) / currentSampleRate);
            // Andamento variável (20 ago. 2026) - grava o BPM instantâneo a
            // cada callback, não só uma vez no fim da tomada; ver o
            // comentário de `MidiCapture::TempoEvent`/`computeCurrentBpm()`.
            midiCapture.recordTempo(computeCurrentBpm());
            const auto principalSounded = sequencer.didStepSoundSincePoll();
            if (principalSounded && !principalMute.getToggleState())
                midiCapture.noteOn(0, pitch01ToMidiNote(sequencer.getLastSoundingPitch01()), velocityFromLevel(sequencer.getLastSoundingLevel()));
            const auto cloneSounded = dualEngine.object5().didStepSoundSincePoll();
            if (cloneSounded && !cloneMute.getToggleState())
                midiCapture.noteOn(1, pitch01ToMidiNote(dualEngine.object5().getLastSoundingPitch01()), velocityFromLevel(dualEngine.object5().getLastSoundingLevel()));
            const auto excitationTriggered = dualEngine.didExcitationTriggerSincePoll();
            if (excitationTriggered && !excitationMute.getToggleState())
                midiCapture.noteOn(2, pitch01ToMidiNote(dualEngine.getExcitationLastTriggerPitch01()), 96);
        }
    }
    void releaseResources() override {}
    // BPM instantâneo real, a partir do PRINCIPAL (`sequencer`) - vem
    // de CLOCK/ENERGIA/SUBDIVISÃO, os mesmos três eixos que
    // `SimpleSequencer::samplesPerStep()` usa de verdade pra tocar
    // áudio (`getAverageSamplesPerStep()` já inclui ENERGIA/
    // `supplyClock`, 0.46x-1.24x, e a média de SUBDIVISÃO/
    // `tupleDuration` - ver o comentário original do bug de BPM, autor:
    // "no midi está super rápido"). Fatorado pra fora de
    // `finishMidiRecording()` (20 ago. 2026, andamento variável) pra
    // poder ser chamado A CADA callback (`recordTempo`) e não só uma
    // vez no fim da tomada - CLONE fica de fora (clock próprio e
    // independente, ver PESQUISA_COMPASSO_E_METRICA_REAL.md, seção 6,
    // item 1) - o mesmo limite que já existia antes desta mudança.
    [[nodiscard]] double computeCurrentBpm() const
    {
        const auto stepsPerBeat = static_cast<double>(juce::jmax<unsigned int>(1U, sequencer.getStepsPerBeat()));
        const auto secondsPerStep = sequencer.getAverageSamplesPerStep() / currentSampleRate;
        const auto secondsPerBeat = secondsPerStep * stepsPerBeat;
        return secondsPerBeat > 0.0 ? 60.0 / secondsPerBeat : 120.0;
    }
    // Fecha e exporta a captura MIDI da tomada atual (docs/
    // PESQUISA_COMPASSO_E_METRICA_REAL.md, seção 5), 20 ago. 2026 - par
    // de `recorder.stop()` em todo ponto que finaliza/cancela REC.
    // Seguro chamar mesmo sem nada gravado (`writeMidiCaptureToFile`
    // não faz nada com uma lista vazia de eventos) - não precisa
    // distinguir "cancelado" de "terminado de verdade" em cada um dos
    // 5 pontos onde `recorder.stop()` já é chamado, só chamar sempre.
    void finishMidiRecording()
    {
        // Último ponto de tempo antes de parar (`recordTempo()` só
        // aceita enquanto `active` - por isso vem ANTES de `stop()`,
        // não depois) - garante que exista pelo menos um `TempoEvent`
        // mesmo numa tomada tão curta que nunca passou por
        // `getNextAudioBlock()` com `recordingActive` == true.
        midiCapture.recordTempo(computeCurrentBpm());
        midiCapture.stop();
        auto capture = midiCapture.finish();
        if (capture.notes.empty()) return;
        const auto configuredDirectory = juce::SystemStats::getEnvironmentVariable("ANTITOTEM_RECORDINGS_DIR", {});
        auto directory = configuredDirectory.isNotEmpty() ? juce::File(configuredDirectory)
                                                           : juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("Antitotem Objeto Sonoro");
        directory.createDirectory();
        const auto file = directory.getChildFile("ANTITOTEM_" + midiRecordingStamp + ".mid");
        const auto timeSignature = sequencer.getTimeSignature();
        if (writeMidiCaptureToFile(capture.notes, capture.tempos, file, timeSignature.beatsPerMeasure, timeSignature.beatUnit))
            appendLog(utf8("MIDI exportado: ") + file.getFileName());
        // MusicXML (20 ago. 2026, docs/PESQUISA_REPRESENTACAO_MUSICAL_
        // RASGO.md, CRI-SCR-001, autor: "vamos implementar o musicxml") -
        // mesmo timestamp, mesmos eventos capturados, terceiro arquivo do
        // mesmo par (.wav/.mid/.musicxml) reconhecível como uma tomada só.
        const auto musicXmlFile = directory.getChildFile("ANTITOTEM_" + midiRecordingStamp + ".musicxml");
        if (writeMusicXmlCaptureToFile(capture.notes, capture.tempos, musicXmlFile, timeSignature.beatsPerMeasure,
                                        timeSignature.beatUnit, sequencer.getStepsPerBeat()))
            appendLog(utf8("MusicXML exportado: ") + musicXmlFile.getFileName());
    }
    // LEARN: mouseEnter (recursive listener installed via
    // addMouseListener(this, true) in the constructor) covers pointer
    // hover; globalFocusChanged covers keyboard/tab focus - Navalha's own
    // LEARNING MODE listens to both for the same reason (a knob reached by
    // Tab, not the mouse, still deserves an explanation).
    void mouseEnter(const juce::MouseEvent& event) override
    {
        explainHovered(event.originalComponent);
    }
    void globalFocusChanged(juce::Component* focusedComponent) override
    {
        explainHovered(focusedComponent);
    }
    // Writes straight into the fixed box at the end of the transport
    // column (learnEditor) - no positioning to compute, unlike an
    // earlier floating version of this the author tried live and asked
    // to replace ("crie a caixa dedicada").
    // Falls back to the idle text when nothing along the ancestor chain
    // has a tooltip, instead of silently leaving whatever was explained
    // last on screen - see ObjectFiveComponent's own copy of this same
    // comment for the full "many knobs have no tooltip yet" context.
    void explainHovered(juce::Component* component)
    {
        while (component != nullptr && component != this)
        {
            if (auto* tooltipClient = dynamic_cast<juce::TooltipClient*>(component))
            {
                const auto text = tooltipClient->getTooltip();
                if (text.isNotEmpty())
                {
                    learnEditor.setText(text, false);
                    return;
                }
            }
            component = component->getParentComponent();
        }
        learnEditor.setText(antitotem::ui::text(antitotem::ui::tooltip::learnPanelIdle, uiLanguage), false);
    }
    void paint(juce::Graphics& g) override
    {
        // Flat fill again - the gradient test (option 1) didn't land, back
        // to the plain colour the author already liked. A subtle reddish
        // tint when CLONE is showing (author, live: "preciso que a cor do
        // fundo do corpo também altere... das tres colunas: corpo coluna
        // direita e esquerda", then "vamos fazer um teste com uma cor de
        // fundo do clone mais avermelhada" - tried CLONE's own amber
        // first, superseded by this) - this one fillAll covers the whole
        // window (left CLOCK column, centre body, right MIXER column all
        // at once), so a single conditional here was enough. Kept subtle
        // (6%) on purpose - a background tint should read at a glance,
        // not compete with the panel's own accent colours.
        g.fillAll(showingCloneBody ? juce::Colour(0xff171511).interpolatedWith(cloneMaterial::cloneBodyTint, 0.06f)
                                    : juce::Colour(0xff171511));
        // Didactic backing panel behind CAOS/VAGA/FRZ (18 ago. 2026) -
        // see ObjectFiveComponent's own copy of this same block for the
        // full comment. Gated on the CAOS button's own visibility so
        // nothing draws while CLONE's body is the one on screen. Must
        // come after fillAll() above - found live, drawing it before was
        // a real bug: fillAll paints over the whole component
        // unconditionally, wiping out anything drawn earlier in the same
        // paint() call.
        // No stroked border (18 ago. 2026, author: "talvez baste eliminar
        // a borda do fundo") - a hard outline drew attention to the
        // padding itself; a soft fill alone reads as closer-fitting even
        // at the same margin.
        if (! chaosFreezeHighlight.isEmpty() && lfoShapeButtons[3].isVisible())
        {
            // Same colour as the CAOS rail column's own sliders - see the
            // matching comment in ObjectFiveComponent::paint() above.
            g.setColour(material::clock.withAlpha(0.28f));
            g.fillRoundedRectangle(chaosFreezeHighlight.toFloat(), 5.0f);
        }
        // The frame itself is the PRINCIPAL/CLONE indicator when the
        // single-window toggle is in play - same amber CLONE's own
        // heading already uses, thicker too, so which body is showing
        // reads at a glance without hunting for a label. Plain wood
        // brown, thin, for PRINCIPAL - unchanged from before this.
        g.setColour(showingCloneBody ? material::board : juce::Colour(0xff665b49));
        g.drawRoundedRectangle(getLocalBounds().reduced(13).toFloat(), 9.0f, showingCloneBody ? 2.4f : 1.2f);
        // No backing card: the expanded(7,4) box used to visually cancel out
        // the header's shared top inset, landing the logo 4px higher than
        // the scope/action buttons next to it even though their bounds
        // agreed. The bare mark now shares their exact top edge.
        if (logo != nullptr && !logoBounds.isEmpty())
            // Top-aligned, not centred: the mark's own artwork is shorter
            // than its 42px-tall box, so centring left empty space above it
            // that read as the mark starting lower than the scope/buttons.
            logo->drawWithin(g, logoBounds.toFloat(),
                juce::RectanglePlacement::xMid | juce::RectanglePlacement::yTop, 1.0f);
        // Option 2: uneven opacity and start height per line instead of a
        // uniform row of identical hairlines - reads closer to traces on a
        // board than a mechanical ruler.
        int lineIndex = 0;
        for (int x = 185; x < getWidth() - 154; x += 92)
        {
            const auto opacity = lineIndex % 3 == 0 ? 1.0f : (lineIndex % 3 == 1 ? 0.5f : 0.75f);
            const auto topOffset = static_cast<float>(lineIndex % 2) * 16.0f;
            g.setColour(juce::Colour(0xff302b23).withAlpha(opacity));
            g.drawVerticalLine(x, 150.0f + topOffset, static_cast<float>(getHeight() - 28));
            ++lineIndex;
        }
    }
    void resized() override
    {
        // A maximised window can cross the performance-surface threshold
        // after construction. Re-apply visibility once, so no module stays
        // hidden just because the initial window was smaller.
        const auto unifiedNow = useUnifiedLayout();
        if (unifiedNow != unifiedVisibilityApplied)
        {
            unifiedVisibilityApplied = unifiedNow;
            setPage(page);
            return;
        }
        auto area = getLocalBounds().reduced(28);
        // Header as a small performance instrument: mark and name at left,
        // final L/R observation beside them, free material space at right.
        auto header = area.removeFromTop(122);
        // Every header element shares the logo's own top inset (4px below
        // the header row) so the mark, scope and action buttons all read as
        // one aligned row instead of drifting to their own centring.
        constexpr int headerContentTop = 4;
        // 380, not 220: widened to also fit CLONE and the monitor-mode
        // toggle, both moved out of the mixer column's variation grid and
        // into the header instead (narrows the oscilloscope a little,
        // which has room to give). LEARN briefly lived here too (18 ago.
        // 2026) but became a toggle-less permanent box instead (author,
        // live: "talvez o botão learn nem seja necessário se a caixa
        // sempre é visível") - no button, so this reverted to 398.
        auto headerActions = header.removeFromRight(398);
        // Smaller still than the transport buttons - these are secondary,
        // occasional actions (help/credits), not primary controls, so they
        // should read as quietly smaller rather than matching that row.
        auto headerActionButtons = headerActions.withTrimmedTop(headerContentTop).removeFromTop(34);
        about.setBounds(headerActionButtons.removeFromRight(64).reduced(3, 1));
        tutorial.setBounds(headerActionButtons.removeFromRight(88).reduced(3, 1));
        languageSwitch.setBounds(headerActionButtons.removeFromRight(54).reduced(3, 1));
        monitorModeToggle.setBounds(headerActionButtons.removeFromRight(88).reduced(3, 1));
        objectFive.setBounds(headerActionButtons.removeFromRight(80).reduced(3, 1));
        // headerActionButtons now holds only the unused margin left over on
        // this row's own left edge (398 - the 5 buttons' 374) - reusing its
        // width below lines the transport row up with CLONE's own left
        // edge instead of the header column's, so both rows read as the
        // same width instead of the second one silently running wider.
        const auto headerButtonsMargin = headerActionButtons.getWidth();
        // PLAY/STOP/RESET/REC + REC TIMERS join the header too, in a second
        // row directly below CLONE/1 MONITOR/PT/TUTORIAL/SOBRE - the header
        // column's own height (122) already left this much room unused
        // below the first row (84px, which is exactly transportRowHeight +
        // recordDurationsLabelH + recordDurationRowHeight's old budget in
        // the transport column), so nothing downstream needs to grow.
        auto headerTransport = headerActions.withTrimmedTop(headerContentTop + 34);
        headerTransport.removeFromLeft(headerButtonsMargin);
        recordingLabel.setBounds(headerTransport.removeFromTop(12));
        auto headerTransportRow = headerTransport.removeFromTop(34);
        const auto headerTransportButtonWidth = headerTransportRow.getWidth() / 4;
        for (auto* button : { &run, &stop, &record, &reset })
            button->setBounds(headerTransportRow.removeFromLeft(headerTransportButtonWidth).reduced(3, 1));
        recordDurationsLabel.setBounds(headerTransport.removeFromTop(12));
        auto headerDurationRow = headerTransport.removeFromTop(24);
        const auto headerDurationButtonWidth = headerDurationRow.getWidth() / static_cast<int>(recordDurations.size());
        for (auto& button : recordDurations)
            button.setBounds(headerDurationRow.removeFromLeft(headerDurationButtonWidth).reduced(3, 1));
        // MASTER moves into the header too - the final output control sits
        // right before the action/transport block, immediately left of it
        // (between the scope and CLONE), not inside the already-full
        // headerActions column (120 of its own 122px height is spoken for).
        // ENERGIA stays behind in the mixer column - only MASTER was asked
        // to move.
        // 110/98, not 100/90 - author asked for the knob "legeiramente
        // maior" (slightly bigger); widened the column to match so it
        // still has real margin around it instead of nearly touching.
        // 110/98, not 100/90 - author asked for the knob "legeiramente
        // maior" (slightly bigger); widened the column to match so it
        // still has real margin around it instead of nearly touching.
        // headerMasterColumn is carved flush against headerActions, no
        // spacer in between - a spacer here used to be tried (18 ago.
        // 2026) to nudge the knob around, but it inserted itself into the
        // SHARED `header` cursor, which also shifts every column carved
        // after it (headerObjectMixColumn's MIXER OBJETOS sliders,
        // markColumn, scopeRow) - side effects on objects the author
        // never asked to move: "separe o master dos objetos que estão ao
        // lado, não quero modificações nos objetos ao lado dele, somente
        // nele, se houve alguma modificação nos objetos ao lado dele
        // desfaça". Reverted entirely; any further nudge to MASTER now
        // has to happen on `master`'s own bounds only, after this column
        // is carved, never on `header` itself.
        constexpr int headerMasterWidth = 110;
        auto headerMasterColumn = header.removeFromRight(headerMasterWidth).withTrimmedTop(headerContentTop);
        // Caption moved inside the knob (18 ago. 2026, author: "faça o mesmo
        // procedimento do energia no knob master no cabeçalho") - same
        // recipe as ENERGIA: knob keeps the column's full height instead of
        // sharing it with a separate caption row above, caption centred
        // inside the knob's own bounds afterwards, toFront(false) since
        // masterLabel was added to the tree before master (see the
        // constructor). Caption box height (13, not the earlier 16) also
        // now matches ENERGIA's own knobCaptionHeight exactly, not just its
        // font size - author: "mesma configuração de layout do titulo de
        // energia".
        // Nudged right within its own column's existing slack (6px each
        // side of the 98px knob in the 110px column) instead of moving the
        // column boundary itself - author asked "mais para a direita"
        // twice; this uses up the column's own right-side margin only,
        // nothing outside headerMasterColumn is touched.
        constexpr int masterKnobRightNudge = 6;
        master.setBounds(headerMasterColumn.withSizeKeepingCentre(98, 98).translated(masterKnobRightNudge, 0));
        masterLabel.setBounds(master.getBounds().withSizeKeepingCentre(master.getWidth(), 13));
        masterLabel.toFront(false);
        // PRINCIPAL/CLONE as a 2-row rail (gain + M/S), same vocabulary as
        // the 4-channel mixer's own ON/M/S - lives in the header, not the
        // mixer column, deliberately: the mixer column's own geometry is
        // shared byte-for-byte with clonePanel's embedded body (see
        // setShowingCloneBody()'s own comment on that identical-geometry
        // requirement), and clonePanel's own resized() has no idea about
        // extra content inserted here - a first attempt placed this rail
        // there and it silently desynced clonePanel's own mixer/MEMÓRIA
        // MIX/CONEXÃO ENTRE OBJETOS position from MainComponent's own
        // shifted layout the moment CLONE was shown (author, live: "no
        // clone não deu certo... na aba clone houve problema com os novos
        // itens"). The header is MainComponent-only already - MASTER
        // proves this spot survives CLONE toggling with zero extra work.
        // 220, not 130 - the slider itself was too thin to grab comfortably;
        // widened deliberately at the oscilloscope's expense (author, live:
        // "vamos deixar o slider mais largo e mais robusto, mesmo que seja
        // as custas de diminuir a largura do osciloscópio").
        constexpr int headerObjectMixWidth = 220;
        auto headerObjectMixColumn = header.removeFromRight(headerObjectMixWidth).withTrimmedTop(headerContentTop);
        // 18, not 14 - matches MASTER's own title size (12.0f) now, needs
        // the taller slot; +12 gap below it (author asked for more twice:
        // "espaçamento maior entre o título e o primeiro slider", then
        // "gostaria de um pouco mais de espaço" after the first bump).
        objectMixLabel.setBounds(headerObjectMixColumn.removeFromTop(18));
        headerObjectMixColumn.removeFromTop(12);
        auto placeObjectMixRow = [&] (juce::Label& caption, juce::Slider& volume, juce::ToggleButton& mute)
        {
            auto row = headerObjectMixColumn.removeFromTop(24);
            caption.setBounds(row.removeFromLeft(56).reduced(0, 3));
            mute.setBounds(row.removeFromRight(26).reduced(1, 2));
            volume.setBounds(row.reduced(6, 3));
        };
        placeObjectMixRow(principalVolumeLabel, principalVolume, principalMute);
        headerObjectMixColumn.removeFromTop(4);
        placeObjectMixRow(cloneVolumeLabel, cloneVolume, cloneMute);
        headerObjectMixColumn.removeFromTop(4);
        // EXCITAÇÃO - now has an M button too (20 ago. 2026, "crie o
        // botão de mute para o excit no object mixer"), same row shape as
        // PRINC/CLONE - reuses placeObjectMixRow directly.
        placeObjectMixRow(excitationLabel, excitationAmount, excitationMute);
        auto markColumn = header.removeFromLeft(350);
        logoBounds = markColumn.removeFromTop(38).reduced(6, 2);
        title.setBounds(markColumn.removeFromTop(30).reduced(4, 1));
        flow.setBounds(markColumn.removeFromTop(22).reduced(4, 0));
        // The scope fills every pixel free between the mark column and the
        // header action buttons, instead of stopping at an arbitrary width -
        // minus a narrow strip on its right for the Y-gain slider.
        auto scopeRow = header.withTrimmedTop(headerContentTop).reduced(8, 0).withTrimmedBottom(5);
        scopeGain.setBounds(scopeRow.removeFromRight(20));
        stereoScope.setBounds(scopeRow.withTrimmedRight(4));
        footer.setBounds(28, getHeight() - 32, getWidth() - 56, 19);
        // In the unified layout, PULSO/POROSA/HETERÓDINA/RND16/DERIVA/CLONE
        // move into the mixer column instead (above MEMÓRIA MIX, see
        // layoutUnified) - this row's own height goes back to the modules
        // above (oscillators) rather than being spent on a button strip.
        if (! useUnifiedLayout())
        {
            auto actionArea = area.removeFromTop(31);
            soundPage.setBounds(actionArea.removeFromLeft(112).reduced(0, 3));
            sequencePage.setBounds(actionArea.removeFromLeft(142).reduced(4, 3));
            mixPage.setBounds(actionArea.removeFromLeft(74).reduced(2, 3));
            deriveButton.setBounds(actionArea.removeFromRight(78).reduced(2, 3));
            randomizeStepsButton.setBounds(actionArea.removeFromRight(74).reduced(2, 3));
            heterodyneVariation.setBounds(actionArea.removeFromRight(116).reduced(2, 3));
            porousVariation.setBounds(actionArea.removeFromRight(92).reduced(2, 3));
            pulseVariation.setBounds(actionArea.removeFromRight(82).reduced(2, 3));
            objectFive.setBounds(actionArea.removeFromRight(76).reduced(2, 3));
        }
        // The CI chain remains legible in the flow line. There is deliberately
        // no spacer row here: playable modules begin immediately after the
        // header/action strip and share their edges on the 1920x1080 surface.
        if (useUnifiedLayout())
        {
            layoutUnified(area);
            // Identical geometry in both states - clonePanel gets the
            // exact same `area` layoutUnified() itself just received, so
            // switching PRINCIPAL/CLONE is only ever a visibility change,
            // never a layout change.
            if (clonePanel != nullptr) clonePanel->setBounds(area);
            return;
        }
        if (page == Page::sequence)
        {
            layoutSequence(area);
            return;
        }
        if (page == Page::mix)
        {
            layoutMix(area);
            return;
        }
        auto stepsArea = area.removeFromBottom(225);
        auto modulationRails = area.removeFromBottom(152);
        auto layoutRail = [this] (juce::Rectangle<int> rail, juce::Label& heading,
                                  std::array<juce::Label, 3>& labels, std::array<juce::Slider, 3>& controls)
        {
            heading.setBounds(rail.removeFromLeft(108).reduced(2, 1));
            const auto cellWidth = rail.getWidth() / static_cast<int>(controls.size());
            for (std::size_t i = 0; i < controls.size(); ++i)
            {
                auto cell = rail.removeFromLeft(cellWidth).reduced(5, 0);
                labels[i].setBounds(cell.removeFromTop(16));
                controls[i].setBounds(cell.removeFromTop(24).reduced(0, 2));
            }
        };
        layoutRail(modulationRails.removeFromTop(38), modulationLabel, modulationControlLabels, modulationControls);
        auto lfoShapeRail = modulationRails.removeFromTop(38);
        lfoShapeLabel.setBounds(lfoShapeRail.removeFromLeft(108).reduced(2, 1));
        // FREEZE reserved first (fixed 34px) so the shape buttons
        // dividing the rest never has fewer than lfoShapeButtons.size()
        // cells - this legacy (<1600px) path isn't the audited one (see
        // VCF's own note on this same path), just kept safe.
        lfoFreeze.setBounds(lfoShapeRail.removeFromRight(34).reduced(2, 5));
        const auto lfoShapeWidth = lfoShapeRail.getWidth() / static_cast<int>(lfoShapeButtons.size());
        for (auto& button : lfoShapeButtons)
            button.setBounds(lfoShapeRail.removeFromLeft(lfoShapeWidth).reduced(4, 5));
        layoutRail(modulationRails.removeFromTop(38), effectsLabel, effectControlLabels, effectControls);
        auto detailRail = modulationRails.removeFromTop(38);
        detailLabel.setBounds(detailRail.removeFromLeft(108).reduced(2, 1));
        const auto detailWidth = detailRail.getWidth() / static_cast<int>(detailControls.size());
        for (std::size_t i = 0; i < detailControls.size(); ++i)
        {
            auto cell = detailRail.removeFromLeft(detailWidth).reduced(4, 0);
            detailControlLabels[i].setBounds(cell.removeFromTop(16));
            detailControls[i].setBounds(cell.reduced(0, 2));
        }
        auto clockArea = area.removeFromLeft(140); clockLabel.setBounds(clockArea.removeFromTop(20)); clock.setBounds(clockArea.removeFromTop(84));
        run.setBounds(clockArea.removeFromTop(24).reduced(8, 2)); stop.setBounds(clockArea.removeFromTop(24).reduced(8, 2)); reset.setBounds(clockArea.removeFromTop(24).reduced(8, 2)); record.setBounds(clockArea.removeFromTop(27).reduced(8, 2));
        auto recordDurationRow = clockArea.removeFromTop(30);
        const auto recordDurationCellWidth = recordDurationRow.getWidth() / static_cast<int>(recordDurations.size());
        for (auto& button : recordDurations) button.setBounds(recordDurationRow.removeFromLeft(recordDurationCellWidth).reduced(2, 2));
        temporalLabel.setBounds(clockArea.removeFromTop(14));
        // 2 rows of 4, not 1 row of 8 - same fix as the metric block right
        // below (temporalButtons grew from 4 to 8, 20 ago. 2026).
        auto temporalArea = clockArea.removeFromTop(44);
        for (std::size_t i = 0; i < temporalButtons.size(); ++i)
            temporalButtons[i].setBounds(temporalArea.getX() + static_cast<int>(i % 4) * temporalArea.getWidth() / 4,
                                          temporalArea.getY() + static_cast<int>(i / 4) * temporalArea.getHeight() / 2,
                                          temporalArea.getWidth() / 4, temporalArea.getHeight() / 2);
        metricLabel.setBounds(clockArea.removeFromTop(14));
        // 2 rows of 4, not 1 row of 8 - the old /4 math would have placed
        // buttons 4-7 past metricArea's right edge (i*(width/4) for i>=4
        // exceeds width) now that metricButtons holds 8, not 4.
        auto metricArea = clockArea.removeFromTop(44);
        for (std::size_t i = 0; i < metricButtons.size(); ++i)
            metricButtons[i].setBounds(metricArea.getX() + static_cast<int>(i % 4) * metricArea.getWidth() / 4,
                                        metricArea.getY() + static_cast<int>(i / 4) * metricArea.getHeight() / 2,
                                        metricArea.getWidth() / 4, metricArea.getHeight() / 2);
        scannerLabel.setBounds(clockArea.removeFromTop(14));
        auto scannerArea = clockArea.removeFromTop(22);
        for (std::size_t i = 0; i < scannerButtons.size(); ++i)
            scannerButtons[i].setBounds(scannerArea.getX() + static_cast<int>(i) * scannerArea.getWidth() / 4, scannerArea.getY(), scannerArea.getWidth() / 4, scannerArea.getHeight());
        loopLabel.setBounds(clockArea.removeFromTop(15));
        constexpr auto loopColumns = 4;
        const auto loopRows = static_cast<int>((loopSwitches.size() + loopColumns - 1U) / loopColumns);
        auto loopArea = clockArea.removeFromTop(loopRows * 20);
        const auto loopWidth = loopArea.getWidth() / 4;
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
            loopSwitches[i].setBounds(loopArea.getX() + static_cast<int>(i % 4) * loopWidth,
                                      loopArea.getY() + static_cast<int>(i / 4) * 20, loopWidth, 20);
        connectionLabel.setBounds(clockArea.removeFromTop(15));
        auto connectionArea = clockArea.removeFromTop(66);
        const auto connectionWidth = connectionArea.getWidth() / 2;
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
            connectionSwitches[i].setBounds(connectionArea.getX() + static_cast<int>(i % 2) * connectionWidth,
                                             connectionArea.getY() + static_cast<int>(i / 2) * 22, connectionWidth, 22);
        feedbackLabel.setBounds(clockArea.removeFromTop(14));
        feedbackGain.setBounds(clockArea.removeFromTop(23).reduced(3, 1));
        deriveLabel.setBounds(clockArea.removeFromTop(14));
        deriveDepth.setBounds(clockArea.removeFromTop(23).reduced(3, 1));
        auto masterArea = area.removeFromRight(120); masterLabel.setBounds(masterArea.removeFromTop(20)); master.setBounds(masterArea.removeFromTop(108));
        noiseSelector.setBounds(masterArea.removeFromTop(112).reduced(2, 0));
        auto energyArea = area.removeFromRight(120); energyLabel.setBounds(energyArea.removeFromTop(20)); energy.setBounds(energyArea.removeFromTop(108));
        auto filterArea = area.removeFromRight(220); filterLabel.setBounds(filterArea.removeFromTop(22));
        {
            // Legacy (<1600px) layout, rarely reached in practice - minimal
            // compile-safe fix for the four independent toggles (17 ago.
            // 2026), not a full audit of this path (same policy already
            // used for other legacy-path items - see "Fixar a escala dos
            // componentes" in docs/TAREFAS.md).
            auto legacyFilterModeRow = filterArea.removeFromTop(25).reduced(4, 2);
            const auto legacyFilterModeWidth = legacyFilterModeRow.getWidth() / 4;
            for (auto& button : filterModeButtons)
                button.setBounds(legacyFilterModeRow.removeFromLeft(legacyFilterModeWidth).reduced(2, 0));
        }
        for (std::size_t i = 0; i < filterControlLabels.size(); ++i)
        {
            auto cell = filterArea.removeFromTop(88);
            filterControlLabels[i].setBounds(cell.removeFromTop(13));
            (i == 0 ? filterCutoff : (i == 1 ? filterResonance : filterDepth)).setBounds(cell.reduced(28, 0));
        }
        auto envelopeArea = area.removeFromRight(220); envelopeLabel.setBounds(envelopeArea.removeFromTop(22));
        const auto envelopeWidth = envelopeArea.getWidth() / 2;
        for (std::size_t i = 0; i < envelopeControls.size(); ++i)
        {
            const auto row = i / 2;
            const auto column = i % 2;
            auto controlArea = juce::Rectangle<int>(envelopeArea.getX() + static_cast<int>(column) * envelopeWidth,
                                                    envelopeArea.getY() + static_cast<int>(row) * 120,
                                                    envelopeWidth, 112).reduced(4, 0);
            envelopeControlLabels[i].setBounds(controlArea.removeFromTop(18));
            envelopeControls[i].setBounds(controlArea.removeFromTop(92));
        }
        // The oscillator bank owns the available central field. A fixed narrow
        // strip made its rotary gestures look decorative instead of playable.
        const auto voiceWidth = std::clamp(area.getWidth(), 390, 540);
        auto voiceArea = area.removeFromRight(voiceWidth); voiceLabel.setBounds(voiceArea.removeFromTop(26));
        auto coreArea = voiceArea.removeFromTop(34);
        const auto coreWidth = coreArea.getWidth() / static_cast<int>(coreSwitches.size());
        for (std::size_t i = 0; i < coreSwitches.size(); ++i)
            coreSwitches[i].setBounds(coreArea.removeFromLeft(coreWidth).reduced(2, 2));
        const auto oscillatorWidth = voiceArea.getWidth() / static_cast<int>(oscillators.size());
        for (std::size_t i = 0; i < oscillators.size(); ++i)
        {
            auto oscillatorArea = voiceArea.removeFromLeft(oscillatorWidth).reduced(3, 0);
            // 58, not 32: this column is only ~80-110px wide (voiceWidth
            // clamped to 390-540 for all 5 columns together). Most labels
            // ("OSC A" + "FREQ / MIX / FORMA") wrap to 3 lines there and fit
            // in 46px, but OSC 5's own subtitle ("4046 · LM13600 HETERO")
            // is long enough to wrap to 4 lines - confirmed by screenshot,
            // the 4th line (HETERO) still spilled into the FREQ caption
            // below it at 46px. 58 covers that worst case too.
            oscillatorLabels[i].setBounds(oscillatorArea.removeFromTop(58));
            auto rateArea = oscillatorArea.removeFromTop(108);
            oscillatorRateLabels[i].setBounds(rateArea.removeFromTop(13));
            oscillatorRates[i].setBounds(rateArea);
            auto levelArea = oscillatorArea.removeFromTop(108);
            oscillatorLevelLabels[i].setBounds(levelArea.removeFromTop(13));
            oscillators[i].setBounds(levelArea);
            auto shapeArea = oscillatorArea.removeFromTop(108);
            oscillatorShapeLabels[i].setBounds(shapeArea.removeFromTop(13));
            shapes[i].setBounds(shapeArea);
            auto panArea = oscillatorArea.removeFromTop(30);
            oscillatorPanCaptions[i].setBounds(panArea.removeFromTop(13));
            oscillatorPans[i].setBounds(panArea.reduced(5, 0));
        }
        // Independent of the sound-panel columns: this is the fixed lower
        // right status window, immediately above the footer.
        logLabel.setBounds(getWidth() - 320, getHeight() - 164, 292, 14);
        log.setBounds(getWidth() - 320, getHeight() - 150, 292, 106);
        const auto stepWidth = std::clamp(stepsArea.getWidth() / static_cast<int>(antitotem::SimpleSequencer::historicalScannerSteps), 100, 200);
        const auto stepHeight = stepsArea.getHeight() / 2;
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            const auto bounds = juce::Rectangle<int>(stepsArea.getX() + static_cast<int>(i % antitotem::SimpleSequencer::historicalScannerSteps) * stepWidth,
                                                      stepsArea.getY() + static_cast<int>(i / antitotem::SimpleSequencer::historicalScannerSteps) * stepHeight,
                                                      stepWidth, stepHeight).reduced(4, 3);
            steps[i].setBounds(bounds);
        }
    }
private:
    enum class Page { sound, sequence, mix };
    enum class Variation { pulse, porous, heterodyne, orbit, pendulum };

    [[nodiscard]] bool useUnifiedLayout() const noexcept
    {
        // The one-surface view is designed for the maximised 1920x1080
        // performance window; smaller windows return to the spacious pages.
        return getWidth() >= 1600;
    }

    [[nodiscard]] float nextDerivationUnit() noexcept
    {
        derivationState ^= derivationState << 13U;
        derivationState ^= derivationState >> 17U;
        derivationState ^= derivationState << 5U;
        return static_cast<float>(derivationState & 0x00ffffffU) / static_cast<float>(0x00ffffffU);
    }
    [[nodiscard]] unsigned char currentFeedbackRoutes() const noexcept
    {
        unsigned char routes = 0;
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
            if (connectionSwitches[i].getToggleState()) routes |= static_cast<unsigned char>(1U << i);
        return routes;
    }
    void captureDerivationMemory()
    {
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            derivationCv[i] = static_cast<float>(steps[i].cv.getValue());
            derivationAmp[i] = static_cast<float>(steps[i].level.getValue());
            derivationFx[i] = static_cast<float>(steps[i].send.getValue());
        }
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
            derivationRatios[i] = static_cast<float>(oscillatorRates[i].getValue());
        for (std::size_t i = 0; i < detailControls.size(); ++i)
            derivationDetail[i] = static_cast<float>(detailControls[i].getValue());
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            derivationMixGain[i] = static_cast<float>(mixGain[i].getValue());
            derivationMixPan[i] = static_cast<float>(mixPan[i].getValue());
            derivationMixReflux[i] = static_cast<float>(mixReflux[i].getValue());
        }
        for (std::size_t i = 0; i < effectControls.size(); ++i)
            derivationEffects[i] = static_cast<float>(effectControls[i].getValue());
        for (std::size_t i = 0; i < envelopeControls.size(); ++i)
            derivationEnvelope[i] = static_cast<float>(envelopeControls[i].getValue());
        for (std::size_t i = 0; i < oscillatorPans.size(); ++i)
            derivationPans[i] = static_cast<float>(oscillatorPans[i].getValue());
        derivationLfo = static_cast<float>(modulationControls[0].getValue());
        derivationNoiseMix = static_cast<float>(modulationControls[2].getValue());
        derivationGroove = static_cast<float>(grooveAmount.getValue());
        derivationFilterCutoff = static_cast<float>(filterCutoff.getValue());
        derivationFilterResonance = static_cast<float>(filterResonance.getValue());
        hungerCv.fill(0.0f); hungerAmp.fill(0.0f); hungerFx.fill(0.0f);
        hungerRatios.fill(0.0f); hungerEffects.fill(0.0f); hungerEnvelope.fill(0.0f);
        hungerLfo = hungerNoiseMix = hungerGroove = hungerFilterCutoff = hungerFilterResonance = hungerFilterMode = hungerCore = hungerMixMemory = 0.0f;
        hungerPans.fill(0.0f); hungerDetail.fill(0.0f);
        hungerMixGain.fill(0.0f); hungerMixPan.fill(0.0f); hungerMixReflux.fill(0.0f);
        hungerMetric = hungerTemporal = hungerNoiseColour = hungerLoopEnd = 0.0f;
        derivationRing = static_cast<float>(modulationControls[1].getValue());
        hungerRing = 0.0f;
        derivationMat = static_cast<float>(materialFilterMix.getValue());
        hungerMat = 0.0f;
        hungerLfoShape = 0.0f;
        derivationClock = static_cast<float>(clock.getValue());
        hungerClock = 0.0f;
        derivationGainToFifth = static_cast<float>(gainToFifth.getValue());
        derivationGainToFirst = static_cast<float>(gainToFirst.getValue());
        derivationAuxToFirst = static_cast<float>(auxToFirst.getValue());
        derivationAuxToFifth = static_cast<float>(auxToFifth.getValue());
        hungerGainToFifth = hungerGainToFirst = hungerAuxToFirst = hungerAuxToFifth = hungerObjectRoute = 0.0f;
        topologyMemory.fill(currentFeedbackRoutes());
        topologyWrite = 0;
        derivationPhrase = 0;
        derivationMotion = 0.0f;
        derivationMotionB = 0.0f;
        derivationMotionC = 0.0f;
        derivationAnchors = { 0, -1 };
        derivationAnchorWrite = 0;
        lastDerivationStep = sequencer.getCurrentStep();
        configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::driftMemoryCaptured, uiLanguage), 12.0f, material::memory);
    }
    void advanceRecordingForm()
    {
        if (!recorder.isRecording()) return;
        const auto progress = recorder.progress();
        const auto nextEvent = progress < 0.18f ? 0 : progress < 0.46f ? 1 : progress < 0.74f ? 2 : 3;
        if (nextEvent == recordingEvent) return;
        recordingEvent = nextEvent;
        const std::array<juce::String, 4> phases {
            antitotem::ui::text(antitotem::ui::logText::recPhases[0], uiLanguage),
            antitotem::ui::text(antitotem::ui::logText::recPhases[1], uiLanguage),
            antitotem::ui::text(antitotem::ui::logText::recPhases[2], uiLanguage),
            antitotem::ui::text(antitotem::ui::logText::recPhases[3], uiLanguage)
        };
        if (deriveButton.getToggleState())
        {
            if (nextEvent == 1 || nextEvent == 2) deriveFromMemory();
            if (nextEvent == 3)
            {
                feedbackGain.setValue(std::min(0.34, feedbackGain.getValue()), juce::dontSendNotification);
                effectControls[0].setValue(std::min(0.42, effectControls[0].getValue()), juce::dontSendNotification);
                effectControls[1].setValue(std::min(0.25, effectControls[1].getValue()), juce::dontSendNotification);
                effectControls[2].setValue(std::min(0.20, effectControls[2].getValue()), juce::dontSendNotification);
                sequencer.setFeedbackAmount(static_cast<float>(feedbackGain.getValue()));
                syncEffects();
                refreshSilentHighlights();
            }
        }
        configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recArmedTrackPrefix, uiLanguage) + phases[static_cast<std::size_t>(nextEvent)] + " · "
                               + juce::String(static_cast<int>(progress * 100.0f)) + "%", 12.0f,
                       nextEvent == 3 ? material::returnPath : material::memory);
    }
    // Autonomia por item - see ObjectFiveComponent's own copy of this
    // function for the full comment.
    void driftAutonomousItem(juce::Slider& control, float& memory, float& hunger,
                              float rangeLo, float rangeHi, float widthFactor, float activeDepthValue)
    {
        // Recalibrado (20 ago. 2026, autor: "a variação é sutil") - fome
        // cresce mais rápido (chega no teto em ~13 ciclos, não 20) e o
        // salto em si ficou maior tanto na chance quanto na mistura.
        hunger = std::min(hunger + 0.075f, 1.0f);
        const auto chance = 0.08f + hunger * 0.6f;
        if (nextDerivationUnit() >= chance) return;
        const auto current = static_cast<float>(control.getValue());
        const auto target = std::clamp(memory + (nextDerivationUnit() - 0.5f) * widthFactor * activeDepthValue, rangeLo, rangeHi);
        const auto blend = 0.14f + hunger * 0.6f;
        const auto value = current + (target - current) * blend;
        control.setValue(value, juce::sendNotificationSync);
        memory += (value - memory) * 0.18f;
        hunger = 0.0f;
    }
    void deriveFromMemory()
    {
        const auto userDepth = static_cast<float>(deriveDepth.getValue());
        // Ativação do meta-sequenciador - ver ObjectFiveComponent's own
        // copy of this function for the full comment.
        sequencer.setMetaSequencerAmount(userDepth);
        // Variation of variation: the phrase memory changes the amount of its
        // own movement as a slow bounded walk, never as raw audio-rate noise.
        // Noise Field ↔ DERIVA - see ObjectFiveComponent's own copy of
        // this function for the full comment.
        const auto sharedInstability = dualEngine.getInstabilityField();
        derivationMotion = std::clamp(derivationMotion + (nextDerivationUnit() - 0.5f) * (0.16f + userDepth * 0.24f + sharedInstability * 0.15f), -0.55f, 0.55f);
        // Atratores - see ObjectFiveComponent's own copy of this function
        // for the full comment (docs/PESQUISA_DERIVA_GENERATIVA.md,
        // seção 6, item 1).
        constexpr std::array<float, 3> derivationAttractors { -0.4f, 0.0f, 0.4f };
        auto nearestAttractor = derivationAttractors[0];
        auto nearestAttractorDistance = std::abs(derivationMotion - nearestAttractor);
        for (const auto attractor : derivationAttractors)
        {
            const auto distance = std::abs(derivationMotion - attractor);
            if (distance < nearestAttractorDistance) { nearestAttractor = attractor; nearestAttractorDistance = distance; }
        }
        derivationMotion = std::clamp(derivationMotion + (nearestAttractor - derivationMotion) * 0.12f, -0.55f, 0.55f);
        // The other half of the coupling above - a DERIVA event that
        // moved a real distance from centre lends a little energy BACK
        // to the shared field (up to +0.011 per event, this only fires
        // once per loop cycle so the cumulative pull is slow, not a
        // sudden spike). Neither system is put in charge of the other -
        // each still runs its own rhythm (DERIVA once per loop, the
        // field continuously every sample with its own elastic pull to
        // 0.2), they just lean on each other a little now.
        dualEngine.nudgeInstability(std::abs(derivationMotion) * 0.02f);
        const auto activeDepth = std::clamp(userDepth * (0.62f + std::abs(derivationMotion)), 0.0f, 1.0f);
        // Instâncias paralelas B e C (ver derivationMotionB/C's próprio
        // comentário de membro) - MESMO mecanismo de A (random walk +
        // atratores), caráter deliberadamente diferente pra ler como
        // processos distintos, não três cópias do mesmo. B: mais lenta/
        // calma (2 atratores, puxão mais forte) - dirige ADSR/LFO/NOISE
        // MIX/GROOVE/filtro. C: mais rápida/inquieta (3 atratores mais
        // largos, puxão mais fraco) - dirige pans dos osciladores/
        // MÉTRICA/SUBDIVISÃO/NOISE COR.
        derivationMotionB = std::clamp(derivationMotionB + (nextDerivationUnit() - 0.5f) * (0.08f + userDepth * 0.12f + sharedInstability * 0.08f), -0.55f, 0.55f);
        {
            constexpr std::array<float, 2> attractorsB { -0.3f, 0.3f };
            auto nearest = attractorsB[0];
            auto nearestDistance = std::abs(derivationMotionB - nearest);
            for (const auto attractor : attractorsB)
            {
                const auto distance = std::abs(derivationMotionB - attractor);
                if (distance < nearestDistance) { nearest = attractor; nearestDistance = distance; }
            }
            derivationMotionB = std::clamp(derivationMotionB + (nearest - derivationMotionB) * 0.18f, -0.55f, 0.55f);
        }
        const auto activeDepthB = std::clamp(userDepth * (0.62f + std::abs(derivationMotionB)), 0.0f, 1.0f);
        derivationMotionC = std::clamp(derivationMotionC + (nextDerivationUnit() - 0.5f) * (0.24f + userDepth * 0.30f + sharedInstability * 0.20f), -0.55f, 0.55f);
        {
            constexpr std::array<float, 3> attractorsC { -0.5f, 0.0f, 0.5f };
            auto nearest = attractorsC[0];
            auto nearestDistance = std::abs(derivationMotionC - nearest);
            for (const auto attractor : attractorsC)
            {
                const auto distance = std::abs(derivationMotionC - attractor);
                if (distance < nearestDistance) { nearest = attractor; nearestDistance = distance; }
            }
            derivationMotionC = std::clamp(derivationMotionC + (nearest - derivationMotionC) * 0.06f, -0.55f, 0.55f);
        }
        const auto activeDepthC = std::clamp(userDepth * (0.62f + std::abs(derivationMotionC)), 0.0f, 1.0f);
        constexpr auto topologyMask = static_cast<unsigned char>(0x3fU);
        const auto historicalRoute = topologyMemory[(topologyWrite + 1U + static_cast<std::size_t>(nextDerivationUnit() * 7.0f)) % topologyMemory.size()];
        auto nextRoute = nextDerivationUnit() < (0.78f - activeDepth * 0.32f) ? historicalRoute : currentFeedbackRoutes();
        float routeDensity = 0.0f;
        // Event budget (docs/PESQUISA_SEQUENCER_GENERATIVO.md, seção 7.1) -
        // see ObjectFiveComponent's own copy of this function for the full
        // comment. Topology costs more (0.35) than a Motion round below -
        // the canonical example of real "ruptura" (GLITCH×RASGO/RUPTURE),
        // not a cheap glitch.
        if (participateRoutes.getToggleState() && sequencer.hasEventBudget(0.35f))
        {
        sequencer.spendEventBudget(0.35f);
        // CAOS/VAGA reseed automatically whenever this roll actually
        // mutates the route (not on every drift event) - see
        // ObjectFiveComponent's own copy of this same function for the
        // full comment on why this specific roll, not a separate one.
        const auto routeMutates = nextDerivationUnit() < 0.18f + activeDepth * 0.66f;
        if (routeMutates)
        {
            nextRoute ^= static_cast<unsigned char>(1U << static_cast<unsigned int>(nextDerivationUnit() * 6.0f));
            sequencer.reseedLfo();
            patchToggleLook().reseedFlash = true;
            lfoShapeButtons[3].repaint(); lfoShapeButtons[4].repaint();
            juce::Component::SafePointer<MainComponent> safeThis(this);
            juce::Timer::callAfterDelay(220, [safeThis]
            {
                if (safeThis == nullptr) return;
                patchToggleLook().reseedFlash = false;
                safeThis->lfoShapeButtons[3].repaint(); safeThis->lfoShapeButtons[4].repaint();
            });
        }
        nextRoute &= topologyMask;
        if (nextRoute == 0) nextRoute = static_cast<unsigned char>(1U << static_cast<unsigned int>(nextDerivationUnit() * 6.0f));
        setFeedbackRoutes(nextRoute);
        topologyMemory[topologyWrite] = nextRoute;
        topologyWrite = (topologyWrite + 1U) % topologyMemory.size();

        unsigned int routeCount = 0;
        for (auto routeBits = static_cast<unsigned int>(nextRoute); routeBits != 0U; routeBits >>= 1U)
            routeCount += routeBits & 1U;
        routeDensity = static_cast<float>(routeCount) / 6.0f;
        const auto currentFeedback = static_cast<float>(feedbackGain.getValue());
        const auto feedbackTarget = 0.14f + routeDensity * (0.12f + activeDepth * 0.24f) + nextDerivationUnit() * (0.03f + activeDepth * 0.11f);
        feedbackGain.setValue(std::clamp(currentFeedback + (feedbackTarget - currentFeedback) * (0.06f + activeDepth * 0.26f), 0.08f, 0.58f), juce::dontSendNotification);
        sequencer.setFeedbackAmount(static_cast<float>(feedbackGain.getValue()));
        }

        // AUTO (índice 3) - see ObjectFiveComponent's own copy of this
        // function for the full comment on this configuration.
        // Event budget - see ObjectFiveComponent's own copy for the full
        // comment; a whole Motion round costs 0.25, not per-item.
        if (derivationLayers[3].getToggleState() && sequencer.hasEventBudget(0.25f))
        {
        sequencer.spendEventBudget(0.25f);
if (participateSteps.getToggleState())
                        for (std::size_t i = 0; i < steps.size(); ++i)
            {
                driftAutonomousItem(steps[i].cv, derivationCv[i], hungerCv[i], 0.0f, 1.0f, 0.30f, activeDepth);
                driftAutonomousItem(steps[i].level, derivationAmp[i], hungerAmp[i], 0.0f, 1.0f, 0.24f, activeDepth);
                driftAutonomousItem(steps[i].send, derivationFx[i], hungerFx[i], 0.0f, 1.0f, 0.34f, activeDepth);
            }
            // Reshape ocasional - ver o comentário completo no ramo
            // `else` acima (mesma ideia: o random walk por item aqui
            // preserva a ordem relativa entre passos quase sempre, uma
            // troca de posição embaralha o próprio desenho). Sem checar
            // âncoras - o modo AUTO não tem esse conceito.
            if (participateSteps.getToggleState() && nextDerivationUnit() < 0.03f + activeDepth * 0.55f)
            {
                const auto a = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
                const auto b = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
                if (a != b)
                {
                    std::swap(derivationCv[a], derivationCv[b]);
                    std::swap(derivationAmp[a], derivationAmp[b]);
                    std::swap(derivationFx[a], derivationFx[b]);
                    const auto cvA = static_cast<float>(steps[a].cv.getValue()), cvB = static_cast<float>(steps[b].cv.getValue());
                    const auto ampA = static_cast<float>(steps[a].level.getValue()), ampB = static_cast<float>(steps[b].level.getValue());
                    const auto fxA = static_cast<float>(steps[a].send.getValue()), fxB = static_cast<float>(steps[b].send.getValue());
                    steps[a].cv.setValue(cvB, juce::sendNotificationSync); steps[b].cv.setValue(cvA, juce::sendNotificationSync);
                    steps[a].level.setValue(ampB, juce::sendNotificationSync); steps[b].level.setValue(ampA, juce::sendNotificationSync);
                    steps[a].send.setValue(fxB, juce::sendNotificationSync); steps[b].send.setValue(fxA, juce::sendNotificationSync);
                }
            }
if (participateVoice.getToggleState())
                        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
                driftAutonomousItem(oscillatorRates[i], derivationRatios[i], hungerRatios[i],
                                     static_cast<float>(oscillatorRates[i].getMinimum()), static_cast<float>(oscillatorRates[i].getMaximum()), 0.6f, activeDepth);
            // Botões CORE dos osciladores (radio group, 40106/8038/
            // 4069UB) - salto discreto, mesma fome dos outros toggles.
            hungerCore = std::min(hungerCore + 0.05f, 1.0f);
            if (participateVoice.getToggleState() && nextDerivationUnit() < 0.04f + hungerCore * 0.5f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(coreSwitches.size())) % coreSwitches.size();
                coreSwitches[index].setToggleState(true, juce::sendNotificationSync);
                hungerCore = 0.0f;
            }
            // FORMA LFO (autor, 20 ago. 2026: "faltaram botoes em:
            // ...FORMA LFO...") - radio group de 6, salto discreto.
            hungerLfoShape = std::min(hungerLfoShape + 0.05f, 1.0f);
            if (participateLfoShape.getToggleState() && nextDerivationUnit() < 0.04f + hungerLfoShape * 0.5f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(lfoShapeButtons.size())) % lfoShapeButtons.size();
                lfoShapeButtons[index].setToggleState(true, juce::sendNotificationSync);
                hungerLfoShape = 0.0f;
            }
if (participateEffects.getToggleState())
                        for (std::size_t i = 0; i < effectControls.size(); ++i)
                driftAutonomousItem(effectControls[i], derivationEffects[i], hungerEffects[i], 0.0f, 1.0f, 0.30f, activeDepth);
for (std::size_t i = 0; i < detailControls.size(); ++i)
            {
                // ROTAS ATIVAS (0-8) / MATÉRIA (9-12) / CAOS (13-15) -
                // três títulos visuais distintos sobre o mesmo array
                // (20 ago. 2026, autor: "faltaram botoes em: MATERIA,
                // CAOS...").
                auto& toggle = i < 9 ? participateDetail : (i < 13 ? participateMaterial : participateChaos);
                if (toggle.getToggleState())
                    driftAutonomousItem(detailControls[i], derivationDetail[i], hungerDetail[i], 0.0f, 1.0f, 0.34f, activeDepth);
            }
if (participateMixer.getToggleState())
                        for (std::size_t i = 0; i < mixGain.size(); ++i)
            {
                driftAutonomousItem(mixPan[i], derivationMixPan[i], hungerMixPan[i], -1.0f, 1.0f, 0.6f, activeDepth);
                driftAutonomousItem(mixReflux[i], derivationMixReflux[i], hungerMixReflux[i], 0.0f, 0.72f, 0.3f, activeDepth);
            }
            // M1-4 RECALL (nunca CAPTURE) - see ObjectFiveComponent's
            // own copy of this function for the full comment.
            hungerMixMemory = std::min(hungerMixMemory + 0.05f, 1.0f);
            if (participateMixMemory.getToggleState() && nextDerivationUnit() < 0.03f + hungerMixMemory * 0.35f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(mixMemorySlots.size())) % mixMemorySlots.size();
                if (mixMemoryCaptured[index])
                {
                    sequencer.recallMixMemory(index);
                    for (std::size_t channel = 0; channel < mixGain.size(); ++channel)
                    {
                        const auto recalled = sequencer.getMixChannel(channel);
                        mixGain[channel].setValue(recalled.gain, juce::dontSendNotification);
                        mixPan[channel].setValue(recalled.pan, juce::dontSendNotification);
                        mixReflux[channel].setValue(recalled.reflux, juce::dontSendNotification);
                        mixEnable[channel].setToggleState(recalled.enabled, juce::dontSendNotification);
                        mixMute[channel].setToggleState(recalled.mute, juce::dontSendNotification);
                        mixSolo[channel].setToggleState(recalled.solo, juce::dontSendNotification);
                        derivationMixGain[channel] = recalled.gain;
                        derivationMixPan[channel] = recalled.pan;
                        derivationMixReflux[channel] = recalled.reflux;
                    }
                    hungerMixMemory = 0.0f;
                }
            }
            // CONEXOES ENTRE OBJETOS (20 ago. 2026, autor: "isso nao e
            // pra duplicar, somente para que haja variacao de deriva
            // nos seus controles") - so existe aqui, `dualEngine` e
            // compartilhado, CLONE nao tem copia propria.
if (participateConnections.getToggleState()) {             driftAutonomousItem(gainToFifth, derivationGainToFifth, hungerGainToFifth, 0.0f, 0.72f, 0.3f, activeDepth);
            driftAutonomousItem(gainToFirst, derivationGainToFirst, hungerGainToFirst, 0.0f, 0.72f, 0.3f, activeDepth);
            driftAutonomousItem(auxToFirst, derivationAuxToFirst, hungerAuxToFirst, 0.0f, 0.72f, 0.3f, activeDepth);
            driftAutonomousItem(auxToFifth, derivationAuxToFifth, hungerAuxToFifth, 0.0f, 0.72f, 0.3f, activeDepth); }
            // routesToFifth/routesToFirst sao toggles independentes (sem
            // radio group) - a "fome" aqui decide quando VIRAR um dos 8,
            // nao um blend continuo nem um salto pra uma selecao unica.
if (participateConnections.getToggleState()) {             hungerObjectRoute = std::min(hungerObjectRoute + 0.05f, 1.0f);
            if (nextDerivationUnit() < 0.04f + hungerObjectRoute * 0.5f)
            {
                const auto pickFirst = nextDerivationUnit() < 0.5f;
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * 4.0f) % 4;
                auto& button = pickFirst ? routesToFifth[index] : routesToFirst[index];
                button.setToggleState(! button.getToggleState(), juce::sendNotificationSync);
                hungerObjectRoute = 0.0f;
            } }
if (participateVoice.getToggleState())
                        for (std::size_t i = 0; i < oscillatorPans.size(); ++i)
                driftAutonomousItem(oscillatorPans[i], derivationPans[i], hungerPans[i], -1.0f, 1.0f, 0.4f, activeDepth);
if (participateModulation.getToggleState()) {             driftAutonomousItem(modulationControls[0], derivationLfo, hungerLfo, 0.0f, 1.0f, 0.30f, activeDepth);
            sequencer.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f)); }
if (participateModulation.getToggleState())             driftAutonomousItem(modulationControls[2], derivationNoiseMix, hungerNoiseMix, 0.0f, 1.0f, 0.30f, activeDepth);
            // RING (autor, 20 ago. 2026: "faltaram botoes em: ...RING"
            // -> "creio que ring já está em modulação") - mesmo toggle
            // de MODULAÇÃO que LFO/NOISE MIX já usam, nunca tinha
            // mecanismo de deriva nenhum antes.
            if (participateModulation.getToggleState())
                driftAutonomousItem(modulationControls[1], derivationRing, hungerRing, 0.0f, 1.0f, 0.30f, activeDepth);
if (participateGroove.getToggleState())             driftAutonomousItem(grooveAmount, derivationGroove, hungerGroove, 0.0f, 1.0f, 0.30f, activeDepth);
            // MAT (autor, 20 ago. 2026: "e MAT" - diferente do rail
            // MATÉRIA, um knob só) e KNOB CLOCK, nunca tinham mecanismo
            // de deriva.
            if (participateMat.getToggleState())
                driftAutonomousItem(materialFilterMix, derivationMat, hungerMat, 0.0f, 1.0f, 0.30f, activeDepth);
            if (participateClock.getToggleState())
                driftAutonomousItem(clock, derivationClock, hungerClock,
                                     static_cast<float>(clock.getMinimum()), static_cast<float>(clock.getMaximum()), 0.6f, activeDepth);
if (participateFilter.getToggleState()) {             driftAutonomousItem(filterCutoff, derivationFilterCutoff, hungerFilterCutoff, 0.0f, 1.0f, 0.30f, activeDepth);
            driftAutonomousItem(filterResonance, derivationFilterResonance, hungerFilterResonance, 0.0f, 1.0f, 0.30f, activeDepth); }
            // Botões do VCF (filterModeButtons) - independentes (multi-
            // select), mesmo mecanismo de INVERTER um sorteado que
            // CONEXÕES ENTRE OBJETOS já usa pros toggles de rota.
            hungerFilterMode = std::min(hungerFilterMode + 0.05f, 1.0f);
            if (participateFilter.getToggleState() && nextDerivationUnit() < 0.04f + hungerFilterMode * 0.5f)
            {
                const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(filterModeButtons.size())) % filterModeButtons.size();
                filterModeButtons[index].setToggleState(! filterModeButtons[index].getToggleState(), juce::sendNotificationSync);
                hungerFilterMode = 0.0f;
            }
if (participateEnvelope.getToggleState()) {             for (std::size_t i = 0; i < envelopeControls.size(); ++i)
                driftAutonomousItem(envelopeControls[i], derivationEnvelope[i], hungerEnvelope[i], 0.0f, 1.0f, 0.30f, activeDepth);
            sequencer.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue()));
            sequencer.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue()));
            sequencer.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue()));
            sequencer.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue())); }
            hungerMetric = std::min(hungerMetric + 0.05f, 1.0f);
            if (participateMetric.getToggleState() && nextDerivationUnit() < 0.03f + hungerMetric * 0.4f)
            {
                metricSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
                metricButtons[static_cast<std::size_t>(metricSelection)].setToggleState(true, juce::dontSendNotification);
                hungerMetric = 0.0f;
            }
            hungerTemporal = std::min(hungerTemporal + 0.05f, 1.0f);
            if (participateTemporal.getToggleState() && nextDerivationUnit() < 0.03f + hungerTemporal * 0.4f)
            {
                temporalSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
                temporalButtons[static_cast<std::size_t>(temporalSelection)].setToggleState(true, juce::dontSendNotification);
                hungerTemporal = 0.0f;
            }
            syncTemporal();
            hungerNoiseColour = std::min(hungerNoiseColour + 0.05f, 1.0f);
            if (participateNoiseColour.getToggleState() && nextDerivationUnit() < 0.03f + hungerNoiseColour * 0.4f)
            {
                noiseSelector.select(static_cast<int>(nextDerivationUnit() * 6.0f) % 6, true);
                hungerNoiseColour = 0.0f;
            }
            // FIM DO LOOP (20 ago. 2026, autor: "sliders vermelhos do
            // mixer... FIM DO LOOP... cada um a sua maneira" - pedido
            // original, só implementado agora) - mesma fome, mesmo
            // salto discreto que MÉTRICA/SUBDIVISÃO/NOISE COR.
            hungerLoopEnd = std::min(hungerLoopEnd + 0.05f, 1.0f);
            if (participateLoopEnd.getToggleState() && nextDerivationUnit() < 0.03f + hungerLoopEnd * 0.4f)
            {
                // Nunca 1 (20 ago. 2026, autor: "travou no 1 do fim do loop, tanto
                // no clone como no principal") - com loopEnd=1 o playhead
                // nunca sai do passo 0, então a condição de disparo de
                // deriveFromMemory() (`active == 0 && lastDerivationStep
                // != 0`) nunca mais fica verdadeira - um deadlock que a
                // própria DERIVA causava em si mesma. Faixa [2,16] em vez
                // de [1,16] garante que o playhead sempre tem que sair do
                // 0 antes de poder voltar pra ele.
                setLoopEnd(static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(loopSwitches.size() - 1)) + 2);
                hungerLoopEnd = 0.0f;
            }
        }
        else
        {
        // Camada A/B/C (botões VCF, docs/PESQUISA_DERIVA_GENERATIVA.md,
        // seção 6) - see ObjectFiveComponent's own copy of this function
        // for the full comment on layer gating and per-block chance
        // (19 ago. 2026, autor: "ainda me dá a impressão que as
        // alterações estão acontecendo todas no mesmo momento").
        // Event budget - see ObjectFiveComponent's own copy for the full
        // comment; A/B/C compete for the same budget when combined.
        if (derivationLayers[0].getToggleState() && sequencer.hasEventBudget(0.25f))
        {
        sequencer.spendEventBudget(0.25f);
        if (participateSteps.getToggleState() && nextDerivationUnit() < 0.85f)
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            // Passo por evento recalibrado (19 ago. 2026, autor: "CV 16
            // steps muda somente o slider verde, bem pouco, o fx e amp
            // não percebo ainda alterações" - CV/AMP/FX usam exatamente
            // a mesma fórmula de `drift` e o mesmo sorteio de chance, e
            // mesmo assim só CV mostrava algo). Causa real: o valor
            // antigo (0.025 a 0.13, vezes activeDepth ~0.3-0.5) resultava
            // num passo absoluto de milésimos por evento - visível só na
            // barra vertical mais alta (CV), invisível nas barras
            // horizontais curtas (AMP/FX). Não é um bug desta rodada, é
            // uma constante antiga nunca recalibrada (ver "Itens ANTIGOS"
            // em PESQUISA_DERIVA_GENERATIVA.md, seção 7) que só ficou
            // evidente agora que o bloco inteiro já não roda em TODO
            // ciclo.
            // Recalibrado de novo (20 ago. 2026, autor: "a variação é
            // sutil", depois do lote de reshape) - mesmo raciocínio de
            // antes (ver comentário de `driftAutonomousItem`), passo
            // ainda maior.
            const auto drift = (0.22f + nextDerivationUnit() * 0.55f) * activeDepth;
            const auto cvTarget = std::clamp(derivationCv[i] + (nextDerivationUnit() - 0.5f) * 0.30f * activeDepth, 0.0f, 1.0f);
            const auto ampTarget = std::clamp(derivationAmp[i] + (nextDerivationUnit() - 0.5f) * 0.24f * activeDepth, 0.12f, 1.0f);
            const auto fxTarget = std::clamp(derivationFx[i] + (nextDerivationUnit() - 0.5f) * 0.34f * activeDepth + routeDensity * 0.08f * activeDepth, 0.0f, 1.0f);
            // Âncoras combináveis (docs/PESQUISA_DERIVA_GENERATIVA.md,
            // seção 6, item 2), 19 ago. 2026 - até 2 passos ao mesmo
            // tempo (`derivationAnchors`, ver seu próprio comentário de
            // membro) nunca derivam de CV, ficam exatamente onde foram
            // capturados, enquanto AMP/FX deles e TUDO dos outros passos
            // continua se movendo normalmente.
            const auto isAnchored = std::find(derivationAnchors.begin(), derivationAnchors.end(), static_cast<int>(i)) != derivationAnchors.end();
            const auto cv = isAnchored ? static_cast<float>(steps[i].cv.getValue())
                                    : static_cast<float>(steps[i].cv.getValue()) + (cvTarget - static_cast<float>(steps[i].cv.getValue())) * drift;
            const auto amp = static_cast<float>(steps[i].level.getValue()) + (ampTarget - static_cast<float>(steps[i].level.getValue())) * drift;
            const auto fx = static_cast<float>(steps[i].send.getValue()) + (fxTarget - static_cast<float>(steps[i].send.getValue())) * drift;
            steps[i].cv.setValue(cv, juce::dontSendNotification);
            steps[i].level.setValue(amp, juce::dontSendNotification);
            steps[i].send.setValue(fx, juce::dontSendNotification);
            derivationCv[i] += (cv - derivationCv[i]) * 0.18f;
            derivationAmp[i] += (amp - derivationAmp[i]) * 0.18f;
            derivationFx[i] += (fx - derivationFx[i]) * 0.18f;
        }
        // Reshape ocasional (20 ago. 2026, autor: "gostaria de mais
        // variação nos sliders do cv 16 steps, eles alteram mas sempre
        // com o mesmo gráfico") - o random walk independente por passo
        // acima preserva a ORDEM relativa entre os passos quase sempre
        // (dois passos raramente se cruzam com um jitter pequeno), por
        // isso os valores mudavam mas o contorno geral do gráfico
        // continuava parecendo o mesmo. Uma troca de posição entre dois
        // passos (CV+AMP+FX juntos, não só CV, pra manter a "voz" de
        // cada passo coerente) embaralha o próprio DESENHO de vez em
        // quando, não só os valores dentro dele - passos ancorados
        // ficam de fora (têm que continuar fixos).
        if (participateSteps.getToggleState() && nextDerivationUnit() < 0.03f + activeDepth * 0.55f)
        {
            const auto a = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
            const auto b = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(steps.size())) % steps.size();
            const auto aAnchored = std::find(derivationAnchors.begin(), derivationAnchors.end(), static_cast<int>(a)) != derivationAnchors.end();
            const auto bAnchored = std::find(derivationAnchors.begin(), derivationAnchors.end(), static_cast<int>(b)) != derivationAnchors.end();
            if (a != b && !aAnchored && !bAnchored)
            {
                std::swap(derivationCv[a], derivationCv[b]);
                std::swap(derivationAmp[a], derivationAmp[b]);
                std::swap(derivationFx[a], derivationFx[b]);
                const auto cvA = static_cast<float>(steps[a].cv.getValue()), cvB = static_cast<float>(steps[b].cv.getValue());
                const auto ampA = static_cast<float>(steps[a].level.getValue()), ampB = static_cast<float>(steps[b].level.getValue());
                const auto fxA = static_cast<float>(steps[a].send.getValue()), fxB = static_cast<float>(steps[b].send.getValue());
                steps[a].cv.setValue(cvB, juce::dontSendNotification); steps[b].cv.setValue(cvA, juce::dontSendNotification);
                steps[a].level.setValue(ampB, juce::dontSendNotification); steps[b].level.setValue(ampA, juce::dontSendNotification);
                steps[a].send.setValue(fxB, juce::dontSendNotification); steps[b].send.setValue(fxA, juce::dontSendNotification);
            }
        }
        if (participateVoice.getToggleState() && nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
        {
            const auto exponent = (nextDerivationUnit() - 0.5f) * (0.10f + activeDepth * 0.38f) + derivationMotion * 0.18f;
            const auto target = std::clamp(derivationRatios[i] * std::pow(2.0f, exponent), 0.25f, 4.0f);
            const auto current = static_cast<float>(oscillatorRates[i].getValue());
            const auto ratio = current + (target - current) * (0.09f + activeDepth * 0.32f);
            oscillatorRates[i].setValue(ratio, juce::dontSendNotification);
            derivationRatios[i] += (ratio - derivationRatios[i]) * 0.16f;
        }
        // Botões CORE dos osciladores (20 ago. 2026, autor: "verifique
        // se os 3 botões dos osciladores se conectam a deriva") - radio
        // group (40106/8038/4069UB), nunca participava. Salto discreto,
        // mesmo espírito de MÉTRICA/SUBDIVISÃO.
        if (participateVoice.getToggleState() && nextDerivationUnit() < 0.06f + activeDepth * 0.18f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(coreSwitches.size())) % coreSwitches.size();
            coreSwitches[index].setToggleState(true, juce::sendNotificationSync);
        }
        // FORMA LFO - radio group de 6, salto discreto.
        if (participateLfoShape.getToggleState() && nextDerivationUnit() < 0.06f + activeDepth * 0.18f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(lfoShapeButtons.size())) % lfoShapeButtons.size();
            lfoShapeButtons[index].setToggleState(true, juce::sendNotificationSync);
        }
        if (participateEffects.getToggleState() && nextDerivationUnit() < 0.45f)
        for (std::size_t i = 0; i < effectControls.size(); ++i)
        {
            const auto current = static_cast<float>(effectControls[i].getValue());
            const auto target = std::clamp(routeDensity * (0.18f + 0.12f * static_cast<float>(i)) * activeDepth + nextDerivationUnit() * 0.24f * activeDepth, 0.0f, 0.56f);
            effectControls[i].setValue(current + (target - current) * (0.10f + activeDepth * 0.28f), juce::dontSendNotification);
        }
        // ROTAS ATIVAS/MATÉRIA/CAOS - see ObjectFiveComponent's own copy
        // of this function for the full comment.
        if (nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < detailControls.size(); ++i)
        {
            auto& toggle = i < 9 ? participateDetail : (i < 13 ? participateMaterial : participateChaos);
            if (!toggle.getToggleState()) continue;
            const auto current = static_cast<float>(detailControls[i].getValue());
            const auto target = std::clamp(derivationDetail[i] + (nextDerivationUnit() - 0.5f) * 0.34f * activeDepth, 0.0f, 1.0f);
            const auto value = current + (target - current) * (0.12f + activeDepth * 0.35f);
            detailControls[i].setValue(value, juce::sendNotificationSync);
            derivationDetail[i] += (value - derivationDetail[i]) * 0.18f;
        }
        }
        // ADSR e LFO rate - see ObjectFiveComponent's own copy of this
        // function for the full comment (docs/
        // PESQUISA_DERIVA_GENERATIVA.md, "diversos fluxos e controles
        // onde a deriva não atua").
        // Event budget - see ObjectFiveComponent's own copy for the full
        // comment.
        if (derivationLayers[1].getToggleState() && sequencer.hasEventBudget(0.25f))
        {
        sequencer.spendEventBudget(0.25f);
        if (participateEnvelope.getToggleState() && nextDerivationUnit() < 0.55f)
        {
        for (std::size_t i = 0; i < envelopeControls.size(); ++i)
        {
            const auto current = static_cast<float>(envelopeControls[i].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            envelopeControls[i].setValue(current + (target - current) * (0.08f + activeDepthB * 0.30f), juce::dontSendNotification);
        }
        sequencer.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue()));
        sequencer.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue()));
        sequencer.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue()));
        sequencer.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue()));
        }
        if (participateModulation.getToggleState() && nextDerivationUnit() < 0.65f)
        {
            const auto current = static_cast<float>(modulationControls[0].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            modulationControls[0].setValue(current + (target - current) * (0.07f + activeDepthB * 0.25f), juce::dontSendNotification);
            sequencer.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f));
        }
        // NOISE MIX, GROOVE, filtro, pans dos osciladores, MÉTRICA/
        // SUBDIVISÃO - see ObjectFiveComponent's own copy of this
        // function for the full comment (docs/
        // PESQUISA_DERIVA_GENERATIVA.md, "métrica, noise, groove,
        // subdivisão" / "pans dos osciladores" / "filtro").
        if (participateModulation.getToggleState() && nextDerivationUnit() < 0.6f)
        {
            const auto current = static_cast<float>(modulationControls[2].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            modulationControls[2].setValue(current + (target - current) * (0.08f + activeDepthB * 0.28f), juce::dontSendNotification);
            sequencer.setNoiseMix(static_cast<float>(modulationControls[2].getValue()));
        }
        // RING - mesmo toggle de MODULAÇÃO (participateModulation).
        if (participateModulation.getToggleState() && nextDerivationUnit() < 0.6f)
        {
            const auto current = static_cast<float>(modulationControls[1].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            modulationControls[1].setValue(current + (target - current) * (0.08f + activeDepthB * 0.28f), juce::sendNotificationSync);
        }
        if (participateGroove.getToggleState() && nextDerivationUnit() < 0.5f)
        {
            const auto current = static_cast<float>(grooveAmount.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            grooveAmount.setValue(current + (target - current) * (0.09f + activeDepthB * 0.30f), juce::dontSendNotification);
            sequencer.setGrooveAmount(static_cast<float>(grooveAmount.getValue()));
        }
        // MAT + KNOB CLOCK - see ObjectFiveComponent's own copy of this
        // function for the full comment.
        if (participateMat.getToggleState() && nextDerivationUnit() < 0.5f)
        {
            const auto current = static_cast<float>(materialFilterMix.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            materialFilterMix.setValue(current + (target - current) * (0.09f + activeDepthB * 0.30f), juce::sendNotificationSync);
        }
        if (participateClock.getToggleState() && nextDerivationUnit() < 0.4f)
        {
            const auto current = static_cast<float>(clock.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (clock.getMaximum() - clock.getMinimum()) * 0.15f,
                                            clock.getMinimum(), clock.getMaximum());
            clock.setValue(current + (target - current) * (0.08f + activeDepthB * 0.24f), juce::sendNotificationSync);
        }
        if (participateFilter.getToggleState() && nextDerivationUnit() < 0.45f)
        {
            const auto current = static_cast<float>(filterCutoff.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            filterCutoff.setValue(current + (target - current) * (0.10f + activeDepthB * 0.32f), juce::dontSendNotification);
            sequencer.setFilterCutoff(static_cast<float>(filterCutoff.getValue()));
        }
        if (participateFilter.getToggleState() && nextDerivationUnit() < 0.4f)
        {
            const auto current = static_cast<float>(filterResonance.getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.2f + activeDepthB * 0.3f), 0.0f, 1.0f);
            filterResonance.setValue(current + (target - current) * (0.11f + activeDepthB * 0.34f), juce::dontSendNotification);
            sequencer.setFilterResonance(static_cast<float>(filterResonance.getValue()));
        }
        // Botões do VCF - see ObjectFiveComponent's own copy of this
        // function for the full comment.
        if (participateFilter.getToggleState() && nextDerivationUnit() < 0.2f + activeDepthB * 0.4f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(filterModeButtons.size())) % filterModeButtons.size();
            filterModeButtons[index].setToggleState(! filterModeButtons[index].getToggleState(), juce::sendNotificationSync);
        }
        // Mixer (sliders vermelhos) - see ObjectFiveComponent's own copy
        // of this function for the full comment.
        if (participateMixer.getToggleState() && nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            // mixGain (vertical) fica de fora - autor: "No mixer os
            // únicos itens com deriva são os sliders horizontais".
            const auto panCurrent = static_cast<float>(mixPan[i].getValue());
            const auto panTarget = std::clamp(derivationMixPan[i] + (nextDerivationUnit() - 0.5f) * 0.6f * activeDepthB, -1.0f, 1.0f);
            const auto pan = panCurrent + (panTarget - panCurrent) * (0.10f + activeDepthB * 0.30f);
            mixPan[i].setValue(pan, juce::sendNotificationSync);
            derivationMixPan[i] += (pan - derivationMixPan[i]) * 0.18f;

            const auto refluxCurrent = static_cast<float>(mixReflux[i].getValue());
            const auto refluxTarget = std::clamp(derivationMixReflux[i] + (nextDerivationUnit() - 0.5f) * 0.3f * activeDepthB, 0.0f, 0.72f);
            const auto reflux = refluxCurrent + (refluxTarget - refluxCurrent) * (0.10f + activeDepthB * 0.30f);
            mixReflux[i].setValue(reflux, juce::sendNotificationSync);
            derivationMixReflux[i] += (reflux - derivationMixReflux[i]) * 0.18f;
        }
        // M1-4 RECALL (nunca CAPTURE) - see ObjectFiveComponent's own
        // copy of this function for the full comment.
        if (participateMixMemory.getToggleState() && nextDerivationUnit() < 0.05f + activeDepthB * 0.25f)
        {
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(mixMemorySlots.size())) % mixMemorySlots.size();
            if (mixMemoryCaptured[index])
            {
                sequencer.recallMixMemory(index);
                for (std::size_t channel = 0; channel < mixGain.size(); ++channel)
                {
                    const auto recalled = sequencer.getMixChannel(channel);
                    mixGain[channel].setValue(recalled.gain, juce::dontSendNotification);
                    mixPan[channel].setValue(recalled.pan, juce::dontSendNotification);
                    mixReflux[channel].setValue(recalled.reflux, juce::dontSendNotification);
                    mixEnable[channel].setToggleState(recalled.enabled, juce::dontSendNotification);
                    mixMute[channel].setToggleState(recalled.mute, juce::dontSendNotification);
                    mixSolo[channel].setToggleState(recalled.solo, juce::dontSendNotification);
                    derivationMixGain[channel] = recalled.gain;
                    derivationMixPan[channel] = recalled.pan;
                    derivationMixReflux[channel] = recalled.reflux;
                }
            }
        }
        // CONEXOES ENTRE OBJETOS - see ObjectFiveComponent's own copy
        // of this function for context (so existe aqui, dualEngine e
        // compartilhado).
        if (participateConnections.getToggleState() && nextDerivationUnit() < 0.5f)
        {
            const auto gainToFifthCurrent = static_cast<float>(gainToFifth.getValue());
            const auto gainToFifthTarget = std::clamp(derivationGainToFifth + (nextDerivationUnit() - 0.5f) * 0.3f * activeDepthB, 0.0f, 0.72f);
            gainToFifth.setValue(gainToFifthCurrent + (gainToFifthTarget - gainToFifthCurrent) * (0.10f + activeDepthB * 0.30f), juce::sendNotificationSync);
            derivationGainToFifth += (static_cast<float>(gainToFifth.getValue()) - derivationGainToFifth) * 0.18f;

            const auto gainToFirstCurrent = static_cast<float>(gainToFirst.getValue());
            const auto gainToFirstTarget = std::clamp(derivationGainToFirst + (nextDerivationUnit() - 0.5f) * 0.3f * activeDepthB, 0.0f, 0.72f);
            gainToFirst.setValue(gainToFirstCurrent + (gainToFirstTarget - gainToFirstCurrent) * (0.10f + activeDepthB * 0.30f), juce::sendNotificationSync);
            derivationGainToFirst += (static_cast<float>(gainToFirst.getValue()) - derivationGainToFirst) * 0.18f;

            const auto auxToFirstCurrent = static_cast<float>(auxToFirst.getValue());
            const auto auxToFirstTarget = std::clamp(derivationAuxToFirst + (nextDerivationUnit() - 0.5f) * 0.3f * activeDepthB, 0.0f, 0.72f);
            auxToFirst.setValue(auxToFirstCurrent + (auxToFirstTarget - auxToFirstCurrent) * (0.10f + activeDepthB * 0.30f), juce::sendNotificationSync);
            derivationAuxToFirst += (static_cast<float>(auxToFirst.getValue()) - derivationAuxToFirst) * 0.18f;

            const auto auxToFifthCurrent = static_cast<float>(auxToFifth.getValue());
            const auto auxToFifthTarget = std::clamp(derivationAuxToFifth + (nextDerivationUnit() - 0.5f) * 0.3f * activeDepthB, 0.0f, 0.72f);
            auxToFifth.setValue(auxToFifthCurrent + (auxToFifthTarget - auxToFifthCurrent) * (0.10f + activeDepthB * 0.30f), juce::sendNotificationSync);
            derivationAuxToFifth += (static_cast<float>(auxToFifth.getValue()) - derivationAuxToFifth) * 0.18f;
        }
        if (participateConnections.getToggleState() && nextDerivationUnit() < 0.2f + activeDepthB * 0.4f)
        {
            const auto pickFirst = nextDerivationUnit() < 0.5f;
            const auto index = static_cast<std::size_t>(nextDerivationUnit() * 4.0f) % 4;
            auto& button = pickFirst ? routesToFifth[index] : routesToFirst[index];
            button.setToggleState(! button.getToggleState(), juce::sendNotificationSync);
        }
        }
        // Event budget - see ObjectFiveComponent's own copy for the full
        // comment.
        if (derivationLayers[2].getToggleState() && sequencer.hasEventBudget(0.25f))
        {
        sequencer.spendEventBudget(0.25f);
        if (participateVoice.getToggleState() && nextDerivationUnit() < 0.5f)
        for (std::size_t i = 0; i < oscillatorPans.size(); ++i)
        {
            const auto current = static_cast<float>(oscillatorPans[i].getValue());
            const auto target = std::clamp(current + (nextDerivationUnit() - 0.5f) * (0.4f + activeDepthC * 0.6f), -1.0f, 1.0f);
            const auto pan = current + (target - current) * (0.09f + activeDepth * 0.30f);
            oscillatorPans[i].setValue(pan, juce::dontSendNotification);
            sequencer.setOscillatorPan(i, pan);
        }
        if (participateMetric.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
        {
            metricSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
            metricButtons[static_cast<std::size_t>(metricSelection)].setToggleState(true, juce::dontSendNotification);
        }
        if (participateTemporal.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
        {
            temporalSelection = static_cast<int>(nextDerivationUnit() * 8.0f) % 8;
            temporalButtons[static_cast<std::size_t>(temporalSelection)].setToggleState(true, juce::dontSendNotification);
            // GLT decide a âncora - see ObjectFiveComponent's own copy
            // of this function for the full comment.
            if (temporalSelection == 7)
            {
                derivationAnchors[static_cast<std::size_t>(derivationAnchorWrite)] = static_cast<int>(nextDerivationUnit() * static_cast<float>(steps.size()));
                derivationAnchorWrite = (derivationAnchorWrite + 1) % static_cast<int>(derivationAnchors.size());
            }
        }
        syncTemporal();
        // NOISE COR - see ObjectFiveComponent's own copy of this
        // function for the full comment.
        if (participateNoiseColour.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
            noiseSelector.select(static_cast<int>(nextDerivationUnit() * 6.0f) % 6, true);
        // FIM DO LOOP - see ObjectFiveComponent's own copy of this
        // function for the full comment.
        if (participateLoopEnd.getToggleState() && nextDerivationUnit() < 0.06f + activeDepthC * 0.18f)
            // Nunca 1 (20 ago. 2026, autor: "travou no 1 do fim do loop, tanto
                // no clone como no principal") - com loopEnd=1 o playhead
                // nunca sai do passo 0, então a condição de disparo de
                // deriveFromMemory() (`active == 0 && lastDerivationStep
                // != 0`) nunca mais fica verdadeira - um deadlock que a
                // própria DERIVA causava em si mesma. Faixa [2,16] em vez
                // de [1,16] garante que o playhead sempre tem que sair do
                // 0 antes de poder voltar pra ele.
                setLoopEnd(static_cast<std::size_t>(nextDerivationUnit() * static_cast<float>(loopSwitches.size() - 1)) + 2);
        }
        }
        syncStepDynamics(); syncRatios(); syncEffects();
        refreshSilentHighlights();
        ++derivationPhrase;
        configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::driftPhraseHeaderPrefix, uiLanguage) + juce::String(derivationPhrase) + antitotem::ui::text(antitotem::ui::logText::driftPhraseHeaderMid, uiLanguage) + juce::String(static_cast<int>(activeDepth * 100.0f)) + antitotem::ui::text(antitotem::ui::logText::driftPhraseHeaderRoute, uiLanguage) + juce::String::toHexString(static_cast<int>(nextRoute)).toUpperCase(), 12.0f, material::memory);
        appendLog(antitotem::ui::text(antitotem::ui::logText::driftPhrasePrefix, uiLanguage) + juce::String(derivationPhrase)
                  + antitotem::ui::text(antitotem::ui::logText::routeSuffix, uiLanguage) + juce::String::toHexString(static_cast<int>(nextRoute)).toUpperCase());
    }

    void refreshStepControls()
    {
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            steps[i].cv.setValue(sequencer.getStepVoltage(i), juce::dontSendNotification);
            steps[i].level.setValue(sequencer.getStepLevel(i), juce::dontSendNotification);
            steps[i].send.setValue(sequencer.getStepEffectSend(i), juce::dontSendNotification);
            steps[i].mute.setToggleState(sequencer.isStepMuted(i), juce::dontSendNotification);
        }
    }
    void setFeedbackRoutes(unsigned char routes)
    {
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
            connectionSwitches[i].setToggleState((routes & static_cast<unsigned char>(1U << i)) != 0, juce::dontSendNotification);
        sequencer.setFeedbackConnections(routes);
    }
    void setNoiseColour(int index)
    {
        constexpr std::array<antitotem::NoisePalette::Colour, 6> colours {
            antitotem::NoisePalette::Colour::white, antitotem::NoisePalette::Colour::pink,
            antitotem::NoisePalette::Colour::brown, antitotem::NoisePalette::Colour::blue,
            antitotem::NoisePalette::Colour::violet, antitotem::NoisePalette::Colour::bit };
        const auto safeIndex = std::clamp(index, 0, static_cast<int>(colours.size()) - 1);
        noiseSelector.select(safeIndex, false);
        sequencer.setNoiseColour(colours[static_cast<std::size_t>(safeIndex)]);
    }
    void applyVariation(Variation variation)
    {
        switch (variation)
        {
            case Variation::pulse:
                antitotem::variations::pulseAndGates(sequencer);
                clock.setValue(3.6, juce::dontSendNotification); energy.setValue(0.78, juce::dontSendNotification);
                oscillatorCoreSelection = 1; coreSwitches[0].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(8); setFeedbackRoutes(0x09U); feedbackGain.setValue(0.30, juce::dontSendNotification);
                modulationControls[1].setValue(0.12, juce::dontSendNotification); modulationControls[2].setValue(0.0, juce::dontSendNotification);
                effectControls[0].setValue(0.0, juce::dontSendNotification); effectControls[1].setValue(0.0, juce::dontSendNotification); effectControls[2].setValue(0.0, juce::dontSendNotification);
                noiseSelector.setSampleHold(false, false); sequencer.setSampleHoldMix(0.0f);
                break;
            case Variation::porous:
                antitotem::variations::porousMemory(sequencer);
                clock.setValue(1.25, juce::dontSendNotification); energy.setValue(0.52, juce::dontSendNotification);
                oscillatorCoreSelection = 3; coreSwitches[2].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x24U); feedbackGain.setValue(0.42, juce::dontSendNotification);
                modulationControls[1].setValue(0.0, juce::dontSendNotification); modulationControls[2].setValue(0.15, juce::dontSendNotification);
                effectControls[0].setValue(0.38, juce::dontSendNotification); effectControls[1].setValue(0.16, juce::dontSendNotification); effectControls[2].setValue(0.08, juce::dontSendNotification);
                noiseSelector.setSampleHold(true, false); sequencer.setSampleHoldMix(0.78f); setNoiseColour(1);
                break;
            case Variation::heterodyne:
                antitotem::variations::heterodyneField(sequencer);
                clock.setValue(4.8, juce::dontSendNotification); energy.setValue(0.86, juce::dontSendNotification);
                oscillatorCoreSelection = 2; coreSwitches[1].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x1aU); feedbackGain.setValue(0.56, juce::dontSendNotification);
                modulationControls[1].setValue(0.36, juce::dontSendNotification); modulationControls[2].setValue(0.08, juce::dontSendNotification);
                effectControls[0].setValue(0.12, juce::dontSendNotification); effectControls[1].setValue(0.34, juce::dontSendNotification); effectControls[2].setValue(0.22, juce::dontSendNotification);
                noiseSelector.setSampleHold(true, false); sequencer.setSampleHoldMix(0.78f); setNoiseColour(5);
                break;
            case Variation::orbit:
                antitotem::variations::orbitAndDrift(sequencer);
                clock.setValue(0.9, juce::dontSendNotification); energy.setValue(0.58, juce::dontSendNotification);
                oscillatorCoreSelection = 2; coreSwitches[1].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x01U); feedbackGain.setValue(0.18, juce::dontSendNotification);
                modulationControls[1].setValue(0.0, juce::dontSendNotification); modulationControls[2].setValue(0.0, juce::dontSendNotification);
                effectControls[0].setValue(0.42, juce::dontSendNotification); effectControls[1].setValue(0.1, juce::dontSendNotification); effectControls[2].setValue(0.0, juce::dontSendNotification);
                {
                    // Same per-oscillator values as antitotem::variations::orbitAndDrift -
                    // the engine call above already set these on sequencer directly;
                    // this just brings the on-screen knobs into visual agreement.
                    constexpr std::array<float, 5> proximity { 0.35f, 0.55f, 0.45f, 0.65f, 0.5f };
                    constexpr std::array<float, 5> orbit { 0.6f, 0.4f, 0.7f, 0.5f, 0.65f };
                    for (std::size_t i = 0; i < proximity.size(); ++i)
                    {
                        oscillatorProximities[i].setValue(proximity[i], juce::dontSendNotification);
                        oscillatorOrbits[i].setValue(orbit[i], juce::dontSendNotification);
                    }
                }
                noiseSelector.setSampleHold(false, false); sequencer.setSampleHoldMix(0.0f);
                break;
            case Variation::pendulum:
                antitotem::variations::pendulumResonance(sequencer);
                clock.setValue(2.4, juce::dontSendNotification); energy.setValue(0.7, juce::dontSendNotification);
                oscillatorCoreSelection = 1; coreSwitches[0].setToggleState(true, juce::dontSendNotification);
                setLoopEnd(16); setFeedbackRoutes(0x08U); feedbackGain.setValue(0.4, juce::dontSendNotification);
                scannerSelection = 2; scannerButtons[2].setToggleState(true, juce::dontSendNotification);
                modulationControls[1].setValue(0.0, juce::dontSendNotification); modulationControls[2].setValue(0.0, juce::dontSendNotification);
                effectControls[0].setValue(0.08, juce::dontSendNotification); effectControls[1].setValue(0.05, juce::dontSendNotification); effectControls[2].setValue(0.0, juce::dontSendNotification);
                detailControls[6].setValue(0.68, juce::dontSendNotification); detailControls[7].setValue(0.62, juce::dontSendNotification); detailControls[8].setValue(0.75, juce::dontSendNotification);
                noiseSelector.setSampleHold(false, false); sequencer.setSampleHoldMix(0.0f);
                break;
        }
        sequencer.setFeedbackAmount(static_cast<float>(feedbackGain.getValue()));
        refreshStepControls();
        refreshSilentHighlights();
        syncDetails();
        const auto name = variation == Variation::pulse ? antitotem::ui::text(antitotem::ui::button::variationPulse, uiLanguage)
                        : variation == Variation::porous ? antitotem::ui::text(antitotem::ui::button::variationPorous, uiLanguage)
                        : variation == Variation::heterodyne ? antitotem::ui::text(antitotem::ui::button::variationHeterodyne, uiLanguage)
                        : variation == Variation::orbit ? antitotem::ui::text(antitotem::ui::button::variationOrbit, uiLanguage)
                        : antitotem::ui::text(antitotem::ui::button::variationPendulum, uiLanguage);
        appendLog(antitotem::ui::text(antitotem::ui::logText::variationPrefix, uiLanguage) + name
                  + antitotem::ui::text(antitotem::ui::logText::routeSuffix, uiLanguage)
                  + juce::String::toHexString(static_cast<int>(currentFeedbackRoutes())).toUpperCase());
    }

    void setPage(Page requested)
    {
        page = requested;
        const bool unified = useUnifiedLayout();
        soundPage.setVisible(! unified);
        sequencePage.setVisible(! unified);
        mixPage.setVisible(! unified);
        const bool sequenceVisible = unified || page == Page::sequence;
        const bool soundVisible = unified || page == Page::sound;
        const bool mixVisible = unified || page == Page::mix;
        const bool temporalVisible = unified || page != Page::mix;
        for (auto& step : steps) step.setVisible(sequenceVisible);
        for (auto& loopSwitch : loopSwitches) loopSwitch.setVisible(sequenceVisible);
        for (auto& connectionSwitch : connectionSwitches) connectionSwitch.setVisible(sequenceVisible);
        feedbackLabel.setVisible(sequenceVisible); feedbackGain.setVisible(sequenceVisible);
        deriveLabel.setVisible(sequenceVisible); deriveDepth.setVisible(sequenceVisible);
        clock.setVisible(sequenceVisible); clockLabel.setVisible(sequenceVisible);
        loopLabel.setVisible(sequenceVisible); connectionLabel.setVisible(sequenceVisible);
        run.setVisible(sequenceVisible); stop.setVisible(sequenceVisible); reset.setVisible(sequenceVisible); record.setVisible(sequenceVisible);
        for (auto& button : recordDurations) button.setVisible(sequenceVisible);
        temporalLabel.setVisible(temporalVisible); metricLabel.setVisible(temporalVisible); scannerLabel.setVisible(temporalVisible);
        for (auto& control : temporalButtons) control.setVisible(temporalVisible);
        for (auto& control : metricButtons) control.setVisible(temporalVisible);
        for (auto& control : scannerButtons) control.setVisible(temporalVisible);

        for (auto& chipCard : concepts) chipCard.setVisible(false);
        for (auto& control : coreSwitches) control.setVisible(soundVisible);
        for (auto& control : envelopeControls) control.setVisible(soundVisible);
        for (auto& label : envelopeControlLabels) label.setVisible(soundVisible);
        for (auto& label : filterControlLabels) label.setVisible(soundVisible);
        for (auto& control : modulationControls) control.setVisible(soundVisible);
        lfoShapeLabel.setVisible(soundVisible);
        for (auto& control : lfoShapeButtons) control.setVisible(soundVisible);
        lfoFreeze.setVisible(soundVisible);
        for (auto& control : effectControls) control.setVisible(soundVisible);
        detailLabel.setVisible(soundVisible);
        materialRailLabel.setVisible(soundVisible);
        for (auto& control : detailControls) control.setVisible(soundVisible);
        for (auto& label : detailControlLabels) label.setVisible(soundVisible);
        for (auto& label : modulationControlLabels) label.setVisible(soundVisible);
        for (auto& label : effectControlLabels) label.setVisible(soundVisible);
        for (auto& control : oscillatorRates) control.setVisible(soundVisible);
        for (auto& control : oscillators) control.setVisible(soundVisible);
        for (auto& control : shapes) control.setVisible(soundVisible);
        for (auto& control : oscillatorPans) control.setVisible(soundVisible);
        for (auto& control : oscillatorProximities) control.setVisible(soundVisible);
        for (auto& control : oscillatorOrbits) control.setVisible(soundVisible);
        for (auto& label : oscillatorLabels) label.setVisible(soundVisible);
        for (auto& label : oscillatorRateLabels) label.setVisible(soundVisible);
        for (auto& label : oscillatorLevelLabels) label.setVisible(soundVisible);
        for (auto& label : oscillatorShapeLabels) label.setVisible(soundVisible);
        for (auto& label : oscillatorPanCaptions) label.setVisible(soundVisible);
        for (auto& label : oscillatorProximityCaptions) label.setVisible(soundVisible);
        for (auto& label : oscillatorOrbitCaptions) label.setVisible(soundVisible);
        for (auto* control : { &energy, &master, &filterCutoff, &filterResonance, &filterDepth }) control->setVisible(soundVisible);
        noiseSelector.setVisible(soundVisible);
        for (auto* label : { &energyLabel, &masterLabel, &modulationLabel, &effectsLabel, &filterLabel, &envelopeLabel, &voiceLabel }) label->setVisible(soundVisible);
        for (auto& button : filterModeButtons) button.setVisible(soundVisible);
        log.setVisible(soundVisible);
        stereoScope.setVisible(true);
        for (auto& label : mixLabels) label.setVisible(mixVisible);
        for (auto& control : mixGain) control.setVisible(mixVisible);
        for (auto& control : mixPan) control.setVisible(mixVisible);
        for (auto& control : mixReflux) control.setVisible(mixVisible);
        for (auto& control : mixEnable) control.setVisible(mixVisible);
        for (auto& control : mixMute) control.setVisible(mixVisible);
        for (auto& control : mixSolo) control.setVisible(mixVisible);
        mixMemoryLabel.setVisible(mixVisible);
        for (auto& control : mixMemorySlots) control.setVisible(mixVisible);
        mixMemoryCapture.setVisible(mixVisible);
        resized();
    }

    void layoutMix(juce::Rectangle<int> area)
    {
        auto memoryRow = area.removeFromTop(40);
        mixMemoryLabel.setBounds(memoryRow.removeFromTop(16));
        const auto memoryButtonWidth = memoryRow.getWidth() / static_cast<int>(mixMemorySlots.size() + 1);
        for (auto& slot : mixMemorySlots) slot.setBounds(memoryRow.removeFromLeft(memoryButtonWidth).reduced(3, 1));
        mixMemoryCapture.setBounds(memoryRow.reduced(3, 1));
        const auto stripWidth = area.getWidth() / static_cast<int>(mixGain.size());
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            auto strip = juce::Rectangle<int>(area.getX() + static_cast<int>(i) * stripWidth, area.getY(), stripWidth, area.getHeight()).reduced(16, 2);
            mixLabels[i].setBounds(strip.removeFromTop(24));
            auto buttons = strip.removeFromTop(28);
            mixEnable[i].setBounds(buttons.removeFromLeft(48).reduced(2, 2));
            mixMute[i].setBounds(buttons.removeFromLeft(34).reduced(2, 2));
            mixSolo[i].setBounds(buttons.removeFromLeft(34).reduced(2, 2));
            mixGain[i].setBounds(strip.removeFromTop(std::max(130, strip.getHeight() - 90)).reduced(28, 2));
            mixPan[i].setBounds(strip.removeFromTop(29).reduced(5, 4));
            mixReflux[i].setBounds(strip.removeFromTop(29).reduced(5, 4));
        }
    }

    void layoutUnified(juce::Rectangle<int> area)
    {
        // Reference canvas: 1920x1080. The order follows signal flow rather
        // than application pages: source -> route -> sequence -> space/mix.
        // 240: taller than the ~92px minimum each row's own budget needs -
        // now that the clock column reaches the footer anyway, this row can
        // afford to give the CV slider and its neighbours real room instead
        // of the smallest size that still technically fits.
        // 235, not 280: EIXO Y/Z joined EIXO X below FREQ/MIX/FORMA,
        // needing one more 45px pan-style row per oscillator column -
        // measured via a temporary DBG that moduleArea needed 418px
        // (40 label + 3x96 knob rows + 45x2 pan/YZ rows) but was only
        // getting 373, 45px short. That 45px comes back out of stepsArea
        // (the sequencer), the same flexible band it came from originally.
        auto stepsArea = area.removeFromBottom(235);

        // 120: rails' own height is still subtracted from moduleArea before
        // oscillators/VCF/ADSR ever see it, so any increase here narrows
        // their own row spacing - confirmed again after testing 220 with
        // fixed-size oscillator knobs still on: the knobs stayed 80px but
        // their cell rows shrank below that, so adjacent rows overlapped.
        // The sequencer (stepsArea) can grow because it doesn't share
        // moduleArea's budget; rails can't, for as long as it does.
        // `routing` (6px) now comes out here, between stepsArea and rails,
        // not after rails (18 ago. 2026, author: "o último slider da
        // coluna matéria... está muito colado ao CV do Sequencer... subir
        // levemente todo o objeto parametros... sem alterar o tamanho dos
        // osciladores") - MATÉRIA's own 4th slider (ASYM) deliberately
        // overflows below rails' own height into stepsArea, and had zero
        // clearance before stepsArea's own content since this budget's
        // only gap used to sit on the OTHER side of rails, between it and
        // moduleArea. Moving it here shifts the whole rails band up by
        // the same 6px, and moduleArea's own height is unaffected - still
        // exactly 235+6+120 removed from `area` in total, just reordered
        // (the height sums a few lines down are order-independent, they
        // just add all four rectangles' heights either way).
        auto routing = area.removeFromBottom(6);
        auto rails = area.removeFromBottom(120);
        auto moduleArea = area;
        // 230, not 180: a wider CLOCK column gives PLAY/STOP/RESET/REC/FAIXA
        // more room, and PULSO/MÉTRICA/PERCURSO below (aligned to match, see
        // clockColumnInset/clockButtonWidth) inherits the same extra space.
        constexpr int clockColumnWidth = 230;
        constexpr int clockColumnInset = 4;
        // The clock column now runs the full height from the top modules
        // all the way down to the footer - through the routing row and the
        // steps row too - so FIM DO LOOP can join PULSO/MÉTRICA/PERCURSO in
        // the same tall stack, and the knob has room to be full-sized again,
        // instead of a separate module below the oscillators that only
        // happened to start at the same x.
        // moduleArea/routing/rails/stepsArea stack top-to-bottom in that
        // exact order (rails sits between routing and stepsArea) - a full-
        // height column has to sum all four, not just three. Missing
        // `rails` here was a real bug: both this column and mixerColumn
        // fell exactly rails.getHeight() short of the true bottom, no
        // matter what rails' own height was set to (DERIVA and LOG both
        // stopped short by that same amount, which is what made the gap
        // below LOG track rails' size instead of ever reaching the footer).
        auto transport = juce::Rectangle<int>(moduleArea.getX(), moduleArea.getY(), clockColumnWidth,
            moduleArea.getHeight() + routing.getHeight() + rails.getHeight() + stepsArea.getHeight()).reduced(clockColumnInset, 2);
        // 14px breathing room between the clock/mixer columns and the
        // central body - CLONE always had this (author, live: "gosto da
        // distancia entre colunas do clone, manter esse critério" /
        // "espaço de margem entre colunas e corpo, está melhor como no
        // clone"); PRINCIPAL used to sit flush against both columns, so
        // this makes PRINCIPAL match CLONE instead of the other way
        // around.
        constexpr int columnGap = 14;
        moduleArea.removeFromLeft(clockColumnWidth + columnGap);
        routing.removeFromLeft(clockColumnWidth + columnGap);
        // The mixer column mirrors CLOCK's own trick on the other edge: it
        // runs the full height too (through routing and the step row), so
        // ENERGIA/MASTER and LOG can join MEMÓRIA MIX and the four channels
        // in one continuous right-hand stack instead of MASTER/ENERGIA
        // needing a home elsewhere and LOG eating into the step grid's row.
        constexpr int mixerColumnWidth = 300;
        auto mixerColumn = juce::Rectangle<int>(moduleArea.getRight() - mixerColumnWidth, moduleArea.getY(), mixerColumnWidth,
            moduleArea.getHeight() + routing.getHeight() + rails.getHeight() + stepsArea.getHeight());
        moduleArea.removeFromRight(mixerColumnWidth + columnGap);
        routing.removeFromRight(mixerColumnWidth + columnGap);
        stepsArea.removeFromRight(mixerColumnWidth + columnGap);
        layoutTransportColumn(transport, modeLabel, clockLabel, clock,
                               temporalLabel, temporalButtons.data(), static_cast<int>(temporalButtons.size()),
                               grooveLabel, grooveAmount,
                               metricLabel, metricButtons.data(), static_cast<int>(metricButtons.size()),
                               scannerLabel, scannerButtons.data(), static_cast<int>(scannerButtons.size()),
                               loopLabel, loopSwitches.data(), static_cast<int>(loopSwitches.size()), 8,
                               connectionLabel, connectionSwitches.data(), static_cast<int>(connectionSwitches.size()),
                               &feedbackLabel, feedbackGain,
                               &deriveLabel, deriveDepth, deriveButton,
                               derivationLayers.data(), static_cast<int>(derivationLayers.size()),
                               variationLabel, pulseVariation, porousVariation, heterodyneVariation,
                               randomizeStepsButton, orbitVariation, pendulumVariation,
                               &learnLabel, &learnEditor);

        // MODULAÇÃO/FORMA LFO/ESPAÇO-FASE/ROTAS ATIVAS moved back to their
        // own horizontal band above the sequencer (see `rails` below) -
        // tried here below VARIAÇÃO, didn't read well.

        // Left to right: transport (already placed) -> oscillators -> ADSR
        // -> VCF -> mixer column (already placed above), so the mixer sits
        // beside the master/output stage it feeds instead of at the
        // opposite end of the row from it. Shared with CLONE via
        // layoutVoiceArea() - see that function's own comment for the
        // audit that led to extracting it and the ADSR/VCF/ENERGIA
        // alignment fix it carries.
        layoutVoiceArea(moduleArea, voiceLabel, coreSwitches,
                         oscillatorLabels, oscillatorRateLabels, oscillatorRates,
                         oscillatorLevelLabels, oscillators, oscillatorShapeLabels, shapes,
                         oscillatorPanCaptions, oscillatorPans,
                         oscillatorProximityCaptions, oscillatorProximities,
                         oscillatorOrbitCaptions, oscillatorOrbits,
                         filterLabel, filterModeButtons, filterControlLabels, filterCutoff, filterResonance, filterDepth,
                         materialFilterLabel, materialFilterMix,
                         envelopeLabel, envelopeControlLabels, envelopeControls,
                         energyLabel, energy, noiseLabel, noiseSelector,
                         modulationLabel, modulationControlLabels, modulationControls,
                         lfoShapeLabel, lfoShapeButtons.data(), lfoFreeze,
                         chaosFreezeHighlight);

        // PULSO/POROSA/HETERÓDINA/RND16/ÓRBITA/PÊNDULO/CLONE moved here from
        // the header's own action row - that row's height goes back to the
        // oscillators above instead (see resized()'s useUnifiedLayout
        // branch), which is the actual point of the move, not just tidying.
        // VARIAÇÃO itself moved on to the transport column (below DERIVA) -
        // it presets the source/sequencer controls over there, not the
        // mixer this column actually holds - so this column now opens
        // straight from its own MIXER title into MEMÓRIA MIX.
        mixerLabel.setBounds(mixerColumn.removeFromTop(15));
        mixerColumn.removeFromTop(4);

        // CONEXÃO ENTRE OBJETOS and LOG now live at the bottom of this same
        // full-height column, below the channels - the channel strips take
        // whatever height is left over after those fixed-size blocks,
        // rather than a hand-guessed fraction (same reasoning as CLOCK's
        // own clockHeight leftover-budget pattern above). MASTER moved to
        // the header, ENERGIA moved beside ADSR (see energyNoiseArea
        // above) - CONEXÃO ENTRE OBJETOS takes their old spot instead,
        // moved here from CLONE's left column: it describes the
        // relationship between the two objects, not a property of either
        // one alone, so it's fixed here regardless of which body (in the
        // eventual PRINCIPAL/CLONE toggle) is showing.
        constexpr int mixerGap = 10;
        // Channels + MEMÓRIA MIX, shared with CLONE via
        // layoutMixerChannels() - see that function's own comment for the
        // audit that led to extracting it. Returns whatever's left of
        // mixerColumn below MEMÓRIA MIX for CONEXÃO ENTRE OBJETOS/LOG.
        mixerColumn = layoutMixerChannels(mixerColumn, mixLabels, mixEnable, mixMute, mixSolo,
                                           mixGain, mixPan, mixReflux, mixMemoryLabel, mixMemorySlots, mixMemoryCapture);
        mixerColumn.removeFromTop(mixerGap);
        objectConnectionLabel.setBounds(mixerColumn.removeFromTop(16));
        auto placeObjectGainVertical = [&] (juce::Label& label, juce::Slider& slider)
        {
            label.setBounds(mixerColumn.removeFromTop(13));
            slider.setBounds(mixerColumn.removeFromTop(20));
            mixerColumn.removeFromTop(6);
        };
        placeObjectGainVertical(gainToFifthLabel, gainToFifth);
        placeObjectGainVertical(gainToFirstLabel, gainToFirst);
        placeObjectGainVertical(auxToFirstLabel, auxToFirst);
        placeObjectGainVertical(auxToFifthLabel, auxToFifth);
        mixerColumn.removeFromTop(10);
        routesToFifthLabel.setBounds(mixerColumn.removeFromTop(13));
        auto objectRoutes1 = mixerColumn.removeFromTop(24);
        const auto objectRouteButtonWidth1 = objectRoutes1.getWidth() / 4;
        for (auto& button : routesToFifth) button.setBounds(objectRoutes1.removeFromLeft(objectRouteButtonWidth1).reduced(2, 1));
        mixerColumn.removeFromTop(10);
        routesToFirstLabel.setBounds(mixerColumn.removeFromTop(13));
        auto objectRoutes2 = mixerColumn.removeFromTop(24);
        const auto objectRouteButtonWidth2 = objectRoutes2.getWidth() / 4;
        for (auto& button : routesToFirst) button.setBounds(objectRoutes2.removeFromLeft(objectRouteButtonWidth2).reduced(2, 1));
        mixerColumn.removeFromTop(mixerGap);
        // The log's own content is a handful of short lines, not a wall of
        // text - it does not need to stretch to fill whatever is left.
        // Anchored to this column's own bottom edge (which lines up with
        // the step grid's bottom, since mixerColumn spans the same total
        // height as transport does) instead of sitting right under
        // ENERGIA/MASTER with a gap below it. logLabel (a real, persistent
        // header - see its own configureLabel() comment) is carved from
        // the top of this same reduced area, so the log box's own bottom
        // edge stays exactly where it was.
        auto logArea = mixerColumn.reduced(4, 4);
        logLabel.setBounds(logArea.removeFromTop(14));
        log.setBounds(logArea);

        // FIM DO LOOP and PORTAS DE FEEDBACK both moved into the clock
        // column itself (above); the routing row they used to occupy is
        // free.

        // FORMA LFO/MODULAÇÃO/ESPAÇO-FASE/ROTAS ATIVAS, same shared
        // layoutRailsBand() CLONE uses - the clock and mixer columns both
        // run the full height including this band (see `transport`/
        // `mixerColumn` above), so rails must skip clockColumnWidth+
        // columnGap on the left and mixerColumnWidth+columnGap on the
        // right - otherwise its right-aligned rows would draw directly
        // under LOG/ENERGIA/MASTER instead of stopping short of them
        // (CLONE's own `area` is already trimmed, so it passes 0/0
        // instead).
        layoutRailsBand(rails, clockColumnWidth + columnGap, mixerColumnWidth + columnGap,
                         parametersLabel,
                         effectsLabel, effectControlLabels.data(), effectControls.data(),
                         detailLabel, detailControlLabels.data(), detailControls.data(),
                         materialRailLabel, chaosRailLabel);

        // Left edge matches the oscillators column, not the CLOCK column -
        // the clock now has its own tall stack of controls above, so the
        // step grid no longer needs to (and visually shouldn't) run under it.
        auto stepGrid = stepsArea.reduced(5, 2);
        stepGrid.removeFromLeft(clockColumnWidth + columnGap);
        stepsLabel.setBounds(stepGrid.removeFromTop(15));
        // Limitado (não só dividido) para o grid de 16 passos não inflar
        // indefinidamente numa janela redimensionada bem larga - 162px é o
        // valor natural na resolução alvo do app (1920x1080); 200 dá
        // alguma folga acima disso sem deixar cada célula desproporcional
        // perto de 400px (o que ocorria sem limite, no teto de 3840px).
        const auto stepWidth = std::clamp(stepGrid.getWidth() / static_cast<int>(antitotem::SimpleSequencer::historicalScannerSteps), 100, 200);
        const auto stepHeight = stepGrid.getHeight() / 2;
        for (std::size_t i = 0; i < steps.size(); ++i)
            steps[i].setBounds(juce::Rectangle<int>(stepGrid.getX() + static_cast<int>(i % antitotem::SimpleSequencer::historicalScannerSteps) * stepWidth,
                                                     stepGrid.getY() + static_cast<int>(i / antitotem::SimpleSequencer::historicalScannerSteps) * stepHeight,
                                                     stepWidth, stepHeight).reduced(5, 4));

        // Participação por título - see ObjectFiveComponent's own copy
        // of this function for the full comment. `objectConnectionLabel`
        // extra aqui, único (CONEXÕES ENTRE OBJETOS não existe em CLONE).
        {
            const std::array<juce::Label*, 21> participationLabels {
                &stepsLabel, &voiceLabel, &effectsLabel, &detailLabel, &mixerLabel, &envelopeLabel,
                &modulationLabel, &grooveLabel, &filterLabel, &metricLabel, &temporalLabel, &noiseLabel,
                &loopLabel, &connectionLabel, &mixMemoryLabel, &objectConnectionLabel,
                &materialRailLabel, &chaosRailLabel, &materialFilterLabel, &lfoShapeLabel, &clockLabel
            };
            const std::array<juce::ToggleButton*, 21> participationTogglesForLayout {
                &participateSteps, &participateVoice, &participateEffects, &participateDetail,
                &participateMixer, &participateEnvelope, &participateModulation, &participateGroove,
                &participateFilter, &participateMetric, &participateTemporal, &participateNoiseColour,
                &participateLoopEnd, &participateRoutes, &participateMixMemory, &participateConnections,
                &participateMaterial, &participateChaos, &participateMat, &participateLfoShape, &participateClock
            };
            // Perto do título de verdade, não da borda direita do
            // Label (20 ago. 2026, autor: "preciso que os botoes
            // fiquem proximos dos titulos e não afastados") -
            // `getRight()` media a largura TOTAL do componente Label
            // (geralmente bem mais largo que o texto visível, já que
            // várias colunas reservam espaço fixo), não onde o texto
            // realmente termina. Medindo a largura real do texto com a
            // própria fonte do label e encostando logo depois dele.
            // `toFront()` também (autor: "alguns botões não consegui
            // clicar") - construídos cedo no construtor, componentes
            // adicionados DEPOIS ficavam por cima na pilha de z-order e
            // roubavam o clique.
            // Gap por item - see ObjectFiveComponent's own copy of this
            // function for the full comment. Índice aqui: 0 steps, 1
            // voice/osc, 2 effects/fx, 3 detail/rotas ativas, 4 mixer,
            // 5 envelope/adsr, 6 modulation, 7 groove, 8 filter/vcf, 9
            // metric, 10 temporal/subdivisão, 11 noise colour, 12 loop
            // end, 13 routes/portas feedback, 14 mix memory, 15
            // connections/conexão entre objetos, 16 material/matéria,
            // 17 chaos/caos, 18 mat, 19 lfo shape/forma lfo, 20 clock.
            // 20 ago. 2026, autor: "conexão entre objetos, portas de
            // feedback e fim do loop (so um pouquinho)" - 12 loop end
            // +5, 13 routes/portas feedback +13, 15 connections +13.
            // autor, mesmo dia: "clock, MAT e noise são os mais
            // distantes... se pudesse priximar um pouco (3px a 5 px)"
            // - 11 noise colour, 18 mat, 20 clock: 17 -> 13.
            constexpr std::array<int, 21> participationGapPx {
                17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
                17, 13, 22, 30, 17, 30, 17, 17, 13, 17, 13
            };
            for (std::size_t i = 0; i < participationLabels.size(); ++i)
            {
                auto* label = participationLabels[i];
                auto* toggle = participationTogglesForLayout[i];
                const auto textWidth = juce::GlyphArrangement::getStringWidthInt(label->getFont(), label->getText());
                const auto textEndX = label->getJustificationType().testFlags(juce::Justification::horizontallyCentred)
                                           ? label->getX() + (label->getWidth() + textWidth) / 2
                                           : label->getX() + textWidth;
                toggle->setBounds(textEndX + participationGapPx[i], label->getY() + (label->getHeight() - 10) / 2, 10, 10);
                toggle->toFront(false);
            }
        }
    }

    void layoutSequence(juce::Rectangle<int> area)
    {
        auto transport = area.removeFromLeft(182);
        clockLabel.setBounds(transport.removeFromTop(24));
        clock.setBounds(transport.removeFromTop(152).reduced(22, 0));
        auto buttons = transport.removeFromTop(126).reduced(18, 0);
        run.setBounds(buttons.removeFromTop(28).reduced(0, 2));
        stop.setBounds(buttons.removeFromTop(28).reduced(0, 2));
        reset.setBounds(buttons.removeFromTop(28).reduced(0, 2));
        record.setBounds(buttons.removeFromTop(28).reduced(0, 2));
        auto recordDurationRow = buttons.removeFromTop(24);
        const auto recordDurationCellWidth2 = recordDurationRow.getWidth() / static_cast<int>(recordDurations.size());
        for (auto& button : recordDurations) button.setBounds(recordDurationRow.removeFromLeft(recordDurationCellWidth2).reduced(0, 2));
        temporalLabel.setBounds(transport.removeFromTop(20));
        // 2 rows of 4, not 1 row of 8 - same fix as the metric block below
        // (temporalButtons grew from 4 to 8, 20 ago. 2026).
        auto temporalArea = transport.removeFromTop(56);
        for (std::size_t i = 0; i < temporalButtons.size(); ++i)
            temporalButtons[i].setBounds(temporalArea.getX() + static_cast<int>(i % 4) * temporalArea.getWidth() / 4,
                                          temporalArea.getY() + static_cast<int>(i / 4) * temporalArea.getHeight() / 2,
                                          temporalArea.getWidth() / 4, temporalArea.getHeight() / 2);
        metricLabel.setBounds(transport.removeFromTop(20));
        // 2 rows of 4, not 1 row of 8 - same fix as the other legacy-path
        // metric block above (metricButtons grew from 4 to 8).
        auto metricArea = transport.removeFromTop(56);
        for (std::size_t i = 0; i < metricButtons.size(); ++i)
            metricButtons[i].setBounds(metricArea.getX() + static_cast<int>(i % 4) * metricArea.getWidth() / 4,
                                        metricArea.getY() + static_cast<int>(i / 4) * metricArea.getHeight() / 2,
                                        metricArea.getWidth() / 4, metricArea.getHeight() / 2);
        scannerLabel.setBounds(transport.removeFromTop(20));
        auto scannerArea = transport.removeFromTop(28);
        for (std::size_t i = 0; i < scannerButtons.size(); ++i)
            scannerButtons[i].setBounds(scannerArea.getX() + static_cast<int>(i) * scannerArea.getWidth() / 4, scannerArea.getY(), scannerArea.getWidth() / 4, scannerArea.getHeight());
        loopLabel.setBounds(transport.removeFromTop(22));
        auto loopArea = transport.removeFromTop(104);
        const auto loopWidth = loopArea.getWidth() / 4;
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
            loopSwitches[i].setBounds(loopArea.getX() + static_cast<int>(i % 4) * loopWidth,
                                      loopArea.getY() + static_cast<int>(i / 4) * 26, loopWidth, 26);
        connectionLabel.setBounds(transport.removeFromTop(22));
        auto routes = transport.removeFromTop(96);
        const auto routeWidth = routes.getWidth() / 2;
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
            connectionSwitches[i].setBounds(routes.getX() + static_cast<int>(i % 2) * routeWidth,
                                             routes.getY() + static_cast<int>(i / 2) * 28, routeWidth, 28);
        feedbackLabel.setBounds(transport.removeFromTop(19));
        feedbackGain.setBounds(transport.removeFromTop(27).reduced(5, 2));
        deriveLabel.setBounds(transport.removeFromTop(19));
        deriveDepth.setBounds(transport.removeFromTop(27).reduced(5, 2));

        auto stepArea = area.reduced(8, 0);
        const auto stepWidth = std::clamp(stepArea.getWidth() / static_cast<int>(antitotem::SimpleSequencer::historicalScannerSteps), 100, 200);
        const auto stepHeight = stepArea.getHeight() / 2;
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            const auto bounds = juce::Rectangle<int>(stepArea.getX() + static_cast<int>(i % antitotem::SimpleSequencer::historicalScannerSteps) * stepWidth,
                                                      stepArea.getY() + static_cast<int>(i / antitotem::SimpleSequencer::historicalScannerSteps) * stepHeight,
                                                      stepWidth, stepHeight).reduced(6, 5);
            steps[i].setBounds(bounds);
        }
    }
    static void configureLabel(juce::Label& label, const juce::String& text, float size, juce::Colour colour)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(uiFont(size));
        label.setColour(juce::Label::textColourId, colour); label.setJustificationType(juce::Justification::centredLeft);
    }
    void syncCV() { for (std::size_t i = 0; i < steps.size(); ++i) sequencer.setStepVoltage(i, static_cast<float>(steps[i].cv.getValue())); }
    void syncStepDynamics()
    {
        for (std::size_t i = 0; i < steps.size(); ++i)
        {
            sequencer.setStepLevel(i, static_cast<float>(steps[i].level.getValue()));
            sequencer.setStepEffectSend(i, static_cast<float>(steps[i].send.getValue()));
            sequencer.setStepMuted(i, steps[i].mute.getToggleState());
        }
    }
    // A "mix"-type control at 0 means that module contributes nothing to
    // the sound at all - OSC4/OSC5 start this way, along with REVERB/
    // PHASER/FLANGER/RES MIX/RING/NOISE. Highlighting the thumb marks it as
    // "turn this to hear anything from here", instead of a control that
    // looks identical whether it does something or nothing. Reverts to the
    // LookAndFeel's own default the moment it is raised above 0.
    static void updateSilentHighlight(juce::Slider& slider, juce::Colour normalColour)
    {
        slider.setColour(juce::Slider::thumbColourId, slider.getValue() <= 0.0005 ? material::board.brighter(0.35f) : normalColour);
    }
    // Rotary knobs (the oscillator MIX knobs) never had an explicit colour
    // of their own - they use the LookAndFeel's default. removeColour()
    // reverts to exactly that once raised above 0, instead of needing a
    // "normal" colour to restore.
    static void updateSilentHighlightDefault(juce::Slider& slider)
    {
        if (slider.getValue() <= 0.0005) slider.setColour(juce::Slider::thumbColourId, material::board.brighter(0.35f));
        else slider.removeColour(juce::Slider::thumbColourId);
    }
    // DERIVA and the VARIAÇÃO presets move these same controls with
    // dontSendNotification (so the drift doesn't re-trigger the engine sync
    // on every micro-step), which means onValueChange never fires and the
    // highlight goes stale. Call this after any such batch update.
    void refreshSilentHighlights()
    {
        for (auto& oscillator : oscillators) updateSilentHighlightDefault(oscillator);
        updateSilentHighlight(modulationControls[1], material::controlBlue);
        updateSilentHighlight(modulationControls[2], material::noiseSend);
        updateSilentHighlight(effectControls[0], material::phaser);
        updateSilentHighlight(effectControls[1], material::phaser);
        updateSilentHighlight(effectControls[2], material::phaser);
        updateSilentHighlight(detailControls[6], material::memory);
    }
    void syncOscillators() { for (std::size_t i = 0; i < oscillators.size(); ++i) sequencer.setOscillatorLevel(i, static_cast<float>(oscillators[i].getValue())); }
    void syncRatios() { for (std::size_t i = 0; i < oscillatorRates.size(); ++i) sequencer.setOscillatorRatio(i, static_cast<float>(oscillatorRates[i].getValue())); }
    void syncShapes() { for (std::size_t i = 0; i < shapes.size(); ++i) sequencer.setOscillatorShape(i, static_cast<float>(shapes[i].getValue())); }
    void syncPans() { for (std::size_t i = 0; i < oscillatorPans.size(); ++i) sequencer.setOscillatorPan(i, static_cast<float>(oscillatorPans[i].getValue())); }
    void syncProximities() { for (std::size_t i = 0; i < oscillatorProximities.size(); ++i) sequencer.setOscillatorProximity(i, static_cast<float>(oscillatorProximities[i].getValue())); }
    void syncOrbits() { for (std::size_t i = 0; i < oscillatorOrbits.size(); ++i) sequencer.setOscillatorOrbit(i, static_cast<float>(oscillatorOrbits[i].getValue())); }
    void appendLog(const juce::String& entry)
    {
        if (! loggingEnabled) return;
        auto text = log.getText();
        if (! text.isEmpty()) text += "\n";
        text += entry;
        constexpr int maximumCharacters = 1500;
        if (text.length() > maximumCharacters) text = text.substring(text.length() - maximumCharacters);
        log.setText(text, false);
        log.moveCaretToEnd();
    }
    void syncFilter()
    {
        sequencer.setFilterCutoff(static_cast<float>(filterCutoff.getValue()));
        sequencer.setFilterResonance(static_cast<float>(filterResonance.getValue()));
        sequencer.setFilterCvDepth(static_cast<float>(filterDepth.getValue()));
        // Mask recomputed from all four independent toggles every time -
        // any combination of LPF/BPF/HPF/NCH is valid (17 ago. 2026,
        // multi-select mode mask, see CmosVcf.h's own comment).
        constexpr std::array<unsigned char, 4> filterModeBits {
            antitotem::CmosVcf::Mode::lowpass, antitotem::CmosVcf::Mode::bandpass,
            antitotem::CmosVcf::Mode::highpass, antitotem::CmosVcf::Mode::notch };
        unsigned char mask = 0;
        for (std::size_t i = 0; i < filterModeButtons.size(); ++i)
            if (filterModeButtons[i].getToggleState()) mask |= filterModeBits[i];
        sequencer.setFilterModeMask(mask);
    }
    void syncEnvelope()
    {
        sequencer.setEnvelopeAttack(static_cast<float>(envelopeControls[0].getValue()));
        sequencer.setEnvelopeDecay(static_cast<float>(envelopeControls[1].getValue()));
        sequencer.setEnvelopeSustain(static_cast<float>(envelopeControls[2].getValue()));
        sequencer.setEnvelopeRelease(static_cast<float>(envelopeControls[3].getValue()));
    }
    void syncModulation()
    {
        sequencer.setLfoRate(0.02f * std::pow(2.0f, static_cast<float>(modulationControls[0].getValue()) * 16.0f));
        sequencer.setRingMix(static_cast<float>(modulationControls[1].getValue()));
        sequencer.setNoiseMix(static_cast<float>(modulationControls[2].getValue()));
    }
    void syncLfoShape()
    {
        using Shape = antitotem::LfoSource::Shape;
        constexpr std::array<Shape, 6> lfoShapes { Shape::sine, Shape::triangle, Shape::square, Shape::chaos, Shape::wander, Shape::step };
        sequencer.setLfoShape(lfoShapes[static_cast<std::size_t>(std::clamp(lfoShapeSelection, 0, 5))]);
        // FREEZE only ever acts on CAOS/VAGA (17 ago. 2026, author, live:
        // "não vejo sentido do botão freeze funcionar quando os botões
        // caos e vaga estão desligados") - disabled rather than just
        // silently inert whenever neither is the selected shape, so a
        // click that would do nothing simply isn't clickable.
        const auto lfoIsChaosOrWander = lfoShapeSelection == 3 || lfoShapeSelection == 4;
        lfoFreeze.setEnabled(lfoIsChaosOrWander);
        // Bug found live (18 ago. 2026, author: "botão freeze tá meio
        // bugado, destaca os botões caos e vaga mesmo quando a forma lfo
        // selecionada é o SEN"): disabling FRZ alone left its toggle
        // state (and the CAOS/VAGA ring it drives) exactly as it was -
        // if the author had frozen CHAOS then switched to SEN, the ring
        // stayed lit on a control that could no longer be un-toggled,
        // reading as stuck/broken. Leaving CAOS/VAGA now force-releases
        // FREEZE for real, not just visually - a held gesture shouldn't
        // keep acting once its own control goes out of reach.
        if (!lfoIsChaosOrWander && lfoFreeze.getToggleState())
        {
            lfoFreeze.setToggleState(false, juce::dontSendNotification);
            sequencer.setLfoFrozen(false);
            patchToggleLook().lfoFrozen = false;
            lfoShapeButtons[3].repaint(); lfoShapeButtons[4].repaint();
        }
    }
    void syncEffects()
    {
        sequencer.setReverbMix(static_cast<float>(effectControls[0].getValue()));
        sequencer.setPhaserMix(static_cast<float>(effectControls[1].getValue()));
        sequencer.setFlangerMix(static_cast<float>(effectControls[2].getValue()));
    }
    void syncDetails()
    {
        const auto v = [this] (std::size_t index) { return static_cast<float>(detailControls[index].getValue()); };
        sequencer.setSampleHoldRate(0.4f * std::pow(2.0f, v(0) * 10.0f));
        sequencer.setReverbFeedback(v(1) * 0.8f);
        sequencer.setPhaserRate(0.04f * std::pow(2.0f, v(2) * 8.0f)); sequencer.setPhaserDepth(v(3));
        sequencer.setFlangerRate(0.04f * std::pow(2.0f, v(4) * 7.0f)); sequencer.setFlangerDepth(v(5));
        sequencer.setResonatorMix(v(6)); sequencer.setResonatorPitch(v(7)); sequencer.setResonatorDamping(v(8));
        sequencer.setMaterialFilterCutoff(v(9)); sequencer.setMaterialFilterResonance(v(10));
        sequencer.setMaterialFilterDrive(v(11)); sequencer.setMaterialFilterAsymmetry(v(12));
        sequencer.setLfoChaosDrive(v(13)); sequencer.setLfoChaosDamping(v(14)); sequencer.setLfoWanderDepth(v(15));
    }
    void syncMixer()
    {
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            auto channel = sequencer.getMixChannel(i);
            channel.gain = static_cast<float>(mixGain[i].getValue());
            channel.pan = static_cast<float>(mixPan[i].getValue());
            channel.reflux = static_cast<float>(mixReflux[i].getValue());
            channel.enabled = mixEnable[i].getToggleState();
            channel.mute = mixMute[i].getToggleState();
            channel.solo = mixSolo[i].getToggleState();
            sequencer.setMixChannel(i, channel);
        }
    }
    void syncObjectMix()
    {
        dualEngine.setObjectChannel(0, { static_cast<float>(principalVolume.getValue()), principalMute.getToggleState() });
        dualEngine.setObjectChannel(1, { static_cast<float>(cloneVolume.getValue()), cloneMute.getToggleState() });
    }
    // Inverse of syncMixer(): after a memory recall changes the engine's
    // channel state directly, the sliders/toggles must be pulled back into
    // sync or they would keep showing the pre-recall values.
    void pullMixerFromEngine()
    {
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            const auto channel = sequencer.getMixChannel(i);
            mixGain[i].setValue(channel.gain, juce::dontSendNotification);
            mixPan[i].setValue(channel.pan, juce::dontSendNotification);
            mixReflux[i].setValue(channel.reflux, juce::dontSendNotification);
            mixEnable[i].setToggleState(channel.enabled, juce::dontSendNotification);
            mixMute[i].setToggleState(channel.mute, juce::dontSendNotification);
            mixSolo[i].setToggleState(channel.solo, juce::dontSendNotification);
        }
    }
    // TUTORIAL/SOBRE's own language buttons call this too, so switching
    // language inside either window keeps the header button and the other
    // window in sync instead of drifting apart.
    void setUiLanguage(antitotem::ui::Language language)
    {
        uiLanguage = language;
        languageSwitch.setButtonText(antitotem::ui::languageLabel(uiLanguage));
        title.setText(antitotem::ui::text(antitotem::ui::mainTitle, uiLanguage), juce::dontSendNotification);
        if (tutorialWindow != nullptr) tutorialWindow->setLanguage(uiLanguage);
        if (appInfoWindow != nullptr) appInfoWindow->setLanguage(uiLanguage);
        // PRINCIPAL's own steps/noiseSelector follow live; CLONE's (clonePanel/
        // objectFiveWindow) are reconstructed fresh with the current uiLanguage
        // whenever their body is (re)built (see setShowingCloneBody() and
        // ObjectFiveWindow::setLanguage() below) rather than tracked live -
        // most of CLONE's own ~40 individual tooltips are still only correct
        // as of whenever that body was last (re)built, not fully reactive to
        // a language switch while already open. A known, documented scope
        // cut (see docs/TAREFAS.md) - PRINCIPAL is the primary, always-visible
        // surface and the one actually reactive here.
        noiseSelector.setLanguage(uiLanguage);
        for (auto& step : steps) step.setLanguage(uiLanguage);
        if (clonePanel != nullptr) clonePanel->setLanguage(uiLanguage);
        if (objectFiveWindow != nullptr) objectFiveWindow->setLanguage(uiLanguage);
        refreshLanguageTexts();
        if (auto* settings = applicationProperties.getUserSettings())
            settings->setValue("uiLanguage", antitotem::ui::languageCode(uiLanguage));
    }
    // Every construction-time configureLabel()/setButtonText()/setTooltip()
    // call in this class used uiLanguage, but only at the moment it ran -
    // switching language later never touched any of them again (author,
    // live, 15 ago. 2026: "ao clicar no botão idioma, ainda não alteram os
    // idiomas dos botoes"). Re-runs each one here instead - every line
    // below is the exact same call already made once during construction,
    // safe to repeat verbatim since setting a Label/Button's text or
    // tooltip has no side effect beyond the text itself. `flow` and
    // `appendLog()` entries are deliberately excluded: those already read
    // uiLanguage fresh at the moment they fire (REC/DERIVA events), so
    // they show the right language the next time they fire naturally,
    // with nothing to refresh here. Knob/slider .setValue() calls that
    // originally shared a line with a .setTooltip() are also excluded -
    // only the tooltip half is repeated, never the value.
    void refreshLanguageTexts()
    {
        configureLabel(footer, antitotem::ui::text(antitotem::ui::label::footerCredit, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
        configureLabel(modeLabel, antitotem::ui::text(antitotem::ui::label::modePrincipal, uiLanguage), 11.0f, juce::Colour(0xffffca5c));
        clock.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockRateKnob, uiLanguage));
        loopLabel.setText(antitotem::ui::text(antitotem::ui::label::loopEndPrefix, uiLanguage) + juce::String(static_cast<int>(sequencer.getLoopEnd())) + antitotem::ui::text(antitotem::ui::label::loopEndSuffix, uiLanguage), juce::dontSendNotification);
        configureLabel(connectionLabel, antitotem::ui::text(antitotem::ui::label::feedbackPorts, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        connectionLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackHeaderTip, uiLanguage));
        configureLabel(energyLabel, antitotem::ui::text(antitotem::ui::label::energy, uiLanguage), 9.0f, juce::Colour(0xffded4be));
        energyLabel.setJustificationType(juce::Justification::centred);
        energy.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::energy, uiLanguage));
        configureLabel(objectMixLabel, antitotem::ui::text(antitotem::ui::label::objectMix, uiLanguage), 12.0f, juce::Colour(0xffded4be));
        objectMixLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixHeaderTip, uiLanguage));
        principalVolume.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixPrincipal, uiLanguage));
        cloneVolume.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixClone, uiLanguage));
        // M buttons reuse each own slider's tooltip too (author, live:
        // "os botoes m dos instumentos object mixer ainda não constam no
        // learn") - none of the three had one at all until now.
        principalMute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixPrincipal, uiLanguage));
        cloneMute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectMixClone, uiLanguage));
        // Missed in the original wiring (found auditing, 20 ago. 2026,
        // "prossiga com inteligencia e perspicacia") - excitationLabel/
        // excitationAmount's tooltip was only ever set once in the
        // constructor, unlike every other tooltip on this row, so it
        // would have stayed in whatever language was active at launch
        // instead of updating on a live language switch like PRINC/CLONE
        // already do just above.
        excitationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::excitationAmount, uiLanguage));
        excitationAmount.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::excitationAmount, uiLanguage));
        excitationMute.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::excitationAmount, uiLanguage));
        configureLabel(recordingLabel, antitotem::ui::text(antitotem::ui::label::recControls, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        configureLabel(modulationLabel, antitotem::ui::text(antitotem::ui::label::modulation, uiLanguage), 14.0f, juce::Colour(0xffffca5c));
        modulationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::modulationHeaderTip, uiLanguage));
        // Not the shared section-title gold anymore - see the matching comment
        // in ObjectFiveComponent's own constructor.
        configureLabel(effectsLabel, antitotem::ui::text(antitotem::ui::label::spacePhase, uiLanguage), 10.0f, material::phaser);
        effectsLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::effectsHeaderTip, uiLanguage));
        configureLabel(detailLabel, antitotem::ui::text(antitotem::ui::label::activeRoutes, uiLanguage), 10.0f, material::memory);
        detailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailHeaderTip, uiLanguage));
        configureLabel(parametersLabel, antitotem::ui::text(antitotem::ui::label::parametersRail, uiLanguage), 14.0f, juce::Colour(0xffffca5c));
        parametersLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::parametersHeaderTip, uiLanguage));
        configureLabel(materialRailLabel, antitotem::ui::text(antitotem::ui::label::materialRail, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        materialRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialHeaderTip, uiLanguage));
        configureLabel(chaosRailLabel, antitotem::ui::text(antitotem::ui::label::chaosRail, uiLanguage), 10.0f, material::clock);
        chaosRailLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::chaosHeaderTip, uiLanguage));
        configureLabel(filterLabel, antitotem::ui::text(antitotem::ui::label::vcfMultimode, uiLanguage), 14.0f, juce::Colour(0xffffca5c));
        filterLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::vcfHeaderTip, uiLanguage));
        configureLabel(voiceLabel, antitotem::ui::text(antitotem::ui::label::oscHeaderTitle, uiLanguage), 15.0f, juce::Colour(0xffffca5c));
        voiceLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::voiceHeaderTip, uiLanguage));
        configureLabel(logLabel, antitotem::ui::text(antitotem::ui::logText::title, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        log.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::activityLog, uiLanguage));
        configureLabel(objectConnectionLabel, antitotem::ui::text(antitotem::ui::label::objectConnection, uiLanguage), 13.0f, material::returnPath);
        objectConnectionLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectConnectionHeaderTip, uiLanguage));
        configureLabel(routesToFifthLabel, antitotem::ui::text(antitotem::ui::label::routeToClone, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        configureLabel(routesToFirstLabel, antitotem::ui::text(antitotem::ui::label::routeToPrincipal, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        soundPage.setButtonText(antitotem::ui::text(antitotem::ui::button::soundPage, uiLanguage));
        sequencePage.setButtonText(antitotem::ui::text(antitotem::ui::button::sequencePage, uiLanguage));
        configureLabel(variationLabel, antitotem::ui::text(antitotem::ui::label::variation, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        variationLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeaderTip, uiLanguage));
        pulseVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPulse, uiLanguage));
        pulseVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPulse, uiLanguage));
        porousVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPorous, uiLanguage));
        porousVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPorous, uiLanguage));
        heterodyneVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationHeterodyne, uiLanguage));
        heterodyneVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationHeterodyne, uiLanguage));
        randomizeStepsButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::randomizeSteps, uiLanguage));
        orbitVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationOrbit, uiLanguage));
        orbitVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationOrbit, uiLanguage));
        pendulumVariation.setButtonText(antitotem::ui::text(antitotem::ui::button::variationPendulum, uiLanguage));
        pendulumVariation.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::variationPendulum, uiLanguage));
        deriveButton.setButtonText(antitotem::ui::text(antitotem::ui::label::drift, uiLanguage));
        deriveButton.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveButton, uiLanguage));
        // setButtonText() alone shouldn't need this, but the author saw
        // DERIVA's accent colour/border genuinely disappear after a
        // language switch (live, 15 ago. 2026: "o botão deriva perdeu a
        // configuração de fonte e cor") - reasserting the LookAndFeel
        // pointer defensively, since it's idempotent and cheap either way.
        deriveButton.setLookAndFeel(&patchToggleLook());
        tutorial.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::openTutorial, uiLanguage));
        about.setButtonText(antitotem::ui::text(antitotem::ui::button::about, uiLanguage));
        about.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::openAbout, uiLanguage));
        // Re-explains whatever is currently under the mouse in the new
        // language, or falls back to the idle text - the alternative
        // (leaving stale-language text on screen) reads as a bug the next
        // time the box is looked at.
        if (auto* under = getComponentAt(getMouseXYRelative()))
            explainHovered(under);
        else
            learnEditor.setText(antitotem::ui::text(antitotem::ui::tooltip::learnPanelIdle, uiLanguage), false);
        languageSwitch.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::languageSwitch, uiLanguage));
        monitorModeToggle.setButtonText(dualMonitorMode ? antitotem::ui::text(antitotem::ui::button::twoMonitors, uiLanguage) : antitotem::ui::text(antitotem::ui::button::oneMonitor, uiLanguage));
        monitorModeToggle.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::monitorModeToggle, uiLanguage));
        objectFive.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::cloneToggle, uiLanguage));
        for (auto& button : loopSwitches) button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::loopEnd, uiLanguage));
        configureLabel(temporalLabel, antitotem::ui::text(antitotem::ui::label::pulse, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        temporalLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::temporalHeaderTip, uiLanguage));
        configureLabel(grooveLabel, antitotem::ui::text(antitotem::ui::label::groove, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        grooveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, uiLanguage));
        grooveAmount.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::grooveAmount, uiLanguage));
        configureLabel(metricLabel, antitotem::ui::text(antitotem::ui::label::meter, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        metricLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricHeaderTip, uiLanguage));
        configureLabel(scannerLabel, antitotem::ui::text(antitotem::ui::label::path, uiLanguage), 10.0f, juce::Colour(0xffded4be));
        scannerLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerHeaderTip, uiLanguage));
        for (std::size_t i = 0; i < temporalButtons.size(); ++i) temporalButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::clockFeelTips[i], uiLanguage));
        for (auto& button : metricButtons) button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::metricButton, uiLanguage));
        for (std::size_t i = 0; i < scannerButtons.size(); ++i) scannerButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scannerTips[i], uiLanguage));
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
            connectionSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackDoorTips[i], uiLanguage));
        feedbackGain.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::feedbackGain, uiLanguage));
        configureLabel(deriveLabel, antitotem::ui::text(antitotem::ui::label::driftDepthLabel, uiLanguage), 10.0f, material::clock);
        deriveLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::deriveHeaderTip, uiLanguage));
        deriveDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::driftDepthPhrase, uiLanguage));
        modulationControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfo, uiLanguage));
        modulationControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::ring, uiLanguage));
        modulationControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::noiseMod, uiLanguage));
        effectControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::reverb, uiLanguage));
        effectControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::phaser, uiLanguage));
        effectControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::flanger, uiLanguage));
        configureLabel(lfoShapeLabel, antitotem::ui::text(antitotem::ui::label::lfoShape, uiLanguage), 12.0f, material::clock);
        for (std::size_t i = 0; i < lfoShapeButtons.size(); ++i) lfoShapeButtons[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoShapeTips[i], uiLanguage));
        lfoFreeze.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::lfoFreeze, uiLanguage));
        for (std::size_t i = 0; i < mixGain.size(); ++i)
        {
            mixGain[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelGain, uiLanguage));
            mixPan[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelPan, uiLanguage));
            mixReflux[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::channelReturn, uiLanguage));
        }
        configureLabel(mixMemoryLabel, antitotem::ui::text(antitotem::ui::label::mixMemory, uiLanguage), 10.0f, juce::Colour(0xff8f856f));
        mixMemoryLabel.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixMemoryHeaderTip, uiLanguage));
        mixMemoryCapture.setButtonText(antitotem::ui::text(antitotem::ui::button::capture, uiLanguage));
        mixMemoryCapture.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixCapture, uiLanguage));
        for (auto& slot : mixMemorySlots) slot.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::mixSlotRecall, uiLanguage));
        scopeGain.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scopeGain, uiLanguage));
        stereoScope.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::scopeTrace, uiLanguage));
        for (std::size_t i = 0; i < coreSwitches.size(); ++i) coreSwitches[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::coreTips[i], uiLanguage));
        filterCutoff.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterCutoff, uiLanguage));
        filterResonance.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterResonance, uiLanguage));
        filterDepth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterCvDepth, uiLanguage));
        for (auto& button : filterModeButtons) button.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::filterMode, uiLanguage));
        materialFilterMix.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::materialFilterMix, uiLanguage));
        envelopeControls[0].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeAttack, uiLanguage));
        envelopeControls[1].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeDecay, uiLanguage));
        envelopeControls[2].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeSustain, uiLanguage));
        envelopeControls[3].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::envelopeRelease, uiLanguage));
        for (std::size_t i = 0; i < oscillatorRates.size(); ++i)
        {
            oscillatorRates[i].setTooltip(i == 3 ? antitotem::ui::text(antitotem::ui::tooltip::osc4Freq, uiLanguage)
                                         : i == 4 ? antitotem::ui::text(antitotem::ui::tooltip::osc5Freq, uiLanguage)
                                                   : antitotem::ui::text(antitotem::ui::tooltip::oscFreqGeneric, uiLanguage));
            oscillators[i].setTooltip(i == 4 ? antitotem::ui::text(antitotem::ui::tooltip::mixRingProduct, uiLanguage)
                                              : antitotem::ui::text(antitotem::ui::tooltip::mixGeneric, uiLanguage));
            configureLabel(oscillatorShapeLabels[i], antitotem::ui::text(antitotem::ui::label::shape, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            // See ObjectFiveComponent's own copy of this same fix for
            // the full comment (configureLabel() resets justification
            // to centredLeft every call - must be re-applied here too).
            oscillatorShapeLabels[i].setJustificationType(juce::Justification::centred);
            oscillatorPans[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisXGeneric, uiLanguage));
            configureLabel(oscillatorPanCaptions[i], antitotem::ui::text(antitotem::ui::label::axisX, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorPanCaptions[i].setJustificationType(juce::Justification::centred);
            oscillatorProximities[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisYGeneric, uiLanguage));
            configureLabel(oscillatorProximityCaptions[i], antitotem::ui::text(antitotem::ui::label::axisY, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorProximityCaptions[i].setJustificationType(juce::Justification::centred);
            oscillatorOrbits[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::axisZGeneric, uiLanguage));
            configureLabel(oscillatorOrbitCaptions[i], antitotem::ui::text(antitotem::ui::label::axisZ, uiLanguage), 9.0f, juce::Colour(0xff8f856f));
            oscillatorOrbitCaptions[i].setJustificationType(juce::Justification::centred);
        }
        constexpr std::array<double, 4> recordDurationSecondsRefresh { 60.0, 120.0, 180.0, 300.0 };
        for (std::size_t i = 0; i < recordDurations.size(); ++i)
            recordDurations[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::recordDurationPrefix, uiLanguage)
                + juce::String(static_cast<int>(recordDurationSecondsRefresh[i] / 60.0))
                + antitotem::ui::text(antitotem::ui::tooltip::recordDurationSuffix, uiLanguage));
        const std::array<juce::String, 4> mixerNamesRefresh {
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[0], uiLanguage),
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[1], uiLanguage),
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[2], uiLanguage),
            antitotem::ui::text(antitotem::ui::label::mixerChannelNames[3], uiLanguage)
        };
        for (std::size_t i = 0; i < mixLabels.size(); ++i) configureLabel(mixLabels[i], mixerNamesRefresh[i], 11.0f, material::metal);
        const std::array<juce::String, 4> objectRouteNamesRefresh {
            antitotem::ui::text(antitotem::ui::button::routeDirect, uiLanguage),
            antitotem::ui::text(antitotem::ui::button::routeDiode, uiLanguage),
            "CAP",
            antitotem::ui::text(antitotem::ui::button::routePulse, uiLanguage)
        };
        for (std::size_t i = 0; i < 4; ++i)
        {
            routesToFifth[i].setButtonText(objectRouteNamesRefresh[i]);
            routesToFirst[i].setButtonText(objectRouteNamesRefresh[i]);
            routesToFifth[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectRouteTips[i], uiLanguage));
            routesToFirst[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::objectRouteTips[i], uiLanguage));
        }
        gainToFifth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::gainToClone, uiLanguage));
        gainToFirst.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::gainToPrincipal, uiLanguage));
        auxToFirst.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::auxToPrincipal, uiLanguage));
        auxToFifth.setTooltip(antitotem::ui::text(antitotem::ui::tooltip::auxToCloneObject, uiLanguage));
        const std::array<juce::String, 16> detailNamesRefresh { "S&H RATE", "RVB RET", "PHS RATE", "PHS PROF", "FLG RATE", "FLG PROF",
            "RES MIX", antitotem::ui::text(antitotem::ui::label::resPitch, uiLanguage), antitotem::ui::text(antitotem::ui::label::resBody, uiLanguage),
            "CUTOFF", "RESON", "DRIVE", "ASYM",
            "DRIVE", "DAMPING", "DEPTH" };
        for (std::size_t i = 0; i < detailControlLabels.size(); ++i)
        {
            configureLabel(detailControlLabels[i], detailNamesRefresh[i], 9.0f,
                            i >= 9 && i <= 12 ? juce::Colour(0xff8f856f) : (i >= 13 ? material::clock : material::memory));
            detailControls[i].setTooltip(antitotem::ui::text(antitotem::ui::tooltip::detailControlTips[i], uiLanguage));
        }
    }

    // Base RASGO policy (docs/../RASGO_DOCUMENTATION/design/INTERFACES_E_LAYOUTS.md
    // §5.1) is single-monitor only, centred within one screen, never the
    // combined virtual desktop rectangle. dualMonitorMode is a deliberate,
    // documented divergence from that (docs/JANELA_UNICA_E_MONITORES.md):
    // when on and a second display genuinely exists, CLONE fills that
    // display's own usable area instead - never a scaled copy of the
    // first monitor's bounds.
    // Returns the usable area of "the other monitor" if a second one
    // genuinely appears to exist, or an empty optional for a real single-
    // monitor rig. Two paths, because juce::Desktop::getDisplays() isn't
    // trustworthy alone here: some multi-monitor X11 setups report each
    // physical screen as its own Displays::Display (the clean case, used
    // directly), but others collapse everything into one Display whose
    // userBounds spans every screen combined (already documented in
    // MainWindow's own placement code above) - confirmed happening on
    // this exact rig via a temporary DBG dump (one Display, userBounds
    // 0,0,3288,1038, exactly VGA-0 + HDMI-0 added together). For that
    // case, split the combined region in half along its longer axis and
    // return whichever half doesn't contain this window - not exact
    // monitor geometry, but the best available signal without dropping
    // to raw X11/RandR calls JUCE's own abstraction exists to avoid.
    // Existence check only (no "which half" decision), safe to call from
    // the constructor before this component has real screen bounds -
    // secondMonitorArea() below needs getScreenBounds() and so can only
    // be trusted once actually on screen.
    [[nodiscard]] static bool hasSecondMonitor()
    {
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        if (displays.displays.size() > 1) return true;
        if (displays.displays.isEmpty()) return false;
        const auto combined = displays.displays.getReference(0).userBounds;
        return combined.getWidth() >= combined.getHeight() * 2.5f;
    }
    [[nodiscard]] std::optional<juce::Rectangle<int>> secondMonitorArea() const
    {
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        if (displays.displays.size() > 1)
        {
            const auto* mainDisplay = displays.getDisplayForRect(getScreenBounds());
            for (auto& candidate : displays.displays)
                if (mainDisplay == nullptr || &candidate != mainDisplay)
                    return candidate.userBounds.toNearestInt();
            return std::nullopt;
        }
        if (displays.displays.isEmpty()) return std::nullopt;
        const auto combined = displays.displays.getReference(0).userBounds.toNearestInt();
        // A genuine single monitor reads close to a normal aspect ratio;
        // two side by side reads distinctly panoramic. 2.5 sits well
        // above even a 21:9 ultrawide (~2.4) but below two 4:3/16:9
        // screens joined (typically >3).
        if (combined.getWidth() < combined.getHeight() * 2.5) return std::nullopt;
        const auto mainBounds = getScreenBounds();
        const auto midX = combined.getX() + combined.getWidth() / 2;
        const auto leftHalf = combined.withWidth(combined.getWidth() / 2);
        const auto rightHalf = combined.withX(midX).withWidth(combined.getRight() - midX);
        return (mainBounds.getCentreX() < midX) ? rightHalf : leftHalf;
    }
    void positionObjectFiveWindow()
    {
        if (objectFiveWindow == nullptr) return;
        if (dualMonitorMode)
        {
            if (const auto area = secondMonitorArea())
            {
                // 88%, not 100%: same margin the base policy asks for
                // (85-90%) so the window never claims the taskbar/dock
                // strip even on a monitor with none of its own.
                const auto width = juce::roundToInt(area->getWidth() * 0.88f);
                const auto height = juce::roundToInt(area->getHeight() * 0.88f);
                objectFiveWindow->setBounds(area->withSizeKeepingCentre(width, height));
                return;
            }
        }
        // Single-monitor default: unchanged from before this existed.
        objectFiveWindow->centreWithSize(1860, 950);
    }

    // Single-window mode: CLONE toggles into the same body area PRINCIPAL's
    // own modules occupy, instead of opening ObjectFiveWindow (that only
    // happens when dualMonitorMode is actually active - see objectFive's
    // onClick). Geometry is identical either way (both get `area`, the
    // exact rectangle layoutUnified() itself receives) - only which tree
    // is visible changes. Hides every child of this component except the
    // header/LOG/CONEXÃO ENTRE OBJETOS ones (single-instance in the
    // engine, so they stay visible and meaningful regardless of which
    // body is showing) and clonePanel itself.
    void setShowingCloneBody(bool show)
    {
        showingCloneBody = show;
        if (clonePanel == nullptr)
        {
            clonePanel = std::make_unique<ObjectFiveComponent>(dualEngine, true, uiLanguage);
            // LEARN: clonePanel's own hover/focus feeds this same window's
            // single shared box, exactly like PRINCIPAL's own controls do -
            // see ObjectFiveComponent's own onExplain member comment.
            *clonePanel->explainCallback() = [this] (const juce::String& text) { learnEditor.setText(text, false); };
            addAndMakeVisible(*clonePanel);
            // Real bug found live: created last, clonePanel sat in front
            // of every other child in z-order - including CONEXÃO ENTRE
            // OBJETOS and LOG, both `alwaysVisibleInBody`. They still
            // rendered fine (clonePanel paints nothing of its own in that
            // part of its bounds, see embedded's paint() override), but
            // clonePanel's own empty region there was still on top for
            // hit-testing, silently swallowing clicks meant for those
            // controls whenever CLONE was showing (author, live: "o
            // objeto conexão entre objetos não funciona na aba clone...
            // idem para o log"). clonePanel's own interactive widgets
            // (CLOCK, oscillators, VCF/ADSR, CLONE's own 4-channel mixer,
            // rails, steps) never overlap CONEXÃO/LOG's screen position,
            // so sending it to the back is safe - it only stops
            // shadowing space it was never drawing into anyway.
            clonePanel->toBack();
        }
        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            auto* child = getChildComponent(i);
            if (child == clonePanel.get()) continue;
            if (std::find(alwaysVisibleInBody.begin(), alwaysVisibleInBody.end(), child) != alwaysVisibleInBody.end())
                continue;
            child->setVisible(! show);
        }
        clonePanel->setVisible(show);
        objectFive.setButtonText(show ? utf8("PRINCIPAL") : utf8("CLONE"));
        resized();
        // Forces clonePanel to relayout every time it's shown, even when
        // `area` below happens to be numerically identical to the last
        // time resized() ran - setBounds() only re-triggers a child's own
        // resized() when the rectangle actually changes, so a plain
        // setBounds(area) here can silently leave clonePanel's own
        // internal layout (oscillator FORM captions and others) stuck on
        // whatever it last computed, however stale (author, live,
        // reported more than once: "titulos dos knobs forma" desalinhados
        // / "novamente FORM desalinhado no knob do oscilador"). Cheap and
        // idempotent - recomputing the same layout twice is harmless.
        if (show) clonePanel->resized();
        // The frame's own colour depends on showingCloneBody (see paint())
        // - setVisible() on the children doesn't invalidate that outer
        // border on its own.
        repaint();
    }

    void syncTemporal()
    {
        using Feel = antitotem::SimpleSequencer::ClockFeel;
        // 8 feels - see the matching array-size comment in
        // ObjectFiveComponent's own updateTemporal() for the full context.
        constexpr std::array<Feel, 8> feels { Feel::straight, Feel::triplet, Feel::quintuplet, Feel::swing,
                                               Feel::septuplet, Feel::nonuplet, Feel::undecuplet, Feel::glitch };
        // Plain beat counts, not fractions - see the matching comment in
        // ObjectFiveComponent's own updateTemporal() for the full context.
        constexpr std::array<unsigned int, 8> beats { 2, 3, 4, 5, 6, 7, 8, 9 };
        constexpr std::array<unsigned int, 8> units { 4, 4, 4, 4, 4, 4, 4, 4 };
        sequencer.setClockFeel(feels[static_cast<std::size_t>(std::clamp(temporalSelection, 0, 7))]);
        const auto metric = static_cast<std::size_t>(std::clamp(metricSelection, 0, 7));
        sequencer.setMetric(beats[metric], units[metric]);
    }
    void syncScanner()
    {
        using Direction = antitotem::SimpleSequencer::ScannerDirection;
        constexpr std::array<Direction, 4> directions { Direction::forward, Direction::reverse, Direction::pendulum, Direction::memoryAddress };
        sequencer.setScannerDirection(directions[static_cast<std::size_t>(std::clamp(scannerSelection, 0, 3))]);
        constexpr std::array<const char*, 4> names { "FWD", "REV", "ALT", "MEM" };
        appendLog(juce::String("SCANNER · ") + juce::String(names[static_cast<std::size_t>(std::clamp(scannerSelection, 0, 3))]));
    }
    void syncFeedbackConnections()
    {
        unsigned char routes = 0;
        for (std::size_t i = 0; i < connectionSwitches.size(); ++i)
            if (connectionSwitches[i].getToggleState()) routes |= static_cast<unsigned char>(1U << i);
        sequencer.setFeedbackConnections(routes);
        appendLog(antitotem::ui::text(antitotem::ui::logText::feedbackPrefix, uiLanguage) + juce::String::toHexString(static_cast<int>(routes)).toUpperCase());
    }
    void syncCore()
    {
        const auto selected = oscillatorCoreSelection;
        const auto mode = selected == 1 ? antitotem::CmosVoice::OscillatorCore::schmittPulse
                        : selected == 3 ? antitotem::CmosVoice::OscillatorCore::unbufferedDrift
                                        : antitotem::CmosVoice::OscillatorCore::functionForms;
        sequencer.setOscillatorCore(mode);
    }
    void setLoopEnd(std::size_t end)
    {
        const auto bounded = std::clamp(end, std::size_t { 1 }, loopSwitches.size());
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
            loopSwitches[i].setToggleState(i + 1 == bounded, juce::dontSendNotification);
        sequencer.setLoopEnd(bounded);
        // Was a fixed "· 1 ATIVO" that never changed no matter which step
        // was actually selected (the default is step 16) - now reflects
        // the real state.
        loopLabel.setText(antitotem::ui::text(antitotem::ui::label::loopEndPrefix, uiLanguage) + juce::String(static_cast<int>(bounded)) + antitotem::ui::text(antitotem::ui::label::loopEndSuffix, uiLanguage), juce::dontSendNotification);
        appendLog(antitotem::ui::text(antitotem::ui::logText::loopPrefix, uiLanguage) + juce::String(static_cast<int>(bounded)) + antitotem::ui::text(antitotem::ui::logText::loopStepsSuffix, uiLanguage));
    }
    [[nodiscard]] std::size_t selectedLoopEnd() const noexcept
    {
        for (std::size_t i = 0; i < loopSwitches.size(); ++i)
            if (loopSwitches[i].getToggleState()) return i + 1;
        return loopSwitches.size();
    }
    void timerCallback() override
    {
        const auto active = sequencer.getCurrentStep();
        // Same step-0 edge this already used for DERIVA's own phrase-loop
        // detection, reused here to quantize REC to the PRINCIPAL
        // sequencer's loop instead of raw wall-clock time: `currentStep`
        // already wraps to 0 exactly at whatever FIM DO LOOP is
        // configured to, so this one edge is both the "step 1" start
        // boundary and the "last active step" end boundary - no separate
        // getLoopEnd() check needed.
        const bool atLoopStart = sequencer.isRunning() && active == 0 && lastRecordStep != 0;
        if (recordingArmed && atLoopStart)
        {
            recordingArmed = false;
            recordingActive = true;
            configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recArmedTrackPrefix, uiLanguage) + antitotem::ui::text(antitotem::ui::logText::recPhases[0], uiLanguage) + antitotem::ui::text(antitotem::ui::logText::recArmedTrackMid, uiLanguage) + juce::String(static_cast<int>(recordingLengthSeconds / 60.0)) + ":00", 12.0f, material::returnPath);
            appendLog(antitotem::ui::text(antitotem::ui::logText::recStartedAtStep1, uiLanguage));
        }
        else if (recordingStopPending && atLoopStart)
        {
            recordingStopPending = false;
            recordingActive = false;
            recorder.stop();
            finishMidiRecording();
            record.setToggleState(false, juce::dontSendNotification);
            for (auto& button : recordDurations) button.setToggleState(false, juce::dontSendNotification);
            configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recFinishedSimple, uiLanguage), 12.0f, material::returnPath);
            appendLog(antitotem::ui::text(antitotem::ui::logText::recFinishedAtLoopEndPrefix, uiLanguage)
                + juce::String(static_cast<int>(sequencer.getLoopEnd()))
                + antitotem::ui::text(antitotem::ui::logText::recFinishedAtLoopEndSuffix, uiLanguage));
        }
        lastRecordStep = active;

        if (recordingActive && recorder.hasReachedLimit())
        {
            // Duration elapsed while actually recording - don't cut
            // immediately, wait for the same loop-end boundary the manual
            // stop path (record.onClick) uses.
            recordingStopPending = true;
        }
        if (recordingStopPending)
        {
            // A live test found this genuinely unclear before: the
            // countdown kept ticking after STOP was pressed, reading as
            // "still recording toward the original duration" instead of
            // "finishing the current loop before it actually stops" - a
            // steady message instead of a number that's no longer counting
            // toward anything reads more honestly.
            configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recFinishingAtLoopEndEllipsis, uiLanguage), 12.0f, material::returnPath);
        }
        else if (recordingActive)
        {
            // Live countdown, not the static "FAIXA X:00" set once at REC
            // start - this is the actual temporizador the duration buttons
            // promise, ticking down every frame from recorder.progress().
            // Only advances once actual writing has started (armed time
            // waiting for step 1 doesn't count against the requested
            // duration).
            const auto remaining = juce::jmax(0.0, recordingLengthSeconds * (1.0 - static_cast<double>(recorder.progress())));
            const auto minutesLeft = static_cast<int>(remaining) / 60;
            const auto secondsLeft = static_cast<int>(remaining) % 60;
            configureLabel(flow, antitotem::ui::text(antitotem::ui::logText::recArmedTrackPrefix, uiLanguage) + juce::String(minutesLeft) + ":"
                + (secondsLeft < 10 ? "0" : "") + juce::String(secondsLeft) + antitotem::ui::text(antitotem::ui::logText::recCountdownSuffix, uiLanguage), 12.0f, material::returnPath);
        }
        // REC's own button face: armed (steady amber) while waiting for
        // step 1, blinking red while samples are actually being written
        // (including the "finishing the loop" tail after a stop was
        // requested - still genuinely recording until the boundary
        // lands), back to its ordinary toggle-state look once fully
        // idle. ~3Hz blink (every 5 frames at the 30Hz UI timer).
        const auto newRecordPhase = recordingActive ? 2 : (recordingArmed ? 1 : 0);
        if (newRecordPhase != panelButtonLook().recordPhase)
        {
            panelButtonLook().recordPhase = newRecordPhase;
            recordBlinkCounter = 0;
            panelButtonLook().recordBlinkOn = true;
            record.repaint();
        }
        else if (newRecordPhase == 2)
        {
            if (++recordBlinkCounter >= 5)
            {
                recordBlinkCounter = 0;
                panelButtonLook().recordBlinkOn = ! panelButtonLook().recordBlinkOn;
                record.repaint();
            }
        }
        advanceRecordingForm();
        // Leve oscilação do PLAY enquanto toca - ~0.4Hz (um ciclo a cada
        // 2.5s) para ler como respiração, não como alarme. Só repinta o
        // próprio botão a cada frame enquanto está rodando; parado, volta
        // à cor sólida normal sem custo extra.
        panelButtonLook().playRunning = sequencer.isRunning();
        if (panelButtonLook().playRunning)
        {
            panelButtonLook().playPulsePhase += juce::MathConstants<float>::twoPi * (0.4f / 30.0f);
            if (panelButtonLook().playPulsePhase > juce::MathConstants<float>::twoPi)
                panelButtonLook().playPulsePhase -= juce::MathConstants<float>::twoPi;
            run.repaint();
        }
        if (deriveButton.getToggleState() && sequencer.isRunning() && active == 0 && lastDerivationStep != 0)
            deriveFromMemory();
        lastDerivationStep = active;
        for (std::size_t i = 0; i < steps.size(); ++i) steps[i].setActive(sequencer.isRunning() && i == active);
        // Audio writes the scope rings on the device thread. The UI timer
        // repaints the MIX view at a modest rate, so the drawing is live while
        // remaining completely downstream from the final L/R signal.
        stereoScope.repaint();
    }
    // sequencer is a reference to dualEngine.object1() so every existing
    // "sequencer.setXxx(...)" call site (there are ~50) keeps working
    // unchanged - only the whole-engine lifecycle calls (prepare/render/
    // setRunning/reset) needed to move to dualEngine itself, since those
    // now drive both object1 and object5 together.
    antitotem::DualObjectEngine dualEngine;
    antitotem::SimpleSequencer& sequencer;
    std::unique_ptr<ObjectFiveWindow> objectFiveWindow;
    // Single-window mode's own CLONE body - see setShowingCloneBody().
    // Separate from objectFiveWindow (dual-monitor mode's own window):
    // only one of the two is ever constructed for a given session, based
    // on which mode the user picks first, since they're mutually
    // exclusive ways of showing the same underlying dualEngine.object5().
    std::unique_ptr<ObjectFiveComponent> clonePanel;
    bool showingCloneBody = false;
    std::vector<juce::Component*> alwaysVisibleInBody;
    // Opt-in divergence from the base RASGO single-monitor window policy -
    // see docs/JANELA_UNICA_E_MONITORES.md. Only ever offered when 2+
    // displays actually exist; persisted like uiLanguage, never part of
    // the musical project state.
    bool dualMonitorMode = false;
    juce::TextButton monitorModeToggle;
    WavRecorder recorder;
    // Captura MIDI (ver o comentário de classe de MidiCapture) - mesmo
    // ciclo de vida que `recorder`, atrelada ao MESMO botão REC (sem UI
    // nova). `midiRecordingStamp` capturado no mesmo instante que
    // `recorder.start()` cria seu próprio arquivo internamente (stamp
    // privado, sem getter) - recalculado aqui, mesmo formato, pra que o
    // .wav e o .mid da mesma tomada acabem com o MESMO timestamp no
    // nome, reconhecíveis como um par.
    MidiCapture midiCapture;
    juce::String midiRecordingStamp;
    double currentSampleRate = 48000.0;
    bool loggingEnabled = false;
    bool unifiedVisibilityApplied = false;
    juce::Label title, flow, footer, clockLabel, loopLabel, connectionLabel, feedbackLabel, deriveLabel, energyLabel, masterLabel, cloneVolumeLabel, noiseLabel, modulationLabel, lfoShapeLabel, effectsLabel, detailLabel, filterLabel, envelopeLabel, voiceLabel, temporalLabel, metricLabel, scannerLabel, recordDurationsLabel, variationLabel, mixerLabel, recordingLabel, stepsLabel, modeLabel;
    // CONEXÃO ENTRE OBJETOS, moved here from CLONE's own left column - it
    // describes the relationship between the two objects, not a property
    // of either one, so it lives fixed in MainComponent above LOG.
    juce::Label objectConnectionLabel, gainToFifthLabel, gainToFirstLabel, auxToFirstLabel, auxToFifthLabel;
    juce::Slider gainToFifth, gainToFirst, auxToFirst, auxToFifth;
    juce::Label routesToFifthLabel, routesToFirstLabel;
    std::array<juce::ToggleButton, 4> routesToFifth, routesToFirst;
    std::array<ChipConcept, 5> concepts { ChipConcept { "40106", utf8("LIMIAR · CLOCK") },
                                          ChipConcept { "4040", utf8("DIVISÃO · ENDEREÇO") },
                                          ChipConcept { "4051", utf8("SCANNER · 8 CV") },
                                          ChipConcept { "8038*", utf8("FORMAS CONTÍNUAS"), true },
                                          ChipConcept { "4069UB*", utf8("DERIVA · GANHO"), true } };
    std::unique_ptr<juce::Drawable> logo;
    juce::Rectangle<int> logoBounds;
    std::array<juce::Label, 5> oscillatorLabels;
    // Per-knob captions ("FREQ", "MIX", "FORMA", "EIXO X") - the column
    // header used to be the only place naming the controls, which read fine
    // once but did not say which specific knob was which at a glance.
    std::array<juce::Label, 5> oscillatorRateLabels, oscillatorLevelLabels, oscillatorShapeLabels, oscillatorPanCaptions;
    // Y (proximity) and Z (orbit), one pair per oscillator - the DESIGN.md
    // X/Y/Z topology coordinates, per voice like EIXO X (X) already is.
    std::array<juce::Label, 5> oscillatorProximityCaptions, oscillatorOrbitCaptions;
    std::array<juce::Slider, 5> oscillatorProximities, oscillatorOrbits;
    juce::Slider clock, feedbackGain, deriveDepth, energy, master, cloneVolume, filterCutoff, filterResonance, filterDepth;
    // MaterialFilter MIX - see ObjectFiveComponent's own materialFilter
    // member comment (same module, no on/off switch - a continuum, not a
    // toggle).
    juce::Slider materialFilterMix;
    juce::Label materialFilterLabel;
    juce::TextButton run, stop, reset, record, soundPage, sequencePage, mixPage, tutorial, about, pulseVariation, porousVariation, heterodyneVariation, randomizeStepsButton, orbitVariation, pendulumVariation;
    // LPF/BPF/HPF/NCH, independent toggles - see ObjectFiveComponent's own
    // filterModeButtons member comment (same module, same design).
    std::array<juce::TextButton, 4> filterModeButtons;
    juce::TextButton languageSwitch, objectFive;
    // LEARN: no button, no toggle - always on, same persistent-object
    // category as LOG (author, live: "sempre visivel independe que qual
    // instrumento clone ou principal" / "talvez o botão learn nem seja
    // necessário se a caixa sempre é visível" / "sempre funcionará o
    // learn"). Dedicated box at the end of the transport column
    // (learnLabel/learnEditor, same juce::TextEditor + logPanelLook()
    // treatment as LOG) - replaces an earlier floating, cursor-anchored
    // panel the author tried live and disliked ("não gostei dessas abas
    // abrindo o tempo todo, crie a caixa dedicada"), see
    // METODOLOGIA_DE_DESENVOLVIMENTO.md Section 11.
    juce::Label learnLabel;
    juce::TextEditor learnEditor;
    antitotem::ui::Language uiLanguage = antitotem::ui::Language::english;
    juce::ApplicationProperties applicationProperties;
    std::unique_ptr<TutorialWindow> tutorialWindow;
    std::unique_ptr<AppInfoWindow> appInfoWindow;
    // 1/2/3/5 minutes: picking one starts REC at that length and highlights
    // only itself; none stay highlighted once nothing is recording.
    std::array<juce::ToggleButton, 4> recordDurations;
    juce::ToggleButton deriveButton;
    // Camadas de deriva (19 ago. 2026, autor: "os do tipo vcf, no mesmo
    // espaço do botão deriva (independente para principal e clone)") -
    // multi-select real como os botões de modo do VCF (LPF/BPF/HPF/NCH:
    // setClickingTogglesState, sem radio group, cada um liga/desliga
    // sozinho), não um grupo exclusivo. A/B/C correspondem às três
    // "instâncias paralelas" (derivationMotion/B/C) - desligar uma
    // impede aquele grupo de parâmetros de derivar neste ciclo, sem
    // afetar as outras duas. Todas ligadas por padrão (mesmo
    // comportamento de antes destes botões existirem).
    // AUTO (índice 3, 20 ago. 2026) liga um MODO inteiro diferente de
    // deriveFromMemory() - ver seu próprio comentário lá dentro. A/B/C
    // (0/1/2) continuam exatamente como eram, intactos - "sem destruir
    // também o que já temos que é outra configuração possível" (autor).
    std::array<juce::ToggleButton, 4> derivationLayers;
    NoiseSelector noiseSelector;
    StereoScope stereoScope;
    juce::Slider scopeGain;
    juce::Label logLabel;
    juce::TextEditor log;
    std::array<juce::ToggleButton, 3> coreSwitches;
    std::array<juce::ToggleButton, antitotem::SimpleSequencer::stepCount> loopSwitches;
    std::array<juce::ToggleButton, 6> connectionSwitches;
    std::array<juce::ToggleButton, 4> scannerButtons;
    // 8, not 4 - two rows of 4 (see the array-size comments where
    // beats/units and feels are defined for the full context).
    std::array<juce::ToggleButton, 8> temporalButtons;
    // 0-1, layered on every SUBDIVISÃO feel, not exclusive to SWG (20 ago.
    // 2026, "ao invés de um knob um slider swing", then "deixa o swing
    // somente enquanto botão, e utilise esse slide atual do swing para o
    // groove").
    juce::Label grooveLabel;
    juce::Slider grooveAmount;
    std::array<juce::ToggleButton, 8> metricButtons;
    std::array<juce::ToggleButton, 6> lfoShapeButtons;
    std::array<juce::Slider, 4> envelopeControls;
    std::array<juce::Label, 4> envelopeControlLabels;
    std::array<juce::Label, 3> filterControlLabels;
    std::array<juce::Slider, 3> modulationControls, effectControls;
    std::array<juce::Label, 3> modulationControlLabels, effectControlLabels;
    std::array<juce::Slider, 16> detailControls;
    std::array<juce::Label, 16> detailControlLabels;
    juce::Label materialRailLabel;
    juce::Label chaosRailLabel;
    // Umbrella title for the whole rails band (18 ago. 2026) - see
    // layoutRailsBand()'s own comment.
    juce::Label parametersLabel;
    juce::ToggleButton lfoFreeze;
    // Didactic backing panel behind CAOS/VAGA/FRZ (18 ago. 2026) - see
    // layoutVoiceArea()'s own comment. Filled in by that function, drawn
    // in this component's own paint().
    juce::Rectangle<int> chaosFreezeHighlight;
    int lfoShapeSelection = 0;
    std::array<juce::Label, 4> mixLabels;
    std::array<juce::Slider, 4> mixGain, mixPan, mixReflux;
    std::array<juce::ToggleButton, 4> mixEnable, mixMute, mixSolo;
    juce::Label objectMixLabel, principalVolumeLabel;
    juce::Slider principalVolume;
    juce::ToggleButton principalMute, cloneMute;
    // EXCITAÇÃO - a third, generative "object" alongside PRINCIPAL/CLONE
    // (20 ago. 2026, author: "entendo o treremin como um outro objeto").
    // Started gain-only ("zerado é ele desligado"), then author asked for
    // an M button too ("crie o botão de mute para o excit no object
    // mixer") - same PRINC/CLONE vocabulary, muting the final output
    // without discarding the slider's own value (the voice keeps
    // gliding/walking internally while muted, see DualObjectEngine.cpp).
    // See DualObjectEngine::setExcitationAmount()/setExcitationMute() for
    // the generative engine.
    juce::Label excitationLabel;
    juce::Slider excitationAmount;
    juce::ToggleButton excitationMute;
    juce::Label mixMemoryLabel;
    std::array<juce::TextButton, 4> mixMemorySlots;
    // M1-4/CAPTURAR (20 ago. 2026, autor: "e os botoes de memoria
    // captura também" -> escolheu só RECALL, nunca CAPTURE, via
    // AskUserQuestion). Rastreado aqui porque `MutableMixer` não expõe
    // se um slot já foi capturado - um slot nunca capturado tem
    // `enabled = false` nos 4 canais por padrão, então recall nele
    // silenciaria o mixer inteiro. Marcado `true` só no próprio
    // `onClick` de captura (nunca pela DERIVA).
    std::array<bool, 4> mixMemoryCaptured {};
    // Participação por título (20 ago. 2026, autor: "tive uma ideia
    // para as seleções dos conteúdos a fazerem parte dos item a
    // partiticar da deriva, ao lado de cada título um pequeno botão
    // toogle" - confirmado via AskUserQuestion: vale pros dois modos
    // [A/B/C e AUTO], todos os títulos de uma vez, exceto a barra de
    // transporte no topo). Um bool por título já existente na tela -
    // desligar um congela só aquele bloco, sem depender de A/B/C/AUTO.
    // Todos começam ligados (mesmo padrão de "sem destruir o que já
    // temos"). Sem fome/memória própria - são só um portão extra em
    // cima do que já existe, checado com `&&` nas condições que já
    // tinham (não uma reestruturação).
    juce::ToggleButton participateSteps, participateVoice, participateEffects, participateDetail,
                        participateMixer, participateEnvelope, participateModulation, participateGroove,
                        participateFilter, participateMetric, participateTemporal, participateNoiseColour,
                        participateLoopEnd, participateRoutes, participateMixMemory;
    // Faltavam na primeira leva (20 ago. 2026, autor: "faltaram botoes
    // em: MATERIA, CAOS, FORMA LFO, RING, KNOB CLOCK" / "e MAT") -
    // MATÉRIA (rail CUTOFF/RESON/DRIVE/ASYM, detailControls[9..12]) e
    // CAOS (rail DRIVE/DAMPING/DEPTH, detailControls[13..15]) tinham
    // título visual próprio mas caíam dentro do `participateDetail`
    // único (ROTAS ATIVAS, detailControls[0..8]) - separados aqui.
    // MAT (`materialFilterMix`, um knob só) é uma coisa DIFERENTE do
    // rail MATÉRIA (mesmo nome curto, controles diferentes). RING
    // (`modulationControls[1]`) nunca teve mecanismo de deriva nenhum -
    // entra no MESMO toggle de MODULAÇÃO que LFO/NOISE MIX já usam
    // (autor: "creio que ring já está em modulação"), sem toggle novo.
    juce::ToggleButton participateMaterial, participateChaos, participateMat, participateLfoShape, participateClock;
    // Só aqui - CONEXÕES ENTRE OBJETOS não existe em CLONE.
    juce::ToggleButton participateConnections;
    juce::TextButton mixMemoryCapture;
    std::array<juce::Slider, 5> oscillatorRates, oscillators, shapes, oscillatorPans;
    int oscillatorCoreSelection = 2;
    int temporalSelection = 0, metricSelection = 0, scannerSelection = 0;
    Page page = Page::sound;
    std::array<float, antitotem::SimpleSequencer::stepCount> derivationCv {}, derivationAmp {}, derivationFx {};
    std::array<float, 5> derivationRatios {};
    // Alcance estendido, 19 ago. 2026, autor: "rotas ativas não percebo" /
    // "matéria também não" / "CAOS também não" / "sliders horizontais do
    // mixer também não" - ROTAS ATIVAS/MATÉRIA/CAOS são o mesmo painel
    // (`detailControls[0..15]`, ver seu próprio comentário de membro em
    // ambas as cópias) e nunca tinham participado da DERIVA; mixer
    // (`mixGain`/`mixPan`/`mixReflux`) também não - mesmo padrão de
    // memória por slider que steps/ratios já usam.
    std::array<float, 16> derivationDetail {};
    // Memória própria só pro modo AUTO (20 ago. 2026) - o modo A/B/C
    // nunca precisou de uma âncora capturada pra esses grupos (ADSR/
    // LFO/NOISE MIX/GROOVE/filtro/pans/FX derivam livremente em torno
    // do valor ATUAL, não de um valor capturado - ver o comentário
    // original acima de ADSR/LFO no ramo `else`), mas
    // `driftAutonomousItem()` sempre precisa de uma memória própria por
    // item pra funcionar.
    std::array<float, 3> derivationEffects {};
    std::array<float, 4> derivationEnvelope {};
    float derivationLfo = 0.0f, derivationNoiseMix = 0.0f, derivationGroove = 0.0f, derivationFilterCutoff = 0.0f, derivationFilterResonance = 0.0f;
    std::array<float, 5> derivationPans {};
    std::array<float, 4> derivationMixGain {}, derivationMixPan {}, derivationMixReflux {};
    // AUTO (20 ago. 2026, autor: "penso em algo que cada item é
    // autônomo") - fome por item, ver `driftAutonomousItem()`. Cresce a
    // cada ciclo em que aquele item específico NÃO age, reseta quando
    // age - sem coordenador central, cada slider se revezando sozinho
    // em vez de um Motion A/B/C decidindo por grupos inteiros.
    std::array<float, antitotem::SimpleSequencer::stepCount> hungerCv {}, hungerAmp {}, hungerFx {};
    std::array<float, 5> hungerRatios {};
    std::array<float, 3> hungerEffects {};
    std::array<float, 4> hungerEnvelope {};
    float hungerLfo = 0.0f, hungerNoiseMix = 0.0f, hungerGroove = 0.0f, hungerFilterCutoff = 0.0f, hungerFilterResonance = 0.0f;
    // Botões do VCF/filterModeButtons (20 ago. 2026, autor: "os botões
    // do vcf não estão conectados a deriva") - filterCutoff/
    // filterResonance já participavam, mas o MODO do filtro em si
    // (combinação de LP/BP/HP/NOTCH, `filterModeButtons`, multi-select
    // igual A/B/C/AUTO) nunca tinha entrado. Fome só, sem memória - são
    // toggles, não sliders (mesmo padrão de `hungerObjectRoute`).
    float hungerFilterMode = 0.0f;
    // Botões CORE dos osciladores (20 ago. 2026, autor: "verifique se
    // os 3 botões dos osciladores se conectam a deriva") -
    // `coreSwitches` (40106/8038/4069UB, radio group), nunca
    // participava. Salto discreto, mesmo padrão de MÉTRICA/SUBDIVISÃO.
    float hungerCore = 0.0f;
    // M1-4 (RECALL só, nunca CAPTURE - ver `mixMemoryCaptured`'s próprio
    // comentário de membro).
    float hungerMixMemory = 0.0f;
    std::array<float, 5> hungerPans {};
    std::array<float, 16> hungerDetail {};
    std::array<float, 4> hungerMixGain {}, hungerMixPan {}, hungerMixReflux {};
    float hungerMetric = 0.0f, hungerTemporal = 0.0f, hungerNoiseColour = 0.0f, hungerLoopEnd = 0.0f;
    // RING/MAT/FORMA LFO/KNOB CLOCK (20 ago. 2026) - memória+fome no
    // mesmo padrão de tudo mais.
    float derivationRing = 0.0f, hungerRing = 0.0f;
    float derivationMat = 0.0f, hungerMat = 0.0f;
    float hungerLfoShape = 0.0f;
    float derivationClock = 0.0f, hungerClock = 0.0f;
    // CONEXOES ENTRE OBJETOS (20 ago. 2026, autor: "conexoes entre
    // objetos ?" - registrado sem decisao na hora; retomado agora:
    // "proximos itens da lista"). So existe aqui em MainComponent -
    // `gainToFifth`/`gainToFirst`/`auxToFirst`/`auxToFifth`/
    // `routesToFifth`/`routesToFirst` vivem no `dualEngine`
    // compartilhado, nao em cada objeto (CLONE nao tem copia propria
    // desses controles).
    float derivationGainToFifth = 0.0f, derivationGainToFirst = 0.0f, derivationAuxToFirst = 0.0f, derivationAuxToFifth = 0.0f;
    float hungerGainToFifth = 0.0f, hungerGainToFirst = 0.0f, hungerAuxToFirst = 0.0f, hungerAuxToFifth = 0.0f;
    float hungerObjectRoute = 0.0f;
    float derivationMotion = 0.0f;
    // Instâncias paralelas (19 ago. 2026, autor: "as duas" - confirmando
    // tanto múltiplas âncoras quanto camadas paralelas de deriva depois
    // de brainstormar "níveis de trocas em steps diferentes... steps
    // distintos... instancias paralelas"). derivationMotion (acima)
    // continua o núcleo original (steps CV/AMP/FX, topologia/feedback,
    // razão dos osciladores). B e C são réplicas independentes do MESMO
    // mecanismo (random walk + atratores), cada uma com sua própria
    // velocidade/caráter, dirigindo grupos de parâmetros DIFERENTES -
    // ver deriveFromMemory() pra qual grupo cada uma controla. O
    // instrumento passa a evoluir como processos paralelos em vez de um
    // bloco só se movendo junto.
    float derivationMotionB = 0.0f, derivationMotionC = 0.0f;
    // Âncoras combináveis (19 ago. 2026, mesmo brainstorm - "talvez como
    // fizemos nos botoes do vcf, combinações de botões de derivas": o
    // VCF já tem toggles multi-select reais/independentes - LPF/BPF/HPF/
    // NCH, não um modo exclusivo). Pool de 2 posições, -1 = vazia.
    // Começa só com o passo 1 (índice 0); cada evento GLT ocupa a
    // próxima posição livre e, uma vez as duas ocupadas, substitui a
    // mais antiga (round-robin, `derivationAnchorWrite`) - nunca cresce
    // sem limite.
    std::array<int, 2> derivationAnchors { 0, -1 };
    int derivationAnchorWrite = 0;
    std::array<unsigned char, 8> topologyMemory {};
    std::size_t topologyWrite = 0, derivationPhrase = 0, lastDerivationStep = 0;
    unsigned int derivationState = 0xA17E70U;
    double recordingLengthSeconds = 180.0;
    int recordingEvent = 0;
    // REC is quantized to the PRINCIPAL sequencer's own loop, not raw
    // wall-clock time - see timerCallback(). recordingArmed: REC/a
    // duration button was pressed, waiting for the sequencer to reach
    // step 1 before any sample is actually written. recordingActive:
    // between that step-1 boundary and the loop-end boundary, samples
    // are being written for real. recordingStopPending: a stop was
    // requested (manually or the requested duration elapsed) while
    // recordingActive, waiting for the loop to actually finish instead
    // of cutting off mid-loop.
    bool recordingArmed = false, recordingActive = false, recordingStopPending = false;
    std::size_t lastRecordStep = 0;
    int recordBlinkCounter = 0;
    std::array<StepControl, antitotem::SimpleSequencer::stepCount> steps { StepControl {0}, StepControl {1}, StepControl {2}, StepControl {3}, StepControl {4}, StepControl {5}, StepControl {6}, StepControl {7}, StepControl {8}, StepControl {9}, StepControl {10}, StepControl {11}, StepControl {12}, StepControl {13}, StepControl {14}, StepControl {15} };
};

class PerformanceViewport final : public juce::Viewport
{
public:
    PerformanceViewport()
    {
        setScrollBarsShown(true, false, true, true);
        setViewedComponent(&panel, false);
    }

    void resized() override
    {
        juce::Viewport::resized();
        panel.setSize(std::max(1280, getWidth()), 1500);
    }

private:
    MainComponent panel;
};

// Zoom of the whole panel, browser-style (18 ago. 2026, author: "faça o
// zoom de conteúdo por aba (estilo Ctrl+/Ctrl- do navegador)"). Only
// wired up for the normal (maximised, non-scrolling) window path - the
// small-screen PerformanceViewport fallback above already has its own
// scrolling story and is rare in practice (see MainWindow's own
// needsScroll comment).
//
// Component::setTransform() scales rendering without re-running any
// layout math, so every fixed-pixel dimension inside the panel (knob
// sizes, fonts, column widths) grows/shrinks together - true zoom, not a
// resize that would just leave more/less empty space around
// still-89px knobs.
//
// juce::Viewport computes its own scrollbar range from the VIEWED
// component's plain, untransformed getBounds() (confirmed in JUCE's own
// Viewport::updateVisibleArea(), which never reads getTransform()) - a
// transformed `panel` alone would report its pre-zoom size, so scrolling
// would silently stop working past the edge once zoomed in. `sizer` is a
// plain, untransformed shim between the Viewport and the real
// (transformed) `panel`: it holds no content of its own, just reports the
// already-zoomed footprint as its own size, so the Viewport's scrollbar
// math is correct at any zoom level. Chosen over clipping at the window
// edge or capping zoom to "shrink only" (both offered, author picked
// scrolling) since the window already opens maximised with no room of
// its own to grow into when zooming in.
class ZoomableViewport final : public juce::Viewport
{
public:
    ZoomableViewport()
    {
        setScrollBarsShown(true, false, true, true);
        panel.setTopLeftPosition(0, 0);
        sizer.addAndMakeVisible(panel);
        setViewedComponent(&sizer, false);
    }

    void resized() override
    {
        juce::Viewport::resized();
        // The Viewport's own (unscaled) visible area - the same starting
        // size MainComponent always received directly before this class
        // existed, so zoom = 1.0 renders pixel-identical to before.
        logicalWidth = std::max(1280, getWidth());
        logicalHeight = std::max(720, getHeight());
        applyZoom();
    }

    // 0.7-1.5, 0.1 steps - narrow enough that the fixed-pixel layout
    // never has to fight std::clamp() ranges tuned for ~1920x1080 at
    // either extreme.
    void setZoom(float newZoom)
    {
        const auto clamped = std::clamp(newZoom, 0.7f, 1.5f);
        if (std::abs(clamped - zoom) < 0.001f)
            return;
        zoom = clamped;
        applyZoom();
    }
    float getZoom() const noexcept { return zoom; }
    // Shift+C shortcut (20 ago. 2026) - MainWindow's keyPressed() needs
    // the real MainComponent to call toggleCloneView() on, not just
    // this Viewport wrapper.
    MainComponent& getPanel() noexcept { return panel; }

private:
    void applyZoom()
    {
        panel.setSize(logicalWidth, logicalHeight);
        panel.setTransform(juce::AffineTransform::scale(zoom));
        sizer.setSize(juce::roundToInt(static_cast<float>(logicalWidth) * zoom),
                      juce::roundToInt(static_cast<float>(logicalHeight) * zoom));
    }

    juce::Component sizer;
    MainComponent panel;
    int logicalWidth = 1280, logicalHeight = 720;
    float zoom = 1.0f;
};

class MainWindow final : public juce::DocumentWindow, public juce::KeyListener
{
public:
    MainWindow() : DocumentWindow("Antitotem - Objeto Sonoro", juce::Colour(0xff171511), allButtons)
    {
        addKeyListener(this);
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        // Below the reference canvas the viewport supplies vertical scroll;
        // the panel itself still keeps its physical control dimensions.
        setResizeLimits(1280, 720, 3840, 2160);
        // Compute the real target bounds before this window is shown at
        // all, not centreWithSize(1600, 920) first and setBounds() to the
        // real monitor size after - that used to visibly flash the
        // smaller/wrong size for the first frame or two before settling
        // (author, live: "sempre quando abre o software ele fica com uma
        // resolução muito mais larga nos primeiros segundos, e depois se
        // encaixa corretamente no monitor"). Multi-monitor rigs with a
        // dead/EDID-less output still reported as connected (e.g. an
        // unplugged VGA port) can make JUCE's Displays list collapse into
        // a single bogus region spanning every screen - querying "the
        // primary display" for its exact size is therefore not fully
        // reliable, but still works out in practice: the window manager
        // clips an over-large setBounds() request down to whichever
        // single monitor the window actually lands on. (JUCE's own
        // setFullScreen() was tried here too, but it races the window
        // manager's own maximise handling on this native-title-bar/X11
        // combination and loses.) 1600x920 remains the fallback only for
        // the rare case getPrimaryDisplay() itself returns nullptr.
        //
        // That "still works out in practice" turned out not to hold on
        // the author's own rig: the window opened genuinely oversized
        // ("muito expandido no sentido lateral, cortando boa parte do
        // layout") and stayed that way for 1-10 real seconds before the
        // window manager corrected it - not a one-frame render flash,
        // an actual bad bounds value the WM took its time clipping down.
        // setResizeLimits() above only constrains interactive dragging,
        // not a raw setBounds() call, so it did nothing here on its own.
        // Clamping to that same declared maximum (3840x2160) before ever
        // calling setBounds() means this window can never request a size
        // the WM has to fight, regardless of what the Displays list
        // (bogus combined multi-monitor region or not) actually reports.
        if (const auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
        {
            auto bounds = display->userBounds.toNearestInt();
            // Confirmed live by the author (screenshot, 14 ago. 2026 20:07):
            // the 3840x2160 clamp below doesn't help when the bogus combined
            // region this rig reports is already at or under that ceiling -
            // two 1920x1080 monitors side by side is exactly 3840x1080,
            // still "≤3840 wide" but genuinely spanning both screens, not a
            // normal single-monitor size. Same panoramic heuristic already
            // used by hasSecondMonitor()/secondMonitorArea() below (>=2.5:1
            // reads as two side-by-side screens, not one real monitor) -
            // Reducing "the full combined span" to "one screen's worth" is
            // the missing piece, but a plain half-split (tried first) isn't
            // right either: the author's two monitors aren't equal width
            // (xrandr: VGA-0 secondary 1368x768+0+0, HDMI-0 primary
            // 1920x1080+1368+0, combined 3288x1080), so half of 3288
            // (1644) undershoots the real primary by ~276px - it neither
            // clips anything nor fills the screen, just leaves a visible
            // gap and drops below the 1900px non-scrolling threshold
            // (author, live, after that attempt: "não abre maximizado").
            // Using this app's own known reference width (1920 - the
            // resolution the whole UI is designed/tested against, see
            // docs/TAREFAS.md "Cabe em 1920x1080 sem scroll") instead of a
            // guessed fraction reconstructs the real primary exactly on
            // this rig: anchored at the right edge (same reasoning as
            // before - the primary sits there, confirmed by 1368+1920 =
            // the 3288 reported here), min()'d against the combined width
            // itself in case that's ever narrower than 1920. This has no
            // reliable per-output data to work from once the bogus report
            // collapses both screens into one (that's the underlying bug),
            // so it's tuned to what this rig's own primary actually is,
            // not a general solution for arbitrary monitor layouts.
            if (bounds.getWidth() >= bounds.getHeight() * 2.5)
            {
                const auto targetWidth = std::min(bounds.getWidth(), 1920);
                bounds = bounds.withX(bounds.getRight() - targetWidth).withWidth(targetWidth);
            }
            bounds.setWidth(std::min(bounds.getWidth(), 3840));
            bounds.setHeight(std::min(bounds.getHeight(), 2160));
            setBounds(bounds);
        }
        else
            centreWithSize(1600, 920);
        // Reacts to the window's own real, final size (not a pre-computed
        // guess) to decide whether the 1920x1080 single-page surface fits
        // or the smaller-screen scrolling fallback is required - still
        // correct done once here, now that there is only one bounds pass
        // to react to.
        applyContentForCurrentSize();
        setVisible(true);
    }
    void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
    void resized() override
    {
        DocumentWindow::resized();
        applyContentForCurrentSize();
    }
    // KeyListener - Ctrl+=/Ctrl+-/Ctrl+0 zoom the panel, browser-style
    // (18 ago. 2026, author's own idea). Only does anything on the normal
    // (maximised) path - `getContentComponent()` is a plain MainComponent
    // when the small-screen PerformanceViewport fallback is active, so the
    // dynamic_cast below quietly no-ops there instead of needing a second
    // code path.
    // Un-hides Component::keyPressed(const KeyPress&) (DocumentWindow's
    // own base) - this class only implements KeyListener's 2-argument
    // overload below, not that one, but a derived class declaring any
    // `keyPressed` hides every overload of that name from its bases
    // unless told otherwise.
    using juce::Component::keyPressed;
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override
    {
        // Shift+C (20 ago. 2026, autor: "pode criar um atalho para o
        // boltão clone principal" -> "shift") - dispara o mesmo
        // `onClick` do botão CLONE/PRINCIPAL. Checado ANTES do zoom
        // abaixo (que só faz sentido com o ZoomableViewport) porque
        // precisa funcionar nos dois modos de conteúdo - Viewport
        // normal ou MainComponent direto no fallback de tela pequena
        // (ver o comentário do zoom logo abaixo pra esse mesmo caso).
        if (key == juce::KeyPress('C', juce::ModifierKeys::shiftModifier, 0))
        {
            if (auto* zoomableForClone = dynamic_cast<ZoomableViewport*>(getContentComponent()))
            {
                zoomableForClone->getPanel().toggleCloneView();
                return true;
            }
            if (auto* directPanel = dynamic_cast<MainComponent*>(getContentComponent()))
            {
                directPanel->toggleCloneView();
                return true;
            }
            return false;
        }
        auto* zoomable = dynamic_cast<ZoomableViewport*>(getContentComponent());
        if (zoomable == nullptr)
            return false;
        // '+' too, not just '=' - '+' is what Shift+= actually sends on
        // most layouts, and a user thinking "Ctrl-plus" is as likely to
        // press that combination as the unshifted key browsers rely on.
        if (key == juce::KeyPress('=', juce::ModifierKeys::commandModifier, 0)
            || key == juce::KeyPress('+', juce::ModifierKeys::commandModifier, 0))
        {
            zoomable->setZoom(zoomable->getZoom() + 0.1f);
            return true;
        }
        if (key == juce::KeyPress('-', juce::ModifierKeys::commandModifier, 0))
        {
            zoomable->setZoom(zoomable->getZoom() - 0.1f);
            return true;
        }
        if (key == juce::KeyPress('0', juce::ModifierKeys::commandModifier, 0))
        {
            zoomable->setZoom(1.0f);
            return true;
        }
        return false;
    }
private:
    void applyContentForCurrentSize()
    {
        // A tolerance below the raw 1920x1080 reference accounts for the
        // native title bar and any window-manager decoration eating into
        // the maximised window's own bounds - that loss is normal and must
        // not itself trigger the smaller-screen scrolling fallback.
        const auto bounds = getLocalBounds();
        const bool needsScroll = bounds.getWidth() < 1900 || bounds.getHeight() < 950;
        if (contentApplied && needsScroll == scrollingContent)
            return;
        contentApplied = true;
        scrollingContent = needsScroll;
        // false: the window's own bounds (set explicitly above) stay
        // authoritative. MainComponent's constructor calls setSize() with an
        // arbitrary starting hint; with resizeToFit=true that call would
        // fight back and shrink this window to match it on every content
        // swap, undoing the maximised/placed size.
        setContentOwned(needsScroll ? static_cast<juce::Component*>(new PerformanceViewport())
                                    : static_cast<juce::Component*>(new ZoomableViewport()), false);
    }
    bool contentApplied = false;
    bool scrollingContent = false;
};
class Application final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Antitotem - Objeto Sonoro"; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    void initialise(const juce::String&) override
    {
        juce::LookAndFeel::setDefaultLookAndFeel(&antitotemLookAndFeel());
        window = std::make_unique<MainWindow>();
    }
    void shutdown() override
    {
        window.reset();
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }
private:
    std::unique_ptr<MainWindow> window;
};
}
START_JUCE_APPLICATION(Application)
