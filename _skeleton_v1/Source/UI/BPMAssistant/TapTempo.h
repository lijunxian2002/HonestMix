#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 按速度检测 (Tap Tempo)
 *
 * 通过检测用户连续点击的时间间隔来计算 BPM。
 * 算法：
 *   1. 记录最近 N 次点击的时间戳
 *   2. 计算连续点击之间的平均间隔
 *   3. BPM = 60000 / 平均间隔(ms)
 *
 * 防抖：
 *   - 忽略间隔 < 100ms 的点击（防抖动）
 *   - 如果两次点击间隔 > 3s，重置历史
 */
class TapTempo
{
public:
    //==============================================================================
    TapTempo();
    ~TapTempo() = default;

    //==============================================================================
    /**
     * 注册一次点击。返回当前估算的 BPM。
     * 如果历史不足（< 2 次点击），返回 0。
     */
    int registerTap();

    /** 重置点击历史 */
    void reset();

    /** 返回最近计算的 BPM */
    int getCurrentBPM() const noexcept { return currentBPM; }

    /** 返回最近计算的 BPM 的置信度 (0–1) */
    float getConfidence() const noexcept;

    /** 返回已记录的点击次数 */
    int getTapCount() const noexcept { return tapTimes.size(); }

private:
    //==============================================================================
    static constexpr int maxTaps = 8;       ///< 保留最近 8 次点击
    static constexpr int minIntervalMs = 100;  ///< 最小有效间隔
    static constexpr int resetTimeoutMs = 3000; ///< 超时重置

    juce::Array<juce::Time> tapTimes;
    int currentBPM = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapTempo)
};
