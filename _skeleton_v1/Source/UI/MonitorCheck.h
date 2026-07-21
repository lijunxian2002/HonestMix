#pragma once

#include <JuceHeader.h>
#include "../Core/AppState.h"

//==============================================================================
/**
 * 1 小时检查提醒 (Monitor Check)
 *
 * 混音 1 小时后自动弹出的检查弹窗。
 * 复刻 HTML 中的 .fb-overlay + .fb-box 样式：
 *   - 标题: "混了 1 小时了，要不要检查一下监听环境？"
 *   - 选项：切换单声道 / 导出手机 / 车内验听 / 继续混
 *   - "好，听完了告诉我翻译度"
 */
class MonitorCheck : public juce::Component
{
public:
    //==============================================================================
    MonitorCheck(AppState& appState, std::function<void()> onDismiss);
    ~MonitorCheck() override = default;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    AppState& appState;
    std::function<void()> onDismiss;

    struct CheckOption {
        juce::Rectangle<int> bounds;
        juce::String label;
        bool selected = false;
    };
    CheckOption options[4];
    int selectedIndex = -1;

    juce::Rectangle<int> questionArea;
    juce::Rectangle<int> optionsArea;
    juce::Rectangle<int> dismissArea;

    void onOptionClicked(int index);
    void mouseDown(const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MonitorCheck)
};
