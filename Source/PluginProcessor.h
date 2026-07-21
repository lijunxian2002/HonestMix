#pragma once

//==============================================================================
/**
 * HonestMix BETA2 音频处理器 —— 模块装配点
 *
 * 信号链（数据中心原样，抽为模块后顺序不变）：
 *   输入 → WetDryMixer::captureDry（预存干信号）
 *        → CorrectionEngine（FIR 卷积，始终 ON）
 *        → WetDryMixer::mixInPlace（翻译度 0~200 混合）
 *        → 监听模式模拟（Mono / iPhone / Car）
 *
 * 持有：HeadphoneDatabase（数据）、CorrectionEngine、WetDryMixer、
 *       AppState（持久化）。UI 只通过本类的窄接口操作，不碰内部模块。
 */
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Core/AppState.h"
#include "Core/HeadphoneDatabase.h"
#include "DSP/CorrectionEngine.h"
#include "DSP/WetDryMixer.h"

/** 监听模式（矫正结束后额外模拟）—— 与数据中心枚举值保持一致 */
enum ListenMode { ListenNormal = 0, ListenMono, ListeniPhone, ListenCar };

class HonestMixAudioProcessor final : public juce::AudioProcessor
{
public:
    HonestMixAudioProcessor();
    ~HonestMixAudioProcessor() override = default;

    // ── AudioProcessor 接口 ─────────────────────────────────
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "BETA2"; }

    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override     { return 1; }
    int getCurrentProgram() override  { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ── UI 窄接口（UI 线程）──────────────────────────────────

    /** 翻译度 0~200：写混合器（音频线程可见）+ 落盘状态 */
    void  setTranslationDegree (float v);
    float getTranslationDegree() const noexcept { return mixer_.getTranslation(); }

    /** 换耳机：引擎切 FIR + 状态记录全名（调用方负责 suspendProcessing 包裹） */
    void setProfileIndex (int idx);
    void setProfileByName (const juce::String& name);
    juce::String getProfileName() const { return appState_.getProfileName(); }

    /** 监听模式模拟 */
    void setListenMode (ListenMode m);
    ListenMode getListenMode() const noexcept { return (ListenMode) listenMode_.load(); }

    bool isOnboarded() const noexcept { return appState_.isOnboarded(); }
    void setOnboarded()               { appState_.setOnboarded (true); }

    float getHostBpm() const noexcept { return hostBpm_.load(); }

    // ── 模块访问（UI 只读使用）───────────────────────────────
    CorrectionEngine&        getCorrectionEngine() noexcept { return correctionEngine_; }
    const HeadphoneDatabase& getDatabase() const noexcept   { return headphoneDB_; }
    AppState&                getAppState() noexcept         { return appState_; }

private:
    void updateSimulationFilters();

    // 注意声明顺序：数据库必须先于引擎构造（引擎持有其引用）
    HeadphoneDatabase headphoneDB_;
    CorrectionEngine  correctionEngine_ { headphoneDB_ };
    WetDryMixer       mixer_;
    AppState          appState_;

    std::atomic<float> hostBpm_ { 0.0f };
    std::atomic<int>   listenMode_ { ListenNormal };

    // 监听模式模拟滤波
    double sampleRate_ = 44100.0;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> filterL_, filterR_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessor)
};
