#include "PluginEditor.h"

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef_ (p)
{
    // ── 标题 ──
    titleLabel_.setText ("HonestMix", juce::dontSendNotification);
    titleLabel_.setFont (juce::Font (juce::FontOptions (15.0f)).boldened());
    titleLabel_.setJustificationType (juce::Justification::centred);
    titleLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.55f));
    addAndMakeVisible (titleLabel_);

    // ── 信息行 ──
    infoLabel_.setText (u8"ATH-M50X  |  Harman OE  |  RME", juce::dontSendNotification);
    infoLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    infoLabel_.setJustificationType (juce::Justification::centred);
    infoLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.3f));
    addAndMakeVisible (infoLabel_);

    // ── 干湿比旋钮 ──
    dryWetKnob_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    dryWetKnob_.setRange (0.0, 100.0, 1.0);
    dryWetKnob_.setValue (processorRef_.getDryWetParam()->get(), juce::dontSendNotification);
    dryWetKnob_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    dryWetKnob_.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::white.withAlpha (0.08f));
    dryWetKnob_.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::white.withAlpha (0.04f));
    dryWetKnob_.setColour (juce::Slider::thumbColourId, juce::Colours::white.withAlpha (0.15f));
    dryWetKnob_.onValueChange = [this] {
        *processorRef_.getDryWetParam() = static_cast<float> (dryWetKnob_.getValue());
    };
    addAndMakeVisible (dryWetKnob_);

    dryWetValue_.setFont (juce::Font (juce::FontOptions (22.0f)));
    dryWetValue_.setJustificationType (juce::Justification::centred);
    dryWetValue_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.5f));
    addAndMakeVisible (dryWetValue_);

    dryWetLabel_.setText (u8"干湿比", juce::dontSendNotification);
    dryWetLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    dryWetLabel_.setJustificationType (juce::Justification::centred);
    dryWetLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.15f));
    addAndMakeVisible (dryWetLabel_);

    // ── 校正开关 ──
    correctionToggle_.setButtonText (u8"校正");
    correctionToggle_.setToggleState (processorRef_.getCorrectionParam()->get(), juce::dontSendNotification);
    correctionToggle_.setColour (juce::ToggleButton::textColourId, juce::Colours::white.withAlpha (0.2f));
    correctionToggle_.setColour (juce::ToggleButton::tickColourId, juce::Colours::white.withAlpha (0.3f));
    correctionToggle_.onClick = [this] {
        *processorRef_.getCorrectionParam() = correctionToggle_.getToggleState();
    };
    addAndMakeVisible (correctionToggle_);

    correctionLabel_.setText (u8"校正开关", juce::dontSendNotification);
    correctionLabel_.setFont (juce::Font (juce::FontOptions (8.0f)));
    correctionLabel_.setJustificationType (juce::Justification::centred);
    correctionLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.08f));
    addAndMakeVisible (correctionLabel_);

    startTimerHz (30);
    setSize (280, 430);
}

//==============================================================================
void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 半透明玻璃底色
    g.fillAll (juce::Colour::fromRGBA (22, 22, 26, 180));

    // 顶边微光
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 8));
    g.fillRect (getLocalBounds().removeFromTop (1));
}

void HonestMixAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (14);

    // 标题行
    titleLabel_.setBounds (area.removeFromTop (36));
    area.removeFromTop (4);

    // 信息行
    infoLabel_.setBounds (area.removeFromTop (22));

    // 底部区域（开关 + 标签）
    auto bottomSection = area.removeFromBottom (52);
    correctionToggle_.setBounds (bottomSection.withSizeKeepingCentre (80, 20));
    correctionLabel_.setBounds (bottomSection.withTrimmedTop (22).removeFromTop (14));

    // 剩余空间全部给旋钮
    const int knobSize = juce::jmin (area.getWidth(), area.getHeight()) - 20;
    dryWetKnob_.setBounds (area.withSizeKeepingCentre (knobSize, knobSize));

    // 数值标签贴在旋钮下方
    auto knobBottom = dryWetKnob_.getBounds().getBottom();
    dryWetValue_.setBounds (knobBottom - 10, area.getX(), area.getWidth(), 26);
    dryWetLabel_.setBounds (knobBottom + 14, area.getX(), area.getWidth(), 16);
}

//==============================================================================
void HonestMixAudioProcessorEditor::timerCallback()
{
    auto& dwParam = *processorRef_.getDryWetParam();
    auto& corrParam = *processorRef_.getCorrectionParam();

    const double knobVal = dryWetKnob_.getValue();
    const double paramVal = dwParam.get();
    if (std::abs (knobVal - paramVal) > 0.5)
        dryWetKnob_.setValue (paramVal, juce::dontSendNotification);

    dryWetValue_.setText (juce::String (static_cast<int> (paramVal)) + u8" %", juce::dontSendNotification);

    if (correctionToggle_.getToggleState() != corrParam.get())
        correctionToggle_.setToggleState (corrParam.get(), juce::dontSendNotification);
}
