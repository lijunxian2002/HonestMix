#include "dsp/CorrectionEngine.h"
#include "dsp/fir_data.h"

static const HeadphoneProfile profiles[] = {
    { "ATH-M50X",   fir_m50x    },
    { "DT 770 Pro",  fir_dt770   },
    { "HD 600",     fir_hd600   },
    { "DT 990 Pro", fir_dt990pro }
};

void CorrectionEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSpec_ = spec;
    prepared_ = true;
    loadProfile();
}

void CorrectionEngine::setProfile (int index)
{
    if (index < 0 || index >= 4) index = 0;
    currentProfile_ = index;
    if (prepared_)
        loadProfile();
}

void CorrectionEngine::loadProfile()
{
    auto& src = profiles[currentProfile_];
    juce::Array<float> ir;
    ir.resize (1024);
    std::copy (src.firData, src.firData + 1024, ir.begin());

    convolution_.loadImpulseResponse (
        static_cast<const void*> (ir.getRawDataPointer()),
        1024 * sizeof (float),
        juce::dsp::Convolution::Stereo::no,
        juce::dsp::Convolution::Trim::no,
        currentSpec_.maximumBlockSize,
        juce::dsp::Convolution::Normalise::yes);
}

void CorrectionEngine::reset()
{
    if (prepared_) loadProfile();
}

void CorrectionEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled_) return;
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    convolution_.process (context);
}

const char* CorrectionEngine::getProfileName (int index) const
{
    if (index < 0 || index >= 4) return "Unknown";
    return profiles[index].name;
}
