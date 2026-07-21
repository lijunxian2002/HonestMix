#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 声卡浏览器（展开列表）
 *
 * 当用户在 StepInterface 中选择"更多声卡"时弹出。
 * 显示完整的声卡型号列表，支持搜索过滤。
 *
 * 复刻 HTML 中的 .expand-panel 样式：
 *   - 标题 "选择音频接口"
 *   - 搜索输入框
 *   - 可滚动列表（含热门标签）
 *   - "收起"按钮
 */
class AudioInterfaceBrowser : public juce::Component,
                              private juce::ListBoxModel
{
public:
    //==============================================================================
    AudioInterfaceBrowser();

    /** 显示弹窗 */
    void show();

    /** 隐藏弹窗 */
    void hide();

    /** 选中回调（返回选中的索引和名称） */
    std::function<void(int index, const juce::String& name)> onInterfaceSelected;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

private:
    //==============================================================================
    juce::TextEditor searchBox;
    juce::ListBox listBox;
    juce::TextButton closeButton;

    /** 当前过滤后的列表 */
    juce::Array<int> filteredIndices;

    juce::String searchText;

    void refreshFilter();
    void onItemClicked(int index);

    // ——— ListBoxModel ———
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g,
                          int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    juce::String getTooltipForRow(int row) override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioInterfaceBrowser)
};
