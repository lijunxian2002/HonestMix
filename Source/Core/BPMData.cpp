#include "Core/BPMData.h"

BPMData::BPMData (int bpm) : bpm_ (juce::jlimit (20, 300, bpm)) {}

void BPMData::setBPM (int bpm)
{
    bpm_ = juce::jlimit (20, 300, bpm);
}

int BPMData::safeBeatMs() const noexcept
{
    return juce::roundToInt (60000.0 / (double) bpm_);
}

// ── 基本 ──
int BPMData::beatMs()    const noexcept { return safeBeatMs(); }
int BPMData::twoBeatMs() const noexcept { return beatMs() * 2; }

// ── 预延迟 ──
int BPMData::preShort()  const noexcept { return beatMs() / 32; }
int BPMData::preMedium() const noexcept { return beatMs() / 16; }
int BPMData::preLong()   const noexcept { return beatMs() / 8; }

// ── 混响 ──
BPMData::Pair BPMData::roomReverb()  const noexcept { int b = beatMs(); return { (int)(b * 0.5), b }; }
BPMData::Pair BPMData::plateReverb() const noexcept { int b = beatMs(); return { b * 2, b * 4 }; }
BPMData::Pair BPMData::hallReverb()  const noexcept { int b = beatMs(); return { b * 4, b * 8 }; }

// ── 压缩 ──
BPMData::Pair BPMData::compFast()   const noexcept { int b = beatMs(); return { b / 16, b / 8 }; }
BPMData::Pair BPMData::compMedium() const noexcept { int b = beatMs(); return { b / 4,  b / 2 }; }
BPMData::Pair BPMData::compSlow()   const noexcept { int b = beatMs(); return { b,     b * 2 }; }

// ── 延迟 ──
int BPMData::delayHalf()    const noexcept { return beatMs(); }
int BPMData::delayQuarter() const noexcept { return beatMs() / 2; }
int BPMData::delayEighth()  const noexcept { return beatMs() / 4; }
int BPMData::delay16th()    const noexcept { return beatMs() / 8; }
int BPMData::delay64th()    const noexcept { return beatMs() / 32; }

// ── 格式化 ──
juce::String BPMData::fmtMs (int ms)
{
    return ms >= 1000 ? juce::String (ms / 1000.0, 2) + " s"
                      : juce::String (ms) + " ms";
}
