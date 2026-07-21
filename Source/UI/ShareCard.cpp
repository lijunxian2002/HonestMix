#include "UI/ShareCard.h"
#include "Core/Design.h"

ShareCard::ShareCard()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

juce::Rectangle<int> ShareCard::cardRect() const
{
    return { (getWidth() - 396) / 2, juce::jmax (12, (getHeight() - 354) / 2), 396, 354 };
}

void ShareCard::captureBackdrop()
{
    if (auto* p = getParentComponent())
    {
        setVisible (false);   // 快照不含自身
        backdrop_ = hm::blurredBackdrop (*p, getBounds(), 6.0f);
        setVisible (true);
        repaint();
    }
}

void ShareCard::setInfo (const juce::String& fullName, const juce::String& brand, int translationDegree)
{
    fullName_ = fullName;
    brand_    = brand;
    degree_   = translationDegree;
    repaint();
}

void ShareCard::paint (juce::Graphics& g)
{
    // Apple 毛玻璃：先铺模糊底图，再压暗
    if (backdrop_.isValid())
        g.drawImageAt (backdrop_, 0, 0);
    g.setColour (hm::overlayBg());
    g.fillRect (getLocalBounds());

    auto c = cardRect();
    hm::drawCard (g, c.toFloat(), 10.0f, hm::panelBg());
    const auto mouse = getMouseXYRelative();
    const int x = c.getX(), y = c.getY(), w = c.getWidth();

    // 碰撞矩形（paint 内赋值，与数据中心一致）
    closeRect_ = { x + 27, y + 309, w - 54, 24 };

    g.setFont (18.0f);
    g.setColour (hm::textSec());
    g.drawText ("H o n e s t M i x", x + 27, y + 20, 240, 24, juce::Justification::centredLeft);
    g.setFont (10.0f);
    g.setColour (hm::textDim());
    g.drawText (juce::String::fromUTF8 (u8"诚 · 翻译度已验证"), x + 27, y + 24, w - 54, 18,
                juce::Justification::right);

    // 封面区
    auto art = juce::Rectangle<int> (x + 27, y + 57, w - 54, 96);
    g.setColour (hm::innerBg());
    g.fillRoundedRectangle (art.toFloat(), 6.0f);
    g.setColour (hm::border());
    g.drawRoundedRectangle (art.toFloat().reduced (0.5f), 6.0f, 1.0f);

    static const float waveH[] = { 9, 21, 30, 18, 27, 12, 24, 15 };
    const int nBars = 8, bw = 3, gap = 3;
    const int totalW = nBars * bw + (nBars - 1) * gap;
    float wx = (float) art.getCentreX() - totalW * 0.5f;
    const float wy = (float) art.getY() + 27.0f;
    g.setColour (hm::whiteA (0.16f));
    for (int i = 0; i < nBars; ++i)
    {
        g.fillRoundedRectangle (wx, wy + (30.0f - waveH[i]) * 0.5f, (float) bw, waveH[i], 1.5f);
        wx += bw + gap;
    }
    g.setFont (10.0f);
    g.setColour (hm::textLabel());
    g.drawFittedText (fullName_, art.getX(), art.getY() + 74, art.getWidth(), 16,
                      juce::Justification::centred, 1);

    // 信息行
    const int iy = y + 165;
    g.setColour (hm::border());
    g.fillRect (x + 27, iy, w - 54, 1);
    g.fillRect (x + 27, iy + 45, w - 54, 1);
    const juce::String vals[]   = { juce::String (degree_) + "%", brand_, "Harman OE" };
    static const char* labels[] = { u8"翻译度", u8"耳机", u8"目标" };
    for (int i = 0; i < 3; ++i)
    {
        const int colX = x + 27 + i * ((w - 54) / 3);
        g.setFont (hm::mono (17.0f));
        g.setColour (hm::textSec());
        g.drawFittedText (vals[i], colX, iy + 6, (w - 54) / 3, 21, juce::Justification::centred, 1);
        g.setFont (9.0f);
        g.setColour (hm::textDim());
        g.drawText (juce::String::fromUTF8 (labels[i]), colX, iy + 28, (w - 54) / 3, 13,
                    juce::Justification::centred);
    }

    // QR 占位（由耳机+翻译度生成确定性图案）
    auto qr = juce::Rectangle<int> (x + 27, y + 225, 66, 66);
    g.setColour (hm::innerBg());
    g.fillRoundedRectangle (qr.toFloat(), 5.0f);
    g.setColour (hm::border());
    g.drawRoundedRectangle (qr.toFloat().reduced (0.5f), 5.0f, 1.0f);
    unsigned h = 2166136261u;
    const auto seedStr = fullName_ + juce::String (degree_);
    auto seed = seedStr.toUTF8();
    while (*seed) { h ^= (unsigned char) *seed++; h *= 16777619u; }
    for (int i = 0; i < 25; ++i)
    {
        const int gx = i % 5, gy = i / 5;
        const bool finder = (gx < 2 && gy < 2) || (gx > 2 && gy < 2) || (gx < 2 && gy > 2);
        const bool on = finder ? !(gx == 1 && gy == 1)
                               : ((h >> (i % 28)) & 1u) != 0;
        g.setColour (hm::whiteA (on ? 0.30f : 0.08f));
        g.fillRoundedRectangle ((float) qr.getX() + 12.0f + gx * 8.4f,
                                (float) qr.getY() + 9.0f + gy * 8.4f, 7.0f, 7.0f, 1.5f);
    }
    g.setFont (7.0f);
    g.setColour (hm::textDim());
    g.drawText (juce::String::fromUTF8 (u8"扫 码"), qr.getX(), qr.getBottom() - 12, qr.getWidth(), 10,
                juce::Justification::centred);

    g.setFont (11.0f);
    g.setColour (hm::textLabel());
    g.drawFittedText (juce::String::fromUTF8 (u8"这首混音通过了 HonestMix 翻译度验证。"),
                      x + 108, y + 228, w - 135, 42, juce::Justification::centredLeft, 2);
    g.setFont (10.0f);
    g.setColour (hm::textDim());
    g.drawText (juce::String::fromUTF8 (u8"免费 · 开源 · 社区驱动"), x + 108, y + 273, w - 135, 15,
                juce::Justification::centredLeft);

    g.setFont (10.0f);
    g.setColour (closeRect_.contains (mouse) ? hm::textLabel() : hm::textDim());
    g.drawText (juce::String::fromUTF8 (u8"收 起"), closeRect_, juce::Justification::centred);
}

void ShareCard::mouseDown (const juce::MouseEvent& e)
{
    if (closeRect_.contains (e.getPosition()) || ! cardRect().contains (e.getPosition()))
        if (onClose) onClose();
}
