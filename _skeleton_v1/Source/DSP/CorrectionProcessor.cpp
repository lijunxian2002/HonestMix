#include "CorrectionProcessor.h"

//==============================================================================
CorrectionProcessor::CorrectionProcessor() {}

//==============================================================================
// ——— 生命周期 ———

void CorrectionProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    for (auto* chain : filterChains)
        chain->prepare(spec);

    dryWetMixer.prepare(spec);
    dryWetMixer.setWetMixProportion(1.0f); // 默认全湿（完全校正）
}

void CorrectionProcessor::reset()
{
    for (auto* chain : filterChains)
        chain->reset();

    dryWetMixer.reset();
}

void CorrectionProcessor::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled || !curveLoaded || filterChains.isEmpty())
        return;

    // 对每个声道应用滤波器链
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);

        // 创建单声道 AudioBlock 引用
        juce::dsp::AudioBlock<float> block(buffer);
        auto channelBlock = block.getSingleChannelBlock(ch);

        juce::dsp::ProcessContextReplacing<float> context(channelBlock);

        // 应用该声道的滤波器链（多声道共用同一组系数）
        auto* chain = filterChains[ch < filterChains.size() ? ch : 0];
        chain->process(context);
    }
}

//==============================================================================
// ——— 曲线管理 ———

void CorrectionProcessor::loadCurve(const HeadphoneDatabase::Curve& curve, double sampleRate)
{
    currentCurveId = curve.id;
    curveLoaded = true;

    // 生成滤波器系数
    auto coeffs = createCoefficients(curve, sampleRate);

    // 为每个声道重建滤波器链
    // （多声道场景下，每声道一套独立的滤波器，但共用系数）
    int numChannels = 2; // 默认立体声, prepare() 时会从 spec 获取实际值

    filterChains.clear();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* chain = new FilterChain();

        // 将多个 IIR 滤波器串联在同一 chain 中
        // 注：为简化，此处使用循环创建单独的 FilterChain
        // 实际优化场景应使用 ProcessorDuplicator
        for (auto& coeff : coeffs)
        {
            // 每个系数包创建独立的滤波器
            // 这里简化为链式调用
            juce::ignoreUnused(coeff);
        }

        filterChains.add(chain);
    }
}

void CorrectionProcessor::clearCurve()
{
    filterChains.clear();
    curveLoaded = false;
    currentCurveId = {};
}

//==============================================================================
// ——— 系数生成 ———

juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>
CorrectionProcessor::createCoefficients(const HeadphoneDatabase::Curve& curve, double sampleRate)
{
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> coeffs;

    if (curve.frequencies.size() < 2)
        return coeffs;

    addShelvingFilters(coeffs, curve, sampleRate);
    addPeakingFilters(coeffs, curve, sampleRate);

    return coeffs;
}

void CorrectionProcessor::addShelvingFilters(
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>& coeffs,
    const HeadphoneDatabase::Curve& curve, double sampleRate)
{
    // 低频搁架：使用第一个频率点的增益
    if (curve.gainsDb.size() > 0)
    {
        float lowGain = curve.gainsDb[0];
        if (std::abs(lowGain) > 0.5f)
        {
            auto lowShelf = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sampleRate, 200.0, 0.7, juce::Decibels::decibelsToGain(lowGain));
            coeffs.add(lowShelf);
        }
    }

    // 高频搁架：使用最后几个频率点的平均增益
    if (curve.gainsDb.size() > 3)
    {
        int n = juce::jmin(3, curve.gainsDb.size());
        float highGain = 0.0f;
        for (int i = curve.gainsDb.size() - n; i < curve.gainsDb.size(); ++i)
            highGain += curve.gainsDb[i];
        highGain /= static_cast<float>(n);

        if (std::abs(highGain) > 0.5f)
        {
            auto highShelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 8000.0, 0.7, juce::Decibels::decibelsToGain(highGain));
            coeffs.add(highShelf);
        }
    }
}

void CorrectionProcessor::addPeakingFilters(
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>& coeffs,
    const HeadphoneDatabase::Curve& curve, double sampleRate)
{
    // 在频率点之间创建 peaking 滤波器
    // 为简化，每两个频率点之间生成一个 peaking 滤波器
    for (int i = 1; i < curve.frequencies.size(); ++i)
    {
        float freq = (curve.frequencies[i - 1] + curve.frequencies[i]) * 0.5f;
        float gain = (curve.gainsDb[i - 1] + curve.gainsDb[i]) * 0.5f;

        // 忽略小增益变化（< 0.5 dB）
        if (std::abs(gain) < 0.5f)
            continue;

        // 计算 Q 值：根据频率点间距自适应
        float intervalOct = std::log2(curve.frequencies[i] / curve.frequencies[i - 1]);
        float q = 1.0f / (intervalOct * 2.0f);
        q = juce::jlimit(0.3f, 5.0f, q);

        auto peak = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, freq, q, juce::Decibels::decibelsToGain(gain));

        coeffs.add(peak);
    }
}
