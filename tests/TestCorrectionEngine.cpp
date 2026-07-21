/**
 * @file  TestCorrectionEngine.cpp
 * @brief Unit tests for the FIR convolution engine.
 *
 * Run with:
 *   ./HonestMix_Tests
 *
 * Uses JUCE's built-in unit-test framework (JUCE_UNIT_TESTS=1).
 */

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp/CorrectionEngine.h"

//==============================================================================
class CorrectionEngineTest final : public juce::UnitTest
{
public:
    CorrectionEngineTest()
        : juce::UnitTest ("CorrectionEngine", "HonestMix")
    {}

    void runTest() override
    {
        // ── Profile management ──────────────────────────────────
        beginTest ("profile bounds");

        CorrectionEngine engine;
        expect (engine.getNumProfiles() == 4);
        expect (engine.getCurrentProfile() == 0);
        expectEquals (juce::String (engine.getProfileName (0)), juce::String ("ATH-M50X"));
        expectEquals (juce::String (engine.getProfileName (1)), juce::String ("DT 770 Pro"));
        expectEquals (juce::String (engine.getProfileName (2)), juce::String ("HD 600"));
        expectEquals (juce::String (engine.getProfileName (3)), juce::String ("DT 990 Pro"));
        expectEquals (juce::String (engine.getProfileName (-1)), juce::String ("Unknown"));
        expectEquals (juce::String (engine.getProfileName (99)), juce::String ("Unknown"));

        // ── Profile switching ───────────────────────────────────
        beginTest ("profile switching");

        engine.setProfile (2);
        expect (engine.getCurrentProfile() == 2);

        engine.setProfile (-1);  // out-of-range → wrap to 0
        expect (engine.getCurrentProfile() == 0);

        // ── Prepare + process (silent input → silent output) ────
        beginTest ("prepare & process");

        juce::dsp::ProcessSpec spec;
        spec.sampleRate       = 44100.0;
        spec.maximumBlockSize = 512;
        spec.numChannels      = 2;

        engine.prepare (spec);

        juce::AudioBuffer<float> buffer (2, 512);
        buffer.clear();

        // With correction off, buffer must remain unchanged.
        engine.setEnabled (false);
        engine.process (buffer);
        float sum = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 512; ++s)
                sum += std::abs (buffer.getSample (ch, s));
        expect (sum == 0.0f);

        // With correction on, output may differ but no crash.
        engine.setEnabled (true);
        engine.process (buffer);  // should not crash or assert
        expect (! std::isnan (buffer.getSample (0, 0)));

        // ── Profile reset ───────────────────────────────────────
        beginTest ("reset");

        engine.reset();  // should re-load current profile
        engine.process (buffer);
        expect (! std::isnan (buffer.getSample (0, 0)));
    }
};

static CorrectionEngineTest correctionEngineTest;
