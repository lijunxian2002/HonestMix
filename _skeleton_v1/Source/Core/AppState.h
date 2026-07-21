#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 应用阶段枚举
 *
 * 控制 HonestMix 整个生命周期的 UI 状态机：
 *   Unconfigured  →  Transition  →  Active  →  Feedback
 *       (3 步设置)    (干湿比选择)   (混音中)    (完成后反馈)
 */
enum class AppPhase
{
    Unconfigured,   ///< 首次使用 / 重置后：显示 SetupWizard
    Transition,     ///< 设置完成 → 显示过渡卡选择干湿比
    Active,         ///< 开始混音 → 显示 MainUI
    Feedback        ///< 混音完成 → 显示 FeedbackDialog
};

//==============================================================================
/**
 * HonestMix 应用状态中心
 *
 * 职责：
 *   - 维护当前 AppPhase 状态机
 *   - 存储用户配置（耳机 / 声卡 / 习惯）
 *   - 跟踪运行时数据（干湿比、校正开关、BPM）
 *   - 管理混音计时（1 小时检查提醒）
 *   - 序列化 / 反序列化（基于 JUCE ValueTree）
 *
 * 设计原则：
 *   - 所有状态变更通过 setter 方法，触发 ValueTree::Listener 通知
 *   - UI 层通过 ValueTree::Listener 或 ChangeBroadcaster 订阅变更
 *   - 不直接持有任何 UI 或 DSP 对象引用 —— 纯数据层
 */
class AppState
{
public:
    //==============================================================================
    /** 构造。rootState 必须是由 PluginProcessor 持久化管理的 ValueTree。 */
    explicit AppState(juce::ValueTree rootState);

    //==============================================================================
    // ——— 阶段管理 ———

    /** 返回当前阶段 */
    AppPhase getCurrentPhase() const noexcept;

    /** 设置当前阶段（会触发 ValueTree 变更回调） */
    void setPhase(AppPhase newPhase);

    /** 重置为 Unconfigured（清空所有配置） */
    void reset();

    //==============================================================================
    // ——— 设置数据 ———

    void  setHeadphoneModel(int index);
    int   getHeadphoneModel() const noexcept;

    void  setHeadphoneModelName(const juce::String& name);
    juce::String getHeadphoneModelName() const;

    void  setAudioInterface(int index);
    int   getAudioInterface() const noexcept;

    void  setAudioInterfaceName(const juce::String& name);
    juce::String getAudioInterfaceName() const;

    void  setMixingHabit(int index);
    int   getMixingHabit() const noexcept;

    void  setCheckOtherEnvironment(int index);
    int   getCheckOtherEnvironment() const noexcept;

    /** 三步设置是否全部完成 */
    bool isSetupComplete() const noexcept;

    //==============================================================================
    // ——— 运行时数据 ———

    void  setWetDryMix(float percent);          ///< 0–100 %
    float getWetDryMix() const noexcept;

    void  setCorrectionEnabled(bool enabled);
    bool  isCorrectionEnabled() const noexcept;

    void  setBPM(int bpm);
    int   getBPM() const noexcept;

    //==============================================================================
    // ——— 混音计时 ———

    /** 开始／重置混音计时器（调用 startMix() 时自动调用） */
    void resetMixTimer();

    /** 返回自 resetMixTimer 以来经过的分钟数 */
    double getMixElapsedMinutes() const;

    /** 判断是否应该触发 1 小时检查弹窗 */
    bool shouldShowHourlyCheck() const;

    /** 标记 "1 小时检查" 已被处理 */
    void dismissHourlyCheck();

    /** 1 小时检查是否已在本次会话中被 dismiss */
    bool isHourlyCheckDismissed() const noexcept;

    //==============================================================================
    // ——— 持久化 ———

    /** 导出完整状态为 ValueTree（PluginProcessor 保存到 DAW 工程） */
    juce::ValueTree serialize() const;

    /** 从 ValueTree 恢复状态 */
    void deserialize(const juce::ValueTree& state);

    //==============================================================================
    // ——— Event / Listener 支持 ———

    /**
     * 当状态发生任何变化时发送变更通告。
     * UI 组件可附加 ChangeListener 以响应式更新。
     */
    juce::ChangeBroadcaster& getChangeBroadcaster() noexcept { return broadcaster; }

    /** 底层 ValueTree（可直接附加 ValueTree::Listener） */
    juce::ValueTree& getValueTree() noexcept { return root; }
    const juce::ValueTree& getValueTree() const noexcept { return root; }

private:
    //==============================================================================
    juce::ValueTree root;
    juce::ChangeBroadcaster broadcaster;

    // 计时相关
    juce::Time mixStartTime;
    bool hourlyCheckHandled = false;

    //==============================================================================
    // ValueTree 结点 ID 和属性名（包内可见，方便持久化）
    static constexpr auto tagRoot       = "HonestMixState";
    static constexpr auto propPhase     = "phase";
    static constexpr auto propHeadphone = "headphoneModel";
    static constexpr auto propHeadphoneName = "headphoneName";
    static constexpr auto propInterface = "audioInterface";
    static constexpr auto propInterfaceName = "interfaceName";
    static constexpr auto propHabit     = "mixingHabit";
    static constexpr auto propCheckEnv  = "checkEnvironment";
    static constexpr auto propWetDry    = "wetDryMix";
    static constexpr auto propCorrect   = "correctionEnabled";
    static constexpr auto propBPM       = "bpmValue";

    /** 辅助：获取或创建 int 属性 */
    int getIntProp(const char* propName, int defaultVal) const;
    void setIntProp(const char* propName, int value);
    float getFloatProp(const char* propName, float defaultVal) const;
    void setFloatProp(const char* propName, float value);
    juce::String getStrProp(const char* propName, const juce::String& defaultVal) const;
    void setStrProp(const char* propName, const juce::String& value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppState)
};
