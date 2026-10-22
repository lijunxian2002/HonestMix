#include "HoverButton.h"
#include "Core/Design.h"

HoverButton::HoverButton() { setInterceptsMouseClicks(true, false); }

void HoverButton::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat().reduced(0.5f);
    // HTML: box-shadow 0 1px 2px rgba(0,0,0,0.2)
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillRoundedRectangle(b.translated(0.f,2.f), 4.0f);
    // 背景
    auto bg = juce::Colour::fromRGBA(0, 0, 0, (juce::uint8)(255 * 0.20f));
    if (isPressed_) bg = bg.brighter(0.15f);
    else if (isHovered_) bg = bg.brighter(0.1f);
    g.setColour(bg);
    g.fillRoundedRectangle(b, 4.0f);
    // HTML: inset 0 1px 0 rgba(255,255,255,0.01) — 顶部高光线
    g.setColour(juce::Colours::white.withAlpha(0.01f));
    g.fillRect(b.getX()+2.f,b.getY()+1.f,b.getWidth()-4.f,1.f);
    // 边框
    g.setColour(juce::Colour::fromRGBA(0, 0, 0, (juce::uint8)(255 * 0.42f)));
    g.drawRoundedRectangle(b, 4.0f, 1.0f);
    // hover 辉光 — 飞船舷窗亮起（三层光晕边框）
    bool lit = isHovered_;
    if (lit) {
        juce::Colour glow = juce::Colour(190, 200, 215);
        g.setColour(glow.withAlpha(0.05f)); g.drawRoundedRectangle(b.expanded(5.f), 7.f, 2.f);
        g.setColour(glow.withAlpha(0.09f)); g.drawRoundedRectangle(b.expanded(3.f), 6.f, 1.5f);
        g.setColour(glow.withAlpha(0.16f)); g.drawRoundedRectangle(b.expanded(1.f), 5.f, 1.f);
    }
    // 文字 — 默认恢复低调，hover 时点亮；辉光用同字号 8 向微散（仪表盘式，无重影）
    float fs = 11.0f;
    auto txtCol = lit ? juce::Colour(245, 245, 240).withAlpha(0.92f)
                : isAccent_ ? juce::Colour(255, 255, 250).withAlpha(0.50f)
                : juce::Colour(235, 235, 230).withAlpha(0.38f);
    if (lit) {
        auto tb = getLocalBounds();
        g.setFont(juce::FontOptions(fs));
        g.setColour(juce::Colour(190, 200, 215).withAlpha(0.10f));
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                g.drawText(text_, tb.translated(dx, dy), juce::Justification::centred);
            }
        g.setColour(juce::Colour(190, 200, 215).withAlpha(0.22f));
        g.drawText(text_, tb, juce::Justification::centred);
    }
    g.setFont(juce::FontOptions(fs));
    g.setColour(txtCol);
    g.drawText(text_, getLocalBounds(), juce::Justification::centred);
}

void HoverButton::mouseUp(const juce::MouseEvent&) {
    isPressed_ = false;
    repaint();
    if (onClick) onClick();
}
