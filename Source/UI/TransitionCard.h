#pragma once

//==============================================================================
/**
 * 过渡卡（首次引导完成 → 主界面；数据中心原样抽取）
 * 文案即"唯一旋杆规则"：0 ~ 200 · 拧多少矫正多少
 */
#include <juce_gui_basics/juce_gui_basics.h>

class TransitionCard : public juce::Component
{
public:
    std::function<void (int)> onStart;   // 参数：起始翻译度（默认 50）
    std::function<void()>     onDismiss; // 再调一下（预留）

    TransitionCard();

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

    /** 显示前由 Editor 调用：捕获主界面快照并高斯模糊（Apple 毛玻璃底） */
    void captureBackdrop() { backdrop_ = captureSelf(); }

private:
    int choice_ = 50;
    juce::Rectangle<int> btnStart_;
    juce::Image backdrop_;

    juce::Image captureSelf();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransitionCard)
};
