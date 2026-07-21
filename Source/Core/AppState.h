#pragma once

//==============================================================================
/**
 * 应用状态中心 —— 骨架 AppState 的 ValueTree 模式 × 数据中心持久化字段
 *
 * 职责（单一数据源）：
 *   - 翻译度 degree (0~200)、当前耳机全名、引导完成标记、监听模式、
 *     曲线/声卡选择 —— 全部落在一棵 ValueTree 上
 *   - 序列化 ↔ PluginProcessor::getStateInformation / setStateInformation
 *   - UI 线程写、音频线程只读镜像（PluginProcessor 内的原子/副本），
 *     本类本身不进音频线程
 *
 * 注意：骨架里的"阶段机 AppPhase"未并入 —— 终局 UI 的阶段由
 *   遮罩可见性唯一表达（setup → transition → 主界面），双状态机 = bug 温床。
 */
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>

class AppState
{
public:
    AppState();

    // ── 持久化字段 ──────────────────────────────────────────
    void  setTranslationDegree (float v);        ///< 自动夹取 0~200
    float getTranslationDegree() const noexcept;

    void         setProfileName (const juce::String& name);
    juce::String getProfileName() const;

    void setOnboarded (bool v) noexcept;
    bool isOnboarded() const noexcept;

    void setListenMode (int v) noexcept;
    int  getListenMode() const noexcept;

    void setCurveIndex (int v) noexcept;
    int  getCurveIndex() const noexcept;

    void setInterfaceIndex (int v) noexcept;
    int  getInterfaceIndex() const noexcept;

    /** 混音时长习惯（分钟）—— 驱动"换监听提醒"节奏，默认 60 */
    void setMixHabitMinutes (int v) noexcept;
    int  getMixHabitMinutes() const noexcept;

    // ── 序列化（DAW 工程存取）────────────────────────────────
    juce::ValueTree serialize() const;
    void            deserialize (const juce::ValueTree& state);

    /** 状态变更广播（UI 可挂 ChangeListener 响应式刷新） */
    juce::ChangeBroadcaster& getBroadcaster() noexcept { return broadcaster_; }

private:
    juce::ValueTree root_;
    juce::ChangeBroadcaster broadcaster_;

    static const juce::Identifier tagRoot, propDegree, propProfile,
        propOnboarded, propListen, propCurve, propInterface, propMixHabit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppState)
};
