#include "PluginEditor.h"

// ── 自定义旋钮 ──────────────────────────────────────
class HonestMixKnobLNF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float sliderPos, float /*sa*/, float /*ea*/,
                           juce::Slider& s) override
    {
        auto cx = x + w / 2.0f, cy = y + h / 2.0f;
        auto r  = (float) juce::jmin (w, h) / 2.0f - 6.0f;
        auto ar = r - 3.0f;
        auto sa = s.getRotaryParameters().startAngleRadians;
        auto ea = s.getRotaryParameters().endAngleRadians;
        auto ca = sa + (ea - sa) * sliderPos;

        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 10));
        g.drawEllipse (cx - r, cy - r, r * 2, r * 2, 1.5f);

        juce::Path arc; arc.addArc (cx - ar, cy - ar, ar * 2, ar * 2, sa, ca, true);
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 40));
        g.strokePath (arc, juce::PathStrokeType (2.5f));

        float px = cx + ar * std::cos (ca);
        float py = cy + ar * std::sin (ca);
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 60));
        g.fillEllipse (px - 3, py - 3, 6, 6);

        // 圆心小光晕
        juce::Colour glowColour (juce::Colour::fromRGBA (255, 255, 255, 12));
        for (int i = 0; i < 4; ++i)
        {
            auto gr = r * (0.25f + i * 0.05f);
            g.setColour (glowColour.withMultipliedAlpha (1.0f - i * 0.2f));
            g.drawEllipse (cx - gr, cy - gr, gr * 2, gr * 2, 0.5f);
        }
    }
};
static HonestMixKnobLNF knobLNF;

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef_ (p)
{
    sealLabel_.setText ("诚", juce::dontSendNotification);
    sealLabel_.setFont (juce::Font (juce::FontOptions (16.0f)));
    sealLabel_.setJustificationType (juce::Justification::centred);
    sealLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.06f));
    addAndMakeVisible (sealLabel_);

    titleLabel_.setText ("HonestMix", juce::dontSendNotification);
    titleLabel_.setFont (juce::Font (juce::FontOptions (18.0f)).boldened());
    titleLabel_.setJustificationType (juce::Justification::centred);
    titleLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.55f));
    addAndMakeVisible (titleLabel_);

    infoLabel_.setText ("ATH-M50X  |  Harman OE  |  RME", juce::dontSendNotification);
    infoLabel_.setFont (juce::Font (juce::FontOptions (10.0f)));
    infoLabel_.setJustificationType (juce::Justification::centred);
    infoLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.25f));
    addAndMakeVisible (infoLabel_);

    // ── 旋钮 ──
    dryWetKnob_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    dryWetKnob_.setRange (0.0, 100.0, 1.0);
    dryWetKnob_.setValue (processorRef_.getDryWetParam()->get(), juce::dontSendNotification);
    dryWetKnob_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    dryWetKnob_.setLookAndFeel (&knobLNF);
    dryWetKnob_.setRotaryParameters (
        juce::MathConstants<float>::pi * 1.25f,
        juce::MathConstants<float>::pi * 2.75f, true);
    dryWetKnob_.onValueChange = [this] {
        *processorRef_.getDryWetParam() = static_cast<float> (dryWetKnob_.getValue());
    };
    addAndMakeVisible (dryWetKnob_);

    dryWetValue_.setFont (juce::Font (juce::FontOptions (28.0f)));
    dryWetValue_.setJustificationType (juce::Justification::centred);
    dryWetValue_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.5f));
    addAndMakeVisible (dryWetValue_);
    dryWetLabel_.setText ("干湿比", juce::dontSendNotification);
    dryWetLabel_.setFont (juce::Font (juce::FontOptions (10.0f)));
    dryWetLabel_.setJustificationType (juce::Justification::centred);
    dryWetLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.15f));
    addAndMakeVisible (dryWetLabel_);

    // ── 开关 ──
    correctionToggle_.setButtonText ("校正");
    correctionToggle_.setToggleState (processorRef_.getCorrectionParam()->get(), juce::dontSendNotification);
    correctionToggle_.setColour (juce::ToggleButton::textColourId, juce::Colours::white.withAlpha (0.25f));
    correctionToggle_.setColour (juce::ToggleButton::tickColourId, juce::Colours::white.withAlpha (0.3f));
    correctionToggle_.onClick = [this] {
        *processorRef_.getCorrectionParam() = correctionToggle_.getToggleState();
    };
    addAndMakeVisible (correctionToggle_);
    correctionLabel_.setText ("校正开关", juce::dontSendNotification);
    correctionLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    correctionLabel_.setJustificationType (juce::Justification::centred);
    correctionLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.08f));
    addAndMakeVisible (correctionLabel_);

    // ── 底部切换按钮 ──
    deviceBtn_.setButtonText ("设备");
    deviceBtn_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    deviceBtn_.setColour (juce::TextButton::textColourOnId, juce::Colours::white.withAlpha (0.25f));
    deviceBtn_.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.1f));
    deviceBtn_.setConnectedEdges (juce::Button::ConnectedOnRight);
    deviceBtn_.setClickingTogglesState (true);
    deviceBtn_.onClick = [this] { showBPM_ = false; refreshBPMPanel (117); };
    addAndMakeVisible (deviceBtn_);

    bpmBtn_.setButtonText ("BPM");
    bpmBtn_.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    bpmBtn_.setColour (juce::TextButton::textColourOnId, juce::Colours::white.withAlpha (0.25f));
    bpmBtn_.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.1f));
    bpmBtn_.setConnectedEdges (juce::Button::ConnectedOnLeft);
    bpmBtn_.setClickingTogglesState (true);
    bpmBtn_.onClick = [this] { showBPM_ = true; refreshBPMPanel (117); };
    addAndMakeVisible (bpmBtn_);
    deviceBtn_.setToggleState (true, juce::dontSendNotification);

    // ── BPM 面板与设备面板 ──
    devicePanel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    devicePanel_.setJustificationType (juce::Justification::centred);
    devicePanel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    devicePanel_.setText ("ATH-M50X  ·  RME Babyface Pro FS", juce::dontSendNotification);
    addAndMakeVisible (devicePanel_);

    bpmPanel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    bpmPanel_.setJustificationType (juce::Justification::centred);
    bpmPanel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.12f));
    bpmPanel_.setVisible (false);
    addAndMakeVisible (bpmPanel_);

    startTimerHz (30);
    setSize (340, 550);
}

HonestMixAudioProcessorEditor::~HonestMixAudioProcessorEditor()
{
    dryWetKnob_.setLookAndFeel (nullptr);
}

//==============================================================================
void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 玻璃底色（渐变更自然）
    g.fillAll (juce::Colour::fromRGBA (18, 18, 22, 200));

    // 顶部柔和光照效果
    auto topGlow = bounds.removeFromTop (getHeight() / 3);
    g.setGradientFill (juce::ColourGradient (
        juce::Colour::fromRGBA (255, 255, 255, 4), 0, 0,
        juce::Colour::fromRGBA (255, 255, 255, 0), 0, (float) topGlow.getHeight(), false));
    g.fillRect (topGlow);

    // 顶边微光
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 8));
    g.fillRect (getLocalBounds().removeFromTop (1));

    // 底部信息区分隔线
    auto bottomInfo = getLocalBounds().removeFromBottom (44);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 6));
    g.fillRect (bottomInfo.removeFromTop (1));
}

void HonestMixAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);

    sealLabel_.setBounds (getWidth() - 40, 18, 24, 24);

    titleLabel_.setBounds (area.removeFromTop (44));
    area.removeFromTop (6);
    infoLabel_.setBounds (area.removeFromTop (26));

    const int knobSize = juce::jmin (area.getWidth(), area.getHeight()) - 100;
    auto knobRect = area.withSizeKeepingCentre (knobSize, knobSize);
    dryWetKnob_.setBounds (knobRect);

    auto valRect = knobRect.translated (0, knobSize / 2 + 6);
    dryWetValue_.setBounds (valRect.getX(), valRect.getY(), knobSize, 32);
    dryWetLabel_.setBounds (valRect.getX(), valRect.getBottom() + 2, knobSize, 20);

    auto bottomSection = area.removeFromBottom (72);
    correctionToggle_.setBounds (bottomSection.withSizeKeepingCentre (100, 28));
    correctionLabel_.setBounds (bottomSection.withTrimmedTop (32).removeFromTop (16));

    // 底部信息栏 + 按钮
    auto bottom = getLocalBounds().removeFromBottom (44);
    auto btnRow = bottom.removeFromTop (20).withTrimmedLeft (20).withTrimmedRight (20);
    deviceBtn_.setBounds (btnRow.removeFromLeft (60).reduced (2));
    bpmBtn_.setBounds (btnRow.removeFromLeft (60).reduced (2));

    auto infoRow = bottom.reduced (4, 0);
    devicePanel_.setBounds (infoRow);
    bpmPanel_.setBounds (infoRow);
}

//==============================================================================
void HonestMixAudioProcessorEditor::refreshBPMPanel (int bpm)
{
    if (bpm <= 0) bpm = 117;
    const double beatMs = 60000.0 / bpm;

    devicePanel_.setVisible (! showBPM_);
    bpmPanel_.setVisible (showBPM_);

    if (showBPM_)
    {
        int pD16 = (int)(beatMs / 8.0 + 0.5);
        int pD32 = (int)(beatMs / 16.0 + 0.5);
        int pD64 = (int)(beatMs / 32.0 + 0.5);
        int d8  = (int)(beatMs / 4.0 + 0.5);
        int d4  = (int)(beatMs / 2.0 + 0.5);
        int d2  = (int)(beatMs + 0.5);

        juce::String txt;
        txt += juce::String (bpm) + " BPM  |  " + juce::String (beatMs, 1) + " ms/拍\n";
        txt += "预延迟: " + juce::String (pDelay64) + "/" + juce::String (pDelay32) + "/" + juce::String (pDelay16) + " ms\n";
        txt += "延迟: 1/8=" + juce::String (d8) + " 1/4=" + juce::String (d4) + " 1/2=" + juce::String (d2) + " ms\n";
        txt += "混响: 房间0.26s 板式1.03s 大厅2.05s";
        bpmPanel_.setText (txt, juce::dontSendNotification);
    }
}

//==============================================================================
void HonestMixAudioProcessorEditor::timerCallback()
{
    auto& dwParam = *processorRef_.getDryWetParam();
    auto& corrParam = *processorRef_.getCorrectionParam();

    double knobVal = dryWetKnob_.getValue();
    double paramVal = dwParam.get();
    if (std::abs (knobVal - paramVal) > 0.5)
        dryWetKnob_.setValue (paramVal, juce::dontSendNotification);
    dryWetValue_.setText (juce::String (static_cast<int> (paramVal)) + " %", juce::dontSendNotification);

    if (correctionToggle_.getToggleState() != corrParam.get())
        correctionToggle_.setToggleState (corrParam.get(), juce::dontSendNotification);
}
