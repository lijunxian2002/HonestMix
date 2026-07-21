#pragma once

#include <JuceHeader.h>
#include "../Core/AppState.h"

//==============================================================================
/**
 * 混音完成分享卡片
 *
 * 复刻 HTML 中的 .share-card 样式：
 *   - 标题: HonestMix / 翻译度已确认
 *   - 波形图占位 + 工程名
 *   - 信息栏：干湿比 / 耳机型号 / 目标曲线
 *   - 模拟二维码矩阵
 *   - 文字说明
 *   - "保存卡片" / "分享" 按钮
 *
 * 用于社交分享（朋友圈裂变）。
 */
class ShareCard : public juce::Component
{
public:
    //==============================================================================
    explicit ShareCard(AppState& appState);
    ~ShareCard() override = default;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    AppState& appState;

    juce::TextButton saveButton;
    juce::TextButton shareButton;

    // 布局区域
    juce::Rectangle<int> titleArea;
    juce::Rectangle<int> waveformArea;
    juce::Rectangle<int> infoArea;
    juce::Rectangle<int> qrArea;
    juce::Rectangle<int> textArea;
    juce::Rectangle<int> buttonsArea;

    /** 绘制模拟二维码 */
    void drawQRCode(juce::Graphics& g, juce::Rectangle<int> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShareCard)
};
