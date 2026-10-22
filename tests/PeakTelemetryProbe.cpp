#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

// Same deliberately limited allocation entry-point hooks as ResponseContractTests.
// Count only the actual callback thread; do not infer all allocator entry points.
static thread_local bool countAudioNew = false;
static thread_local std::size_t audioNewCalls = 0;
void* operator new(std::size_t size) {
    if (countAudioNew) ++audioNewCalls;
    if (auto* p = std::malloc(size ? size : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

template<class P, class = void> struct HasAccumulatedPeak : std::false_type {};
template<class P> struct HasAccumulatedPeak<P, std::void_t<
    decltype(std::declval<P&>().consumeOutputPeak()),
    decltype(std::declval<const P&>().getOutputPeakResetGeneration())>> : std::true_type {};
template<class P> static float readAccumulated(P& p) {
    if constexpr (HasAccumulatedPeak<P>::value) return p.consumeOutputPeak();
    else return p.getOutputPeak();
}
template<class P> static std::uint32_t resetGeneration(const P& p) {
    if constexpr (HasAccumulatedPeak<P>::value) return p.getOutputPeakResetGeneration();
    else return 0;
}
static void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
static bool close(double a, double b) { return std::abs(a - b) < 1.0e-7; }
static juce::var object() { return juce::var(new juce::DynamicObject()); }
static void set(juce::var& value, const char* key, const juce::var& field) {
    value.getDynamicObject()->setProperty(key, field);
}
static double nowMs() { return juce::Time::getMillisecondCounterHiRes(); }
static void waitUntil(double target) {
    while (nowMs() < target) juce::Thread::sleep(1);
}
struct AudioDriver {
    juce::AudioBuffer<float> buffer {2, 256};
    juce::MidiBuffer midi;
    std::uint64_t callbacks = 0;
    std::uint64_t measuredCallbacks = 0;
    bool measureAllocation = true;
    double maximumWaveformError = 0;
    float process(HonestMixAudioProcessor& p, float left = 0, float right = 0) {
        buffer.clear();
        buffer.setSample(0, 31, left);
        buffer.setSample(1, 97, right);
        countAudioNew = measureAllocation;
        try { p.processBlock(buffer, midi); }
        catch (...) { countAudioNew = false; throw; }
        countAudioNew = false;
        ++callbacks;
        if (measureAllocation) ++measuredCallbacks;
        float maximum = 0;
        for (int ch = 0; ch != 2; ++ch) {
            for (int i = 0; i != buffer.getNumSamples(); ++i) {
                const auto actual = buffer.getSample(ch, i);
                require(std::isfinite(actual), "Non-finite real output sample");
                const float expected = ch == 0 ? (i == 31 ? left : 0.0f) : (i == 97 ? right : 0.0f);
                maximumWaveformError = std::max(maximumWaveformError, (double)std::abs(actual - expected));
                require(close(actual, expected), "Degree0 float output changed or clipped");
                maximum = std::max(maximum, std::abs(actual));
            }
        }
        require(close(p.getOutputPeak(), maximum), "Instantaneous accessor differs from actual output samples");
        return maximum;
    }
};
static void prepare(HonestMixAudioProcessor& p, AudioDriver& audio) {
    p.setProfileIndex(0);
    p.setTranslationDegree(0);
    p.setListenMode(ListenNormal);
    p.prepareToPlay(44100, 256);
    require(p.getLatencySamples() == 0, "Probe fixture requires actual zero-delay44.1k degree0 output");
    const bool previousMeasure = audio.measureAllocation;
    audio.measureAllocation = false; // Warm-up is separate from declared steady callback cases.
    const auto deadline = nowMs() + 10000;
    try {
        while (nowMs() < deadline) {
            audio.process(p);
            const auto s = p.captureResponseState();
            if (s.activeProfile == 0 && s.isSteady() && close(s.appliedDegree, 0)) {
                audio.measureAllocation = previousMeasure;
                return;
            }
            juce::Thread::sleep(1);
        }
    } catch (...) {
        audio.measureAllocation = previousMeasure;
        throw;
    }
    audio.measureAllocation = previousMeasure;
    throw std::runtime_error("Actual processor failed to settle profile0 degree0");
}
static void append(juce::var& rows, const juce::var& row) { rows.getArray()->add(row); }

template<class P>
static juce::var telemetryCases(P& p, AudioDriver& audio, bool candidate, juce::var& report) {
    auto result = object();
    auto rows = juce::var(juce::Array<juce::var>());
    set(result, "rows", rows);
    set(report, "telemetry", result); // Preserve partial observations on first failure.
    readAccumulated(p);
    const float pulseOutput = audio.process(p, 1.25f, -0.75f);
    const float instantaneousAfterPulse = p.getOutputPeak();
    audio.process(p); // Deliberately no UI/editor refresh between pulse and clean callback.
    const float instantaneousAfterClean = p.getOutputPeak();
    const float firstRead = readAccumulated(p), secondRead = readAccumulated(p);
    auto row = object();
    set(row, "case", "actual1.25PulseThenCleanNoUiPoll");
    set(row, "actualPulseOutputPeak", pulseOutput);
    set(row, "instantaneousAfterPulse", instantaneousAfterPulse);
    set(row, "instantaneousAfterClean", instantaneousAfterClean);
    set(row, "firstRead", firstRead); set(row, "secondRead", secondRead);
    set(row, "baselineLostPeakObserved", !candidate && close(firstRead, 0));
    set(row, "candidatePreservedPeakObserved", candidate && close(firstRead, 1.25));
    append(rows, row);
    require(close(pulseOutput, 1.25), "Declared1.25 pulse did not reach actual float output");
    require(close(instantaneousAfterPulse, 1.25), "Instantaneous pulse peak was not1.25");
    require(close(instantaneousAfterClean, 0), "Instantaneous accessor did not clear after clean output");
    require(close(firstRead, candidate ? 1.25 : 0), "Accumulated peak contract/baseline loss differs");
    require(close(secondRead, 0), "Second accumulated read must be zero");

    for (const float amplitude : {0.5f, 1.0f, -1.0f}) {
        readAccumulated(p);
        const auto actual = audio.process(p, amplitude, -0.25f);
        const auto instant = p.getOutputPeak();
        audio.process(p);
        const auto accumulated = readAccumulated(p);
        require(close(actual, std::max(std::abs(amplitude), 0.25f)), "Subunity/unity pulse output changed");
        require(close(accumulated, candidate ? actual : 0), "Subunity/unity accumulation differs");
        row = object(); set(row, "case", "subunityOrUnityUnchanged");
        set(row, "input", amplitude); set(row, "actualSamplePeak", actual);
        set(row, "instantaneousBeforeClean", instant); set(row, "readAfterClean", accumulated);
        set(row, "aboveUnity", actual > 1); append(rows, row);
    }

    readAccumulated(p);
    audio.process(p, 1.5f, -0.2f); audio.process(p, 1.125f, 0);
    audio.process(p);
    const auto maximumAcrossCallbacks = readAccumulated(p);
    require(close(maximumAcrossCallbacks, candidate ? 1.5 : 0), "Accumulator did not preserve maximum across callbacks");
    row = object(); set(row, "case", "maximumNotLastCallback");
    set(row, "readAfter1_5Then1_125ThenClean", maximumAcrossCallbacks); append(rows, row);
    require(close(readAccumulated(p), 0), "Maximum read did not consume value");

    auto oldGeneration = resetGeneration(p);
    audio.process(p, 1.25f);
    p.prepareToPlay(44100, 256);
    require(close(p.getOutputPeak(), 0) && close(readAccumulated(p), 0), "prepareToPlay retained a stale peak");
    if (candidate) require(resetGeneration(p) != oldGeneration, "prepareToPlay did not advance reset generation");
    row = object(); set(row, "case", "prepareClearsPendingAndInstantaneous");
    set(row, "generationBefore", (juce::int64)oldGeneration);
    set(row, "generationAfter", (juce::int64)resetGeneration(p)); append(rows, row);
    prepare(p, audio);
    readAccumulated(p);
    oldGeneration = resetGeneration(p);
    audio.process(p, 1.25f);
    p.releaseResources();
    require(close(p.getOutputPeak(), 0) && close(readAccumulated(p), 0), "releaseResources retained a stale peak");
    if (candidate) require(resetGeneration(p) != oldGeneration, "releaseResources did not advance reset generation");
    row = object(); set(row, "case", "releaseClearsPendingAndInstantaneous");
    set(row, "generationBefore", (juce::int64)oldGeneration);
    set(row, "generationAfter", (juce::int64)resetGeneration(p)); append(rows, row);
    set(result, "rows", rows);
    set(result, "uiRefreshesBetweenDeclaredCallbacks", 0);
    return result;
}

struct FramePair { juce::Image one, two; };
static FramePair capture(HonestMixAudioProcessorEditor& editor, const juce::File& out, const juce::String& name) {
    FramePair result;
    for (const int scale : {1, 2}) {
        juce::Rectangle<int> painted;
        auto image = editor.renderSurface(scale, painted);
        require(image.isValid() && image.getWidth() == 1000 * scale && image.getHeight() == 480 * scale, "Unexpected real editor image size");
        require(painted == juce::Rectangle<int>(0, 0, 1000 * scale, 480 * scale), "Unexpected real editor painted bounds");
        const auto file = out.getChildFile(name + "-" + juce::String(scale * 100) + "pct.png");
        require(!file.exists(), "Preserve first snapshot");
        auto stream = file.createOutputStream();
        require(stream != nullptr && juce::PNGImageFormat().writeImageToStream(image, *stream), "Cannot save actual editor image");
        if (scale == 1) result.one = image; else result.two = image;
    }
    return result;
}
static std::uint64_t changedPixels(const juce::Image& first, const juce::Image& second, int scale, bool insideFooter) {
    require(first.getBounds() == second.getBounds(), "Image dimensions differ");
    const juce::Rectangle<int> footer(738 * scale, 442 * scale, 160 * scale, 20 * scale);
    std::uint64_t changed = 0;
    for (int y = 0; y < first.getHeight(); ++y)
        for (int x = 0; x < first.getWidth(); ++x)
            if (footer.contains(x, y) == insideFooter && first.getPixelAt(x, y) != second.getPixelAt(x, y)) ++changed;
    return changed;
}
static juce::var compare(const FramePair& blank, const FramePair& image, const char* name, bool expectWarning, bool requireOutsideSame) {
    auto row = object(); set(row, "stage", name); set(row, "expectedWarning", expectWarning);
    for (int scale : {1, 2}) {
        const auto& a = scale == 1 ? blank.one : blank.two;
        const auto& b = scale == 1 ? image.one : image.two;
        const auto footer = changedPixels(a, b, scale, true);
        const auto outside = changedPixels(a, b, scale, false);
        set(row, scale == 1 ? "footerChangedPixels100" : "footerChangedPixels200", (juce::int64)footer);
        set(row, scale == 1 ? "outsideFooterChangedPixels100" : "outsideFooterChangedPixels200", (juce::int64)outside);
        require(expectWarning ? footer > 10 : footer == 0, "Actual footer warning visibility differs from declared stage");
        if (requireOutsideSame) require(outside == 0, "Telemetry modified pixels outside footer while DSP/graph state stayed fixed");
    }
    return row;
}
static void readyEditor(HonestMixAudioProcessorEditor& editor) {
    editor.getProperties().set("headlessTest", true);
    const auto deadline = nowMs() + 10000;
    while (nowMs() < deadline) {
        editor.refreshFromProcessor();
        if (editor.predictionReady()) return;
        juce::Thread::sleep(1);
    }
    throw std::runtime_error("Real editor prediction did not settle for deterministic image comparison");
}
template<class P>
static juce::var uiCases(P& p, AudioDriver& audio, bool candidate, const juce::File& out, juce::var& report) {
    prepare(p, audio); readAccumulated(p);
    auto editor = std::make_unique<HonestMixAudioProcessorEditor>(p);
    require(!editor->isShowing(), "Probe editor must stay off desktop; no native audio/window run");
    editor->applyOption(10); // Existing reduce-motion option, never a new product test getter.
    readyEditor(*editor);
    auto blank = capture(*editor, out, "00-clean");
    auto rows = juce::var(juce::Array<juce::var>());
    auto result = object(); set(result, "rows", rows); set(report, "ui", result);

    // Positive control proves footer pixel detector sees the existing/baseline
    // instantaneous PEAK and candidate OUT PK when actual output is currently high.
    audio.process(p, 1.25f);
    editor->refreshFromProcessor();
    append(rows, compare(blank, capture(*editor, out, "01-positive-control"), "positiveControl", true, true));
    audio.process(p); p.releaseResources(); prepare(p, audio); editor->refreshFromProcessor(); readyEditor(*editor);
    append(rows, compare(blank, capture(*editor, out, "02-reset-clean"), "resetAfterPositiveControl", false, true));

    audio.process(p, 1.25f, -0.75f); audio.process(p);
    require(close(p.getOutputPeak(), 0), "Clean callback required before no-poll UI capture");
    append(rows, compare(blank, capture(*editor, out, "03-after-callbacks-before-ui-poll"), "afterCallbacksBeforeUiPoll", false, true));
    editor->refreshFromProcessor();
    const double firstPollAt = nowMs();
    auto polled = capture(*editor, out, "04-after-first-ui-poll");
    append(rows, compare(blank, polled, "afterFirstUiPoll", candidate, true));

    waitUntil(firstPollAt + 500);
    const auto elapsed500 = nowMs() - firstPollAt;
    require(elapsed500 < 1400, "Environment delayed500ms stage beyond the intended hold window");
    // A smaller new peak must not prolong/reduce the already-held1.25 warning.
    audio.process(p, 1.125f); audio.process(p);
    editor->refreshFromProcessor();
    auto at500 = capture(*editor, out, "05-at500ms-after-smaller-pulse");
    auto row500 = compare(blank, at500, "at500msAfterSmallerPulse", candidate, true);
    set(row500, "actualElapsedMs", elapsed500); append(rows, row500);
    if (candidate) {
        require(changedPixels(polled.one, at500.one, 1, true) == 0
             && changedPixels(polled.two, at500.two, 2, true) == 0, "Smaller peak changed larger held readout");
    }

    waitUntil(firstPollAt + 1600);
    const auto elapsed1600 = nowMs() - firstPollAt;
    editor->refreshFromProcessor();
    auto row1600 = compare(blank, capture(*editor, out, "06-at1600ms"), "at1600msSmallerDidNotExtend", false, true);
    set(row1600, "actualElapsedMs", elapsed1600); append(rows, row1600);

    for (const float value : {0.5f, 1.0f, -1.0f}) {
        audio.process(p, value); editor->refreshFromProcessor();
        append(rows, compare(blank, capture(*editor, out, "07-no-warning-" + juce::String(value, 1)), "unityOrBelowNoWarning", false, true));
        audio.process(p); editor->refreshFromProcessor();
    }

    audio.process(p, 1.25f); audio.process(p); editor->refreshFromProcessor();
    p.prepareToPlay(44100, 256); editor->refreshFromProcessor();
    append(rows, compare(blank, capture(*editor, out, "08-prepare-clears-hold"), "prepareClearsUiHold", false, false));
    prepare(p, audio); editor->refreshFromProcessor(); readyEditor(*editor);
    audio.process(p, 1.25f); audio.process(p); editor->refreshFromProcessor();
    p.releaseResources(); editor->refreshFromProcessor();
    // AUDIO INACTIVE changes the footer status text and removes the plot; only
    // compare the dedicated warning rectangle for this lifecycle stage.
    append(rows, compare(blank, capture(*editor, out, "09-release-clears-hold"), "releaseClearsUiHold", false, false));
    prepare(p, audio); editor->refreshFromProcessor(); readyEditor(*editor);
    audio.process(p, 1.25f); audio.process(p); editor->refreshFromProcessor();
    editor.reset();
    audio.process(p, 1.5f); audio.process(p); // Pending event occurred while no editor existed.
    editor = std::make_unique<HonestMixAudioProcessorEditor>(p);
    require(!editor->isShowing(), "Reopened editor must stay off desktop");
    editor->applyOption(10); readyEditor(*editor);
    append(rows, compare(blank, capture(*editor, out, "10-reopen-discards-closed-editor-events"), "reopenDiscardsStalePendingPeak", false, true));
    require(close(readAccumulated(p), 0), "New editor did not consume/discard closed-editor pending peaks");
    set(result, "rows", rows);
    set(result, "scalesPercent", juce::var(juce::Array<juce::var>{100, 200}));
    set(result, "usesNewProductUiTelemetryTestGetters", false);
    set(result, "windowShownOrAudioDeviceOpened", false);
    set(result, "timeMethod", "Real monotonic wallclock; no message pumping/animation advancement. Hidden editor timers do not refresh. Explicit processor polling and real rendered footer pixel comparisons.");
    return result;
}

int main(int argc, char** argv) {
    juce::var report = object();
    juce::File output;
    bool outputCreated = false;
    try {
        require(argc == 3 || argc == 4, "Usage: PeakTelemetryProbe.exe baseline|candidate NEW_OUTPUT_DIRECTORY [--telemetry-only]");
        const bool telemetryOnly = argc == 4;
        require(!telemetryOnly || juce::String::fromUTF8(argv[3]) == "--telemetry-only", "Unknown runtime option");
        const juce::String mode = juce::String::fromUTF8(argv[1]);
        require(mode == "baseline" || mode == "candidate", "Mode must be baseline or candidate");
        const bool candidate = mode == "candidate";
        output = juce::File(juce::String::fromUTF8(argv[2]));
        require(!output.exists(), "Output directory must be new; preserve first run/failures");
        require(output.createDirectory().wasOk(), "Cannot create isolated output directory");
        outputCreated = true;
        set(report, "schema", "honestmix.peak-telemetry-probe/1");
        set(report, "mode", mode); set(report, "version", JucePlugin_VersionString);
        set(report, "hasAccumulatedAccessorAndGeneration", HasAccumulatedPeak<HonestMixAudioProcessor>::value);
        set(report, "hardwareAudioOpened", false); set(report, "realDawLaunched", false);
        set(report, "networkRequested", false); set(report, "sampleRate", 44100);
        set(report, "scope", telemetryOnly
            ? "Actual chosen SharedCode synthetic degree0 float processing only; UI NOT run. Baseline success means defect reproduction, not candidate acceptance. No device/host/network/SPL/true-peak claim."
            : "Actual chosen SharedCode synthetic degree0 float processing and actual hidden editor100/200 rendering. Baseline success means defect reproduction, not candidate acceptance. No device/host/network/SPL/true-peak claim.");
        require(candidate == HasAccumulatedPeak<HonestMixAudioProcessor>::value, "Runtime mode and actually-linked processor API disagree");
        juce::ScopedJuceInitialiser_GUI init;
        HonestMixAudioProcessor p;
        AudioDriver audio;
        prepare(p, audio);
        set(report, "telemetry", telemetryCases(p, audio, candidate, report));
        set(report, "uiExecuted", !telemetryOnly);
        if (!telemetryOnly) set(report, "ui", uiCases(p, audio, candidate, output, report));
        p.releaseResources();
        set(report, "actualAudioCallbacks", (juce::int64)audio.callbacks);
        set(report, "allocationMeasuredSteadyCallbacks", (juce::int64)audio.measuredCallbacks);
        set(report, "maximumAbsoluteSampleError", audio.maximumWaveformError);
        set(report, "audioCallbackCppNew", (juce::int64)audioNewCalls);
        set(report, "allocationScope", "Only overridden scalar/array C++new entry points on the callback thread; not all allocator paths.");
        require(audioNewCalls == 0, "Actual measured processBlock allocated through C++new");
        set(report, "baselineDefectReproduced", !candidate);
        set(report, "candidateTelemetryPassed", candidate);
        set(report, "candidateUiAcceptancePassed", candidate && !telemetryOnly);
        set(report, "passed", true);
        require(output.getChildFile("report.json").replaceWithText(juce::JSON::toString(report)), "Cannot write actual report");
        std::cout << (candidate ? (telemetryOnly ? "Candidate telemetry passed; UI NOT run" : "Candidate telemetry/UI acceptance passed") : "Baseline missed-peak defect reproduced") << std::endl;
        return 0;
    } catch (const std::exception& error) {
        countAudioNew = false;
        set(report, "passed", false); set(report, "firstFailure", error.what());
        set(report, "audioCallbackCppNewAtFailure", (juce::int64)audioNewCalls);
        if (outputCreated) output.getChildFile("report.json").replaceWithText(juce::JSON::toString(report));
        std::cerr << "FIRST FAILURE: " << error.what() << std::endl;
        return 1;
    }
}
