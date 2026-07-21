/**
 * @file  PluginEditor.cpp
 * @brief Plugin UI — 6 interactive modules, all mouse-driven.
 *
 * @section Layout constants
 * All pixel values are derived from the design spec (HonestMix_最终版.html).
 * See 🤖_新任务启动点.md §B for the canonical layout diagram.
 */

#include "PluginEditor.h"

//==============================================================================
// ── Layout constants ──────────────────────────────────────────
// (摘自 HonestMix_最终版.html, 23:16 版本)
constexpr int PW = 280;   ///< Plugin window width
constexpr int PH = 430;   ///< Plugin window height

constexpr int KNOB_SIZE    = 72;  ///< Dry/Wet knob diameter (px)
constexpr int INFO_Y       = 32;  ///< Info row (耳机/曲线/声卡) Y offset
constexpr int INFO_COL_W   = 50;  ///< Info column width
constexpr int INFO_GAP     = 18;  ///< Gap between info columns
constexpr int BOTTOM_Y     = 282; ///< Bottom row (校正/BPM) Y offset
constexpr int SEAL_SIZE    = 30;  ///< 诚 stamp size
constexpr int SEAL_MARGIN  = 6;   ///< 诚 stamp bottom-right margin

// ── Headphone display names & target curves ───────────────────
// Must match CorrectionEngine::profiles[] order.
const char* HonestMixAudioProcessorEditor::profileNames_[numProfiles_] = {
    "ATH-M50X", "DT 770 Pro", "HD 600", "DT 990 Pro"
};
const char* HonestMixAudioProcessorEditor::profileCurves_[numProfiles_] = {
    "Harman OE", "Harman OE", "Harman OE", "Harman OE"
};

//==============================================================================
namespace {
// ── Custom rotary-slider look ─────────────────────────────────
class KnobLNF final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float sa, float ea, juce::Slider&) override
    {
        auto cx = x + w / 2.0f;
        auto cy = y + h / 2.0f;
        auto r  = static_cast<float> (juce::jmin (w, h)) / 2.0f - 4.0f;

        // Outer ring
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 10));
        g.drawEllipse (cx - r, cy - r, r * 2, r * 2, 1.5f);

        // Arc
        auto a = sa + (ea - sa) * pos;
        juce::Path arc;
        arc.addArc (cx - r + 2, cy - r + 2, r * 2 - 4, r * 2 - 4, sa, a, true);
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 30));
        g.strokePath (arc, juce::PathStrokeType (2.0f));

        // Dot
        float px = cx + (r - 2) * std::cos (a);
        float py = cy + (r - 2) * std::sin (a);
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 45));
        g.fillEllipse (px - 2.5f, py - 2.5f, 5.0f, 5.0f);
    }
};
static KnobLNF knobLNF;
} // namespace

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (
        HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef_ (p)
{
    // ═══════════════════════════════════════════════════════════
    // ── 1. Transition card ────────────────────────────────────
    // ═══════════════════════════════════════════════════════════

    tcSeal_.setText (juce::String::fromUTF8 ("\350\257\232"), juce::dontSendNotification);
    tcSeal_.setFont (juce::Font (juce::FontOptions (28.0f)));
    tcSeal_.setJustificationType (juce::Justification::centred);
    tcSeal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.04f));
    addAndMakeVisible (tcSeal_);

    tcTitle_.setText (juce::String::fromUTF8 ("\345\207\206\345\244\207\345\245\275\344\272\206"), juce::dontSendNotification);
    tcTitle_.setFont (juce::Font (juce::FontOptions (12.0f)));
    tcTitle_.setJustificationType (juce::Justification::centred);
    tcTitle_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.35f));
    addAndMakeVisible (tcTitle_);

    tc50_.setButtonText (juce::String::fromUTF8 ("50% \346\267\261\345\210\273\345\257\271\346\257\224"));
    tc50_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    tc50_.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.2f));
    tc50_.setClickingTogglesState (true);
    tc50_.setToggleState (true, juce::dontSendNotification);
    tc50_.onClick = [this] { dryWetChoice_ = 50; };
    addAndMakeVisible (tc50_);

    tc100_.setButtonText (juce::String::fromUTF8 ("100% \347\233\264\346\216\245\344\277\241\344\273\273"));
    tc100_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    tc100_.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.12f));
    tc100_.setClickingTogglesState (true);
    tc100_.onClick = [this] { dryWetChoice_ = 100; };
    addAndMakeVisible (tc100_);

    tcHow_.setText (juce::String::fromUTF8 (
        "\342\221\240 \344\271\213\345\220\216\345\206\262\347\274\251\345\210\260\343\200\214\350\257\232\343\200\215\n"
        "\342\221\241 40\345\210\206\351\222\237\346\217\220\351\206\222\n"
        "\342\221\242 \344\275\240\345\217\215\351\246\210\342\200\224\346\225\260\346\215\256\350\277\255\344\273\243"),
        juce::dontSendNotification);
    tcHow_.setFont (juce::Font (juce::FontOptions (8.0f)));
    tcHow_.setJustificationType (juce::Justification::centred);
    tcHow_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.1f));
    addAndMakeVisible (tcHow_);

    tcDismiss_.setButtonText (juce::String::fromUTF8 ("\345\206\215\350\260\203\344\270\200\344\270\213"));
    tcDismiss_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    tcDismiss_.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.06f));
    tcDismiss_.onClick = [this] { dismissTransition(); };
    addAndMakeVisible (tcDismiss_);

    tcStart_.setButtonText (juce::String::fromUTF8 ("\345\274\200\345\247\213\346\267\267\351\237\263"));
    tcStart_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    tcStart_.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.2f));
    tcStart_.onClick = [this] { startMix(); };
    addAndMakeVisible (tcStart_);

    // ═══════════════════════════════════════════════════════════
    // ── 2. Main interface ─────────────────────────────────────
    // ═══════════════════════════════════════════════════════════

    // 2a. Header
    title_.setText ("HonestMix", juce::dontSendNotification);
    title_.setFont (juce::Font (juce::FontOptions (11.0f)).boldened());
    title_.setJustificationType (juce::Justification::centred);
    title_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.6f));
    addAndMakeVisible (title_);

    version_.setText ("v0.1.0", juce::dontSendNotification);
    version_.setFont (juce::Font (juce::FontOptions (6.0f)));
    version_.setJustificationType (juce::Justification::right);
    version_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.1f));
    addAndMakeVisible (version_);

    // 2b. Info row: 耳机 / 曲线 / 声卡
    hpLbl_.setText (juce::String::fromUTF8 ("\350\200\263\346\234\272"), juce::dontSendNotification);
    hpLbl_.setFont (juce::Font (juce::FontOptions (6.0f)));
    hpLbl_.setJustificationType (juce::Justification::centred);
    hpLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    addAndMakeVisible (hpLbl_);

    hpVal_.setFont (juce::Font (juce::FontOptions (9.0f)));
    hpVal_.setJustificationType (juce::Justification::centred);
    hpVal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.45f));
    hpVal_.addMouseListener (this, false); // ← click to cycle profiles
    addAndMakeVisible (hpVal_);

    cvLbl_.setText (juce::String::fromUTF8 ("\346\233\262\347\272\277"), juce::dontSendNotification);
    cvLbl_.setFont (juce::Font (juce::FontOptions (6.0f)));
    cvLbl_.setJustificationType (juce::Justification::centred);
    cvLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    addAndMakeVisible (cvLbl_);

    cvVal_.setText ("Harman OE", juce::dontSendNotification);
    cvVal_.setFont (juce::Font (juce::FontOptions (9.0f)));
    cvVal_.setJustificationType (juce::Justification::centred);
    cvVal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.45f));
    addAndMakeVisible (cvVal_);

    ifLbl_.setText (juce::String::fromUTF8 ("\345\243\260\345\215\241"), juce::dontSendNotification);
    ifLbl_.setFont (juce::Font (juce::FontOptions (6.0f)));
    ifLbl_.setJustificationType (juce::Justification::centred);
    ifLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    addAndMakeVisible (ifLbl_);

    ifVal_.setText ("RME", juce::dontSendNotification);
    ifVal_.setFont (juce::Font (juce::FontOptions (9.0f)));
    ifVal_.setJustificationType (juce::Justification::centred);
    ifVal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.45f));
    addAndMakeVisible (ifVal_);

    // 2c. Dry/Wet knob
    knob_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob_.setRange (0, 100, 1);
    knob_.setValue (processorRef_.getDryWetParam()->get(), juce::dontSendNotification);
    knob_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob_.setLookAndFeel (&knobLNF);
    knob_.setRotaryParameters (juce::MathConstants<float>::pi * 0.75f,
                               juce::MathConstants<float>::pi * 2.25f, true);
    knob_.onValueChange = [this] { *processorRef_.getDryWetParam() = (float) knob_.getValue(); };
    addAndMakeVisible (knob_);

    knobVal_.setFont (juce::Font (juce::FontOptions (18.0f)));
    knobVal_.setJustificationType (juce::Justification::centred);
    knobVal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.45f));
    addAndMakeVisible (knobVal_);

    knobLbl_.setText (juce::String::fromUTF8 ("\345\271\262\346\271\277\346\257\224"), juce::dontSendNotification);
    knobLbl_.setFont (juce::Font (juce::FontOptions (8.0f)));
    knobLbl_.setJustificationType (juce::Justification::centred);
    knobLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.1f));
    addAndMakeVisible (knobLbl_);

    // 2d. Correction toggle + BPM trigger
    corrLbl_.setText (juce::String::fromUTF8 ("\346\240\241\346\255\243"), juce::dontSendNotification);
    corrLbl_.setFont (juce::Font (juce::FontOptions (7.0f)));
    corrLbl_.setJustificationType (juce::Justification::centred);
    corrLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.2f));
    addAndMakeVisible (corrLbl_);

    corrBtn_.setText ("ON", juce::dontSendNotification);
    corrBtn_.setFont (juce::Font (juce::FontOptions (8.0f)));
    corrBtn_.setJustificationType (juce::Justification::centred);
    corrBtn_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    corrBtn_.addMouseListener (this, false);
    addAndMakeVisible (corrBtn_);

    bpmLbl_.setText ("BPM", juce::dontSendNotification);
    bpmLbl_.setFont (juce::Font (juce::FontOptions (7.0f)));
    bpmLbl_.setJustificationType (juce::Justification::centred);
    bpmLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.2f));
    bpmLbl_.addMouseListener (this, false);
    addAndMakeVisible (bpmLbl_);

    // 2e. 诚 stamp (feedback trigger)
    seal_.setText (juce::String::fromUTF8 ("\350\257\232"), juce::dontSendNotification);
    seal_.setFont (juce::Font (juce::FontOptions (18.0f)));
    seal_.setJustificationType (juce::Justification::centred);
    seal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.05f));
    seal_.addMouseListener (this, false);
    addAndMakeVisible (seal_);

    // ═══════════════════════════════════════════════════════════
    // ── 3. BPM panel (hidden by default) ──────────────────────
    // ═══════════════════════════════════════════════════════════

    bpmTitle_.setText (juce::String::fromUTF8 ("BPM \345\212\251\346\211\213"), juce::dontSendNotification);
    bpmTitle_.setFont (juce::Font (juce::FontOptions (10.0f)));
    bpmTitle_.setJustificationType (juce::Justification::centred);
    bpmTitle_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.4f));
    bpmTitle_.setVisible (false);
    addAndMakeVisible (bpmTitle_);

    bpmClose_.setText ("X", juce::dontSendNotification);
    bpmClose_.setFont (juce::Font (juce::FontOptions (8.0f)));
    bpmClose_.setJustificationType (juce::Justification::centred);
    bpmClose_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.06f));
    bpmClose_.addMouseListener (this, false);
    bpmClose_.setVisible (false);
    addAndMakeVisible (bpmClose_);

    bpmInput_.setMultiLine (false);
    bpmInput_.setFont (juce::Font (juce::FontOptions (16.0f)));
    bpmInput_.setText ("117", false);
    bpmInput_.setColour (juce::TextEditor::textColourId, juce::Colours::white.withAlpha (0.35f));
    bpmInput_.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    bpmInput_.setColour (juce::TextEditor::outlineColourId, juce::Colours::white.withAlpha (0.03f));
    bpmInput_.setInputRestrictions (3, "0123456789");
    bpmInput_.setVisible (false);
    bpmInput_.onTextChange = [this] {
        const int v = bpmInput_.getText().getIntValue();
        if (v >= 40 && v <= 240)
            updateBPM (v);
    };
    addAndMakeVisible (bpmInput_);

    bpmTap_.setText (juce::String::fromUTF8 ("\346\214\211\351\200\237\345\272\246"), juce::dontSendNotification);
    bpmTap_.setFont (juce::Font (juce::FontOptions (7.0f)));
    bpmTap_.setJustificationType (juce::Justification::centred);
    bpmTap_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.1f));
    bpmTap_.addMouseListener (this, false);
    bpmTap_.setVisible (false);
    addAndMakeVisible (bpmTap_);

    bpmDisplay_.setFont (juce::Font (juce::FontOptions (7.0f)));
    bpmDisplay_.setJustificationType (juce::Justification::centred);
    bpmDisplay_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.15f));
    bpmDisplay_.setVisible (false);
    addAndMakeVisible (bpmDisplay_);

    // ═══════════════════════════════════════════════════════════
    // ── 4. Feedback overlay (hidden) ──────────────────────────
    // ═══════════════════════════════════════════════════════════

    fbOverlay_.setText ("", juce::dontSendNotification);
    fbOverlay_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.0f));
    fbOverlay_.addMouseListener (this, false);
    fbOverlay_.setVisible (false);
    addAndMakeVisible (fbOverlay_);

    fbTitle_.setText (juce::String::fromUTF8 ("\347\277\273\350\257\221\345\272\246\345\217\215\351\246\210"), juce::dontSendNotification);
    fbTitle_.setFont (juce::Font (juce::FontOptions (10.0f)));
    fbTitle_.setJustificationType (juce::Justification::centred);
    fbTitle_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.35f));
    fbTitle_.setVisible (false);
    addAndMakeVisible (fbTitle_);

    const auto fbTx = [] (float alpha) { return juce::Colours::white.withAlpha (alpha); };

    auto mkFB = [&] (juce::TextButton& btn, const char* textUtf8, float alpha)
    {
        btn.setButtonText (juce::String::fromUTF8 (textUtf8));
        btn.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn.setColour (juce::TextButton::textColourOffId, fbTx (alpha));
        btn.setClickingTogglesState (true);
        btn.setVisible (false);
        btn.addMouseListener (this, false);
        addAndMakeVisible (btn);
    };
    mkFB (fbBassOk_,    "\344\275\216\351\242\221\345\210\232\345\245\275", 0.2f);
    mkFB (fbBassMore_,  "\344\275\216\351\242\221\345\244\232\344\272\206", 0.2f);
    mkFB (fbBassLess_,  "\344\275\216\351\242\221\345\260\221\344\272\206", 0.2f);
    mkFB (fbTrebleOk_,    "\351\253\230\351\242\221\345\210\232\345\245\275", 0.2f);
    mkFB (fbTrebleBright_, "\351\253\230\351\242\221\344\272\256\344\272\206", 0.2f);
    mkFB (fbTrebleDark_, "\351\253\230\351\242\221\346\232\227\344\272\206", 0.2f);
    fbBassOk_.setToggleState (true, juce::dontSendNotification);
    fbTrebleOk_.setToggleState (true, juce::dontSendNotification);

    fbSubmit_.setButtonText (juce::String::fromUTF8 ("\346\217\220\344\272\244"));
    fbSubmit_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    fbSubmit_.setColour (juce::TextButton::textColourOffId, fbTx (0.15f));
    fbSubmit_.setVisible (false);
    fbSubmit_.addMouseListener (this, false);
    addAndMakeVisible (fbSubmit_);

    fbClose_.setButtonText (juce::String::fromUTF8 ("\345\205\263\351\227\255"));
    fbClose_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    fbClose_.setColour (juce::TextButton::textColourOffId, fbTx (0.06f));
    fbClose_.setVisible (false);
    fbClose_.addMouseListener (this, false);
    addAndMakeVisible (fbClose_);

    // ═══════════════════════════════════════════════════════════
    // ── 5. 1-hour check overlay (hidden) ──────────────────────
    // ═══════════════════════════════════════════════════════════

    auto mkChk = [&] (juce::Label& lbl, const char* textUtf8, float alpha)
    {
        lbl.setText (juce::String::fromUTF8 (textUtf8), juce::dontSendNotification);
        lbl.setFont (juce::Font (juce::FontOptions (8.0f)));
        lbl.setJustificationType (juce::Justification::centred);
        lbl.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (alpha));
        lbl.setVisible (false);
        lbl.addMouseListener (this, false);
        addAndMakeVisible (lbl);
    };
    mkChk (chkOverlay_, "", 0.0f);
    mkChk (chkTitle_,  "\346\267\267\351\237\2631\345\260\217\346\227\266\357\274\214\346\243\200\346\237\245\347\233\221\345\220\254\347\216\257\345\242\203\357\274\237", 0.3f);
    mkChk (chkOpt1_,   "\345\210\207\346\215\242\345\215\225\345\243\260\351\201\223", 0.12f);
    mkChk (chkOpt2_,   "\345\257\274\345\207\272\345\210\260\346\211\213\346\234\272", 0.12f);
    mkChk (chkOpt3_,   "\345\210\260\350\275\246/\351\237\263\345\223\215\345\220\254", 0.12f);
    mkChk (chkOpt4_,   "\347\273\247\347\273\255\346\267\267\357\274\214\344\270\215\346\211\223\346\211\260", 0.12f);

    // ═══════════════════════════════════════════════════════════
    // ── 6. Share card overlay (hidden) ────────────────────────
    // ═══════════════════════════════════════════════════════════

    mkChk (shareOverlay_, "", 0.0f);
    mkChk (shareTitle_,  "HonestMix  \342\200\224  \347\277\273\350\257\221\345\272\246\345\267\262\347\241\256\350\256\244", 0.3f);

    shareWave_.setText (juce::String::fromUTF8 ("\343\200\224\346\263\242\345\275\242\345\233\276\343\200\225"), juce::dontSendNotification);
    shareWave_.setFont (juce::Font (juce::FontOptions (7.0f)));
    shareWave_.setJustificationType (juce::Justification::centred);
    shareWave_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.04f));
    shareWave_.setVisible (false);
    addAndMakeVisible (shareWave_);

    shareProject_.setText (juce::String::fromUTF8 ("\343\200\212\346\234\252\345\221\275\345\220\215\345\267\245\347\250\213\343\200\213"), juce::dontSendNotification);
    shareProject_.setFont (juce::Font (juce::FontOptions (8.0f)));
    shareProject_.setJustificationType (juce::Justification::centred);
    shareProject_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.2f));
    shareProject_.setVisible (false);
    addAndMakeVisible (shareProject_);

    shareBody_.setFont (juce::Font (juce::FontOptions (7.0f)));
    shareBody_.setJustificationType (juce::Justification::centred);
    shareBody_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.1f));
    shareBody_.setVisible (false);
    addAndMakeVisible (shareBody_);

    mkChk (shareClose_, "\346\224\266\350\265\267", 0.08f);

    // ═══════════════════════════════════════════════════════════
    // ── 7. Restore saved state ────────────────────────────────
    // ═══════════════════════════════════════════════════════════
    {
        auto& engine = processorRef_.getCorrectionEngine();
        const int savedIdx = processorRef_.getProfileIndex();
        engine.setProfile (savedIdx);
        hpVal_.setText (profileNames_[savedIdx], juce::dontSendNotification);
        cvVal_.setText (profileCurves_[savedIdx], juce::dontSendNotification);
    }

    updateShareBody();
    setSize (PW, PH);

    // Audio-thread-safe timer: 20 Hz polling for parameter sync.
    startTimerHz (20);
}

//==============================================================================
HonestMixAudioProcessorEditor::~HonestMixAudioProcessorEditor()
{
    // Must reset the LNF before the slider is destroyed.
    knob_.setLookAndFeel (nullptr);
}

//==============================================================================
void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 22));
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 6));
    g.fillRect (getLocalBounds().removeFromTop (1));
}

//==============================================================================
void HonestMixAudioProcessorEditor::resized()
{
    auto a = getLocalBounds();

    // ── Transition card ──────────────────────────────────────────
    tcSeal_.setBounds (a.getCentreX() - 50, 90, 100, 40);
    tcTitle_.setBounds (a.getCentreX() - 80, 128, 160, 20);
    tc50_.setBounds (a.getCentreX() - 80, 156, 75, 26);
    tc100_.setBounds (a.getCentreX() + 5, 156, 75, 26);
    tcHow_.setBounds (a.getCentreX() - 80, 192, 160, 60);
    tcDismiss_.setBounds (a.getCentreX() - 80, 260, 75, 18);
    tcStart_.setBounds (a.getCentreX() + 5, 260, 75, 18);

    // ── Main interface ──────────────────────────────────────────
    title_.setBounds (20, 12, PW - 40, 16);
    version_.setBounds (PW - 60, 12, 40, 12);

    // Info row: 耳机 | 曲线 | 声卡
    const int ix = (PW - INFO_COL_W * 3 - INFO_GAP * 2) / 2;
    hpLbl_.setBounds (ix, INFO_Y, INFO_COL_W, 10);
    hpVal_.setBounds (ix, INFO_Y + 10, INFO_COL_W, 14);
    cvLbl_.setBounds (ix + INFO_COL_W + INFO_GAP, INFO_Y, INFO_COL_W, 10);
    cvVal_.setBounds (ix + INFO_COL_W + INFO_GAP, INFO_Y + 10, INFO_COL_W, 14);
    ifLbl_.setBounds (ix + (INFO_COL_W + INFO_GAP) * 2, INFO_Y, INFO_COL_W, 10);
    ifVal_.setBounds (ix + (INFO_COL_W + INFO_GAP) * 2, INFO_Y + 10, INFO_COL_W, 14);

    // Knob
    knob_.setBounds ((PW - KNOB_SIZE) / 2, 68, KNOB_SIZE, KNOB_SIZE);
    knobVal_.setBounds ((PW - 60) / 2, 68 + KNOB_SIZE + 2, 60, 16);
    knobLbl_.setBounds ((PW - 60) / 2, 68 + KNOB_SIZE + 18, 60, 14);

    // Bottom row
    corrLbl_.setBounds (24, BOTTOM_Y, 36, 14);
    corrBtn_.setBounds (28, BOTTOM_Y + 16, 28, 20);
    bpmLbl_.setBounds (PW - 50, BOTTOM_Y + 18, 30, 14);

    // 诚 seal
    seal_.setBounds (PW - SEAL_SIZE - SEAL_MARGIN,
                     PH - SEAL_SIZE - SEAL_MARGIN,
                     SEAL_SIZE, SEAL_SIZE);

    // ── BPM panel ───────────────────────────────────────────────
    constexpr int bx = 72, by = 10, bw = 200, bh = 260;
    bpmTitle_.setBounds (bx + 8, by + 8, bw - 16, 16);
    bpmClose_.setBounds (bx + bw - 24, by + 6, 20, 16);
    bpmInput_.setBounds (bx + 8, by + 30, 110, 28);
    bpmTap_.setBounds (bx + 124, by + 30, 60, 28);
    bpmDisplay_.setBounds (bx + 8, by + 64, bw - 16, bh - 70);

    // ── Feedback overlay ────────────────────────────────────────
    fbOverlay_.setBounds (0, 0, PW, PH);
    fbTitle_.setBounds (PW / 2 - 60, 100, 120, 20);

    constexpr int fbY = 130;
    fbBassOk_.setBounds (PW / 2 - 60, fbY, 50, 22);
    fbBassMore_.setBounds (PW / 2 - 8,  fbY, 38, 22);
    fbBassLess_.setBounds (PW / 2 + 32, fbY, 38, 22);

    fbTrebleOk_.setBounds (PW / 2 - 60, fbY + 28, 50, 22);
    fbTrebleBright_.setBounds (PW / 2 - 8,  fbY + 28, 38, 22);
    fbTrebleDark_.setBounds (PW / 2 + 32, fbY + 28, 38, 22);

    fbSubmit_.setBounds (PW / 2 - 58, fbY + 60, 50, 22);
    fbClose_.setBounds (PW / 2 + 8,  fbY + 60, 50, 22);

    // ── 1-hour check ────────────────────────────────────────────
    chkOverlay_.setBounds (0, 0, PW, PH);
    chkTitle_.setBounds (PW / 2 - 80, 100, 160, 20);
    chkOpt1_.setBounds (PW / 2 - 60, 135, 120, 18);
    chkOpt2_.setBounds (PW / 2 - 60, 158, 120, 18);
    chkOpt3_.setBounds (PW / 2 - 60, 181, 120, 18);
    chkOpt4_.setBounds (PW / 2 - 60, 204, 120, 18);

    // ── Share card ──────────────────────────────────────────────
    shareOverlay_.setBounds (0, 0, PW, PH);
    shareTitle_.setBounds (PW / 2 - 80, 80, 160, 18);
    shareWave_.setBounds (PW / 2 - 60, 104, 120, 16);
    shareProject_.setBounds (PW / 2 - 60, 122, 120, 16);
    shareBody_.setBounds (PW / 2 - 80, 145, 160, 60);
    shareClose_.setBounds (PW / 2 - 30, 210, 60, 20);
}

//==============================================================================
void HonestMixAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    auto* c = e.eventComponent;

    // ── Headphone profile cycle ──────────────────────────────────
    if (c == &hpVal_)
    {
        auto& engine = processorRef_.getCorrectionEngine();
        const int next = (engine.getCurrentProfile() + 1) % engine.getNumProfiles();
        engine.setProfile (next);
        processorRef_.setProfileIndex (next);
        hpVal_.setText (profileNames_[next], juce::dontSendNotification);
        cvVal_.setText (profileCurves_[next], juce::dontSendNotification);
        updateShareBody();
    }
    // ── Correction toggle ───────────────────────────────────────
    else if (c == &corrBtn_)
    {
        toggleCorrection();
    }
    // ── BPM helper toggle ───────────────────────────────────────
    else if (c == &bpmLbl_)
    {
        toggleBPM();
    }
    // ── BPM close ───────────────────────────────────────────────
    else if (c == &bpmClose_)
    {
        if (showBPM_)
            toggleBPM();
    }
    // ── 诚 stamp → feedback overlay toggle ─────────────────────
    else if (c == &seal_)
    {
        showFB_ = ! showFB_;
        showFB_ ? showFeedback() : hideFeedback();
    }
    // ── Feedback: close via overlay or button ───────────────────
    else if (c == &fbOverlay_ || c == &fbClose_)
    {
        showFB_ = false;
        hideFeedback();
    }
    // ── Feedback: submit ────────────────────────────────────────
    else if (c == &fbSubmit_)
    {
        showFB_ = false;
        hideFeedback();
        submitFeedback();
    }
    // ── Feedback: bass ──────────────────────────────────────────
    else if (c == &fbBassOk_)    { fbBass_ = FeedbackValue::okay;     toggleFBButtons(); }
    else if (c == &fbBassMore_)  { fbBass_ = FeedbackValue::positive; toggleFBButtons(); }
    else if (c == &fbBassLess_)  { fbBass_ = FeedbackValue::negative; toggleFBButtons(); }
    // ── Feedback: treble ────────────────────────────────────────
    else if (c == &fbTrebleOk_)      { fbTreble_ = FeedbackValue::okay;     toggleFBButtons(); }
    else if (c == &fbTrebleBright_)  { fbTreble_ = FeedbackValue::positive; toggleFBButtons(); }
    else if (c == &fbTrebleDark_)    { fbTreble_ = FeedbackValue::negative; toggleFBButtons(); }
    // ── 1-hour check: any option closes ─────────────────────────
    else if (c == &chkOpt1_ || c == &chkOpt2_ || c == &chkOpt3_ || c == &chkOpt4_ || c == &chkOverlay_)
    {
        showChk_ = false;
        chkOverlay_.setVisible (false);
        chkTitle_.setVisible (false);
        chkOpt1_.setVisible (false);
        chkOpt2_.setVisible (false);
        chkOpt3_.setVisible (false);
        chkOpt4_.setVisible (false);

        if (c == &chkOpt2_)
            showShare(); // "导出到手机" → shows share card
    }
    // ── Share card close ────────────────────────────────────────
    else if (c == &shareOverlay_ || c == &shareClose_)
    {
        closeShare();
    }
    // ── BPM tap ─────────────────────────────────────────────────
    else if (c == &bpmTap_)
    {
        auto now = juce::Time::getCurrentTime();
        taps_.add (now);
        if (taps_.size() > 8)
            taps_.remove (0);

        if (taps_.size() >= 3)
        {
            auto ms = (taps_.getLast().toMilliseconds()
                       - taps_.getFirst().toMilliseconds())
                      / (taps_.size() - 1);
            if (ms > 0)
                updateBPM (juce::jlimit (40, 240, static_cast<int> (60000.0 / ms + 0.5)));
        }
    }
}

//==============================================================================
// ── Transition helpers ────────────────────────────────────────

void HonestMixAudioProcessorEditor::dismissTransition()
{
    showTrans_ = false;
    juce::Component* comps[] = { &tcSeal_, &tcTitle_, &tc50_, &tc100_,
                                 &tcHow_, &tcDismiss_, &tcStart_ };
    for (auto* c : comps)
        c->setVisible (false);
}

void HonestMixAudioProcessorEditor::startMix()
{
    dismissTransition();
    *processorRef_.getDryWetParam() = static_cast<float> (dryWetChoice_);
}

//==============================================================================
// ── Correction toggle ─────────────────────────────────────────

void HonestMixAudioProcessorEditor::toggleCorrection()
{
    *processorRef_.getCorrectionParam() = ! processorRef_.getCorrectionParam()->get();
}

//==============================================================================
// ── Feedback overlay ──────────────────────────────────────────

void HonestMixAudioProcessorEditor::showFeedback()
{
    fbOverlay_.setVisible (true);
    fbTitle_.setVisible (true);
    fbBassOk_.setVisible (true);
    fbBassMore_.setVisible (true);
    fbBassLess_.setVisible (true);
    fbTrebleOk_.setVisible (true);
    fbTrebleBright_.setVisible (true);
    fbTrebleDark_.setVisible (true);
    fbSubmit_.setVisible (true);
    fbClose_.setVisible (true);
}

void HonestMixAudioProcessorEditor::hideFeedback()
{
    fbOverlay_.setVisible (false);
    fbTitle_.setVisible (false);
    fbBassOk_.setVisible (false);
    fbBassMore_.setVisible (false);
    fbBassLess_.setVisible (false);
    fbTrebleOk_.setVisible (false);
    fbTrebleBright_.setVisible (false);
    fbTrebleDark_.setVisible (false);
    fbSubmit_.setVisible (false);
    fbClose_.setVisible (false);
}

void HonestMixAudioProcessorEditor::toggleFBButtons()
{
    auto setBass = [this] (FeedbackValue v)
    {
        fbBassOk_.setToggleState   (v == FeedbackValue::okay,     juce::dontSendNotification);
        fbBassMore_.setToggleState (v == FeedbackValue::positive, juce::dontSendNotification);
        fbBassLess_.setToggleState (v == FeedbackValue::negative, juce::dontSendNotification);
    };
    auto setTreble = [this] (FeedbackValue v)
    {
        fbTrebleOk_.setToggleState      (v == FeedbackValue::okay,     juce::dontSendNotification);
        fbTrebleBright_.setToggleState  (v == FeedbackValue::positive, juce::dontSendNotification);
        fbTrebleDark_.setToggleState    (v == FeedbackValue::negative, juce::dontSendNotification);
    };
    setBass (fbBass_);
    setTreble (fbTreble_);
}

void HonestMixAudioProcessorEditor::submitFeedback()
{
    auto& engine = processorRef_.getCorrectionEngine();

    auto* obj = new juce::DynamicObject();
    obj->setProperty ("headphone",  juce::var (engine.getProfileName (engine.getCurrentProfile())));
    obj->setProperty ("interface",  juce::var ("RME"));
    obj->setProperty ("drywet",     static_cast<double> (processorRef_.getDryWetParam()->get()));
    obj->setProperty ("correction", processorRef_.getCorrectionParam()->get());

    // Map enum → Supabase text values.
    static constexpr const char* bassMap[]   = { "too_little", "okay", "too_much" };
    static constexpr const char* trebleMap[] = { "too_dark", "okay", "too_bright" };
    // The enum values are -1, 0, 1 → shift by +1 for array index.
    auto fbToIdx = [] (FeedbackValue v) { return static_cast<int> (v) + 1; };
    obj->setProperty ("bass_fb",   juce::var (bassMap  [fbToIdx (fbBass_)]));
    obj->setProperty ("treble_fb", juce::var (trebleMap[fbToIdx (fbTreble_)]));
    obj->setProperty ("comment",   juce::var (""));

    feedbackClient_.sendFeedback (juce::var (obj));
}

//==============================================================================
// ── Share card ────────────────────────────────────────────────

void HonestMixAudioProcessorEditor::showShare()
{
    shareOverlay_.setVisible (true);
    shareTitle_.setVisible (true);
    shareWave_.setVisible (true);
    shareProject_.setVisible (true);
    shareBody_.setVisible (true);
    shareClose_.setVisible (true);
}

void HonestMixAudioProcessorEditor::closeShare()
{
    shareOverlay_.setVisible (false);
    shareTitle_.setVisible (false);
    shareWave_.setVisible (false);
    shareProject_.setVisible (false);
    shareBody_.setVisible (false);
    shareClose_.setVisible (false);
}

void HonestMixAudioProcessorEditor::updateShareBody()
{
    auto& engine = processorRef_.getCorrectionEngine();
    const int idx = engine.getCurrentProfile();

    shareBody_.setText (
        juce::String (profileNames_[idx]) + " | "
        + juce::String (profileCurves_[idx]) + " | "
        + juce::String (static_cast<int> (processorRef_.getDryWetParam()->get()))
        + "%\n\n"
        + juce::String::fromUTF8 ("\343\200\224\346\211\253\347\240\201\343\200\225 \346\211\253\347\240\201\344\270\213\350\275\275 HonestMix\n")
        + juce::String::fromUTF8 ("\345\205\215\350\264\271 \302\267 \345\274\200\346\272\220 \302\267 \347\244\276\345\214\272\351\251\261\345\212\250"),
        juce::dontSendNotification);
}

//==============================================================================
// ── BPM helper ────────────────────────────────────────────────

void HonestMixAudioProcessorEditor::toggleBPM()
{
    showBPM_ = ! showBPM_;
    bpmTitle_.setVisible (showBPM_);
    bpmClose_.setVisible (showBPM_);
    bpmInput_.setVisible (showBPM_);
    bpmTap_.setVisible (showBPM_);
    bpmDisplay_.setVisible (showBPM_);
}

void HonestMixAudioProcessorEditor::updateBPM (int bpm)
{
    curBPM_ = bpm;
    const double ms = 60000.0 / bpm;

    juce::String t;
    t += "BPM: " + juce::String (bpm) + "    "
         + juce::String::fromUTF8 ("\346\257\217\346\213\215") + ": "
         + juce::String (static_cast<int> (ms)) + " ms\n\n";
    t += juce::String::fromUTF8 ("\351\242\204\345\273\266\350\277\237") + ": "
         + juce::String (static_cast<int> (ms / 32 + 0.5)) + "/"
         + juce::String (static_cast<int> (ms / 16 + 0.5)) + "/"
         + juce::String (static_cast<int> (ms / 8 + 0.5)) + " ms\n";
    t += juce::String::fromUTF8 ("\345\273\266\350\277\237") + ": "
         + "1/8=" + juce::String (static_cast<int> (ms / 4 + 0.5))
         + " 1/4=" + juce::String (static_cast<int> (ms / 2 + 0.5))
         + " 1/2=" + juce::String (static_cast<int> (ms + 0.5)) + " ms\n\n";
    t += "Room " + juce::String (ms * 0.5 / 1000.0, 2)
         + "s  Plate " + juce::String (ms * 2.0 / 1000.0, 2)
         + "s  Hall " + juce::String (ms * 4.0 / 1000.0, 2) + "s";

    bpmDisplay_.setText (t, juce::dontSendNotification);
}

//==============================================================================
// ── Timer: parameter sync + deferred checks ───────────────────

void HonestMixAudioProcessorEditor::timerCallback()
{
    auto& dw = *processorRef_.getDryWetParam();
    auto& co = *processorRef_.getCorrectionParam();

    // Sync knob to parameter (DAW automation may change it externally).
    const double pv = dw.get();
    if (std::abs (knob_.getValue() - pv) > 0.5)
        knob_.setValue (pv, juce::dontSendNotification);

    knobVal_.setText (juce::String (static_cast<int> (pv)) + " %",
                      juce::dontSendNotification);

    corrBtn_.setText (co.get() ? "ON" : "OFF", juce::dontSendNotification);
    corrBtn_.setColour (juce::Label::textColourId,
                        juce::Colours::white.withAlpha (co.get() ? 0.12f : 0.04f));

    // Only update share card when dry/wet ratio changes.
    static double lastDw = -1.0;
    if (std::abs (lastDw - pv) > 0.5)
    {
        lastDw = pv;
        updateShareBody();
    }

    // 1-hour check (simulated: 1200 ticks × 50 ms ≈ 60 s).
    if (! showTrans_ && ! showChk_ && ++tick_ == 1200)
    {
        tick_ = 0;
        showChk_ = true;
        chkOverlay_.setVisible (true);
        chkTitle_.setVisible (true);
        chkOpt1_.setVisible (true);
        chkOpt2_.setVisible (true);
        chkOpt3_.setVisible (true);
        chkOpt4_.setVisible (true);
    }
}
