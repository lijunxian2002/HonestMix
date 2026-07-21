#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 干湿比混合器
 *
 * 将原始信号（干）与校正后信号（湿）按比例混合：
 *   output = dry * (1 - mix) + wet * mix
 *
 * 在 HonestMix 中：
 *   - mix = 0%   → 输出原始信号（校正 bypass）
 *   - mix = 50%  → 一半校正，一半原始（强烈对比模式）
 *   - mix = 100% → 完全校正信号（直接信任模式）
 *
 * 使用 juce::dsp::DryWetMixer 实现，带平滑过渡避免咔嗒声。
 */
class WetDryMixer
{
public:
    //==============================================================================
    WetDryMixer() = default;
    ~WetDryMixer() = default;

    //==============================================================================
    // ——— 生命周期 ———

    void prepare(const juce::dsp::ProcessSpec& spec);

    void reset();

    /**
     * 处理音频 block
     * @param dryBuffer  干信号（原始输入）
     * @param wetBuffer  湿信号（校正处理后）
     * @param outputBuffer 输出 buffer
     */
    void process(const juce::AudioBuffer<float>& dryBuffer,
                 const juce::AudioBuffer<float>& wetBuffer,
                 juce::AudioBuffer<float>& outputBuffer);

    //==============================================================================
    // ——— 参数 ———

    /**
     * 设置干湿混合比例
     * @param mixPercent 0–100 (%)
     *   0   = 全干（原始信号）
     *   50  = 等量混合
     *   100 = 全湿（完全校正）
     */
    void setMixPercent(float mixPercent);

    /** 返回当前干湿比例 (0–100) */
    float getMixPercent() const noexcept { return mixPercent; }

private:
    //==============================================================================
    juce::dsp::DryWetMixer<float> dryWetMixer;
    float mixPercent = 50.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WetDryMixer)
};
