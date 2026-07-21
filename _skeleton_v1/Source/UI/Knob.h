#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 自定义 WET 旋钮
 *
 * 复刻 HTML 原型中的 .mu-knob 样式：
 *   - 弧形渐变背景（conic-gradient）
 *   - 内圆发光（radial-gradient）
 *   - 中央显示 "WET" 文字
 *   - 下方数值 + 单位（如 "50%"）
 *
 * 基于 juce::Slider 扩展，使用 RotaryVerticalDrag 模式。
 */
class Knob : public juce::Slider
{
public:
    //==============================================================================
    Knob();

    //==============================================================================
    // ——— 样式 ———

    /** 设置旋钮中央标签文字（如 "WET"） */
    void setLabelText(const juce::String& label);

    /** 设置数值后缀（如 "%"） */
    void setValueSuffix(const juce::String& suffix);

    //==============================================================================
    // ——— 绘制 ———

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    juce::String labelText   = "WET";
    juce::String valueSuffix = "%";
    juce::Label  valueLabel;

    /** 获取当前值字符串 */
    juce::String getValueText() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob)
};
