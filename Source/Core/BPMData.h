#pragma once

//==============================================================================
/**
 * BPM 数据模块 — 纯计算、无 UI
 *
 * 输入 BPM → 输出所有混音常用时间值（ms）。
 * 每拍时长 / 预延迟 / 混响 / 压缩释放 / 延迟时间 —— 全在这里。
 *
 * 用法：
 *   BPMData bpm (120);
 *   int ms = bpm.beatMs();           // 每拍 500ms
 *   int predelay = bpm.preShort();   // 16ms（短预延迟）
 *   ...
 *   bpm.setBPM (117);                // 宿主 BPM 变化时更新
 */
#include <juce_core/juce_core.h>

class BPMData
{
public:
    explicit BPMData (double bpm = 120);

    void setBPM (double bpm);
    double getBPM() const noexcept { return bpm_; }

    // ── 基本 ──
    double beatMs()       const noexcept;    // 每拍 (ms)
    double twoBeatMs()    const noexcept;    // 每两拍

    // ── 预延迟 (PREDELAY) ──
    double preShort()     const noexcept;    // 短 (ms/32)
    double preMedium()    const noexcept;    // 中 (ms/16)
    double preLong()      const noexcept;    // 长 (ms/8)

    // ── 混响 (REVERB) ── 每项返回两个值：短/长
    struct Pair { double a, b; };
    Pair roomReverb()  const noexcept;    // 房间 {ms*0.5, ms*1.0}
    Pair plateReverb() const noexcept;    // 板式 {ms*2.0, ms*4.0}
    Pair hallReverb()  const noexcept;    // 大厅 {ms*4.0, ms*8.0}

    // ── 压缩释放 (COMPRESSOR) ──
    Pair compFast()    const noexcept;    // {ms/16, ms/8}
    Pair compMedium()  const noexcept;    // {ms/4,  ms/2}
    Pair compSlow()    const noexcept;    // {ms,    ms*2}

    // ── 延迟 (DELAY) ── 按音符分度
    double delayHalf()    const noexcept;    // 1/2 whole note = 2 quarter beats
    double delayQuarter() const noexcept;    // 1/4 = one quarter beat
    double delayEighth()  const noexcept;    // 1/8 = half quarter beat
    double delay16th()    const noexcept;    // 1/16 = quarter beat / 4
    double delay64th()    const noexcept;    // 1/64 = quarter beat / 16

    // ── 格式化（带单位）──
    static juce::String fmtMs (double ms);

private:
    double bpm_ = 120;
    double safeBeatMs() const noexcept;      // 防零除

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BPMData)
};
