#pragma once

#include <JuceHeader.h>
#include "../../Core/AppState.h"

class TimeReferenceTable;
class TapTempo;

//==============================================================================
/**
 * BPM 助手面板
 */
class BPMAssistant : public juce::Component
{
public:
    //==============================================================================
    explicit BPMAssistant(AppState& appState);
    ~BPMAssistant() override;

    //==============================================================================
    /** 刷新数据（从 AppState 读取最新 BPM） */
    void refreshData();

    /** 设置 BPM */
    void setBPM(int bpm);

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    AppState& appState;

    // ——— 子组件 ———
    juce::TextEditor bpmInput;
    juce::TextButton tapButton;
    juce::TextButton closeButton;
    juce::Label bpmValueLabel;
    juce::Label beatDurationLabel;

    std::unique_ptr<TimeReferenceTable> preDelayTable;
    std::unique_ptr<TimeReferenceTable> reverbTable;
    std::unique_ptr<TimeReferenceTable> releaseTable;
    std::unique_ptr<TimeReferenceTable> delayTable;
    std::unique_ptr<TapTempo> tapTempo;

    // ——— 布局缓存 ———
    juce::Rectangle<int> titleArea;
    juce::Rectangle<int> inputArea;
    juce::Rectangle<int> summaryArea;
    juce::Rectangle<int> tablesArea;

    void onTapTempo();
    void onBPMInputChanged();
    void updateDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BPMAssistant)
};
