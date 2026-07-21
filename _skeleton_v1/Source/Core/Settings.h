#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * HonestMix 用户配置 —— 纯数据容器
 *
 * 职责：
 *   - 持有所有用户可选列表（耳机型号、声卡型号、混音习惯选项）
 *   - 提供选中项的索引 ↔ 显示名称 转换
 *   - 提供默认值常量
 *
 * 设计原则：
 *   - 无业务逻辑，纯数据 + 常量
 *   - 静态工厂方法创建设备列表
 *   - 与 AppState 配合使用：AppState 存索引，Settings 提供映射表
 */
class Settings
{
public:
    //==============================================================================
    // ——— 耳机型号预设 ———

    struct HeadphoneEntry
    {
        juce::String name;      ///< 显示名称
        bool         isPopular; ///< 是否标记为"热门"
        juce::String curveId;   ///< 对应 HeadphoneDatabase 中的校正曲线 ID
    };

    /** 返回内置耳机型号列表 */
    static const juce::Array<HeadphoneEntry>& getHeadphonePresets();

    /** 根据索引获取耳机名称 */
    static juce::String getHeadphoneName(int index);
    /** 根据名称查找索引（未找到返回 -1） */
    static int findHeadphoneIndex(const juce::String& name);

    //==============================================================================
    // ——— 声卡型号预设 ———

    struct InterfaceEntry
    {
        juce::String name;
        bool         isPopular;
    };

    /** 返回内置声卡型号列表 */
    static const juce::Array<InterfaceEntry>& getInterfacePresets();

    static juce::String getInterfaceName(int index);
    static int findInterfaceIndex(const juce::String& name);

    //==============================================================================
    // ——— 混音习惯选项 ———

    struct HabitEntry
    {
        juce::String label;
    };

    static const juce::Array<HabitEntry>& getHabitPresets();

    struct CheckEnvironmentEntry
    {
        juce::String label;
    };

    static const juce::Array<CheckEnvironmentEntry>& getCheckEnvironmentPresets();

    //==============================================================================
    // ——— 干湿比预设 ———

    struct WetDryPreset
    {
        float        percent; ///< 0–100
        juce::String label;
        juce::String subtitle;
    };

    static const juce::Array<WetDryPreset>& getWetDryPresets();

    //==============================================================================
    // ——— 默认值 ———

    static constexpr int   defaultBPM    = 120;
    static constexpr float defaultWetDry = 50.0f;
    static constexpr bool  defaultCorrectionEnabled = true;

    //==============================================================================
    // ——— 时间参考标签 ———

    /** 预延迟参考值对应的标签 */
    static const juce::Array<std::pair<int, juce::String>>& getPreDelayLabels();

    /** 混响参考值标签 */
    static const juce::Array<std::pair<int, juce::String>>& getReverbLabels();

    /** 压缩释放标签 */
    static const juce::Array<std::pair<int, juce::String>>& getReleaseLabels();

    /** 延迟音符对应标签 */
    static const juce::Array<std::pair<juce::String, double>>& getDelayNoteRatios();

private:
    Settings() = default; // 静态类，禁止实例化

    static juce::Array<HeadphoneEntry>    initHeadphones();
    static juce::Array<InterfaceEntry>    initInterfaces();
    static juce::Array<HabitEntry>        initHabits();
    static juce::Array<CheckEnvironmentEntry> initCheckEnvs();
    static juce::Array<WetDryPreset>      initWetDry();
    static juce::Array<std::pair<int, juce::String>> initPreDelay();
    static juce::Array<std::pair<int, juce::String>> initReverb();
    static juce::Array<std::pair<int, juce::String>> initRelease();
    static juce::Array<std::pair<juce::String, double>> initDelayNotes();
};
