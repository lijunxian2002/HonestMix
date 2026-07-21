#include "Settings.h"

//==============================================================================
// Settings — 纯接口骨架
//
// 所有硬编码数据已被剥离。返回空列表，保持接口完整。
// 工程师后续通过以下方式注入数据：
//   1. 替换 init*() 方法的返回值
//   2. 或从外部 JSON 文件加载
//==============================================================================

//==============================================================================
// ——— 耳机型号 ———
const juce::Array<Settings::HeadphoneEntry>& Settings::getHeadphonePresets()
{
    static juce::Array<HeadphoneEntry> empty;
    return empty;
}

juce::String Settings::getHeadphoneName(int)
{
    return {};
}

int Settings::findHeadphoneIndex(const juce::String&)
{
    return -1;
}

//==============================================================================
// ——— 声卡型号 ———
const juce::Array<Settings::InterfaceEntry>& Settings::getInterfacePresets()
{
    static juce::Array<InterfaceEntry> empty;
    return empty;
}

juce::String Settings::getInterfaceName(int)
{
    return {};
}

int Settings::findInterfaceIndex(const juce::String&)
{
    return -1;
}

//==============================================================================
// ——— 混音习惯 ———
const juce::Array<Settings::HabitEntry>& Settings::getHabitPresets()
{
    static juce::Array<HabitEntry> empty;
    return empty;
}

const juce::Array<Settings::CheckEnvironmentEntry>& Settings::getCheckEnvironmentPresets()
{
    static juce::Array<CheckEnvironmentEntry> empty;
    return empty;
}

//==============================================================================
// ——— 干湿比 ———
const juce::Array<Settings::WetDryPreset>& Settings::getWetDryPresets()
{
    static juce::Array<WetDryPreset> empty;
    return empty;
}

//==============================================================================
// ——— 时间参考标签 ———
const juce::Array<std::pair<int, juce::String>>& Settings::getPreDelayLabels()
{
    static juce::Array<std::pair<int, juce::String>> empty;
    return empty;
}

const juce::Array<std::pair<int, juce::String>>& Settings::getReverbLabels()
{
    static juce::Array<std::pair<int, juce::String>> empty;
    return empty;
}

const juce::Array<std::pair<int, juce::String>>& Settings::getReleaseLabels()
{
    static juce::Array<std::pair<int, juce::String>> empty;
    return empty;
}

const juce::Array<std::pair<juce::String, double>>& Settings::getDelayNoteRatios()
{
    static juce::Array<std::pair<juce::String, double>> empty;
    return empty;
}
