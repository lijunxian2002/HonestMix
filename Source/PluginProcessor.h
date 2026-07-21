#pragma once
/**
 * @file  PluginProcessor.h
 * @brief JUCE audio processor — dry/wet convolution pipeline.
 *
 * HonestMix 诚听 — free open-source GPL-3.0 headphone-correction VST3.
 *
 * Architecture:
 *   ┌─────────────────────────────────────────────────┐
 *   │  PluginProcessor                                │
 *   │  ├─ dryWetParam_   (0–100 %)                    │
 *   │  ├─ correctionParam_ (on/off)                   │
 *   │  └─ correctionEngine_ → FIR convolution         │
 *   └─────────────────────────────────────────────────┘
 *            ↑ creates
 *   ┌─────────────────────────────────────────────────┐
 *   │  PluginEditor  (all UI)                         │
 *   └─────────────────────────────────────────────────┘
 */

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp/CorrectionEngine.h"

//==============================================================================
/**
 * Main audio processor for HonestMix.
 *
 * @par Audio thread
 *     processBlock() runs on the real-time audio thread.
 *     No allocations, locks, or I/O in that path.
 *
 * @par Parameters
 *     - drywet:    0 % = original, 100 % = fully corrected (default 50 %)
 *     - correction: on/off toggle
 *
 * @par State persistence
 *     Current drywet, correction, and profile index are saved/restored
 *     via getStateInformation() / setStateInformation().
 */
class HonestMixAudioProcessor final : public juce::AudioProcessor
{
public:
    HonestMixAudioProcessor();
    ~HonestMixAudioProcessor() override = default;

    //==============================================================================
    /// @name JUCE processor overrides
    ///@{
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                  { return true; }

    const juce::String getName() const override      { return "HonestMix"; }

    bool acceptsMidi() const override                { return false; }
    bool producesMidi() const override               { return false; }
    double getTailLengthSeconds() const override     { return 0.0; }

    int getNumPrograms() override                    { return 1; }
    int getCurrentProgram() override                 { return 0; }
    void setCurrentProgram (int) override            {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    ///@}

    //==============================================================================
    /// @name State persistence
    ///@{
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    ///@}

    //==============================================================================
    /// @name Editor accessors
    ///@{
    juce::AudioParameterFloat* getDryWetParam() const noexcept    { return dryWetParam_; }
    juce::AudioParameterBool*  getCorrectionParam() const noexcept { return correctionParam_; }
    CorrectionEngine& getCorrectionEngine() noexcept              { return correctionEngine_; }

    int  getProfileIndex() const noexcept { return profileIndex_; }
    void setProfileIndex (int idx) noexcept { profileIndex_ = idx; }
    ///@}

private:
    juce::AudioParameterFloat* dryWetParam_     = nullptr;
    juce::AudioParameterBool*  correctionParam_ = nullptr;
    CorrectionEngine correctionEngine_;

    int   profileIndex_      = 0;
    bool  lastCorrectionOn_  = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessor)
};
