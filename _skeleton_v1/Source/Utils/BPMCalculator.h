#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * BPM 时间值计算器
 *
 * 将 BPM（每分钟拍数）转换为各类混音中常用的时间值：
 *   - 每拍时长 (ms)
 *   - 预延迟 (PREDELAY) 推荐值
 *   - 混响时间 (REVERB TIME) 推荐值
 *   - 压缩释放时间 (COMPRESSOR RELEASE) 推荐值
 *   - 延迟时间 (DELAY TIME) 各种音符分度值
 *
 * 用法：
 *   BPMCalculator calc(120);
 *   auto beatMs = calc.getBeatDurationMs();        // → 500 ms
 *   auto halfNoteDelay = calc.getDelayMs(2.0);     // → 1000 ms
 *   auto eighthDelay   = calc.getDelayMs(0.5);     // → 250 ms
 */
class BPMCalculator
{
public:
    //==============================================================================
    explicit BPMCalculator(int bpm = 120);

    //==============================================================================
    // ——— BPM ———

    void setBPM(int newBPM);
    int  getBPM() const noexcept { return bpm; }

    //==============================================================================
    // ——— 基本计算 ———

    /** 每拍时长 (ms) = 60000 / BPM */
    double getBeatDurationMs() const noexcept;

    /** 每两拍时长 (ms) */
    double getTwoBeatsMs() const noexcept;

    /** 四分之一音符时长 (ms) = 每拍时长 */
    double getQuarterMs() const noexcept;

    //==============================================================================
    // ——— 延迟时间 (DELAY) ———

    /**
     * 根据音符分度比计算延迟时间
     * @param noteRatio 音符分度比: 1.0 = 四分音符, 0.5 = 八分音符, 2.0 = 二分音符
     */
    double getDelayMs(double noteRatio) const noexcept;

    //==============================================================================
    // ——— 预延迟 (PREDELAY) ———

    /** 推荐预延迟值（短 / 中 / 长 三档） */
    double getShortPreDelayMs()  const noexcept { return 16.0; }
    double getMediumPreDelayMs() const noexcept { return 32.0; }
    double getLongPreDelayMs()   const noexcept { return 64.0; }

    //==============================================================================
    // ——— 混响衰减时间 (REVERB DECAY) ———

    /** 根据 BPM 推荐的房间混响时间 (ms) */
    double getRoomReverbMs() const noexcept;
    /** 推荐的板式混响时间 (ms) */
    double getPlateReverbMs() const noexcept;
    /** 推荐的大厅混响时间 (ms) */
    double getHallReverbMs() const noexcept;

    //==============================================================================
    // ——— 压缩释放 (COMPRESSOR RELEASE) ———

    /** 推荐的快速释放时间 (ms) */
    double getFastReleaseMs()   const noexcept;
    double getMediumReleaseMs() const noexcept;
    double getSlowReleaseMs()   const noexcept;

    //==============================================================================
    // ——— 辅助 ———

    /** 格式化为带单位的字符串，如 "513 ms" */
    static juce::String formatMs(double ms, int decimalPlaces = 0);

private:
    //==============================================================================
    int bpm = 120;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BPMCalculator)
};
