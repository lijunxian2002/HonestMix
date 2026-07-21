#pragma once
/**
 * @file  PluginEditor.h
 * @brief Main plugin UI — all panels and overlays.
 *
 * Layout (280 × 430 px):
 *   ┌──────────────────────────────────┐
 *   │ HonestMix               v0.1.0   │  top
 *   │ 耳机   曲线   声卡               │  info row
 *   │          ┌──────┐                │
 *   │          │ WET  │  72 px knob    │
 *   │          └──────┘                │
 *   │  校正  ON          BPM           │  bottom row
 *   │                        诚        │  feedback trigger
 *   └──────────────────────────────────┘
 *
 * Transient overlays (stacked via visibility):
 *   - Transition card  (first launch)
 *   - BPM helper panel
 *   - Feedback dialog
 *   - 1-hour check dialog
 *   - Share card
 *
 * @par Thread safety
 *     All UI runs on the message thread.
 *     Audio parameters are read via processorRef_ (atomic in JUCE).
 */

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "dsp/FeedbackClient.h"

//==============================================================================
/** Numerical feedback values used by the ear-training questionnaire. */
enum class FeedbackValue : int
{
    okay      =  0,
    positive  =  1,  // "too_much" / "too_bright"
    negative  = -1   // "too_little" / "too_dark"
};

//==============================================================================
/**
 * Plugin editor window.
 *
 * Manages all UI components, overlays, and user interactions.
 * Transition card is shown on first launch; all other panels are
 * triggered by user action or the 1-hour timer.
 */
class HonestMixAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit HonestMixAudioProcessorEditor (HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override;

    //==============================================================================
    /// @name JUCE overrides
    ///@{
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    ///@}

private:
    void timerCallback() override;

    //==============================================================================
    /// @name Transition
    ///@{
    void dismissTransition();
    void startMix();
    ///@}

    //==============================================================================
    /// @name Correction
    ///@{
    void toggleCorrection();
    ///@}

    //==============================================================================
    /// @name BPM helper
    ///@{
    void toggleBPM();
    void updateBPM (int bpm);
    ///@}

    //==============================================================================
    /// @name Feedback overlay
    ///@{
    void showFeedback();
    void hideFeedback();
    void toggleFBButtons();
    void submitFeedback();
    ///@}

    //==============================================================================
    /// @name Share card
    ///@{
    void showShare();
    void closeShare();
    void updateShareBody();
    ///@}

    HonestMixAudioProcessor& processorRef_;

    //==============================================================================
    // ── Transition card ─────────────────────────────────────────
    juce::Label tcSeal_, tcTitle_, tcHow_;
    juce::TextButton tc50_, tc100_, tcDismiss_, tcStart_;
    int dryWetChoice_ = 50;

    // ── Main interface ─────────────────────────────────────────
    juce::Label title_, version_;
    juce::Label hpLbl_, cvLbl_, ifLbl_;
    juce::Label hpVal_, cvVal_, ifVal_;
    juce::Slider knob_;
    juce::Label  knobVal_, knobLbl_;
    juce::Label corrLbl_, corrBtn_;
    juce::Label bpmLbl_;
    juce::Label seal_;

    // ── BPM panel ──────────────────────────────────────────────
    juce::Label bpmTitle_, bpmClose_;
    juce::TextEditor bpmInput_;
    juce::Label bpmTap_, bpmDisplay_;

    // ── Feedback overlay ───────────────────────────────────────
    juce::Label fbOverlay_, fbTitle_;
    juce::TextButton fbBassOk_, fbBassMore_, fbBassLess_;
    juce::TextButton fbTrebleOk_, fbTrebleBright_, fbTrebleDark_;
    juce::TextButton fbSubmit_, fbClose_;
    FeedbackValue fbBass_   = FeedbackValue::okay;
    FeedbackValue fbTreble_ = FeedbackValue::okay;

    // ── 1-hour check ───────────────────────────────────────────
    juce::Label chkOverlay_, chkTitle_;
    juce::Label chkOpt1_, chkOpt2_, chkOpt3_, chkOpt4_;
    bool showChk_ = false;

    // ── Share card ─────────────────────────────────────────────
    juce::Label shareOverlay_, shareTitle_, shareWave_, shareProject_, shareBody_, shareClose_;

    // ── Services ───────────────────────────────────────────────
    FeedbackClient feedbackClient_;

    // ── State ──────────────────────────────────────────────────
    bool showTrans_ = true, showBPM_ = false, showFB_ = false;
    int curBPM_ = 117;
    int tick_ = 0;
    juce::Array<juce::Time> taps_;

    // ── Headphone profiles ─────────────────────────────────────
    static constexpr int numProfiles_ = 4;
    static const char* profileNames_[numProfiles_];
    static const char* profileCurves_[numProfiles_];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
