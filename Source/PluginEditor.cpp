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
    infoLabel_.setText ("ATH-M50X  |  Harman OE  |  RME", juce::dontSendNotification);
    infoLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    infoLabel_.setJustificationType (juce::Justification::centred);
    infoLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.3f));
    addAndMakeVisible (infoLabel_);

    // ── 干湿比旋钮 ──
    dryWetKnob_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    dryWetKnob_.setRange (0.0, 100.0, 1.0);
    dryWetKnob_.setValue (processorRef_.getDryWetParam()->get(), juce::dontSendNotification);
    dryWetKnob_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    dryWetKnob_.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::white.withAlpha (0.06f));
    dryWetKnob_.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    dryWetKnob_.setColour (juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    dryWetKnob_.setColour (juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    dryWetKnob_.setRotaryParameters (
        juce::MathConstants<float>::pi * 1.25f,
        juce::MathConstants<float>::pi * 2.75f, true);
    dryWetKnob_.onValueChange = [this] {
        *processorRef_.getDryWetParam() = static_cast<float> (dryWetKnob_.getValue());
    };
    addAndMakeVisible (dryWetKnob_);

    dryWetValue_.setFont (juce::Font (juce::FontOptions (22.0f)));
    dryWetValue_.setJustificationType (juce::Justification::centred);
    dryWetValue_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.5f));
    addAndMakeVisible (dryWetValue_);

    dryWetLabel_.setText ("干湿比", juce::dontSendNotification);
    dryWetLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));
    dryWetLabel_.setJustificationType (juce::Justification::centred);
    dryWetLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.15f));
    addAndMakeVisible (dryWetLabel_);

    // ── 校正开关 ──
    correctionToggle_.setButtonText ("校正");
    correctionToggle_.setToggleState (processorRef_.getCorrectionParam()->get(), juce::dontSendNotification);
    correctionToggle_.setColour (juce::ToggleButton::textColourId, juce::Colours::white.withAlpha (0.2f));
    correctionToggle_.setColour (juce::ToggleButton::tickColourId, juce::Colours::white.withAlpha (0.3f));
    correctionToggle_.onClick = [this] {
        *processorRef_.getCorrectionParam() = correctionToggle_.getToggleState();
    };
    addAndMakeVisible (correctionToggle_);

    correctionLabel_.setText ("校正开关", juce::dontSendNotification);
    correctionLabel_.setFont (juce::Font (juce::FontOptions (8.0f)));
    correctionLabel_.setJustificationType (juce::Justification::centred);
    correctionLabel_.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.08f));
    addAndMakeVisible (correctionLabel_);

    startTimerHz (30);
    setSize (340, 550);
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
    auto area = getLocalBounds().reduced (20);

    // 标题
    titleLabel_.setFont (juce::Font (juce::FontOptions (18.0f)).boldened());
    titleLabel_.setBounds (area.removeFromTop (44));
    area.removeFromTop (8);

    // 信息行
    infoLabel_.setFont (juce::Font (juce::FontOptions (10.0f)));
    infoLabel_.setBounds (area.removeFromTop (26));

    // 旋钮
    const int knobSize = juce::jmin (area.getWidth(), area.getHeight()) - 80;
    auto knobRect = area.withSizeKeepingCentre (knobSize, knobSize);
    dryWetKnob_.setBounds (knobRect);

    // 数值（紧贴旋钮下方）
    auto valRect = knobRect.translated (0, knobSize / 2 + 6);
    dryWetValue_.setFont (juce::Font (juce::FontOptions (28.0f)));
    dryWetValue_.setBounds (valRect.getX(), valRect.getY(), knobSize, 32);

    // 干湿比标签
    dryWetLabel_.setBounds (valRect.getX(), valRect.getBottom() + 4, knobSize, 20);

    // 底部开关区
    auto bottomSection = area.removeFromBottom (72);
    correctionToggle_.setBounds (bottomSection.withSizeKeepingCentre (100, 28));
    correctionToggle_.setColour (juce::ToggleButton::textColourId, juce::Colours::white.withAlpha (0.25f));
    correctionLabel_.setBounds (bottomSection.withTrimmedTop (32).removeFromTop (16));
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

    dryWetValue_.setText (juce::String (static_cast<int> (paramVal)) + " %", juce::dontSendNotification);

    if (correctionToggle_.getToggleState() != corrParam.get())
        correctionToggle_.setToggleState (corrParam.get(), juce::dontSendNotification);
}
