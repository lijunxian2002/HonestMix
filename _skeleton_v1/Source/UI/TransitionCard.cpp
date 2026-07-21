#include "TransitionCard.h"
#include "LookAndFeel.h"
#include "../Core/Settings.h"

//==============================================================================
TransitionCard::TransitionCard(AppState& state,
                               std::function<void()> confirm)
    : appState(state), onConfirm(std::move(confirm))
{
    const auto& presets = Settings::getWetDryPresets();
    for (int i = 0; i < juce::jmin(2, presets.size()); ++i)
    {
        options[i].percent  = presets[i].percent;
        options[i].label    = presets[i].label;
        options[i].subtitle = presets[i].subtitle;
    }
    selectedOption = 0;

    backButton.setButtonText(TRANS("再调一下"));
    backButton.onClick = [this]()
    {
        // 返回设置
        if (auto* parent = findParentComponentOfClass<juce::Component>())
            parent->setVisible(false);
        setVisible(false);
    };
    addAndMakeVisible(backButton);

    startButton.setButtonText(TRANS("开始混音"));
    startButton.onClick = [this]()
    {
        appState.setWetDryMix(options[selectedOption].percent);
        if (onConfirm)
            onConfirm();
    };
    addAndMakeVisible(startButton);
}

//==============================================================================
void TransitionCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 卡片背景
    HonestMixLookAndFeel::drawCardBackground(g, bounds);

    // ★ 图标
    g.setColour(HonestMixLookAndFeel::accent.withAlpha(0.35f));
    g.setFont(juce::FontOptions(24.0f));
    g.drawText("✦", iconArea.toFloat(),
               juce::Justification::centred, true);

    // 标题
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText(TRANS("准备好了"), titleArea.toFloat(),
               juce::Justification::centred, true);

    // 干湿比选项
    for (int i = 0; i < 2; ++i)
    {
        auto& opt = options[i];
        bool sel = (i == selectedOption);

        auto bg = sel ? HonestMixLookAndFeel::optionBg.brighter(0.03f)
                      : HonestMixLookAndFeel::optionBg;
        auto border = sel ? HonestMixLookAndFeel::optionBorder.brighter(0.08f)
                          : HonestMixLookAndFeel::cardBorder;

        g.setColour(bg);
        g.fillRoundedRectangle(opt.bounds.toFloat(), 5.0f);
        g.setColour(border);
        g.drawRoundedRectangle(opt.bounds.toFloat(), 5.0f, 1.0f);

        // 数值
        g.setColour(HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(16.0f));
        g.drawText(opt.label, opt.bounds.removeFromTop(30).toFloat(),
                   juce::Justification::centred, true);

        // 副标题
        g.setColour(HonestMixLookAndFeel::textSubtle);
        g.setFont(juce::FontOptions(8.0f));
        g.drawText(opt.subtitle, opt.bounds.toFloat(),
                   juce::Justification::centred, true);
    }

    // 使用说明
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(9.0f));

    auto instrBounds = instructionsArea;
    instrBounds.removeFromTop(4);
    const juce::String instructions[] = {
        "① 之后我会缩到 ★ 里，不挡你的轨道",
        "② 40 分钟提醒导出 · 1 小时提醒检查环境",
        "③ 你回来后告诉我翻译度 — 你帮我，我帮你",
        "④ 任何时候点 ★ 展开调参数 · 算 BPM"
    };
    for (auto& instr : instructions)
    {
        auto line = instrBounds.removeFromTop(18);
        g.drawText(instr, line.toFloat(),
                   juce::Justification::centredLeft, true);
    }
}

void TransitionCard::resized()
{
    auto bounds = getLocalBounds().reduced(22);

    // ★ 图标
    iconArea = bounds.removeFromTop(30);

    // 标题
    titleArea = bounds.removeFromTop(24);

    bounds.removeFromTop(8);

    // 干湿比选项
    optionsArea = bounds.removeFromTop(72);
    int halfW = optionsArea.getWidth() / 2 - 4;
    options[0].bounds = optionsArea.removeFromLeft(halfW);
    optionsArea.removeFromLeft(8);
    options[1].bounds = optionsArea;

    bounds.removeFromTop(8);

    // 使用说明（深色背景区域）
    instructionsArea = bounds.removeFromTop(96);
    instructionsArea.reduce(0, 4);

    // 按钮
    buttonArea = bounds.removeFromTop(32);
    int btnW = (buttonArea.getWidth() - 8) / 2;
    backButton.setBounds(buttonArea.removeFromLeft(btnW));
    buttonArea.removeFromLeft(8);
    startButton.setBounds(buttonArea);
}

//==============================================================================
void TransitionCard::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();
    for (int i = 0; i < 2; ++i)
    {
        if (options[i].bounds.contains(pos))
        {
            onOptionClicked(i);
            return;
        }
    }
}

float TransitionCard::getSelectedWetDry() const noexcept
{
    return options[selectedOption].percent;
}

void TransitionCard::onOptionClicked(int index)
{
    if (index >= 0 && index < 2)
    {
        selectedOption = index;
        repaint();
    }
}
