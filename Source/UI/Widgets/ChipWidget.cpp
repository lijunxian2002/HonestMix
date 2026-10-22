#include "ChipWidget.h"

ChipWidget::ChipWidget() { setInterceptsMouseClicks(true, false); }

void ChipWidget::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();
    float s = (float)getWidth() / 560.0f;

    auto bg = isHovered_ ? juce::Colours::white.withAlpha(0.02f)
                         : juce::Colour::fromRGBA(0, 0, 0, (juce::uint8)(255 * 0.28f));
    g.setColour(bg);
    g.fillRoundedRectangle(b, 5.0f);
    g.setColour(juce::Colour::fromRGBA(0, 0, 0, (juce::uint8)(255 * 0.28f)));
    g.drawRoundedRectangle(b.reduced(0.75f), 5.0f, 1.5f);
    // HTML: inset 0 1px 0 rgba(255,255,255,0.01) + inset 0 2px 4px rgba(0,0,0,0.42)
    g.setColour(juce::Colours::white.withAlpha(0.01f));g.fillRect(b.getX()+2.f,b.getY()+1.f,b.getWidth()-4.f,1.f);
    g.setColour(juce::Colour::fromRGBA(0,0,0,107));g.fillRect(b.getX()+2.f,b.getY()+2.f,b.getWidth()-4.f,3.f);

    // hover 辉光 — 飞船舷窗亮起（三层光晕边框）
    if (isHovered_) {
        juce::Colour glow = juce::Colour(190, 200, 215);
        g.setColour(glow.withAlpha(0.05f)); g.drawRoundedRectangle(b.expanded(5.f), 7.f, 2.f);
        g.setColour(glow.withAlpha(0.09f)); g.drawRoundedRectangle(b.expanded(3.f), 6.f, 1.5f);
        g.setColour(glow.withAlpha(0.16f)); g.drawRoundedRectangle(b.expanded(1.f), 5.f, 1.f);
    }
    // 标签 — 默认低调，hover 点亮
    g.setFont(juce::FontOptions(9.0f));
    g.setColour(juce::Colour(235, 235, 230).withAlpha(isHovered_ ? 0.80f : 0.45f));
    g.drawText(label_, labelRect_, juce::Justification::centred);

    // 数值 — hover 时同字号 8 向微散辉光（仪表盘式，无重影），再画亮字
    if (isHovered_) {
        g.setFont(juce::FontOptions(12.0f));
        g.setColour(juce::Colour(190, 200, 215).withAlpha(0.12f));
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                g.drawText(value_, valueRect_.translated(dx, dy), juce::Justification::centred);
            }
        g.setColour(juce::Colour(190, 200, 215).withAlpha(0.25f));
        g.drawText(value_, valueRect_, juce::Justification::centred);
    }
    g.setFont(juce::FontOptions(12.0f));
    g.setColour(juce::Colour(240, 240, 235).withAlpha(isHovered_ ? 0.95f : 0.68f));
    g.drawText(value_, valueRect_, juce::Justification::centred);
}

void ChipWidget::resized() {
    auto b = getLocalBounds();
    labelRect_ = { b.getX(), b.getY() + 7, b.getWidth(), 11 };
    valueRect_ = { b.getX() + 4, labelRect_.getBottom() + 2,
                   b.getWidth() - 8, b.getHeight() - labelRect_.getHeight() - 9 };
}

void ChipWidget::mouseEnter(const juce::MouseEvent&) { if (hoverable_) { isHovered_ = true; repaint(); } }
void ChipWidget::mouseExit(const juce::MouseEvent&) { if (hoverable_) { isHovered_ = false; repaint(); } }
void ChipWidget::mouseDown(const juce::MouseEvent&) { if (onOpen) onOpen(); }
