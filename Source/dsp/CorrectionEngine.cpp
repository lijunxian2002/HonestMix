#include "dsp/CorrectionEngine.h"
#include "dsp/fir_data.h"

//==============================================================================
void CorrectionEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    // 使用预计算的静态 FIR 系数（44100 Hz，1024 taps）
    // 其他采样率时做线性拉伸
    juce::Array<float> ir;
    ir.resize (1024);

    const double rateRatio = spec.sampleRate / 44100.0;

    if (std::abs (rateRatio - 1.0) < 0.01)
    {
        // 44100 Hz → 直接拷贝
        std::copy (firData, firData + 1024, ir.begin());
    }
    else
    {
        // 其他采样率 → 线性插值
        for (int i = 0; i < 1024; ++i)
        {
            double srcPos = i / rateRatio;
            int idx = (int) srcPos;
            double frac = srcPos - idx;
            if (idx < 1023)
                ir.getReference (i) = (float) (firData[idx] * (1.0 - frac) + firData[idx + 1] * frac);
            else
                ir.getReference (i) = firData[1023];
        }
    }

    convolution_.loadImpulseResponse (
        static_cast<const void*> (ir.getRawDataPointer()),
        static_cast<size_t> (ir.size()) * sizeof (float),
        juce::dsp::Convolution::Stereo::no,
        juce::dsp::Convolution::Trim::no,
        spec.maximumBlockSize,
        juce::dsp::Convolution::Normalise::yes);
}

void CorrectionEngine::reset()
{
    convolution_.reset();
}

void CorrectionEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled_) return;

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    convolution_.process (context);
}
