#include "DSP/WetDryMixer.h"

void WetDryMixer::prepare (int numChannels, int maxBlockSize)
{
    dry_.setSize (numChannels, maxBlockSize, false, false, true);
}

void WetDryMixer::captureDry (const juce::AudioBuffer<float>& buffer)
{
    dry_.makeCopyOf (buffer);
}

void WetDryMixer::mixInPlace (juce::AudioBuffer<float>& wetBuffer)
{
    // 与数据中心 processBlock 中的 SIMD 混合逐指令一致
    const float intensity   = degree_.load() / 100.0f;   // 0.0 ~ 2.0
    const int   numChannels = wetBuffer.getNumChannels();
    const int   numSamples  = wetBuffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dry = dry_.getReadPointer (ch);
        auto* out = wetBuffer.getWritePointer (ch);
        juce::FloatVectorOperations::multiply (out, intensity, numSamples);
        juce::FloatVectorOperations::addWithMultiply (out, dry, 1.0f - intensity, numSamples);
    }
}
