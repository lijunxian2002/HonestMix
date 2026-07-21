#include "UI/FeedbackCard.h"
#include "Core/Design.h"

FeedbackCard::FeedbackCard()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Rectangle<int> FeedbackCard::cardRect() const
{
    return { (getWidth() - 396) / 2, juce::jmax (12, (getHeight() - 408) / 2), 396, 408 };
}

void FeedbackCard::captureBackdrop()
{
    if (auto* p = getParentComponent())
    {
        setVisible (false);   // 快照不含自身
        backdrop_ = hm::blurredBackdrop (*p, getBounds(), 6.0f);
        setVisible (true);
        repaint();
    }
}

juce::Rectangle<int> FeedbackCard::rowRect (int section, int row) const
{
    auto c = cardRect();
    return { c.getX() + 27, c.getY() + (section == 0 ? 84 : 228) + row * 39, c.getWidth() - 54, 33 };
}

void FeedbackCard::paint (juce::Graphics& g)
{
    // Apple 毛玻璃：先铺模糊底图，再压暗
    if (backdrop_.isValid())
        g.drawImageAt (backdrop_, 0, 0);
    g.setColour (hm::overlayBg());
    g.fillRect (getLocalBounds());

    auto c = cardRect();
    hm::drawCard (g, c.toFloat(), 12.0f, hm::panelBg());
    const auto mouse = getMouseXYRelative();

    // 碰撞矩形（paint 内赋值，与数据中心一致）
    submitRect_ = { c.getX() + 27, c.getY() + 342, 160, 32 };
    closeRect_  = { c.getX() + 209, c.getY() + 342, 160, 32 };

    g.setFont (16.0f);
    g.setColour (hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"翻译度反馈"), c.getX(), c.getY() + 24, c.getWidth(), 24,
                juce::Justification::centred);

    static const char* sectNames[] = { u8"低频", u8"高频" };
    static const char* opts[2][3] = {
        { u8"低频刚好", u8"低频多了", u8"低频少了" },
        { u8"高频刚好", u8"高频亮了", u8"高频暗了" }
    };
    const int sels[2] = { bass_, treble_ };

    for (int s = 0; s < 2; ++s)
    {
        g.setFont (11.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (sectNames[s]), c.getX() + 27,
                    c.getY() + (s == 0 ? 60 : 204), 150, 18, juce::Justification::centredLeft);

        for (int r = 0; r < 3; ++r)
        {
            auto rr = rowRect (s, r);
            const bool sel = sels[s] == r;
            const bool hov = rr.contains (mouse);
            g.setColour (hm::whiteA (sel ? 0.07f : (hov ? 0.05f : 0.03f)));
            g.fillRoundedRectangle (rr.toFloat(), 6.0f);
            g.setColour (hm::whiteA (sel ? 0.14f : 0.06f));
            g.drawRoundedRectangle (rr.toFloat().reduced (0.5f), 6.0f, 1.0f);

            const float rcx = (float) rr.getX() + 19.0f, rcy = (float) rr.getCentreY();
            g.setColour (hm::whiteA (sel ? 0.35f : 0.16f));
            g.drawEllipse (rcx - 8.0f, rcy - 8.0f, 16.0f, 16.0f, 2.0f);
            if (sel)
            {
                g.setColour (hm::accent()); // 选中 = 灰调朱砂
                g.fillEllipse (rcx - 3.5f, rcy - 3.5f, 7.0f, 7.0f);
            }
            g.setFont (12.0f);
            g.setColour (sel ? hm::textMain() : (hov ? hm::textSec() : hm::textLabel()));
            g.drawText (juce::String::fromUTF8 (opts[s][r]), rr.getX() + 42, rr.getY(),
                        rr.getWidth() - 48, rr.getHeight(), juce::Justification::centredLeft);
        }
    }

    const bool subHov = submitRect_.contains (mouse);
    g.setColour (hm::whiteA (subHov ? 0.10f : 0.06f));
    g.fillRoundedRectangle (submitRect_.toFloat(), 6.0f);
    g.setColour (hm::whiteA (subHov ? 0.16f : 0.10f));
    g.drawRoundedRectangle (submitRect_.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setFont (11.0f);
    g.setColour (subHov ? hm::textMain() : hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"提 交"), submitRect_, juce::Justification::centred);

    g.setColour (closeRect_.contains (mouse) ? hm::textLabel() : hm::textDim());
    g.drawText (juce::String::fromUTF8 (u8"关 闭"), closeRect_, juce::Justification::centred);
}

void FeedbackCard::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();
    for (int s = 0; s < 2; ++s)
        for (int r = 0; r < 3; ++r)
            if (rowRect (s, r).contains (p))
            {
                if (s == 0) bass_ = r; else treble_ = r;
                repaint();
                return;
            }
    if (submitRect_.contains (p))           { if (onSubmit) onSubmit (bass_, treble_); }
    else if (closeRect_.contains (p))       { if (onClose)  onClose(); }
    else if (! cardRect().contains (p))     { if (onClose)  onClose(); } // 点遮罩关闭
}

void FeedbackCard::mouseMove (const juce::MouseEvent&) { repaint(); }
