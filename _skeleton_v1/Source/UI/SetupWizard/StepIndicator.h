#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 步骤进度条 (Step Indicator)
 *
 * 显示 3 圆点进度，对应 SetupWizard 的 3 个步骤：
 *   ● — —   步骤 1
 *   ● ● —   步骤 2
 *   ● ● ●   步骤 3
 *
 * 复刻 HTML 中的 .bar 样式：已完成步骤为白色圆点，未完成步骤为半透明。
 */
class StepIndicator : public juce::Component
{
public:
    //==============================================================================
    StepIndicator();

    /** 设置当前步骤 (1–3) */
    void setCurrentStep(int step);

    /** 返回当前步骤 */
    int getCurrentStep() const noexcept { return currentStep; }

    /** 设置总步骤数 */
    void setTotalSteps(int total);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    int currentStep = 1;
    int totalSteps  = 3;

    /** 每个圆点的直径 */
    static constexpr int dotDiameter = 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepIndicator)
};
