#pragma once

//==============================================================================
/**
 * HonestMix BETA2 — 骨架编辑器（UI 层接口桩）
 *
 * 本类只满足 JUCE AudioProcessorEditor 最低编译要求。
 * 实际 UI 由独立 UI 层实现，通过 HonestMixAudioProcessor 的公共接口接入。
 *
 * 职责：无。仅提供宿主可用的空白窗口。
 */
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class HonestMixAudioProcessorEditor final
    : public juce::AudioProcessorEditor
{
public:
    explicit HonestMixAudioProcessorEditor (HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HonestMixAudioProcessor& processorRef_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
