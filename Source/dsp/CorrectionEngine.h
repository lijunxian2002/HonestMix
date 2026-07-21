#pragma once

//==============================================================================
/**
 * FIR 校正引擎 —— 数据中心原版 + 数据库注入
 *
 * 拼接改动（相对数据中心 CorrectionEngine）：
 *   1. 不再直接包含 fir_data.h，改由构造注入 HeadphoneDatabase
 *      → kProfiles 引用点从 3 个文件收口到 1 个
 *   2. 剔除无人调用的 getProfileList() 死代码
 * 行为与数据中心逐样本一致（juce::dsp::Convolution，1024 taps，Normalise::yes）。
 *
 * 线程：prepare/process 在音频线程；setProfile 可在 UI 线程调用，
 *   调用方（PluginProcessor 的封装）负责 suspendProcessing 包裹。
 */
#include <juce_dsp/juce_dsp.h>
#include "Core/HeadphoneDatabase.h"

class CorrectionEngine
{
public:
    explicit CorrectionEngine (const HeadphoneDatabase& db) : db_ (db) {}
    ~CorrectionEngine() = default;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void process (juce::AudioBuffer<float>& buffer);

    void setEnabled (bool enabled) noexcept { enabled_ = enabled; }
    bool isEnabled() const noexcept         { return enabled_; }

    void setProfile (int index);
    void setProfileByName (const juce::String& name);
    int  getCurrentProfile() const noexcept { return currentProfile_; }

    int         getNumProfiles() const noexcept    { return db_.getNumProfiles(); }
    const char* getProfileName (int index) const   { return db_.getProfileName (index); }

    int getLatencySamples() const noexcept { return HeadphoneDatabase::kFirLength; }

private:
    void loadProfile();

    const HeadphoneDatabase& db_;
    juce::dsp::Convolution convolution_;
    juce::dsp::ProcessSpec currentSpec_ = {};
    bool enabled_ = false, prepared_ = false;
    int currentProfile_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CorrectionEngine)
};
