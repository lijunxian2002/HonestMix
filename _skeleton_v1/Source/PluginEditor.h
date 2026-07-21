#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MainComponent;

//==============================================================================
/**
 * HonestMix 插件编辑器
 *
 * 作为插件 UI 的容器，持有 MainComponent 作为唯一子组件。
 * 负责：
 *   - 设置编辑器初始尺寸 (800×600)
 *   - 传递 AppState 给 MainComponent
 *   - 响应宿主缩放比例变更
 */
class HonestMixAudioProcessorEditor
    : public juce::AudioProcessorEditor
{
public:
    //==============================================================================
    explicit HonestMixAudioProcessorEditor(HonestMixAudioProcessor& processor);
    ~HonestMixAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    HonestMixAudioProcessor& processor;
    std::unique_ptr<MainComponent> mainComponent;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HonestMixAudioProcessorEditor)
};
