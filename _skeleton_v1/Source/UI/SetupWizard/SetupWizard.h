#pragma once

#include <JuceHeader.h>
#include "../../Core/AppState.h"

class StepIndicator;
class StepHeadphone;
class StepInterface;
class StepHabits;
class AudioInterfaceBrowser;

//==============================================================================
/**
 * 3 步设置向导容器
 *
 * 管理以下流程：
 *   步骤 1: StepHeadphone  →  "下一步"
 *   步骤 2: StepInterface  →  "下一步"
 *   步骤 3: StepHabits     →  "完成设置"
 *
 * 步骤间切换动画：淡入淡出
 * 每个步骤验证通过后启用导航按钮。
 */
class SetupWizard : public juce::Component
{
public:
    //==============================================================================
    SetupWizard(AppState& appState, std::function<void()> onComplete);
    ~SetupWizard() override;

    //==============================================================================
    void resized() override;
    void paint(juce::Graphics& g) override;

    //==============================================================================
    /** 切换到指定步骤 (1–3) */
    void goToStep(int step);

    /** 下一步 */
    void nextStep();

private:
    //==============================================================================
    AppState& appState;
    std::function<void()> onComplete;

    // ——— 子组件 ———
    std::unique_ptr<StepIndicator>       stepIndicator;
    std::unique_ptr<StepHeadphone>       stepHeadphone;
    std::unique_ptr<StepInterface>       stepInterface;
    std::unique_ptr<StepHabits>          stepHabits;
    std::unique_ptr<AudioInterfaceBrowser> interfaceBrowser;

    juce::TextButton nextButton;
    juce::TextButton backButton;  // 仅在步骤 2、3 可见

    int currentStep = 1;

    // ——— 内部方法 ———

    void showStep(int step);
    void hideAllSteps();
    void updateButtonState();
    void onSelectionChanged();

    /** 保存选中的耳机型号到 AppState */
    void saveHeadphoneSelection();

    /** 保存选中的声卡型号到 AppState */
    void saveInterfaceSelection(int index, const juce::String& name);

    /** 保存混音习惯到 AppState */
    void saveHabitSelection();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SetupWizard)
};
