#include "UI/TransitionCard.h"
#include "Core/Design.h"

TransitionCard::TransitionCard()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Image TransitionCard::captureSelf()
{
    if (auto* p = getParentComponent())
    {
        setVisible (false);   // 快照不含自身
        auto img = hm::blurredBackdrop (*p, getBounds(), 6.0f);
        setVisible (true);
        return img;
    }
    return {};
}

void TransitionCard::paint (juce::Graphics& g)
{
    // Apple 毛玻璃：先铺模糊底图，再压暗
    if (backdrop_.isValid())
        g.drawImageAt (backdrop_, 0, 0);
    g.setColour (hm::overlayBg());
    g.fillRect (getLocalBounds());

    // 卡片 396×400 随画布居中，宽松行距
    const int w = juce::jmin (396, getWidth() - 24);
    auto card = juce::Rectangle<float> ((getWidth() - w) * 0.5f,
                                        (float) juce::jmax (12, (getHeight() - 400) / 2),
                                        (float) w, 400.0f);
    hm::drawCard (g, card, 12.0f, hm::panelBg());
    const int x0 = (int) card.getX(), y0 = (int) card.getY(), cw = (int) card.getWidth();
    const auto mouse = getMouseXYRelative();

    // 大标题
    g.setFont (24.0f);
    g.setColour (hm::textMain());
    g.drawFittedText (juce::String::fromUTF8 (u8"一切就绪"),
                      x0 + 30, y0 + 34, cw - 60, 32, juce::Justification::centred, 1);

    // 两句真诚的话（行距 26，不再挤）
    g.setFont (15.0f);
    g.setColour (hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"你的耳机已匹配，目标曲线已校准"),
                x0 + 30, y0 + 84, cw - 60, 22, juce::Justification::centred);
    g.drawText (juce::String::fromUTF8 (u8"拧动旋杆，开始听到它该有的样子"),
                x0 + 30, y0 + 110, cw - 60, 22, juce::Justification::centred);

    // 功能点 —— 统一格式：名称 · 短描述
    auto how = juce::Rectangle<int> (x0 + 30, y0 + 158, cw - 60, 92);
    g.setColour (hm::innerBg());
    g.fillRoundedRectangle (how.toFloat(), 8.0f);
    g.setFont (13.0f);
    g.setColour (hm::textLabel());
    static const char* lines[] = {
        u8"旋杆 · 0 ~ 200 翻译度",
        u8"BPM 助手 · 混响 延迟 压缩",
        u8"点「诚」 · 反馈你的听感"
    };
    for (int i = 0; i < 3; ++i)
        g.drawText (juce::String::fromUTF8 (lines[i]), how.getX() + 16,
                    how.getY() + 10 + i * 26, how.getWidth() - 32, 20,
                    juce::Justification::centredLeft);

    // 开始按钮 —— 与向导"下一步"同几何
    btnStart_ = { x0 + 24, y0 + 340, cw - 48, 36 };
    const bool hov = btnStart_.contains (mouse);
    g.setColour (hm::accent().withAlpha (hov ? 0.7f : 0.5f));
    g.fillRoundedRectangle (btnStart_.toFloat(), 18.0f);
    g.setFont (15.0f);
    g.setColour (juce::Colours::white.withAlpha (hov ? 1.0f : 0.85f));
    g.drawText (juce::String::fromUTF8 (u8"开始混音"), btnStart_, juce::Justification::centred);
}

void TransitionCard::mouseDown (const juce::MouseEvent& e)
{
    if (btnStart_.contains (e.getPosition()) && onStart)
        onStart (choice_);
}

void TransitionCard::mouseMove (const juce::MouseEvent&) { repaint(); }
