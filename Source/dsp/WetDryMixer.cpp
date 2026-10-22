#include "DSP/WetDryMixer.h"

void WetDryMixer::prepare (int numChannels, int maxBlockSize,double sampleRate,int latency)
{
    dry_.setSize (numChannels, maxBlockSize, false, false, true);
    delaySamples_=juce::jmax(0,latency);delayPosition_=0;delay_.setSize(numChannels,juce::jmax(1,delaySamples_));delay_.clear();
    intensity_.reset(sampleRate,0.020);
    intensity_.setCurrentAndTargetValue(degree_.load()/100.0f);
}

void WetDryMixer::captureDry (const juce::AudioBuffer<float>& buffer)
{
    jassert(buffer.getNumChannels()<=dry_.getNumChannels()&&buffer.getNumSamples()<=dry_.getNumSamples());
    if(delaySamples_>0){
        for(int sample=0;sample<buffer.getNumSamples();++sample){
            for(int ch=0;ch<buffer.getNumChannels();++ch){dry_.setSample(ch,sample,delay_.getSample(ch,delayPosition_));delay_.setSample(ch,delayPosition_,buffer.getSample(ch,sample));}
            if(++delayPosition_==delaySamples_)delayPosition_=0;
        }
        return;
    }
    for(int ch=0;ch<buffer.getNumChannels();++ch)dry_.copyFrom(ch,0,buffer,ch,0,buffer.getNumSamples());
}

void WetDryMixer::mixInPlace (juce::AudioBuffer<float>& wetBuffer)
{
    intensity_.setTargetValue(degree_.load()/100.0f);
    const int   numChannels = wetBuffer.getNumChannels();
    const int   numSamples  = wetBuffer.getNumSamples();

    if(intensity_.isSmoothing()){
        for(int sample=0;sample<numSamples;++sample){
            const float intensity=intensity_.getNextValue();
            for(int ch=0;ch<numChannels;++ch){auto* out=wetBuffer.getWritePointer(ch);const auto* dry=dry_.getReadPointer(ch);out[sample]=out[sample]*intensity+dry[sample]*(1.0f-intensity);}
        }
        return;
    }
    const float intensity=intensity_.getCurrentValue();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dry = dry_.getReadPointer (ch);
        auto* out = wetBuffer.getWritePointer (ch);
        juce::FloatVectorOperations::multiply (out, intensity, numSamples);
        juce::FloatVectorOperations::addWithMultiply (out, dry, 1.0f - intensity, numSamples);
    }
}
