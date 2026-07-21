#include "PluginEditor.h"

static constexpr int PW = 280, PH = 430;

class KnobLNF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float sa, float ea, juce::Slider&) override
    {
        auto cx = x + w / 2.0f, cy = y + h / 2.0f, r = (float) juce::jmin (w, h) / 2.0f - 4.0f;
        auto a  = sa + (ea - sa) * pos;
        g.setColour (juce::Colour::fromRGBA (255,255,255,10));
        g.drawEllipse (cx - r, cy - r, r * 2, r * 2, 1.5f);
        juce::Path arc; arc.addArc (cx - r + 2, cy - r + 2, r * 2 - 4, r * 2 - 4, sa, a, true);
        g.setColour (juce::Colour::fromRGBA (255,255,255,30));
        g.strokePath (arc, juce::PathStrokeType (2));
        float px = cx + (r - 2) * std::cos (a), py = cy + (r - 2) * std::sin (a);
        g.setColour (juce::Colour::fromRGBA (255,255,255,45));
        g.fillEllipse (px - 2.5f, py - 2.5f, 5, 5);
    }
};
static KnobLNF knobLNF;

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef_ (p)
{
    // ── 过渡卡 ──
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
        "\342\221\242 \344\275\240\345\217\215\351\246\210\342\200\224\346\225\260\346\215\256\350\277\255\344\273\243"
    ), juce::dontSendNotification);
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

    // ── 主界面 ──
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

    hpLbl_.setText (juce::String::fromUTF8 ("\350\200\263\346\234\272"), juce::dontSendNotification);
    hpLbl_.setFont (juce::Font (juce::FontOptions (6.0f)));
    hpLbl_.setJustificationType (juce::Justification::centred);
    hpLbl_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    addAndMakeVisible (hpLbl_);
    hpVal_.setText ("ATH-M50X", juce::dontSendNotification);
    hpVal_.setFont (juce::Font (juce::FontOptions (9.0f)));
    hpVal_.setJustificationType (juce::Justification::centred);
    hpVal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.45f));
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

    // ── 旋钮 ──
    knob_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob_.setRange (0, 100, 1);
    knob_.setValue (processorRef_.getDryWetParam()->get(), juce::dontSendNotification);
    knob_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob_.setLookAndFeel (&knobLNF);
    knob_.setRotaryParameters (juce::MathConstants<float>::pi * 0.75f, juce::MathConstants<float>::pi * 2.25f, true);
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

    // ── 校正开关 ──
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

    // ── 宠物「诚」 ──
    seal_.setText (juce::String::fromUTF8 ("\350\257\232"), juce::dontSendNotification);
    seal_.setFont (juce::Font (juce::FontOptions (18.0f)));
    seal_.setJustificationType (juce::Justification::centred);
    seal_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.05f));
    seal_.addMouseListener (this, false);
    addAndMakeVisible (seal_);

    // ── BPM 面板 ──
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
        int v = bpmInput_.getText().getIntValue();
        if (v >= 40 && v <= 240) updateBPM (v);
    };
    addAndMakeVisible (bpmInput_);

    bpmTap_.setText (juce::String::fromUTF8 ("\346\213\215\350\212\202\346\213\215"), juce::dontSendNotification);
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

    // ── 反馈弹窗 ──
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

    const auto fbBg = [] (auto alpha) { return juce::Colours::white.withAlpha (alpha); };
    const auto fbTx = [] (auto alpha) { return juce::Colours::white.withAlpha (alpha); };

    auto mkFB = [&] (juce::TextButton& btn, const char* t, float a)
    {
        btn.setButtonText (juce::String::fromUTF8 (t));
        btn.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn.setColour (juce::TextButton::textColourOffId, fbTx (a));
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

    // ── 1小时检查 ──
    auto mkChk = [&] (juce::Label& l, const char* t, float a)
    {
        l.setText (juce::String::fromUTF8 (t), juce::dontSendNotification);
        l.setFont (juce::Font (juce::FontOptions (8.0f)));
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (a));
        l.setVisible (false);
        l.addMouseListener (this, false);
        addAndMakeVisible (l);
    };
    mkChk (chkOverlay_, "", 0.0f);
    mkChk (chkTitle_,  "\346\267\267\351\237\2631\345\260\217\346\227\266", 0.3f);
    mkChk (chkOpt1_,   "\345\210\207\346\215\242\345\215\225\345\243\260\351\201\223", 0.12f);
    mkChk (chkOpt2_,   "\345\257\274\345\207\272\345\210\260\346\211\213\346\234\272", 0.12f);
    mkChk (chkOpt3_,   "\345\210\260\350\275\246/\351\237\263\345\223\215\345\220\254", 0.12f);
    mkChk (chkOpt4_,   "\347\273\247\347\273\255\346\267\267\357\274\214\344\270\215\346\211\223\346\211\260", 0.12f);

    // ── 分享卡 ──
    mkChk (shareOverlay_, "", 0.0f);
    mkChk (shareTitle_,  "\346\267\267\351\237\263\345\256\214\346\210\220 \342\200\224 \345\210\206\344\272\253\344\275\240\347\232\204 EDM", 0.3f);
    shareBody_.setFont (juce::Font (juce::FontOptions (7.0f)));
    shareBody_.setJustificationType (juce::Justification::centred);
    shareBody_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.1f));
    shareBody_.setText (juce::String::fromUTF8 (
        "ATH-M50X  |  Harman OE  |  50%\n"
        "\346\211\253\347\240\201\344\270\213\350\275\275 HonestMix\n"
        "\345\205\215\350\264\271 \302\267 \345\274\200\346\272\220 \302\267 \347\244\276\345\214\272\351\251\261\345\212\250"),
        juce::dontSendNotification);
    shareBody_.setVisible (false);
    addAndMakeVisible (shareBody_);
    mkChk (shareClose_, "\346\224\266\350\265\267", 0.08f);

    setSize (PW, PH);
    startTimerHz (20);
}

HonestMixAudioProcessorEditor::~HonestMixAudioProcessorEditor()
{
    knob_.setLookAndFeel (nullptr);
}

void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 22));
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 6));
    g.fillRect (getLocalBounds().removeFromTop (1));
}

void HonestMixAudioProcessorEditor::resized()
{
    auto a = getLocalBounds();

    // 过渡卡
    tcSeal_.setBounds (a.getCentreX() - 50, 90, 100, 40);
    tcTitle_.setBounds (a.getCentreX() - 80, 128, 160, 20);
    tc50_.setBounds (a.getCentreX() - 80, 156, 75, 26);
    tc100_.setBounds (a.getCentreX() + 5, 156, 75, 26);
    tcHow_.setBounds (a.getCentreX() - 80, 192, 160, 60);
    tcDismiss_.setBounds (a.getCentreX() - 80, 260, 75, 18);
    tcStart_.setBounds (a.getCentreX() + 5, 260, 75, 18);

    // 主界面
    title_.setBounds (20, 12, PW - 40, 16);
    version_.setBounds (PW - 60, 12, 40, 12);

    int iy = 32, cw = 50, gap = 18, ix = (PW - cw * 3 - gap * 2) / 2;
    hpLbl_.setBounds (ix, iy, cw, 10); hpVal_.setBounds (ix, iy + 10, cw, 14);
    cvLbl_.setBounds (ix + cw + gap, iy, cw, 10); cvVal_.setBounds (ix + cw + gap, iy + 10, cw, 14);
    ifLbl_.setBounds (ix + (cw + gap) * 2, iy, cw, 10); ifVal_.setBounds (ix + (cw + gap) * 2, iy + 10, cw, 14);

    int ks = 72;
    knob_.setBounds ((PW - ks) / 2, 68, ks, ks);
    knobVal_.setBounds ((PW - 60) / 2, 68 + ks + 2, 60, 16);
    knobLbl_.setBounds ((PW - 60) / 2, 68 + ks + 18, 60, 14);

    corrLbl_.setBounds (24, 282, 36, 14);
    corrBtn_.setBounds (28, 298, 28, 20);
    bpmLbl_.setBounds (PW - 50, 300, 30, 14);

    seal_.setBounds (PW - 36, PH - 36, 30, 30);

    // BPM
    int bx = 72, by = 10, bw = 200, bh = 260;
    bpmTitle_.setBounds (bx + 8, by + 8, bw - 16, 16);
    bpmClose_.setBounds (bx + bw - 24, by + 6, 20, 16);
    bpmInput_.setBounds (bx + 8, by + 30, 110, 28);
    bpmTap_.setBounds (bx + 124, by + 30, 60, 28);
    bpmDisplay_.setBounds (bx + 8, by + 64, bw - 16, bh - 70);

    // ── 反馈弹窗 ──
    fbOverlay_.setBounds (0, 0, PW, PH);
    fbTitle_.setBounds (PW/2 - 60, 100, 120, 20);
    int fbY = 130;
    fbBassOk_.setBounds (PW/2 - 60, fbY, 50, 22); fbBassMore_.setBounds (PW/2 - 8, fbY, 38, 22); fbBassLess_.setBounds (PW/2 + 32, fbY, 38, 22);
    fbTrebleOk_.setBounds (PW/2 - 60, fbY + 28, 50, 22); fbTrebleBright_.setBounds (PW/2 - 8, fbY + 28, 38, 22); fbTrebleDark_.setBounds (PW/2 + 32, fbY + 28, 38, 22);
    fbSubmit_.setBounds (PW/2 - 30, fbY + 60, 60, 22);

    // ── 1小时检查 ──
    chkOverlay_.setBounds (0, 0, PW, PH);
    chkTitle_.setBounds (PW/2 - 80, 100, 160, 20);
    chkOpt1_.setBounds (PW/2 - 60, 135, 120, 18);
    chkOpt2_.setBounds (PW/2 - 60, 158, 120, 18);
    chkOpt3_.setBounds (PW/2 - 60, 181, 120, 18);
    chkOpt4_.setBounds (PW/2 - 60, 204, 120, 18);

    // ── 分享卡 ──
    shareOverlay_.setBounds (0, 0, PW, PH);
    shareTitle_.setBounds (PW/2 - 80, 100, 160, 20);
    shareBody_.setBounds (PW/2 - 80, 130, 160, 60);
    shareClose_.setBounds (PW/2 - 30, 200, 60, 20);
}

void HonestMixAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    auto* c = e.eventComponent;
    if (c == &corrBtn_)          { toggleCorrection(); }
    else if (c == &bpmLbl_)      { toggleBPM(); }
    else if (c == &bpmClose_)    { if (showBPM_) toggleBPM(); }
    else if (c == &seal_)        { showFB_ = ! showFB_; showFB_ ? showFeedback() : hideFeedback(); }
    else if (c == &fbOverlay_)   { showFB_ = false; hideFeedback(); }
    else if (c == &fbSubmit_)    { showFB_ = false; hideFeedback(); submitFeedback(); }
    else if (c == &chkOpt1_ || c == &chkOpt2_ || c == &chkOpt3_ || c == &chkOpt4_ || c == &chkOverlay_)
    { showChk_ = false; chkOverlay_.setVisible (false); chkTitle_.setVisible (false);
      chkOpt1_.setVisible (false); chkOpt2_.setVisible (false); chkOpt3_.setVisible (false); chkOpt4_.setVisible (false);
      if (c == &chkOpt2_) { shareOverlay_.setVisible (true); shareTitle_.setVisible (true); shareBody_.setVisible (true); shareClose_.setVisible (true); } }
    else if (c == &shareOverlay_ || c == &shareClose_)
    { shareOverlay_.setVisible (false); shareTitle_.setVisible (false); shareBody_.setVisible (false); shareClose_.setVisible (false); }
    else if (c == &fbBassOk_)    { fbBass_ = 0; toggleFBButtons(); }
    else if (c == &fbBassMore_)  { fbBass_ = 1; toggleFBButtons(); }
    else if (c == &fbBassLess_)  { fbBass_ = -1; toggleFBButtons(); }
    else if (c == &fbTrebleOk_)    { fbTreble_ = 0; toggleFBButtons(); }
    else if (c == &fbTrebleBright_) { fbTreble_ = 1; toggleFBButtons(); }
    else if (c == &fbTrebleDark_) { fbTreble_ = -1; toggleFBButtons(); }
    else if (c == &bpmTap_)
    {
        auto now = juce::Time::getCurrentTime();
        taps_.add (now);
        if (taps_.size() > 8) taps_.remove (0);
        if (taps_.size() >= 3)
        {
            auto ms = (taps_.getLast().toMilliseconds() - taps_.getFirst().toMilliseconds()) / (taps_.size() - 1);
            if (ms > 0) updateBPM (juce::jlimit (40, 240, (int) (60000.0 / ms + 0.5)));
        }
    }
}

void HonestMixAudioProcessorEditor::dismissTransition()
{
    showTrans_ = false;
    juce::Component* comps[] = { &tcSeal_, &tcTitle_, &tc50_, &tc100_, &tcHow_, &tcDismiss_, &tcStart_ };
    for (auto* c : comps) c->setVisible (false);
}

void HonestMixAudioProcessorEditor::startMix()
{
    dismissTransition();
    *processorRef_.getDryWetParam() = (float) dryWetChoice_;
}

void HonestMixAudioProcessorEditor::toggleCorrection()
{
    *processorRef_.getCorrectionParam() = ! processorRef_.getCorrectionParam()->get();
}

void HonestMixAudioProcessorEditor::showFeedback()
{
    fbOverlay_.setVisible (true); fbTitle_.setVisible (true);
    fbBassOk_.setVisible (true); fbBassMore_.setVisible (true); fbBassLess_.setVisible (true);
    fbTrebleOk_.setVisible (true); fbTrebleBright_.setVisible (true); fbTrebleDark_.setVisible (true);
    fbSubmit_.setVisible (true);
}

void HonestMixAudioProcessorEditor::hideFeedback()
{
    fbOverlay_.setVisible (false); fbTitle_.setVisible (false);
    fbBassOk_.setVisible (false); fbBassMore_.setVisible (false); fbBassLess_.setVisible (false);
    fbTrebleOk_.setVisible (false); fbTrebleBright_.setVisible (false); fbTrebleDark_.setVisible (false);
    fbSubmit_.setVisible (false);
}

void HonestMixAudioProcessorEditor::toggleFBButtons()
{
    fbBassOk_.setToggleState (fbBass_ == 0, juce::dontSendNotification);
    fbBassMore_.setToggleState (fbBass_ == 1, juce::dontSendNotification);
    fbBassLess_.setToggleState (fbBass_ == -1, juce::dontSendNotification);
    fbTrebleOk_.setToggleState (fbTreble_ == 0, juce::dontSendNotification);
    fbTrebleBright_.setToggleState (fbTreble_ == 1, juce::dontSendNotification);
    fbTrebleDark_.setToggleState (fbTreble_ == -1, juce::dontSendNotification);
}

void HonestMixAudioProcessorEditor::submitFeedback()
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("headphone",  juce::var ("ATH-M50X"));
    obj->setProperty ("interface",  juce::var ("RME"));
    obj->setProperty ("drywet",     (double) processorRef_.getDryWetParam()->get());
    obj->setProperty ("correction", processorRef_.getCorrectionParam()->get());
    obj->setProperty ("bass",       fbBass_);
    obj->setProperty ("treble",     fbTreble_);

    juce::var data (obj);
    feedbackClient_.sendFeedback (data);
}

void HonestMixAudioProcessorEditor::toggleBPM()
{
    showBPM_ = ! showBPM_;
    bpmTitle_.setVisible (showBPM_); bpmClose_.setVisible (showBPM_);
    bpmInput_.setVisible (showBPM_); bpmTap_.setVisible (showBPM_);
    bpmDisplay_.setVisible (showBPM_);
}

void HonestMixAudioProcessorEditor::updateBPM (int bpm)
{
    curBPM_ = bpm;
    double ms = 60000.0 / bpm;
    juce::String t;
    t += juce::String (bpm) + " BPM  |  " + juce::String (ms, 1) + " ms/beat\n";
    t += "PreDelay " + juce::String ((int)(ms/32+0.5)) + "/" + juce::String ((int)(ms/16+0.5)) + "/" + juce::String ((int)(ms/8+0.5)) + " ms\n";
    t += "Delay 1/8=" + juce::String ((int)(ms/4+0.5)) + " 1/4=" + juce::String ((int)(ms/2+0.5)) + " 1/2=" + juce::String ((int)(ms+0.5)) + "\n";
    t += "Room " + juce::String (ms*0.5/1000.0, 2) + "s Plate " + juce::String (ms*2.0/1000.0, 2) + "s Hall " + juce::String (ms*4.0/1000.0, 2) + "s";
    bpmDisplay_.setText (t, juce::dontSendNotification);
}

void HonestMixAudioProcessorEditor::timerCallback()
{
    auto& dw = *processorRef_.getDryWetParam();
    auto& co = *processorRef_.getCorrectionParam();
    double kv = knob_.getValue(), pv = dw.get();
    if (std::abs (kv - pv) > 0.5) knob_.setValue (pv, juce::dontSendNotification);
    knobVal_.setText (juce::String ((int) pv) + " %", juce::dontSendNotification);
    corrBtn_.setText (co.get() ? "ON" : "OFF", juce::dontSendNotification);
    corrBtn_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (co.get() ? 0.12f : 0.04f));

    // 自动触发1小时检查（模拟：60秒后触发一次）
    static int tick = 0;
    if (! showTrans_ && ! showChk_ && ++tick == 1200)
    {
        tick = 0;
        showChk_ = true;
        chkOverlay_.setVisible (true); chkTitle_.setVisible (true);
        chkOpt1_.setVisible (true); chkOpt2_.setVisible (true); chkOpt3_.setVisible (true); chkOpt4_.setVisible (true);
    }
}
