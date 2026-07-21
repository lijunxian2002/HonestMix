/**
 * @file  PluginProcessor.cpp
 * @brief Audio processor — dry/wet convolution pipeline.
 *
 * Process flow:
 *   1. Correction off → zero-latency pass-through (buffer unchanged)
 *   2. Correction on + dryWet=0 % → pass-through
 *   3. Correction on + dryWet>0 % → convolve, then dry/wet mix
 *
 * Latency is reported dynamically:
 *   0 samples  (correction off)
 *   1024 samples (correction on, FIR length)
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HonestMixAudioProcessor::HonestMixAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Dry/Wet:  0% = original, 100% = fully corrected, default 50%
    auto dwParam = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("drywet", 1),
        "DryWet",
        juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f),
        50.0f);

    // Correction on/off
    auto corrParam = std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("correction", 1), "Correction", false);

    dryWetParam_     = dwParam.get();
    correctionParam_ = corrParam.get();

    addParameter (dwParam.release());
    addParameter (corrParam.release());
}

//==============================================================================
void HonestMixAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = 2;

    correctionEngine_.prepare (spec);
}

void HonestMixAudioProcessor::releaseResources()
{
    correctionEngine_.reset();
}

//==============================================================================
void HonestMixAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples  = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    const bool correctionOn = correctionParam_->get();
    correctionEngine_.setEnabled (correctionOn);

    // Update DAW latency report only when state changes (TCAI §4.1: no
    // unnecessary per-block side effects).
    if (correctionOn != lastCorrectionOn_)
    {
        lastCorrectionOn_ = correctionOn;
        setLatencySamples (correctionOn ? correctionEngine_.getLatencySamples() : 0);
    }

    if (! correctionOn)
        return; // Zero-latency pass-through: buffer unchanged.

    const float dryWet = dryWetParam_->get() / 100.0f;

    if (dryWet <= 0.0f)
        return; // 0% = fully dry, nothing to do.

    // ── Copy dry signal, then convolve in-place and mix ────────
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    correctionEngine_.process (buffer);

    // output = dry × (1 - wet) + wet × wet
    const int numChannels = buffer.getNumChannels();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* dry = dryBuffer.getReadPointer (ch);
        float*       out = buffer.getWritePointer (ch);

        for (int s = 0; s < numSamples; ++s)
            out[s] = dry[s] * (1.0f - dryWet) + out[s] * dryWet;
    }
}

//==============================================================================
juce::AudioProcessorEditor* HonestMixAudioProcessor::createEditor()
{
    return new HonestMixAudioProcessorEditor (*this);
}

//==============================================================================
void HonestMixAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = juce::ValueTree ("HonestMix");
    state.setProperty ("drywet",     dryWetParam_->get(), nullptr);
    state.setProperty ("correction", correctionParam_->get(), nullptr);
    state.setProperty ("profile",    profileIndex_, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void HonestMixAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (auto state = juce::ValueTree::fromXml (*xml); state.isValid())
        {
            dryWetParam_->setValueNotifyingHost (state.getProperty ("drywet", 50.0f));
            correctionParam_->setValueNotifyingHost (state.getProperty ("correction", false));

            const int savedProfile = (int) state.getProperty ("profile", 0);
            if (savedProfile >= 0 && savedProfile < correctionEngine_.getNumProfiles())
                profileIndex_ = savedProfile;
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HonestMixAudioProcessor();
}
