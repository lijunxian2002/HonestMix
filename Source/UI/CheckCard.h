#pragma once

//==============================================================================
/**
 * 1 小时检查卡（全屏遮罩 + 玻璃卡 396×310；数据中心原样抽取）
 * onAction(opt)：0 单声道 / 1 iPhone 外放 / 2 汽车音响 / 3 继续混
 */
#include <juce_gui_basics/juce_gui_basics.h>

class CheckCard : public juce::Component
{
public:
    std::function<void (int opt)> onAction;
    std::function<void()> onClose;

    CheckCard();

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

    /** 显示前由 Editor 调用：捕获主界面快照并高斯模糊（Apple 毛玻璃底） */
    void captureBackdrop();

private:
    juce::Rectangle<int> cardRect() const;
    juce::Rectangle<int> rowRect (int i) const;
    juce::Rectangle<int> footRect_;
    juce::Image backdrop_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CheckCard)
};
