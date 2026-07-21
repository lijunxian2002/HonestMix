#pragma once

//==============================================================================
/**
 * 分享卡（全屏遮罩 + 玻璃卡 396×354；数据中心原样抽取）
 * 展示耳机/品牌/翻译度 + 确定性伪 QR 图案
 */
#include <juce_gui_basics/juce_gui_basics.h>

class ShareCard : public juce::Component
{
public:
    std::function<void()> onClose;

    ShareCard();

    void setInfo (const juce::String& fullName, const juce::String& brand, int translationDegree);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    /** 显示前由 Editor 调用：捕获主界面快照并高斯模糊（Apple 毛玻璃底） */
    void captureBackdrop();

private:
    juce::Rectangle<int> cardRect() const;

    juce::String fullName_ = "Audio-Technica ATH-M50x";
    juce::String brand_    = "Audio-Technica";
    int degree_ = 50;
    juce::Rectangle<int> closeRect_;
    juce::Image backdrop_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShareCard)
};
