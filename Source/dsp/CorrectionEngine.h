#pragma once

#include <juce_dsp/juce_dsp.h>

/**
 * 校正引擎：内置一段 1024 点 FIR 脉冲响应，
 * 用 JUCE 的分区卷积实现。
 *
 * M0 阶段使用硬编码的简单 EQ 曲线数据，
 * 仅验证卷积管线通断。
 */
class CorrectionEngine
{
public:
    CorrectionEngine() = default;
    ~CorrectionEngine() = default;

    /** 准备卷积器（采样率、块大小须与 processBlock 一致） */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** 重置内部状态（采样率切换时调用） */
    void reset();

    /** 处理立体声 buffer */
    void process (juce::AudioBuffer<float>& buffer);

    /** 启用 / 禁用校正 */
    void setEnabled (bool enabled) noexcept { enabled_ = enabled; }
    bool isEnabled() const noexcept         { return enabled_; }

    /** 返回当前卷积延迟（采样点） */
    int getLatencySamples() const noexcept { return 1024; }

private:
    /** 生成一个 1024 点的简单 EQ 脉冲响应（硬编码占位数据） */
    static juce::Array<float> generateImpulseResponse (double sampleRate);

    juce::dsp::Convolution convolution_;
    bool enabled_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CorrectionEngine)
};
