#pragma once

//==============================================================================
/**
 * 翻译度混合器 —— 骨架 WetDryMixer 槽位 × 数据中心 0~200 语义
 *
 *   out = wet * t + dry * (1 - t)   其中 t = degree / 100
 *     0   → 原始（FIR ×0，即 bypass）
 *     100 → 标准 Harman（FIR ×1.0）
 *     200 → 双倍矫正（FIR ×2.0）
 *
 * 信号链位置：FIR 卷积始终 ON，本类负责"预存干信号 + SIMD 混合"。
 * 数据中心的这段逻辑内联在 processBlock 里，现抽为独立类 ——
 * 翻译度相关的爆音/相位/增益 BUG 只看这一个文件。
 *
 * 线程安全：setTranslation 由 UI 线程写，process 由音频线程读（atomic）。
 */
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class WetDryMixer
{
public:
    WetDryMixer() = default;

    /** 分配干信号缓冲（prepareToPlay 时调用） */
    void prepare (int numChannels, int maxBlockSize);

    /** 设置翻译度 0~200（UI 线程） */
    void  setTranslation (float degree) noexcept { degree_.store (juce::jlimit (0.0f, 200.0f, degree)); }
    float getTranslation() const noexcept        { return degree_.load(); }

    /**
     * 音频线程：buffer 进的是 FIR 卷积后的湿信号，
     * 内部先预存干信号副本，再按翻译度原位混合。
     * （调用方须保证：进本函数前 buffer 已完成卷积，且本函数
     *   自己负责在卷积前抓干信号 —— 见 process(dry, wet) 重载）
     */
    void captureDry (const juce::AudioBuffer<float>& buffer);
    void mixInPlace (juce::AudioBuffer<float>& wetBuffer);

private:
    juce::AudioBuffer<float> dry_;
    std::atomic<float> degree_ { 100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WetDryMixer)
};
