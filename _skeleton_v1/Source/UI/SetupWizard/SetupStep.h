#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 设置步骤基类
 *
 * 为 SetupWizard 的三个步骤提供统一的接口和样式。
 * 每个步骤包含：问题文本 + 选项列表（radio 按钮样式）
 *
 * 子类需实现：
 *   - getStepTitle()     — 标题（如 "步骤 1 / 3 · 监听设备"）
 *   - getQuestionText()  — 问题文本（如 "你在用什么耳机？"）
 *   - populateOptions()  — 调用 addOption() 添加选项
 *
 * 样式：暗色调 radio 按钮列表
 *   选项行：.opt 样式（底色 rgba(255,255,255,0.04)，悬浮和选中态高亮）
 *   选中标记：.opt-r 圆形渐变
 */
class SetupStep : public juce::Component
{
public:
    //==============================================================================
    SetupStep();
    ~SetupStep() override = default;

    //==============================================================================
    // ——— 子类需实现 ———

    virtual juce::String getStepTitle() const = 0;
    virtual juce::String getQuestionText() const = 0;
    virtual void populateOptions() = 0;

    //==============================================================================
    // ——— 公共接口 ———

    /** 添加一个选项 */
    void addOption(const juce::String& label,
                   const juce::String& subtitle = {},
                   bool isPopular = false);

    /** 当前选中的选项索引 (-1 表示未选) */
    int getSelectedIndex() const noexcept { return selectedIndex; }

    /** 是否有选中任何选项 */
    bool isSelectionValid() const { return selectedIndex >= 0; }

    /** 重置选中状态 */
    void resetSelection();

    /** 选中变更回调（由 SetupWizard 订阅以启用/禁用 "下一步" 按钮） */
    std::function<void()> onSelectionChanged;

    //==============================================================================
    // ——— Component ———

    void resized() override;
    void paint(juce::Graphics& g) override;

    //==============================================================================
    // ——— 鼠标交互 ———

    void mouseDown(const juce::MouseEvent& event) override;

private:
    //==============================================================================
    struct Option
    {
        juce::String label;
        juce::String subtitle;
        bool isPopular;
        juce::Rectangle<int> bounds; // 缓存绘制位置
    };

    juce::OwnedArray<Option> options;
    int selectedIndex = -1;
    int hoveredIndex  = -1;

    // 布局缓存
    juce::Rectangle<int> titleArea;
    juce::Rectangle<int> questionArea;
    juce::Rectangle<int> optionsArea;
    juce::Rectangle<int> extraArea;

    /** 计算哪个选项被点击 */
    int hitTestOption(juce::Point<int> position) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SetupStep)
};
