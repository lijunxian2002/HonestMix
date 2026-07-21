#pragma once

//==============================================================================
/**
 * 翻译度反馈弹窗（全屏遮罩 + 玻璃卡 396×408；数据中心原样抽取）
 * 低频/高频各三档，onSubmit(bass, treble)：0=刚好 1=多/亮 2=少/暗
 */
#include <juce_gui_basics/juce_gui_basics.h>

class FeedbackCard : public juce::Component
{
public:
    std::function<void (int bass, int treble)> onSubmit;
    std::function<void()> onClose;

    FeedbackCard();

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

    /** 显示前由 Editor 调用：捕获主界面快照并高斯模糊（Apple 毛玻璃底） */
    void captureBackdrop();

private:
    juce::Rectangle<int> rowRect (int section, int row) const;
    juce::Rectangle<int> cardRect() const;

    int bass_ = 0, treble_ = 0;
    juce::Rectangle<int> submitRect_, closeRect_;
    juce::Image backdrop_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackCard)
};
