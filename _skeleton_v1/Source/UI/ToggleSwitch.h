#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 校正开关 (Toggle Switch)
 *
 * 复刻 HTML 中的 .mu-sw 样式：
 *   - 左侧标签文字（如 "校正"）
 *   - 右侧滑动开关
 *     .mu-sw-b: 32x18 圆角矩形底色
 *     .mu-sw-k: 12x12 圆形滑块
 *
 * 点击切换 on/off 状态。
 * 内部使用 juce::ToggleButton，但完全自定义绘制。
 */
class ToggleSwitch : public juce::Component
{
public:
    //==============================================================================
    ToggleSwitch();

    //==============================================================================
    /** 设置开关文字 */
    void setLabelText(const juce::String& text);

    /** 设置开关状态 */
    void setToggleState(bool on);

    /** 获取开关状态 */
    bool getToggleState() const noexcept { return isOn; }

    /** 状态变更回调 */
    std::function<void(bool on)> onStateChanged;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    //==============================================================================
    juce::String labelText = TRANS("校正");
    bool isOn = true;

    juce::Rectangle<float> trackBounds;
    juce::Rectangle<float> knobBounds;

    void updateKnobPosition();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitch)
};
