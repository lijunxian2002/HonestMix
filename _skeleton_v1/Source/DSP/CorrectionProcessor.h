#pragma once

#include <JuceHeader.h>
#include "../Core/HeadphoneDatabase.h"

//==============================================================================
/**
 * 均衡校正处理器
 *
 * 核心 DSP 模块。职责：
 *   - 根据 HeadphoneDatabase 中的校正曲线，构建 IIR 滤波器组
 *   - 对输入音频应用校正 EQ，使耳机频响趋近目标曲线
 *   - 支持 bypass / enable 切换
 *
 * 设计：
 *   - 使用 juce::dsp::ProcessorChain 管理滤波器
 *   - 曲线以若干个 peaking / shelving 滤波器逼近
 *   - 典型的耳机校正需要 10–20 个 IIR 双二阶节
 *
 * 线程安全：
 *   - prepare() 和 process() 在音频线程调用
 *   - loadCurve() 可能在 UI 线程调用，通过 Atomic 标志同步
 */
class CorrectionProcessor
{
public:
    //==============================================================================
    CorrectionProcessor();
    ~CorrectionProcessor() = default;

    //==============================================================================
    // ——— 生命周期 ———

    /** 准备处理（在音频线程调用） */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /** 重置滤波器内部状态 */
    void reset();

    /** 处理单个音频 buffer */
    void process(juce::AudioBuffer<float>& buffer);

    //==============================================================================
    // ——— 曲线管理 ———

    /**
     * 加载新的校正曲线
     * @param curve  耳机校正曲线数据（来自 HeadphoneDatabase）
     * @param sampleRate 当前采样率（滤波器系数与采样率相关）
     */
    void loadCurve(const HeadphoneDatabase::Curve& curve, double sampleRate);

    /** 清空校正（pass-through） */
    void clearCurve();

    /** 是否有加载曲线 */
    bool hasCurve() const noexcept { return curveLoaded; }

    //==============================================================================
    // ——— 参数 ———

    /** 启用/禁用校正处理 */
    void setEnabled(bool enabled) noexcept { isEnabled = enabled; }
    bool getEnabled() const noexcept { return isEnabled; }

    /** 获取当前使用的曲线 ID */
    juce::String getCurrentCurveId() const noexcept { return currentCurveId; }

private:
    //==============================================================================
    // 使用 IIR 滤波器组来实现 EQ 校正
    // 每个滤波器对应 Curve 中的一个频段调整
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterChain = juce::dsp::ProcessorChain<juce::dsp::IIR::Coefficients<float>>;

    juce::OwnedArray<FilterChain> filterChains;
    juce::dsp::DryWetMixer<float> dryWetMixer;

    bool curveLoaded = false;
    bool isEnabled = true;
    juce::String currentCurveId;

    //==============================================================================
    // 内部方法

    /**
     * 根据 Curve 数据生成滤波器系数数组
     *
     * 使用多个 peaking + shelving 滤波器逼近目标曲线。
     * 使用简单的线性插值在频率点之间平滑过渡。
     */
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>
        createCoefficients(const HeadphoneDatabase::Curve& curve, double sampleRate);

    /** 分析 Curve 中的平坦区域，合并为 shelving 滤波器 */
    static void addShelvingFilters(
        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>& coeffs,
        const HeadphoneDatabase::Curve& curve, double sampleRate);

    /** 为 Curve 中间的起伏区域添加 peaking 滤波器 */
    static void addPeakingFilters(
        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>& coeffs,
        const HeadphoneDatabase::Curve& curve, double sampleRate);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CorrectionProcessor)
};
