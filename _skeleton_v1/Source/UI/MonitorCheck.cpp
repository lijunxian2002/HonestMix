#include "MonitorCheck.h"
#include "LookAndFeel.h"

//==============================================================================
MonitorCheck::MonitorCheck(AppState& state, std::function<void()> dismiss)
    : appState(state), onDismiss(std::move(dismiss))
{
    options[0].label = TRANS("切换到单声道 · 检查相位");
    options[1].label = TRANS("导出到手机上听");
    options[2].label = TRANS("到车里 / 音响上听");
    options[3].label = TRANS("暂时不用，继续混");
}

//==============================================================================
void MonitorCheck::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 叠加层背景
    g.setColour(HonestMixLookAndFeel::overlayBg);
    g.fillAll();

    // 弹窗卡片
    auto boxBounds = bounds.reduced(bounds.getWidth() / 2 - 150,
                                     bounds.getHeight() / 2 - 150);
    boxBounds.setSize(300, 360);
    boxBounds.setCentre(bounds.getCentre());
    HonestMixLookAndFeel::drawCardBackground(g, boxBounds);

    auto contentBounds = boxBounds.reduced(20);

    // 问题
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(13.0f));
    g.drawText(TRANS("混了 1 小时了\n要不要检查一下监听环境？"),
               contentBounds.removeFromTop(50).toFloat(),
               juce::Justification::centred, true);

    // 选项
    g.setFont(juce::FontOptions(9.0f));
    for (int i = 0; i < 4; ++i)
    {
        auto& opt = options[i];
        bool sel = (i == selectedIndex);

        HonestMixLookAndFeel::drawOptionRow(g, opt.bounds, sel, false);

        // radio 圆点
        auto dotBounds = opt.bounds.removeFromLeft(20).reduced(6, 6);
        HonestMixLookAndFeel::drawRadioDot(g, dotBounds, sel);

        g.setColour(sel ? HonestMixLookAndFeel::textPrimary
                        : HonestMixLookAndFeel::textSubtle);
        g.drawText(opt.label, opt.bounds.reduced(4, 0).toFloat(),
                   juce::Justification::centredLeft, true);
    }

    // 底部按钮
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(8.0f));
    g.drawText(TRANS("好，听完了告诉我翻译度"),
               dismissArea.toFloat(),
               juce::Justification::centred, true);
}

void MonitorCheck::resized()
{
    auto bounds = getLocalBounds();
    auto boxBounds = bounds.reduced(bounds.getWidth() / 2 - 150,
                                     bounds.getHeight() / 2 - 150);
    boxBounds.setSize(300, 360);
    boxBounds.setCentre(bounds.getCentre());

    auto contentBounds = boxBounds.reduced(20);

    // 问题区域
    questionArea = contentBounds.removeFromTop(50);
    contentBounds.removeFromTop(8);

    // 4 个选项
    optionsArea = contentBounds.removeFromTop(180);
    int optH = 32;
    for (int i = 0; i < 4; ++i)
    {
        options[i].bounds = optionsArea.removeFromTop(optH);
        if (i < 3) optionsArea.removeFromTop(6);
    }

    // 底部按钮
    contentBounds.removeFromTop(12);
    dismissArea = contentBounds.removeFromTop(24);
}

void MonitorCheck::onOptionClicked(int index)
{
    if (index >= 0 && index < 4)
    {
        for (int i = 0; i < 4; ++i)
            options[i].selected = (i == index);
        selectedIndex = index;
        repaint();
    }
}

//==============================================================================
void MonitorCheck::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();

    // 尝试选项
    for (int i = 0; i < 4; ++i)
    {
        if (options[i].bounds.contains(pos))
        {
            onOptionClicked(i);
            return;
        }
    }

    // 底部 dismiss
    if (dismissArea.contains(pos))
    {
        if (onDismiss)
            onDismiss();
    }
}
