#include "OptionButton.h"

OptionButton::OptionButton() { setInterceptsMouseClicks(true, false); }

void OptionButton::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();
    auto bg = selected_ ? juce::Colours::white.withAlpha(0.08f)
             : isHovered_ ? juce::Colours::white.withAlpha(0.05f)
             : juce::Colours::white.withAlpha(0.03f);
    g.setColour(bg);
    g.fillRoundedRectangle(b, 5.0f);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(b.reduced(0.5f), 5.0f, 1.0f);
    g.setFont(juce::FontOptions(8.0f));
    auto tc = selected_ ? juce::Colours::white.withAlpha(0.55f)
              : isHovered_ ? juce::Colours::white.withAlpha(0.45f)
              : juce::Colours::white.withAlpha(0.30f);
    g.setColour(tc);
    g.drawText(text_, getLocalBounds(), juce::Justification::centred);
}

void OptionButton::mouseDown(const juce::MouseEvent&) {
    selected_ = true;
    repaint();
    if (onClick) onClick();
}
