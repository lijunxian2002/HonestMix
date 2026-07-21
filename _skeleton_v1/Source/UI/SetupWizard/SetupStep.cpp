#include "SetupStep.h"
#include "../LookAndFeel.h"

//==============================================================================
SetupStep::SetupStep() {}

//==============================================================================
void SetupStep::addOption(const juce::String& label,
                          const juce::String& subtitle,
                          bool isPopular)
{
    auto* opt = new Option();
    opt->label    = label;
    opt->subtitle = subtitle;
    opt->isPopular = isPopular;
    options.add(opt);
}

void SetupStep::resetSelection()
{
    selectedIndex = -1;
    repaint();
}

//==============================================================================
void SetupStep::resized()
{
    auto bounds = getLocalBounds().reduced(14);
    int y = 0;

    // 标题区域
    titleArea = bounds.removeFromTop(16);
    y += 16 + 4;

    // 问题区域
    questionArea = bounds.removeFromTop(36);
    y += 36 + 8;

    // 选项区域
    int optionHeight = 32;
    int totalOptionHeight = options.size() * optionHeight
                            + (options.size() - 1) * 4;
    optionsArea = bounds.removeFromTop(totalOptionHeight);
    y += totalOptionHeight + 4;

    // 额外区域（留给子类）
    extraArea = bounds;

    // 更新每个选项的 bounds
    for (int i = 0; i < options.size(); ++i)
    {
        auto optBounds = optionsArea.removeFromTop(optionHeight);
        if (i < options.size() - 1)
            optionsArea.removeFromTop(4); // gap
        options[i]->bounds = optBounds;
    }
}

void SetupStep::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 标题
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(9.0f));
    g.drawText(getStepTitle(), titleArea.toFloat(),
               juce::Justification::centredLeft, true);

    // 问题
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(13.0f));
    g.drawText(getQuestionText(), questionArea.toFloat(),
               juce::Justification::centredLeft, true);

    // 选项
    for (int i = 0; i < options.size(); ++i)
    {
        auto& opt = *options[i];
        bool isHovered = (i == hoveredIndex);
        bool isSelected = (i == selectedIndex);

        // 选项行背景
        HonestMixLookAndFeel::drawOptionRow(g, opt.bounds,
                                             isSelected, isHovered);

        // radio 圆点
        auto dotBounds = opt.bounds.removeFromLeft(20).reduced(8, 8);
        HonestMixLookAndFeel::drawRadioDot(g, dotBounds, isSelected);

        // 选项文字
        g.setColour(isSelected ? HonestMixLookAndFeel::textPrimary
                               : HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(opt.label, opt.bounds.reduced(4, 0).toFloat(),
                   juce::Justification::centredLeft, true);

        // 热门标签
        if (opt.isPopular)
        {
            auto popularBounds = opt.bounds.removeFromRight(40);
            g.setColour(HonestMixLookAndFeel::textSubtle);
            g.setFont(juce::FontOptions(7.0f));
            g.drawText("热门", popularBounds.toFloat(),
                       juce::Justification::centred, true);
        }

        // 副标题
        if (opt.subtitle.isNotEmpty() && !opt.isPopular)
        {
            auto subBounds = opt.bounds.removeFromRight(60);
            g.setColour(HonestMixLookAndFeel::textSubtle);
            g.setFont(juce::FontOptions(7.0f));
            g.drawText(opt.subtitle, subBounds.toFloat(),
                       juce::Justification::centredRight, true);
        }
    }
}

//==============================================================================
void SetupStep::mouseDown(const juce::MouseEvent& event)
{
    int hit = hitTestOption(event.getPosition());
    if (hit >= 0 && hit < options.size())
    {
        selectedIndex = hit;
        repaint();
        if (onSelectionChanged)
            onSelectionChanged();
    }
}

int SetupStep::hitTestOption(juce::Point<int> position) const
{
    for (int i = 0; i < options.size(); ++i)
    {
        if (options[i]->bounds.contains(position))
            return i;
    }
    return -1;
}
