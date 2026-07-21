#include "StepInterface.h"
#include "../../Core/Settings.h"

//==============================================================================
StepInterface::StepInterface()
{
    populateOptions();
}

juce::String StepInterface::getStepTitle() const
{
    return TRANS("步骤 2 / 3 · 音频接口");
}

juce::String StepInterface::getQuestionText() const
{
    return TRANS("你的声卡是？");
}

void StepInterface::populateOptions()
{
    const auto& presets = Settings::getInterfacePresets();
    for (int i = 0; i < presets.size(); ++i)
    {
        const auto& iface = presets[i];
        addOption(iface.name, {}, iface.isPopular);
    }
}

void StepInterface::onOptionClicked(int index)
{
    // 如果点击的是 "更多声卡" 选项（索引 9，即 "集成声卡" 后的选项）
    // 或 "其它"（索引 10），触发浏览器
    const auto& presets = Settings::getInterfacePresets();
    if (juce::isPositiveAndBelow(index, presets.size()))
    {
        const auto& name = presets[index].name;
        if (name == "其它" && onRequestBrowser)
            onRequestBrowser();
    }
}
