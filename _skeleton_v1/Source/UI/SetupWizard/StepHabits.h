#pragma once

#include "SetupStep.h"

//==============================================================================
/**
 * 步骤 3: 混音习惯
 *
 * 分为两部分：
 *   1. "你每次混音大概多久？"（单选）
 *   2. "混完后会去别的环境检查吗？"（单选）
 *
 * 两个问题都选完后，"完成设置" 按钮启用
 */
class StepHabits : public SetupStep
{
public:
    //==============================================================================
    StepHabits();

    juce::String getStepTitle() const override;
    juce::String getQuestionText() const override;
    void populateOptions() override;

    /** 返回第二个问题的选中索引 */
    int getCheckEnvIndex() const noexcept { return checkEnvIndex; }

    /** 两个问题是否都已选 */
    bool isFullySelected() const;

    /** 环境检查的回调 */
    std::function<void()> onCheckEnvChanged;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    //==============================================================================
    // 第二个问题的选项
    struct CheckOption
    {
        juce::String label;
        juce::Rectangle<int> bounds;
    };

    juce::OwnedArray<CheckOption> checkOptions;
    int checkEnvIndex = -1;
    int checkHovered  = -1;

    juce::Rectangle<int> sectionTitleArea;
    juce::Rectangle<int> checkOptionsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepHabits)
};
