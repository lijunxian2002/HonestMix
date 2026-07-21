#include "DSP/CorrectionEngine.h"

void CorrectionEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSpec_ = spec;
    prepared_ = true;
    loadProfile();
}

void CorrectionEngine::setProfile (int index)
{
    if (index < 0 || index >= db_.getNumProfiles())
        index = 0;
    currentProfile_ = index;
    if (prepared_)
        loadProfile();
}

void CorrectionEngine::setProfileByName (const juce::String& name)
{
    const int idx = db_.findProfileByName (name);
    setProfile (idx >= 0 ? idx : 0);
}

void CorrectionEngine::loadProfile()
{
    const float* fir = db_.getProfileFIR (currentProfile_);
    if (fir == nullptr)
        return;

    juce::Array<float> ir;
    ir.resize (HeadphoneDatabase::kFirLength);
    std::copy (fir, fir + HeadphoneDatabase::kFirLength, ir.begin());

    convolution_.loadImpulseResponse (
        static_cast<const void*> (ir.getRawDataPointer()),
        HeadphoneDatabase::kFirLength * sizeof (float),
        juce::dsp::Convolution::Stereo::no,
        juce::dsp::Convolution::Trim::no,
        currentSpec_.maximumBlockSize,
        juce::dsp::Convolution::Normalise::yes);
}

void CorrectionEngine::reset()
{
    if (prepared_)
        loadProfile();
}

void CorrectionEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled_)
        return;
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    convolution_.process (context);
}
