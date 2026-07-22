#pragma once

//==============================================================================
/**
 * BPM 助手（浮动毛玻璃面板 380×522，固定尺寸不随窗口缩放）
 * 预延迟 / 混响 / 压缩释放 / 延迟 四张表，全部按拍长实时换算
 * 支持手输 + 打拍测速 + 宿主 BPM 自动跟随（PluginEditor 驱动）
 *
 * 第 5 轮修复：布局按面板实际宽度均衡排布（原来固定 388px 内绘 +
 * 缩放边界 → 右侧死区）；字体全面加大；收起按钮钉右上角；
 * 打开时面板区域高斯模糊铺底。
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

    /** 显示后由 Editor 调用：捕获面板区域快照并高斯模糊（毛玻璃底） */
    void captureBackdrop();

    static constexpr int kPanelW = 380;
    static constexpr int kPanelH = 460;

private:
    void timerCallback() override;
    void handleTap();

    juce::TextEditor input_;
    int bpm_ = 117;
    juce::Rectangle<int> tapRect_, closeRect_;
    juce::Array<juce::Time> taps_;
    juce::int64 tapFlashMs_ = 0;
    juce::Image backdrop_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BpmPanel)
};
