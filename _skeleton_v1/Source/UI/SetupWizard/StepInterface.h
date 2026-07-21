#pragma once

#include "SetupStep.h"

//==============================================================================
/**
 * 步骤 2: 选择声卡
 *
 * "你的声卡是？"
 * 选项来自 Settings::getInterfacePresets()
 * "更多声卡" 选项触发 AudioInterfaceBrowser（展开列表）
 */
class StepInterface : public SetupStep
{
public:
    //==============================================================================
    StepInterface();

    juce::String getStepTitle() const override;
    juce::String getQuestionText() const override;
    void populateOptions() override;

    /** "更多声卡" 展开回调 */
    std::function<void()> onRequestBrowser;

private:
    void onOptionClicked(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepInterface)
};
