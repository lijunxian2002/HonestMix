#pragma once

//==============================================================================
/**
 * HONESTMIX Headphone 音频处理器
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
#include <mutex>

/** 监听模式（矫正结束后额外模拟）—— 与数据中心枚举值保持一致 */
enum ListenMode { ListenNormal = 0, ListenMono, ListeniPhone, ListenCar };

class HonestMixAudioProcessor final : public juce::AudioProcessor
{
public:
    HonestMixAudioProcessor();
    ~HonestMixAudioProcessor() override;

    // ── AudioProcessor 接口 ─────────────────────────────────
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "HONESTMIX Headphone"; }
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return correctionEngine_.getTailLengthSeconds(); }

    int getNumPrograms() override     { return 1; }
    int getCurrentProgram() override  { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ── UI 窄接口（UI 线程）──────────────────────────────────

    /** 翻译度 0~200：通知宿主；音频回调逐块读取 APVTS 原子值。 */
    void  setTranslationDegree (float v);
    float getTranslationDegree() const noexcept;
    void beginTranslationGesture();
    void endTranslationGesture();
    CorrectionResponseSnapshot captureResponseState()const;

    /** 换耳机：通知宿主；后台准备 FIR，通过无锁队列交给音频线程。 */
    void setProfileIndex (int idx);
    void retryProfileLoad()noexcept {correctionEngine_.retryProfileLoad();}
    void setProfileByName (const juce::String& name);
    juce::String getProfileName() const { return headphoneDB_.getProfileName((int)profileParameter_->load()); }

    /** 监听模式模拟 */
    void setListenMode (ListenMode m);
    ListenMode getListenMode() const noexcept { return (ListenMode)(int)listenParameter_->load(); }
    float getOutputPeak() const noexcept { return outputPeak_.load(); }
    // One UI consumer: preserve maxima across callbacks until the next UI poll.
    float consumeOutputPeak() noexcept { return outputPeakSinceRead_.exchange(0.0f,std::memory_order_relaxed); }
    std::uint32_t getOutputPeakResetGeneration() const noexcept { return outputPeakResetGeneration_.load(std::memory_order_relaxed); }
    bool isProfileLoading() const noexcept { return correctionEngine_.isLoading(); }
    bool isAudioReady() const noexcept { return prepared_.load(); }

    bool isOnboarded() const noexcept { return appState_.isOnboarded(); }
    void setOnboarded()               { appState_.setOnboarded (true); }

    struct TempoPreferences { juce::String manualText{"120"}; int source=-1, feel=0; std::uint64_t generation=0; };
    TempoPreferences getTempoPreferences() const;
    TempoPreferences setTempoPreferences(const TempoPreferences&);
    double getHostBpm() const noexcept;
    HostBpmSnapshot getHostBpmSnapshot(double maximumAgeMs=1000.0)const noexcept;

    // ── 模块访问（UI 只读使用）───────────────────────────────
    CorrectionEngine&        getCorrectionEngine() noexcept { return correctionEngine_; }
    const HeadphoneDatabase& getDatabase() const noexcept   { return headphoneDB_; }
    AppState&                getAppState() noexcept         { return appState_; }

private:
    void updateSimulationFilters();
    float processChunk(juce::AudioBuffer<float>&,bool bypassed);
    void processAudio(juce::AudioBuffer<float>&,bool bypassed);
    void publishResponseSnapshot(bool bypassed)noexcept;
    void publishHostBpm(double bpm,bool valid,double timestamp)noexcept;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(int profiles);

    // 注意声明顺序：数据库必须先于引擎构造（引擎持有其引用）
    HeadphoneDatabase headphoneDB_;
    CorrectionEngine  correctionEngine_ { headphoneDB_ };
    WetDryMixer       mixer_;
    AppState          appState_;
    juce::AudioProcessorValueTreeState apvts_;

    std::atomic<std::uint64_t> bpmSequence_{0};
    std::atomic<double> hostBpm_{0},hostBpmTimestamp_{0};
    std::atomic<bool> hostBpmValid_{false};
    std::atomic<float> outputPeak_{0};
    static_assert(std::atomic<float>::is_always_lock_free,"Output peak telemetry requires lock-free float atomics");
    std::atomic<float> outputPeakSinceRead_{0};
    std::atomic<std::uint32_t> outputPeakResetGeneration_{0};
    std::atomic<float>* translationParameter_{};
    std::atomic<float>* profileParameter_{};
    std::atomic<float>* listenParameter_{};

    // 监听模式模拟滤波
    double sampleRate_ = 44100.0;
    struct FilterState { float z1=0,z2=0; };
    std::array<std::array<FilterState,2>,4> simulationState_{};
    std::array<std::array<float,5>,4> simulationCoefficients_{};
    std::array<juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear>,4> listenWeights_{};
    int previousListenMode_=ListenNormal,maximumBlock_=1;
    std::atomic<bool> prepared_{false};
    mutable std::mutex stateMutex_;
    TempoPreferences tempoPreferences_; // Non-audio UI preferences, guarded by stateMutex_.
    std::atomic<bool> translationGesture_{false};
    struct ResponsePublication {
        std::atomic<std::uint64_t> sequence{0},active{0},incoming{0};
        std::atomic<double> degree{100},blend{0};
        std::atomic<int> phase{(int)ProfilePhase::Unprepared};
        std::array<std::atomic<float>,4> modes{};
        std::atomic<bool> degreeTransition{false},modeTransition{false},ready{false},bypassed{false};
    } responsePublication_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessor)
};
