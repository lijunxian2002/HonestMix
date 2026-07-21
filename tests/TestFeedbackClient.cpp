/**
 * @file  TestFeedbackClient.cpp
 * @brief Unit tests for the Supabase feedback uploader.
 *
 * Tests are offline — they verify configuration logic, not HTTP.
 */

#include <juce_core/juce_core.h>
#include "dsp/FeedbackClient.h"

//==============================================================================
class FeedbackClientTest final : public juce::UnitTest
{
public:
    FeedbackClientTest()
        : juce::UnitTest ("FeedbackClient", "HonestMix")
    {}

    void runTest() override
    {
        // ── Configuration ───────────────────────────────────────
        beginTest ("unconfigured returns false");

        FeedbackClient client;
        expect (! client.isConfigured());

        juce::var dummyData (new juce::DynamicObject());
        expect (! client.sendFeedback (dummyData));

        // ── Valid config ────────────────────────────────────────
        beginTest ("valid https config");

        client.setConfig ("https://mock.supabase.co/rest/v1/feedback",
                          "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9");
        expect (client.isConfigured());

        // ── Reject plaintext HTTP (TCAI §5.3) ───────────────────
        beginTest ("reject http://");

        client.setConfig ("http://insecure.supabase.co/rest/v1/feedback",
                          "some-key");
        expect (! client.isConfigured());

        // ── Data serialisation ──────────────────────────────────
        beginTest ("data serialisation");
        client.setConfig ("https://mock.supabase.co/rest/v1/feedback",
                          "test-key");

        auto* obj = new juce::DynamicObject();
        obj->setProperty ("headphone", juce::var ("ATH-M50X"));
        obj->setProperty ("bass_fb",   juce::var ("okay"));
        obj->setProperty ("treble_fb", juce::var ("okay"));
        obj->setProperty ("drywet",    50.0);

        juce::var data (obj);
        auto json = juce::JSON::toString (data);
        expect (json.isNotEmpty());
        expect (json.contains ("ATH-M50X"));
    }
};

static FeedbackClientTest feedbackClientTest;
