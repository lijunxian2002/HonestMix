#pragma once

#include <JuceHeader.h>
#include "../../Utils/BPMCalculator.h"

//==============================================================================
/**
 * 时间参考表组件
 *
 * 根据 BPM 动态计算并显示混音中的各类时间参考值。
 * 复刻 HTML 中的 .bpm-tbl 样式：
 *   - 预延迟 (PREDELAY)
 *   - 混响时间 (REVERB TIME)
 *   - 压缩释放 (COMPRESSOR RELEASE)
 *   - 延迟时间 (DELAY TIME)
 *
 * 每种表可以指定不同的标签和数据生成方式。
 */
class TimeReferenceTable : public juce::Component
{
public:
    //==============================================================================
    /** 表格类型 */
    enum class Type
    {
        PreDelay,     ///< 预延迟：固定几档
        Reverb,       ///< 混响：基于 BPM 计算
        Release,      ///< 压缩释放：基于 BPM 计算
        Delay         ///< 延迟：音符分度值
    };

    //==============================================================================
    TimeReferenceTable(Type type, const juce::String& title);
    ~TimeReferenceTable() override = default;

    //==============================================================================
    /** 设置 BPM 并重新计算 */
    void setBPM(int bpm);

    /** 获取当前 BPM */
    int getBPM() const noexcept { return currentBPM; }

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    Type type;
    juce::String tableTitle;
    int currentBPM = 120;
    BPMCalculator calculator;

    /** 表头/数据行 */
    struct Row {
        juce::String label;
        double valueMs;
        juce::String description;
        juce::Rectangle<int> bounds;
    };

    juce::Array<Row> rows;
    juce::Rectangle<int> titleBounds;
    juce::Rectangle<int> tableBounds;

    /** 根据类型和 BPM 生成行数据 */
    void regenerateRows();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeReferenceTable)
};
