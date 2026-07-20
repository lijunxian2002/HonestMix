#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HonestMixAudioProcessor::HonestMixAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // 干湿比参数：0% = 原始 / 100% = 完全校正 / 默认 50%
    // 参数名用英文（DAW 列表显示），UI 仍保留中文「干湿比」
    auto dwParam = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("drywet", 1),
        "DryWet",
        juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f),
        50.0f);

    auto corrParam = std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("correction", 1), "Correction", false);

    dryWetParam_      = dwParam.get();
    correctionParam_  = corrParam.get();

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

void HonestMixAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const bool correctionOn = correctionParam_->get();
    correctionEngine_.setEnabled (correctionOn);

    // 报告延迟
    setLatencySamples (correctionOn ? correctionEngine_.getLatencySamples() : 0);

    if (! correctionOn)
        return; // 校正关闭 = 零延迟直通，buffer 不变

    const float dryWet = dryWetParam_->get() / 100.0f; // 0.0 ~ 1.0

    // 干湿比 = 0%：不处理 / 100%：完全校正
    if (dryWet <= 0.0f)
        return;

    // 拷贝一份原始信号
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    // 对 buffer 做卷积校正
    correctionEngine_.process (buffer);

    // 混合：output = dry * (1 - wet) + wet * wet
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* dry = dryBuffer.getReadPointer (ch);
        float* out       = buffer.getWritePointer (ch);

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
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HonestMixAudioProcessor();
}
