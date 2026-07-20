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
    void mouseDown (const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    void dismissTransition();
    void startMix();
    void toggleCorrection();
    void toggleBPM();
    void updateBPM (int bpm);

    HonestMixAudioProcessor& processorRef_;

    // ── 过渡卡 ──
    juce::Label tcSeal_, tcTitle_, tcHow_;
    juce::TextButton tc50_, tc100_, tcDismiss_, tcStart_;
    int dryWetChoice_ = 50;

    // ── 主界面 ──
    juce::Label title_, version_;
    juce::Label hpLbl_, cvLbl_, ifLbl_;
    juce::Label hpVal_, cvVal_, ifVal_;
    juce::Slider knob_;
    juce::Label  knobVal_, knobLbl_;
    juce::Label corrLbl_, corrBtn_;
    juce::Label bpmLbl_;
    juce::Label seal_;

    // ── BPM 面板 ──
    juce::Label bpmTitle_, bpmClose_;
    juce::TextEditor bpmInput_;
    juce::Label bpmTap_, bpmDisplay_;

    bool showTrans_ = true, showBPM_ = false;
    int curBPM_ = 117;
    juce::Array<juce::Time> taps_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
