#pragma once

#include "SetupStep.h"

//==============================================================================
/**
 * 步骤 1: 选择耳机
 *
 * "你在用什么耳机？"
 * 选项来自 Settings::getHeadphonePresets()
 * 底部有"监听音箱"占位项（即将支持）
 */
class StepHeadphone : public SetupStep
{
public:
    //==============================================================================
    StepHeadphone();

    juce::String getStepTitle() const override;
    juce::String getQuestionText() const override;
    void populateOptions() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepHeadphone)
};
