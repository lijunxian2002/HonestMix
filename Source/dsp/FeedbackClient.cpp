/**
 * @file  FeedbackClient.cpp
 * @brief Supabase REST feedback uploader implementation.
 *
 * Reads HONESTMIX_SUPABASE_URL and HONESTMIX_SUPABASE_KEY from
 * environment variables at construction. Falls back to setConfig().
 * Validates HTTP response code — only 2xx is considered success.
 */

#include "dsp/FeedbackClient.h"
#include <cstdlib>

//==============================================================================
FeedbackClient::FeedbackClient()
{
    // Try environment variables first.
    // Set these in your DAW host or shell before launching:
    //   HONESTMIX_SUPABASE_URL=https://xxx.supabase.co/rest/v1/feedback
    //   HONESTMIX_SUPABASE_KEY=eyJhbGciOi...
    if (const char* envUrl = std::getenv ("HONESTMIX_SUPABASE_URL"))
        if (const char* envKey = std::getenv ("HONESTMIX_SUPABASE_KEY"))
            setConfig (juce::String (envUrl), juce::String (envKey));
}

//==============================================================================
void FeedbackClient::setConfig (const juce::String& apiUrl,
                                const juce::String& apiKey) noexcept
{
    url = apiUrl;
    key = apiKey;

    // Basic URL sanity check — must start with https://
    if (! url.startsWith ("https://"))
    {
        url = {};  // Reset — refuse plain-text HTTP for security (TCAI §5.3)
        key = {};
    }
}

//==============================================================================
bool FeedbackClient::sendFeedback (const juce::var& data) noexcept
{
    // ── Guard: must be configured ─────────────────────────────────
    if (! isConfigured())
        return false;

    // ── Serialise ──────────────────────────────────────────────────
    const auto json = juce::JSON::toString (data);
    if (json.isEmpty())
        return false;

    // ── Build request ──────────────────────────────────────────────
    auto target = juce::URL (url).withPOSTData (json);

    auto opts = juce::URL::InputStreamOptions {
        juce::URL::ParameterHandling::inPostData
    }
    .withExtraHeaders (
        "apikey: " + key + "\r\n"
        "Authorization: Bearer " + key + "\r\n"
        "Content-Type: application/json")
    .withConnectionTimeoutMs (5000);

    // ── Send & validate response ──────────────────────────────────
    // JUCE's createInputStream returns null on network failure;
    // for HTTP errors (4xx/5xx) it returns a stream with the error body,
    // so we must also check the status code.
    int statusCode = 0;
    auto optsWithStatus = opts.withStatusCode (&statusCode);

    auto stream = target.createInputStream (optsWithStatus);

    if (stream == nullptr)
        return false;

    // Accept any 2xx response.
    return statusCode >= 200 && statusCode < 300;
}
