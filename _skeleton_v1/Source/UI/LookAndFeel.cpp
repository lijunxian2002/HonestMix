#include "LookAndFeel.h"

//==============================================================================
HonestMixLookAndFeel::HonestMixLookAndFeel()
{
    initFonts();

    // 设置默认颜色
    setColour(juce::Slider::rotarySliderOutlineColourId, cardBorder);
    setColour(juce::Slider::rotarySliderFillColourId, accent);
    setColour(juce::Slider::textBoxTextColourId, textMuted);
    setColour(juce::Slider::textBoxOutlineColourId, cardBorder);

    setColour(juce::TextButton::buttonColourId, optionBg);
    setColour(juce::TextButton::textColourOffId, textMuted);
    setColour(juce::TextButton::textColourOnId, textPrimary);

    setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x05FFFFFF));
    setColour(juce::TextEditor::textColourId, textMuted);
    setColour(juce::TextEditor::outlineColourId, cardBorder);
    setColour(juce::TextEditor::focusedOutlineColourId, accentDim);

    setColour(juce::Label::textColourId, textMuted);
    setColour(juce::Label::textWhenEditingColourId, textPrimary);

    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0x05FFFFFF));
    setColour(juce::ComboBox::textColourId, textMuted);
    setColour(juce::ComboBox::outlineColourId, cardBorder);
    setColour(juce::ComboBox::arrowColourId, textSubtle);

    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xCC121214));
    setColour(juce::PopupMenu::textColourId, textMuted);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, optionBg);

    setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::ScrollBar::thumbColourId, juce::Colour(0x15FFFFFF));
}

void HonestMixLookAndFeel::initFonts()
{
    // 尝试加载系统字体，回退到默认
    fontRegular = juce::Typeface::createSystemTypefaceFor(
        juce::FontOptions("Inter").withHeight(12.0f));
    fontMono = juce::Typeface::createSystemTypefaceFor(
        juce::FontOptions("SF Mono").withHeight(12.0f));

    if (fontRegular == nullptr)
        fontRegular = juce::Typeface::createSystemTypefaceFor(
            juce::FontOptions(juce::Font::getDefaultSansSerifFontName()).withHeight(12.0f));

    if (fontMono == nullptr)
        fontMono = fontRegular;
}

//==============================================================================
// ——— 按钮 ———

void HonestMixLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour&,
                                                 bool isMouseOver, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto bgColour = button.getToggleState() ? optionBg
                   : isDown                     ? optionBg.brighter(0.05f)
                   : isMouseOver                 ? optionBg.brighter(0.02f)
                   :                              optionBg;

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(cardBorder);
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
}

void HonestMixLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                           bool isMouseOver, bool isDown)
{
    juce::ignoreUnused(isMouseOver, isDown);
    g.setColour(button.getToggleState() ? textPrimary : textMuted);
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, true);
}

//==============================================================================
// ——— 旋钮 ———

void HonestMixLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                             int x, int y, int w, int h,
                                             float sliderPos,
                                             float startAngle, float endAngle,
                                             juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, w, h).reduced(2.0f);
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto lineW = radius * 0.15f;

    // 背景弧（未填充部分）
    g.setColour(cardBorder);
    g.drawArc(bounds.reduced(lineW), startAngle, endAngle - startAngle, true, lineW);

    // 填充弧（从 startAngle 到 sliderPos 指示的角度）
    auto fillAngle = startAngle + (endAngle - startAngle) * sliderPos;
    g.setColour(accent);
    g.drawArc(bounds.reduced(lineW), startAngle, fillAngle - startAngle, true, lineW);

    // 内圆
    auto innerRadius = radius * 0.55f;
    g.setColour(cardBg);
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius,
                  innerRadius * 2, innerRadius * 2);

    // 内圆发光
    auto grad = juce::ColourGradient::horizontal(
        juce::Colour(0x33323337), centre.x - innerRadius,
        juce::Colour(0x3F121214), centre.x + innerRadius);
    g.setColour(juce::Colour(0x33323337));
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius,
                  innerRadius * 2, innerRadius * 2);

    // 指示点
    auto dotAngle = fillAngle;
    auto dotR = radius * 0.70f;
    auto dotX = centre.x + std::cos(dotAngle) * dotR;
    auto dotY = centre.y + std::sin(dotAngle) * dotR;
    g.setColour(textMuted);
    g.fillEllipse(dotX - 2.0f, dotY - 2.0f, 4.0f, 4.0f);
}

//==============================================================================
// ——— 文本框 ———

void HonestMixLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int w, int h,
                                                  juce::TextEditor& editor)
{
    if (editor.hasKeyboardFocus(true))
        g.setColour(accentDim);
    else
        g.setColour(cardBorder);

    g.drawRoundedRectangle(0.0f, 0.0f, w, h, 4.0f, 1.0f);
}

void HonestMixLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int w, int h,
                                                     juce::TextEditor&)
{
    g.setColour(juce::Colour(0x05FFFFFF));
    g.fillRoundedRectangle(0.0f, 0.0f, w, h, 4.0f);
}

//==============================================================================
// ——— 标签 ———

void HonestMixLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText(label.getText(), label.getLocalBounds(),
               label.getJustificationType(), true);
}

//==============================================================================
// ——— 组合框 ———

void HonestMixLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h,
                                         bool, int, int, int, int,
                                         juce::ComboBox&)
{
    g.setColour(juce::Colour(0x05FFFFFF));
    g.fillRoundedRectangle(0.0f, 0.0f, w, h, 4.0f);

    g.setColour(cardBorder);
    g.drawRoundedRectangle(0.0f, 0.0f, w, h, 4.0f, 1.0f);
}

//==============================================================================
// ——— 字体 ———

juce::Font HonestMixLookAndFeel::getTextButtonFont(juce::TextButton&, int height)
{
    return juce::Font(fontRegular).withHeight(juce::jmin(11.0f, height * 0.6f));
}

juce::Typeface::Ptr HonestMixLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    juce::ignoreUnused(font);
    return fontRegular;
}

//==============================================================================
// ——— 静态辅助方法 ———

void HonestMixLookAndFeel::drawCardBackground(juce::Graphics& g,
                                               juce::Rectangle<int> bounds,
                                               float cornerRadius)
{
    g.setColour(cardBg);
    g.fillRoundedRectangle(bounds.toFloat(), cornerRadius);

    g.setColour(cardBorder);
    g.drawRoundedRectangle(bounds.toFloat(), cornerRadius, 1.0f);
}

void HonestMixLookAndFeel::drawOptionRow(juce::Graphics& g,
                                          juce::Rectangle<int> bounds,
                                          bool selected, bool hovered)
{
    auto bg = selected ? optionBg.brighter(0.04f)
             : hovered  ? optionBg.brighter(0.02f)
             :            optionBg;

    auto border = selected ? optionBorder.brighter(0.06f)
                 : hovered  ? optionBorder.brighter(0.04f)
                 :            optionBorder;

    g.setColour(bg);
    g.fillRoundedRectangle(bounds.toFloat(), 5.0f);

    g.setColour(border);
    g.drawRoundedRectangle(bounds.toFloat(), 5.0f, 1.0f);
}

void HonestMixLookAndFeel::drawRadioDot(juce::Graphics& g,
                                         juce::Rectangle<int> bounds,
                                         bool selected)
{
    auto dotBounds = bounds.toFloat();
    g.setColour(selected ? textMuted : optionBorder);
    g.drawEllipse(dotBounds, 1.5f);

    if (selected)
    {
        auto inner = dotBounds.reduced(4.0f);
        g.setColour(textMuted);
        g.fillEllipse(inner);
    }
}
