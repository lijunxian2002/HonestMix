#include "UI/SnakeKnob.h"
#include "Core/Design.h"

void SnakeKnobLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                     float pos, float sa, float ea, juce::Slider& s)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
    const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    const float r  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float a  = sa + (ea - sa) * pos;
    const float boost = s.isMouseOverOrDragging() ? 1.35f : 1.0f;

    // 内盘：暖黑底 + 极淡朱砂晕
    g.setColour (hm::innerBg().interpolatedWith (hm::accent(), pos * 0.03f));
    g.fillEllipse (bounds);

    // 全圆轨道
    g.setColour (hm::whiteA (0.04f));
    g.drawEllipse (cx - r + 3, cy - r + 3, (r - 3) * 2, (r - 3) * 2, 3.0f);

    // 蛇身：单条弧
    juce::Path body;
    body.addArc (cx - r + 3, cy - r + 3, (r - 3) * 2, (r - 3) * 2, sa, a, true);
    g.setColour (hm::whiteA (0.20f * boost));
    g.strokePath (body, juce::PathStrokeType (3.0f));

    // 蛇头圆点 — 高强度时暖亮
    const float px = cx + (r - 3) * std::cos (a), py = cy + (r - 3) * std::sin (a);
    g.setColour (pos > 0.75f ? hm::accentWarm().withAlpha (0.5f * boost)
                             : hm::whiteA (0.50f * boost));
    g.fillEllipse (px - 4.0f, py - 4.0f, 8.0f, 8.0f);

    g.setFont (11.0f);
    g.setColour (hm::textLabel());
    g.drawText ("WET", bounds, juce::Justification::centred);
}
