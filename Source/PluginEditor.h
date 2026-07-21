#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "dsp/FeedbackClient.h"

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
    void showFeedback();
    void hideFeedback();
    void toggleFBButtons();
    void submitFeedback();
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

    // ── 反馈弹窗 ──
    juce::Label fbOverlay_, fbTitle_;
    juce::TextButton fbBassOk_, fbBassMore_, fbBassLess_;
    juce::TextButton fbTrebleOk_, fbTrebleBright_, fbTrebleDark_;
    juce::TextButton fbSubmit_;
    int fbBass_ = 0, fbTreble_ = 0; // 0=ok, 1=more/bright, -1=less/dark

    // ── 1小时检查 ──
    juce::Label chkOverlay_, chkTitle_;
    juce::Label chkOpt1_, chkOpt2_, chkOpt3_, chkOpt4_;
    bool showChk_ = false;

    // ── 分享卡 ──
    juce::Label shareOverlay_, shareTitle_, shareBody_, shareClose_;

    // ── 反馈服务 ──
    FeedbackClient feedbackClient_;

    bool showTrans_ = true, showBPM_ = false, showFB_ = false;
    int curBPM_ = 117;
    juce::Array<juce::Time> taps_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
