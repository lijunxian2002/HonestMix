#pragma once
/**
 * @file  CorrectionEngine.h
 * @brief Multi-headphone FIR convolution engine targeting Harman 2018 OE.
 *
 * HonestMix — 诚听
 * Licensed under GPL-3.0.
 *
 * Supports hot-swappable FIR profiles at runtime.
 * All public methods are safe to call from the real-time audio thread
 * once prepare() has been called.
 */

#include <juce_dsp/juce_dsp.h>

/// One headphone-correction profile.
struct HeadphoneProfile final
{
    const char* name;
    const float* firData; ///< 1024-tap FIR coefficients
};

//==============================================================================
/**
 * FIR convolution engine with runtime profile switching.
 *
 * @par Audio thread safety
 *     - setEnabled(), setProfile(), process() are real-time safe.
 *     - prepare() and reset() must be called from the message thread.
 */
class CorrectionEngine final
{
public:
    CorrectionEngine() = default;
    ~CorrectionEngine() = default;

    //==============================================================================
    /// @name Audio processor callbacks
    ///@{
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void process (juce::AudioBuffer<float>& buffer) noexcept;
    ///@}

    //==============================================================================
    /// @name Runtime control
    ///@{
    void setEnabled (bool enabled) noexcept   { enabled_ = enabled; }
    bool isEnabled() const noexcept           { return enabled_; }

    /** Switch to a different headphone profile (0-based).
     *  Out-of-range indices wrap to 0. */
    void setProfile (int index) noexcept;

    int  getCurrentProfile() const noexcept   { return currentProfile_; }
    int  getNumProfiles() const noexcept      { return 4; }

    /** Human-readable profile name. Returns "Unknown" for invalid indices. */
    const char* getProfileName (int index) const noexcept;

    int  getLatencySamples() const noexcept   { return 1024; }
    ///@}

private:
    void loadProfile();

    juce::dsp::Convolution convolution_;
    juce::dsp::ProcessSpec currentSpec_ = {};

    bool enabled_   = false;
    bool prepared_  = false;
    int  currentProfile_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CorrectionEngine)
};
