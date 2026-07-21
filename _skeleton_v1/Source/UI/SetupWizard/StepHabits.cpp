#include "StepHabits.h"
#include "../../Core/Settings.h"
#include "../LookAndFeel.h"

//==============================================================================
StepHabits::StepHabits()
{
    populateOptions();

    // 添加第二个问题的选项
    const auto& checkPresets = Settings::getCheckEnvironmentPresets();
    for (const auto& env : checkPresets)
    {
        auto* opt = new CheckOption();
        opt->label = env.label;
        checkOptions.add(opt);
    }
}

juce::String StepHabits::getStepTitle() const
{
    return TRANS("步骤 3 / 3 · 混音习惯");
}

juce::String StepHabits::getQuestionText() const
{
    return TRANS("你每次混音大概多久？");
}

void StepHabits::populateOptions()
{
    const auto& habits = Settings::getHabitPresets();
    for (const auto& h : habits)
        addOption(h.label);
}

bool StepHabits::isFullySelected() const
{
    return getSelectedIndex() >= 0 && checkEnvIndex >= 0;
}

//==============================================================================
void StepHabits::resized()
{
    SetupStep::resized();

    // 获取父级分配给 extraArea 的空间
    auto bounds = getLocalBounds().reduced(14);

    // 跳过标题、问题、选项区域
    int y = 16 + 4 + 36 + 8;
    int optionHeight = 32;
    int totalOptHeight = getSelectedIndex() >= 0 // simplified
                         ? Settings::getHabitPresets().size() * (optionHeight + 4)
                         : 0;
    y += totalOptHeight + 8;

    // 分隔线之后
    auto sectionBounds = bounds.removeFromBottom(120);
    sectionTitleArea = sectionBounds.removeFromTop(16);

    // 第二个问题的选项区域
    int checkOptionHeight = 28;
    int totalCheckHeight = checkOptions.size() * (checkOptionHeight + 4);
    checkOptionsArea = sectionBounds.removeFromTop(totalCheckHeight);

    for (int i = 0; i < checkOptions.size(); ++i)
    {
        auto optBounds = checkOptionsArea.removeFromTop(checkOptionHeight);
        if (i < checkOptions.size() - 1)
            checkOptionsArea.removeFromTop(4);
        checkOptions[i]->bounds = optBounds;
    }
}

void StepHabits::paint(juce::Graphics& g)
{
    SetupStep::paint(g);

    // 分隔线
    auto bounds = getLocalBounds().reduced(14);
    int y = 0;

    // 第二个问题区域的标题
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(8.0f));
    g.drawText(TRANS("混完后会去别的环境检查吗？"),
               sectionTitleArea.toFloat(),
               juce::Justification::centredLeft, true);

    // 第二个问题的选项
    for (int i = 0; i < checkOptions.size(); ++i)
    {
        auto& opt = *checkOptions[i];
        bool isHovered = (i == checkHovered);
        bool isSelected = (i == checkEnvIndex);

        HonestMixLookAndFeel::drawOptionRow(g, opt.bounds,
                                             isSelected, isHovered);

        auto dotBounds = opt.bounds.removeFromLeft(20).reduced(8, 8);
        HonestMixLookAndFeel::drawRadioDot(g, dotBounds, isSelected);

        g.setColour(isSelected ? HonestMixLookAndFeel::textPrimary
                               : HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(opt.label, opt.bounds.reduced(4, 0).toFloat(),
                   juce::Justification::centredLeft, true);
    }
}

void StepHabits::mouseDown(const juce::MouseEvent& event)
{
    // 首先尝试 SetupStep 的选项（第一个问题）
    SetupStep::mouseDown(event);

    // 然后尝试第二个问题的选项
    for (int i = 0; i < checkOptions.size(); ++i)
    {
        if (checkOptions[i]->bounds.contains(event.getPosition()))
        {
            checkEnvIndex = i;
            repaint();
            if (onCheckEnvChanged)
                onCheckEnvChanged();
            break;
        }
    }
}
