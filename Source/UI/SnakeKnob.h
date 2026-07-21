#pragma once

//==============================================================================
/**
 * 蛇形旋杆 LookAndFeel（数据中心 KnobLNF 原样抽取）
 * conic-gradient 近似：分段渐变弧 + 内盘 + WET 中心字
 * 用法：knob.setLookAndFeel(&snakeKnob); —— 宿主组件负责持有实例
 */
#include <juce_gui_basics/juce_gui_basics.h>

class SnakeKnobLNF : public juce::LookAndFeel_V4
{
public:
    SnakeKnobLNF() = default;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SnakeKnobLNF)
};
