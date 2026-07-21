#pragma once

//==============================================================================
/**
 * 耳机数据库 —— 1603 副 AutoEq FIR 数据的唯一访问闸门
 *
 * 拼接说明（数据中心 → 骨架 Core 槽位）：
 *   骨架的 HeadphoneDatabase 原本是空 JSON 容器；这里改为直接绑定
 *   Source/dsp/fir_data.h 中的编译期常量表 kProfiles（25MB / 1603 副）。
 *   全工程只有本类的 .cpp 包含 fir_data.h —— 改数据脚本/换数据文件
 *   只需动这一个翻译单元。
 *
 * 另含两张小表（原散落于 PluginEditor 的 HeadphoneBrowser 静态成员）：
 *   目标曲线表（4 条）/ 声卡表（8 条）/ 热门耳机预设（8 副）
 */
#include <juce_core/juce_core.h>
#include <vector>

/** 与 fir_data.h 中保持一致的前置声明（避免本头文件引入 25MB 数据） */
struct HeadphoneInfo;

class HeadphoneDatabase
{
public:
    HeadphoneDatabase();

    // ── 耳机档案 ────────────────────────────────────────────
    int         getNumProfiles() const noexcept;
    const char* getProfileName (int index) const noexcept;   ///< 越界返回 "Unknown"
    const float* getProfileFIR (int index) const noexcept;   ///< 越界返回 nullptr（1024 taps）
    int         findProfileByName (const juce::String& name) const noexcept; ///< 未找到 -1

    /** 子串模糊搜索（大小写不敏感），结果写入 out（kProfiles 下标） */
    void search (const juce::String& filter, std::vector<int>& out,
                 int maxResults = -1) const;

    /** "Audio-Technica ATH-M50x" → "ATH-M50x"（去厂商前缀） */
    static juce::String shortName (const juce::String& full);
    /** "Audio-Technica ATH-M50x" → "Audio-Technica"（取厂商前缀） */
    static juce::String brandOf  (const juce::String& full);

    // ── 目标曲线 / 声卡 / 热门预设（数据中心的静态表）────────
    static const juce::StringArray& getCurveNames();       ///< 4 条目标曲线
    static const juce::StringArray& getInterfaceNames();   ///< 8 款声卡
    static const juce::StringArray& getPopularPresets();   ///< 8 副热门耳机全名

    static constexpr int kFirLength = 1024;

private:
    const HeadphoneInfo* table_ = nullptr;  // → kProfiles（fir_data.h）
    int                  count_ = 0;        // → NUM_PROFILES

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeadphoneDatabase)
};
