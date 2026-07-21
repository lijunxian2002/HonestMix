#include "WetDryMixer.h"

//==============================================================================
void WetDryMixer::prepare(const juce::dsp::ProcessSpec& spec)
{
    dryWetMixer.prepare(spec);
    dryWetMixer.setWetMixProportion(mixPercent / 100.0f);
}

void WetDryMixer::reset()
{
    dryWetMixer.reset();
}

void WetDryMixer::process(const juce::AudioBuffer<float>& dryBuffer,
                          const juce::AudioBuffer<float>& wetBuffer,
                          juce::AudioBuffer<float>& outputBuffer)
{
    // DryWetMixer 期望输入为 AudioBlock
    // 这里简化实现：手动线性混合
    auto numSamples = outputBuffer.getNumSamples();
    auto numChannels = juce::jmin(dryBuffer.getNumChannels(),
                                  wetBuffer.getNumChannels(),
                                  outputBuffer.getNumChannels());

    float dryGain = 1.0f - (mixPercent / 100.0f);
    float wetGain = mixPercent / 100.0f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dry = dryBuffer.getReadPointer(ch);
        auto* wet = wetBuffer.getReadPointer(ch);
        auto* out = outputBuffer.getWritePointer(ch);

        for (int s = 0; s < numSamples; ++s)
        {
            out[s] = dry[s] * dryGain + wet[s] * wetGain;
        }
    }
}

//==============================================================================
void WetDryMixer::setMixPercent(float percent)
{
    mixPercent = juce::jlimit(0.0f, 100.0f, percent);
    dryWetMixer.setWetMixProportion(mixPercent / 100.0f);
}
