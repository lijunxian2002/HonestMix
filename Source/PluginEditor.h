#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class HonestMixAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit HonestMixAudioProcessorEditor (HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshBPMPanel (int bpm);

    HonestMixAudioProcessor& processorRef_;

    juce::Label sealLabel_;
    juce::Label titleLabel_;
    juce::Label infoLabel_;

    juce::Slider dryWetKnob_;
    juce::Label  dryWetValue_;
    juce::Label  dryWetLabel_;

    juce::ToggleButton correctionToggle_;
    juce::Label correctionLabel_;

    // 底部切换栏
    juce::TextButton deviceBtn_;
    juce::TextButton bpmBtn_;

    // BPM 面板组件
    juce::Label bpmPanel_;
    juce::Label devicePanel_;

    bool showBPM_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
