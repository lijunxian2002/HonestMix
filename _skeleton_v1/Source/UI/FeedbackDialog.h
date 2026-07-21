#pragma once

#include <JuceHeader.h>
#include "../Core/AppState.h"

//==============================================================================
/**
 * 翻译度反馈对话框
 *
 * 混音结束后或用户从 MonitorCheck 返回后弹出，
 * 询问用户对校正结果的感受：
 *   - 亮了 / 暗了 / 多了 / 少了
 *   - 收集反馈 → 服务器迭代（模拟）
 *
 * 数据最终发送到服务器端，按耳机型号分组统计，
 * 达到置信阈值后自动微调目标曲线。
 */
class FeedbackDialog : public juce::Component
{
public:
    //==============================================================================
    FeedbackDialog();

    /** 显示对话框 */
    void show();

    /** 隐藏对话框 */
    void hide();

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    /** 反馈提交回调 */
    std::function<void(int feedbackIndex)> onFeedbackSubmitted;

private:
    //==============================================================================
    struct FeedbackOption {
        juce::Rectangle<int> bounds;
        juce::String label;
        bool selected = false;
    };

    FeedbackOption options[4];
    int selectedIndex = -1;

    juce::Rectangle<int> questionArea;
    juce::Rectangle<int> optionsArea;
    juce::Rectangle<int> submitArea;

    static constexpr const char* optionLabels[4] = {
        "亮了", "暗了", "多了", "少了"
    };

    void onOptionClicked(int index);
    void onSubmit();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FeedbackDialog)
};
