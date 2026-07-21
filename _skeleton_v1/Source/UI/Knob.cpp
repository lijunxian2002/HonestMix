#include "Knob.h"
#include "LookAndFeel.h"

//==============================================================================
Knob::Knob()
{
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setRange(0.0, 100.0, 1.0);
    setValue(50.0);
    setVelocityModeParameters(0.5, 1, 0.0, false);

    // 数值显示标签
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(juce::FontOptions(20.0f).withWeight(juce::Font::Weight::light));
    addAndMakeVisible(valueLabel);
}

//==============================================================================
void Knob::setLabelText(const juce::String& label)  { labelText = label; repaint(); }
void Knob::setValueSuffix(const juce::String& suffix) { valueSuffix = suffix; updateText(); }

//==============================================================================
void Knob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 4.0f;

    // === 外圈弧 ===
    float startAngle = juce::MathConstants<float>::pi * 0.75f;
    float endAngle   = juce::MathConstants<float>::pi * 2.25f;
    float range      = endAngle - startAngle;
    float currentAngle = startAngle + range * (float)getValue() / 100.0f;

    float lineWidth = radius * 0.25f;

    // 背景弧
    g.setColour(HonestMixLookAndFeel::cardBorder);
    juce::Path bgArc;
    bgArc.addArc(centre.x - radius, centre.y - radius,
                 radius * 2, radius * 2,
                 startAngle, endAngle, true);
    g.strokePath(bgArc, juce::PathStrokeType(lineWidth));

    // 填充弧（从 start 到当前值）
    g.setColour(HonestMixLookAndFeel::accent);
    juce::Path fillArc;
    fillArc.addArc(centre.x - radius, centre.y - radius,
                   radius * 2, radius * 2,
                   startAngle, currentAngle, true);
    g.strokePath(fillArc, juce::PathStrokeType(lineWidth));

    // === 内圆 ===
    float innerRadius = radius * 0.55f;

    // 背景
    g.setColour(HonestMixLookAndFeel::cardBg);
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius,
                  innerRadius * 2, innerRadius * 2);

    // 内圆发光渐变
    auto grad = juce::ColourGradient(
        juce::Colour(0x33323337), centre.x - innerRadius, centre.y,
        juce::Colour(0x3F121214), centre.x + innerRadius, centre.y,
        false);
    g.setGradientFill(grad);
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius,
                  innerRadius * 2, innerRadius * 2);

    // === 中央文字 ===
    float fontH = innerRadius * 0.4f;
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(fontH));
    g.drawText(labelText,
               centre.x - innerRadius,
               centre.y - innerRadius,
               innerRadius * 2, innerRadius * 2,
               juce::Justification::centred, true);

    // === 指示点 ===
    float dotR = radius * 0.72f;
    float dotX = centre.x + std::cos(currentAngle) * dotR;
    float dotY = centre.y + std::sin(currentAngle) * dotR;
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.fillEllipse(dotX - 2.5f, dotY - 2.5f, 5.0f, 5.0f);
}

void Knob::resized()
{
    auto bounds = getLocalBounds();

    // 数值标签在旋钮下方
    valueLabel.setBounds(bounds.removeFromBottom(28));
    updateText();
}

//==============================================================================
juce::String Knob::getValueText() const
{
    return juce::String(static_cast<int>(getValue())) + valueSuffix;
}

void Knob::updateText()
{
    valueLabel.setText(getValueText(), juce::dontSendNotification);
}
