#pragma once

#include <JuceHeader.h>
#include "Core/AppState.h"
#include "Core/HeadphoneDatabase.h"
#include "DSP/CorrectionProcessor.h"
#include "DSP/WetDryMixer.h"

//==============================================================================
/**
 * HonestMix 音频插件处理器
 *
 * 插件入口点。职责：
 *   1. 管理全局参数（通过 AudioProcessorValueTreeState）
 *   2. 持有 DSP 处理链（CorrectionProcessor → WetDryMixer）
 *   3. 管理应用状态（AppState）
 *   4. 管理耳机校正曲线数据库（HeadphoneDatabase）
 *   5. 与 DAW 交互：参数自动化、状态持久化、延迟补偿
 *
 * 参数：
 *   - wetDryMix         0–100%  干湿比
 *   - correctionEnabled bool    校正开关
 *   - headphoneModel     int    耳机型号索引
 *   - audioInterface     int    声卡型号索引
 *   - mixingHabit        int    混音习惯索引
 *   - bpmValue          20–300  BPM 值
 */
class HonestMixAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    HonestMixAudioProcessor();
    ~HonestMixAudioProcessor() override;

    //==============================================================================
    // ——— 参数管理 ———

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();

    //==============================================================================
    // ——— AudioProcessor 接口 ———

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    // ——— 编辑器 ———

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    // ——— 状态持久化 ———

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // ——— 插件元数据 ———

    const juce::String getName() const override { return "HonestMix"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    // ——— 公共访问 ———

    AppState& getAppState() noexcept { return *appState; }
    HeadphoneDatabase& getHeadphoneDatabase() noexcept { return *headphoneDB; }

    /** 是否已完成配置 */
    bool isConfigured() const;

    /** 重置配置 */
    void resetConfiguration();

private:
    //==============================================================================
    // ——— 核心模块 ———
    std::unique_ptr<AppState>            appState;
    std::unique_ptr<HeadphoneDatabase>   headphoneDB;
    std::unique_ptr<CorrectionProcessor> correctionProcessor;
    std::unique_ptr<WetDryMixer>         wetDryMixer;

    // ——— 参数附件（连接 UI 与参数） ———
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        correctionEnabledAttachment;

    // ——— 状态同步 ———
    bool needsConfigRefresh = true;

    /** 从 APVTS 同步参数到 AppState */
    void syncParametersToState();

    /** 从 AppState 同步到 DSP */
    void syncStateToDSP();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HonestMixAudioProcessor)
};
