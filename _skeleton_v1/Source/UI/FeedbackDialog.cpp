#include "FeedbackDialog.h"
#include "LookAndFeel.h"

//==============================================================================
FeedbackDialog::FeedbackDialog()
{
    for (int i = 0; i < 4; ++i)
        options[i].label = optionLabels[i];
}

//==============================================================================
void FeedbackDialog::show()
{
    setVisible(true);
    if (auto* parent = getParentComponent())
        setBounds(parent->getLocalBounds());
    toFront(true);
}

void FeedbackDialog::hide()
{
    setVisible(false);
}

//==============================================================================
void FeedbackDialog::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 叠加层背景
    g.setColour(HonestMixLookAndFeel::overlayBg);
    g.fillAll();

    // 卡片
    auto boxBounds = bounds.reduced(bounds.getWidth() / 2 - 140,
                                     bounds.getHeight() / 2 - 100);
    boxBounds.setSize(280, 200);
    boxBounds.setCentre(bounds.getCentre());
    HonestMixLookAndFeel::drawCardBackground(g, boxBounds);

    auto content = boxBounds.reduced(20);

    // 问题
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(12.0f));
    questionArea = content.removeFromTop(30);
    g.drawText(TRANS("你觉得校正后的声音怎么样？"),
               questionArea.toFloat(),
               juce::Justification::centred, true);

    // 4 个选项（两行两列）
    content.removeFromTop(8);
    optionsArea = content.removeFromTop(80);

    int halfW = optionsArea.getWidth() / 2 - 4;
    int halfH = optionsArea.getHeight() / 2 - 4;

    options[0].bounds = juce::Rectangle<int>(optionsArea.getX(),
                                              optionsArea.getY(),
                                              halfW, halfH);
    options[1].bounds = juce::Rectangle<int>(optionsArea.getX() + halfW + 8,
                                              optionsArea.getY(),
                                              halfW, halfH);
    options[2].bounds = juce::Rectangle<int>(optionsArea.getX(),
                                              optionsArea.getY() + halfH + 8,
                                              halfW, halfH);
    options[3].bounds = juce::Rectangle<int>(optionsArea.getX() + halfW + 8,
                                              optionsArea.getY() + halfH + 8,
                                              halfW, halfH);

    for (int i = 0; i < 4; ++i)
    {
        auto& opt = options[i];
        bool sel = (i == selectedIndex);

        HonestMixLookAndFeel::drawOptionRow(g, opt.bounds, sel, false);

        g.setColour(sel ? HonestMixLookAndFeel::textPrimary
                        : HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(12.0f));
        g.drawText(opt.label, opt.bounds.toFloat(),
                   juce::Justification::centred, true);
    }

    // 提交
    content.removeFromTop(12);
    submitArea = content.removeFromTop(24);
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(8.0f));
    g.drawText(TRANS("提交反馈"), submitArea.toFloat(),
               juce::Justification::centred, true);
}

void FeedbackDialog::resized() {}

void FeedbackDialog::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();

    for (int i = 0; i < 4; ++i)
    {
        if (options[i].bounds.contains(pos))
        {
            onOptionClicked(i);
            return;
        }
    }

    if (submitArea.contains(pos))
        onSubmit();
}

void FeedbackDialog::onOptionClicked(int index)
{
    for (int i = 0; i < 4; ++i)
        options[i].selected = (i == index);
    selectedIndex = index;
    repaint();
}

void FeedbackDialog::onSubmit()
{
    if (selectedIndex >= 0 && onFeedbackSubmitted)
        onFeedbackSubmitted(selectedIndex);
    hide();
}
