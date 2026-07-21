#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp/CorrectionEngine.h"

/**
 * HonestMix 诚听 — 音频插件处理器
 *
 * M0 功能：
 * - 干湿比（0% = 原始 / 100% = 完全校正 / 默认 50%）
 * - 校正开关：开启时卷积内置 FIR，关闭时零延迟直通
 * - DAW 延迟报告
 */
class HonestMixAudioProcessor final : public juce::AudioProcessor
{
public:
    HonestMixAudioProcessor();
    ~HonestMixAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "HonestMix"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // 公开给 Editor 访问的参数
    juce::AudioParameterFloat* getDryWetParam() const noexcept  { return dryWetParam_; }
    juce::AudioParameterBool*  getCorrectionParam() const noexcept { return correctionParam_; }
    CorrectionEngine& getCorrectionEngine() { return correctionEngine_; }
    int getProfileIndex() const noexcept { return profileIndex_; }
    void setProfileIndex (int idx) noexcept { profileIndex_ = idx; }

private:
    juce::AudioParameterFloat* dryWetParam_       = nullptr;
    juce::AudioParameterBool*  correctionParam_   = nullptr;
    CorrectionEngine correctionEngine_;
    int profileIndex_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessor)
};
