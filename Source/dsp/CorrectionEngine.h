#pragma once

#include <juce_dsp/juce_dsp.h>

struct HeadphoneProfile
{
    const char* name;
    const float* firData;
};

class CorrectionEngine
{
public:
    CorrectionEngine() = default;
    ~CorrectionEngine() = default;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void process (juce::AudioBuffer<float>& buffer);

    void setEnabled (bool enabled) noexcept { enabled_ = enabled; }
    bool isEnabled() const noexcept         { return enabled_; }

    void setProfile (int index);
    int  getCurrentProfile() const noexcept { return currentProfile_; }
    int  getNumProfiles() const noexcept    { return 3; }
    const char* getProfileName (int index) const;

    int getLatencySamples() const noexcept { return 1024; }

private:
    void loadProfile();
    juce::dsp::Convolution convolution_;
    juce::dsp::ProcessSpec currentSpec_ = {};
    bool enabled_ = false, prepared_ = false;
    int currentProfile_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CorrectionEngine)
};
