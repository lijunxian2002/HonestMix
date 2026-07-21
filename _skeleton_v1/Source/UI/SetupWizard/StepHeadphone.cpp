#include "StepHeadphone.h"
#include "../../Core/Settings.h"

//==============================================================================
StepHeadphone::StepHeadphone()
{
    populateOptions();
}

juce::String StepHeadphone::getStepTitle() const
{
    return TRANS("步骤 1 / 3 · 监听设备");
}

juce::String StepHeadphone::getQuestionText() const
{
    return TRANS("你在用什么耳机？");
}

void StepHeadphone::populateOptions()
{
    const auto& presets = Settings::getHeadphonePresets();
    for (const auto& hp : presets)
        addOption(hp.name, {}, hp.isPopular);
}
