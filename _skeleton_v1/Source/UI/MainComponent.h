#pragma once

#include <JuceHeader.h>
#include "../Core/AppState.h"

// 前置声明（减少头文件依赖）
class SetupWizard;
class TransitionCard;
class MainUI;
class BPMAssistant;
class MonitorCheck;

//==============================================================================
/**
 * 根 UI 组件 —— 状态机调度与子组件容器
 *
 * 职责：
 *   1. 监听 AppState::AppPhase 切换并呈现对应 UI
 *   2. 管理所有顶级子组件的创建、显示、隐藏
 *   3. 处理 1 小时检查计时器
 *   4. 作为叠加层 (overlay) 的调度中心
 *
 * 布局：
 *   根据 AppPhase 切换布局方式：
 *     Unconfigured → SetupWizard 居中
 *     Transition   → TransitionCard 居中覆盖
 *     Active       → MainUI + BPMAssistant（可切换）
 *
 * 生命周期：
 *   由 HonestMixAudioProcessorEditor 创建并作为其唯一子组件
 */
class MainComponent : public juce::Component,
                      private juce::Timer,
                      private juce::ChangeListener
{
public:
    //==============================================================================
    explicit MainComponent(AppState& appState);
    ~MainComponent() override;

    //==============================================================================
    // ——— Component 接口 ———

    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // ——— 状态机操作 ———

    /** 3 步设置完成 → 进入 Transition 阶段 */
    void onSetupCompleted();

    /** 过渡卡确认 → 进入 Active 阶段 */
    void onTransitionConfirmed();

    /** 开始混音 */
    void startMix();

    /** 重置所有状态（回到第一步） */
    void resetAll();

    //==============================================================================
    // ——— 面板切换 ———

    void showBPMAssistant();
    void hideBPMAssistant();
    void toggleBPMAssistant();

    void showMonitorCheck();
    void hideMonitorCheck();

private:
    //==============================================================================
    AppState& appState;

    // ——— 子组件 ———
    std::unique_ptr<SetupWizard>      setupWizard;
    std::unique_ptr<TransitionCard>    transitionCard;
    std::unique_ptr<MainUI>           mainUI;
    std::unique_ptr<BPMAssistant>     bpmAssistant;
    std::unique_ptr<MonitorCheck>     monitorCheck;

    // ——— 叠加层追踪 ———
    juce::Component* currentOverlay = nullptr;

    //==============================================================================
    // ——— 内部方法 ———

    /** 应用当前阶段的布局 */
    void applyPhaseLayout();

    /** 隐藏所有阶段组件 */
    void hideAllPhaseComponents();

    /** 显示一个叠加层（隐藏之前的） */
    void showOverlay(juce::Component* overlay);

    /** 隐藏当前叠加层 */
    void hideOverlay();

    // ——— ChangeListener ———
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // ——— Timer (1 小时检查) ———
    void timerCallback() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
