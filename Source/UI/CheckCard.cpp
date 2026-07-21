#include "UI/CheckCard.h"
#include "Core/Design.h"

CheckCard::CheckCard()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Rectangle<int> CheckCard::cardRect() const
{
    return { (getWidth() - 396) / 2, juce::jmax (12, (getHeight() - 310) / 2), 396, 310 };
}

void CheckCard::captureBackdrop()
{
    if (auto* p = getParentComponent())
    {
        setVisible (false);   // 快照不含自身
        backdrop_ = hm::blurredBackdrop (*p, getBounds(), 6.0f);
        setVisible (true);
        repaint();
    }
}

juce::Rectangle<int> CheckCard::rowRect (int i) const
{
    auto c = cardRect();
    return { c.getX() + 27, c.getY() + 90 + i * 43, c.getWidth() - 54, 36 };
}

void CheckCard::paint (juce::Graphics& g)
{
    // Apple 毛玻璃：先铺模糊底图，再压暗
    if (backdrop_.isValid())
        g.drawImageAt (backdrop_, 0, 0);
    g.setColour (hm::overlayBg());
    g.fillRect (getLocalBounds());

    auto c = cardRect();
    hm::drawCard (g, c.toFloat(), 12.0f, hm::panelBg());
    const auto mouse = getMouseXYRelative();

    g.setFont (17.0f);
    g.setColour (hm::textMain());
    g.drawFittedText (juce::String::fromUTF8 (u8"抱歉打扰\n需要切换监听方式吗"),
                      c.getX() + 27, c.getY() + 20, c.getWidth() - 54, 56,
                      juce::Justification::centred, 2);

    static const char* texts[] = {
        u8"单声道 — 检查相位抵消",
        u8"iPhone 外放 — 模拟手机扬声器",
        u8"汽车音响 — 模拟车内环境",
        u8"不用了，继续混"
    };
    for (int i = 0; i < 4; ++i)
    {
        auto rr = rowRect (i);
        const bool hov = rr.contains (mouse);
        g.setColour (hm::whiteA (hov ? 0.06f : 0.03f));
        g.fillRoundedRectangle (rr.toFloat(), 6.0f);
        g.setColour (hm::whiteA (hov ? 0.12f : 0.06f));
        g.drawRoundedRectangle (rr.toFloat().reduced (0.5f), 6.0f, 1.0f);
        const float rcx = (float) rr.getX() + 19.0f, rcy = (float) rr.getCentreY();
        g.setColour (hm::whiteA (0.16f));
        g.drawEllipse (rcx - 8.0f, rcy - 8.0f, 16.0f, 16.0f, 2.0f);
        g.setFont (12.0f);
        g.setColour (hov ? hm::textMain() : hm::textSec());
        g.drawText (juce::String::fromUTF8 (texts[i]), rr.getX() + 42, rr.getY(),
                    rr.getWidth() - 48, rr.getHeight(), juce::Justification::centredLeft);
    }

    footRect_ = { c.getX() + 27, c.getY() + 270, c.getWidth() - 54, 22 };
    g.setFont (10.0f);
    g.setColour (footRect_.contains (mouse) ? hm::textLabel() : hm::textDim());
    g.drawText (juce::String::fromUTF8 (u8"好，听完了告诉我翻译度"), footRect_, juce::Justification::centred);
}

void CheckCard::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();
    for (int i = 0; i < 4; ++i)
        if (rowRect (i).contains (p)) { if (onAction) onAction (i); return; }
    if (footRect_.contains (p) || ! cardRect().contains (p))
        if (onClose) onClose();
}

void CheckCard::mouseMove (const juce::MouseEvent&) { repaint(); }
