#include "ToggleSwitch.h"
#include "LookAndFeel.h"

//==============================================================================
ToggleSwitch::ToggleSwitch()
{
    setSize(80, 24);
}

//==============================================================================
void ToggleSwitch::setLabelText(const juce::String& text)
{
    labelText = text;
    repaint();
}

void ToggleSwitch::setToggleState(bool on)
{
    if (isOn != on)
    {
        isOn = on;
        repaint();
        if (onStateChanged)
            onStateChanged(isOn);
    }
}

//==============================================================================
void ToggleSwitch::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 左侧标签
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(9.0f));
    g.drawText(labelText, bounds.removeFromLeft(40).toFloat(),
               juce::Justification::centredLeft, true);

    // 右侧开关轨道
    trackBounds = bounds.removeFromRight(34).reduced(2, 4).toFloat();

    // 背景
    g.setColour(isOn ? HonestMixLookAndFeel::accentDim
                     : HonestMixLookAndFeel::divider);
    g.fillRoundedRectangle(trackBounds, trackBounds.getHeight() / 2.0f);

    // 边框
    g.setColour(HonestMixLookAndFeel::cardBorder);
    g.drawRoundedRectangle(trackBounds, trackBounds.getHeight() / 2.0f, 1.0f);

    // 旋钮
    float knobDiameter = trackBounds.getHeight() - 4.0f;
    float knobX = isOn ? trackBounds.getRight() - knobDiameter - 2.0f
                       : trackBounds.getX() + 2.0f;

    knobBounds = juce::Rectangle<float>(knobX, trackBounds.getY() + 2.0f,
                                         knobDiameter, knobDiameter);

    g.setColour(isOn ? HonestMixLookAndFeel::accent
                     : HonestMixLookAndFeel::textSubtle);
    g.fillEllipse(knobBounds);
}

void ToggleSwitch::resized()
{
    // 绘制时计算位置
}

void ToggleSwitch::mouseDown(const juce::MouseEvent&)
{
    setToggleState(!isOn);
}

void ToggleSwitch::updateKnobPosition()
{
    repaint();
}
