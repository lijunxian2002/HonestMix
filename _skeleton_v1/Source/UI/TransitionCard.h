#pragma once

#include <JuceHeader.h>
#include "../Core/AppState.h"

//==============================================================================
/**
 * 过渡卡 (Transition Card)
 *
 * 设置完成后、"开始混音"前显示的过渡界面。
 * 复刻 HTML 中的 .tc-overlay + .tc-box 样式：
 *   - 标题 "准备好了"
 *   - 干湿比选择（50% 强烈对比 / 100% 完全校正）
 *   - 4 条使用说明
 *   - "再调一下" / "开始混音" 按钮
 */
class TransitionCard : public juce::Component
{
public:
    //==============================================================================
    TransitionCard(AppState& appState, std::function<void()> onConfirm);
    ~TransitionCard() override = default;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    /** 获取当前选择的干湿比 */
    float getSelectedWetDry() const noexcept;

    void mouseDown(const juce::MouseEvent& event) override;

private:
    //==============================================================================
    AppState& appState;
    std::function<void()> onConfirm;

    // 选项按钮区域
    struct WetDryOption {
        juce::Rectangle<int> bounds;
        float percent;
        juce::String label;
        juce::String subtitle;
    };
    WetDryOption options[2];
    int selectedOption = 0;

    juce::Rectangle<int> iconArea;
    juce::Rectangle<int> titleArea;
    juce::Rectangle<int> optionsArea;
    juce::Rectangle<int> instructionsArea;
    juce::Rectangle<int> buttonArea;

    juce::TextButton backButton;
    juce::TextButton startButton;

    void onOptionClicked(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransitionCard)
};
