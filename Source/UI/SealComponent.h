#pragma once

//==============================================================================
/**
 * 宠物「诚」66×66 玻璃印章（数据中心原样抽取）
 * 点击 → 反馈卡；右上角小点 = 翻译度 > 0 指示
 */
#include <juce_gui_basics/juce_gui_basics.h>

class SealComponent : public juce::Component
{
public:
    SealComponent() = default;

    std::function<void()> onClick;

    void setDotOn (bool on);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { repaint(); }

private:
    bool dotOn_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SealComponent)
};
