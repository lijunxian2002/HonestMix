#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

/**
 * HonestMix 诚听 — 插件界面
 * 约 280×430 · 玻璃面板 · 白字 · 干湿比旋钮
 */
class HonestMixAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit HonestMixAudioProcessorEditor (HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    HonestMixAudioProcessor& processorRef_;

    juce::Slider dryWetKnob_;
    juce::Label  dryWetValue_;
    juce::Label  dryWetLabel_;

    juce::ToggleButton correctionToggle_;
    juce::Label correctionLabel_;

    juce::Label titleLabel_;
    juce::Label infoLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
