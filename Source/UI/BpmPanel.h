#pragma once

//==============================================================================
/**
 * BPM 助手（浮动面板 396×570；数据中心原样抽取）
 * 预延迟 / 混响 / 压缩释放 / 延迟 四张表，全部按拍长实时换算；
 * 支持手输 + 打拍测速 + 宿主 BPM 自动跟随（PluginEditor 驱动）
 */
#include <juce_gui_basics/juce_gui_basics.h>

class BpmPanel : public juce::Component, private juce::Timer
{
public:
    std::function<void()> onClose;

    BpmPanel();

    void setBpm (int bpm);
    int  getBpm() const noexcept { return bpm_; }

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void visibilityChanged() override;

private:
    void timerCallback() override;
    void handleTap();

    juce::TextEditor input_;
    int bpm_ = 117;
    juce::Rectangle<int> tapRect_, closeRect_;
    juce::Array<juce::Time> taps_;
    juce::int64 tapFlashMs_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BpmPanel)
};
