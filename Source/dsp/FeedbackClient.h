#pragma once
/**
 * @file  FeedbackClient.h
 * @brief Supabase REST feedback uploader.
 *
 * Sends user ear-training feedback to a Supabase `feedback` table.
 * Configured via HONESTMIX_SUPABASE_URL and HONESTMIX_SUPABASE_KEY
 * environment variables at construction time.
 *
 * Schema (auto-created by supabase_schema.sql):
 *   headphone  TEXT     — headphone model name
 *   interface  TEXT     — audio-interface name
 *   drywet     FLOAT4   — current dry/wet ratio
 *   correction BOOL     — correction enabled
 *   bass_fb    TEXT     — "okay" | "too_much" | "too_little"
 *   treble_fb  TEXT     — "okay" | "too_bright" | "too_dark"
 *
 * @par Thread safety
 *     sendFeedback() can be called from any thread.
 */

#include <juce_core/juce_core.h>

//==============================================================================
class FeedbackClient final
{
public:
    FeedbackClient();

    //==============================================================================
    /// @name Configuration
    ///@{
    /** Set Supabase project credentials (overrides env vars). */
    void setConfig (const juce::String& apiUrl, const juce::String& apiKey) noexcept;

    bool isConfigured() const noexcept { return url.isNotEmpty() && key.isNotEmpty(); }
    ///@}

    //==============================================================================
    /// @name Sending
    ///@{
    /** POST feedback as JSON to Supabase.
     *  @return true if server returned HTTP 2xx. */
    bool sendFeedback (const juce::var& data) noexcept;
    ///@}

private:
    juce::String url;
    juce::String key;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackClient)
};
