#include "MainComponent.h"
#include "LookAndFeel.h"
#include "SetupWizard/SetupWizard.h"
#include "TransitionCard.h"
#include "MainUI.h"
#include "BPMAssistant/BPMAssistant.h"
#include "MonitorCheck.h"

//==============================================================================
MainComponent::MainComponent(AppState& state)
    : appState(state)
{
    setLookAndFeel(&HonestMixLookAndFeel::getInstance());

    // 监听 AppState 变更
    appState.getChangeBroadcaster().addChangeListener(this);

    // 创建子组件（懒加载模式）
    setupWizard = std::make_unique<SetupWizard>(appState,
        [this]() { onSetupCompleted(); });

    transitionCard = std::make_unique<TransitionCard>(appState,
        [this]() { onTransitionConfirmed(); });

    mainUI      = std::make_unique<MainUI>(appState);

    bpmAssistant = std::make_unique<BPMAssistant>(appState);

    monitorCheck = std::make_unique<MonitorCheck>(appState,
        [this]() { hideMonitorCheck(); });

    // 添加并隐藏所有子组件
    for (auto* comp : { setupWizard.get(), transitionCard.get(),
                        mainUI.get(),
                        bpmAssistant.get(), monitorCheck.get() })
    {
        addAndMakeVisible(comp);
        comp->setVisible(false);
    }

    // 应用初始布局
    applyPhaseLayout();

    // 启动计时器（每秒检查一次是否需要 1 小时提醒）
    startTimerHz(1);
}

MainComponent::~MainComponent()
{
    stopTimer();
    appState.getChangeBroadcaster().removeChangeListener(this);
    setLookAndFeel(nullptr);
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // 全局背景
    g.fillAll(HonestMixLookAndFeel::bgDark);
}

void MainComponent::resized()
{
    applyPhaseLayout();
}

//==============================================================================
// ——— 状态机操作 ———

void MainComponent::onSetupCompleted()
{
    appState.setPhase(AppPhase::Transition);
    applyPhaseLayout();
}

void MainComponent::onTransitionConfirmed()
{
    appState.setPhase(AppPhase::Active);
    appState.resetMixTimer();
    applyPhaseLayout();
}

void MainComponent::startMix()
{
    onTransitionConfirmed();
}

void MainComponent::resetAll()
{
    hideOverlay();
    appState.reset();
    applyPhaseLayout();
}

//==============================================================================
// ——— 面板切换 ———

void MainComponent::showBPMAssistant()
{
    if (bpmAssistant != nullptr)
    {
        bpmAssistant->refreshData();
        bpmAssistant->setVisible(true);
        bpmAssistant->toFront(false);
        resized();
    }
}

void MainComponent::hideBPMAssistant()
{
    if (bpmAssistant != nullptr)
    {
        bpmAssistant->setVisible(false);
        resized();
    }
}

void MainComponent::toggleBPMAssistant()
{
    if (bpmAssistant->isVisible())
        hideBPMAssistant();
    else
        showBPMAssistant();
}

void MainComponent::showMonitorCheck()
{
    showOverlay(monitorCheck.get());
}

void MainComponent::hideMonitorCheck()
{
    hideOverlay();
    appState.dismissHourlyCheck();
}

//==============================================================================
// ——— 内部方法 ———

void MainComponent::hideAllPhaseComponents()
{
    for (auto* comp : { setupWizard.get(), transitionCard.get(),
                        mainUI.get() })
    {
        if (comp != nullptr)
            comp->setVisible(false);
    }

    // BPM 助手和 MonitorCheck 是叠加层，不由 phase 直接控制
}

void MainComponent::showOverlay(juce::Component* overlay)
{
    if (currentOverlay != nullptr && currentOverlay != overlay)
        currentOverlay->setVisible(false);

    if (overlay != nullptr)
    {
        overlay->setVisible(true);
        overlay->toFront(false);
        currentOverlay = overlay;
        resized();
    }
}

void MainComponent::hideOverlay()
{
    if (currentOverlay != nullptr)
    {
        currentOverlay->setVisible(false);
        currentOverlay = nullptr;
        resized();
    }
}

void MainComponent::applyPhaseLayout()
{
    auto bounds = getLocalBounds();

    hideAllPhaseComponents();

    switch (appState.getCurrentPhase())
    {
    case AppPhase::Unconfigured:
    {
        // SetupWizard 居中
        if (setupWizard != nullptr)
        {
            setupWizard->setVisible(true);
            auto wizardSize = juce::jmin(bounds.getWidth() - 40, 320);
            setupWizard->setBounds(
                bounds.getCentreX() - wizardSize / 2,
                bounds.getCentreY() - 200,
                wizardSize, 400);
            setupWizard->toFront(false);
        }
        break;
    }

    case AppPhase::Transition:
    {
        // TransitionCard 居中覆盖
        if (transitionCard != nullptr)
        {
            transitionCard->setVisible(true);
            auto cardW = juce::jmin(bounds.getWidth() - 40, 340);
            transitionCard->setBounds(
                bounds.getCentreX() - cardW / 2,
                bounds.getCentreY() - 180,
                cardW, 360);
            transitionCard->toFront(false);
        }
        break;
    }

    case AppPhase::Active:
    case AppPhase::Feedback:
    {
        // MainUI 在右侧
        if (mainUI != nullptr)
        {
            mainUI->setVisible(true);
            mainUI->setBounds(bounds.getRight() - 280,
                              20, 260, 240);
        }

        // BPM 助手（如可见则布局）
        if (bpmAssistant != nullptr && bpmAssistant->isVisible())
        {
            bpmAssistant->setBounds(bounds.getRight() - 350,
                                    20, 330, 460);
            bpmAssistant->toFront(false);
        }

        // MonitorCheck 全屏叠加
        if (monitorCheck != nullptr && monitorCheck->isVisible())
        {
            monitorCheck->setBounds(bounds);
            monitorCheck->toFront(false);
        }
        break;
    }
    }
}

//==============================================================================
// ——— ChangeListener ———

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
    // AppState 发生变更时重新应用布局
    applyPhaseLayout();
}

//==============================================================================
// ——— Timer ———

void MainComponent::timerCallback()
{
    if (appState.getCurrentPhase() == AppPhase::Active
        && appState.shouldShowHourlyCheck()
        && !monitorCheck->isVisible())
    {
        showMonitorCheck();
    }
}
