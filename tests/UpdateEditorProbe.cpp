#include <JuceHeader.h>
#if HM_UPDATE_PROBE_CONNECT
 #include "Editor.h"
#else
 #include "PluginEditor.h"
#endif
#include <iostream>

// Fresh hidden 014/612 menu/lifetime proof, prepared for Root. No PopupMenu is
// shown, no standalone/device/room/processBlock/browser/network is invoked.
int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    if (argc != 2) return 2;
    const juce::File output(juce::String::fromUTF8(argv[1]));
    if (output.exists() || output.createDirectory().failed()) return 3;
    int assertions = 0, failures = 0;
    double maximumCloseMs = 0;
    auto expect = [&](bool value) { ++assertions; if (!value) ++failures; };
    for (int cycle = 0; cycle < 16; ++cycle) {
       #if HM_UPDATE_PROBE_CONNECT
        hm::Processor processor;
        auto editor = std::make_unique<hm::Editor>(processor);
        constexpr auto expectedVersion = "0.2.14";
        constexpr int updateId = 4, feedbackId = 5;
       #else
        HonestMixAudioProcessor processor;
        auto editor = std::make_unique<HonestMixAudioProcessorEditor>(processor);
        constexpr auto expectedVersion = "0.6.12";
        constexpr int updateId = 20, feedbackId = 21;
       #endif
        editor->getProperties().set("headlessTest", true);
        expect(juce::String(JucePlugin_VersionString) == expectedVersion);
        const auto menu = editor->makeOptionsMenu();
        juce::PopupMenu::MenuItemIterator items(menu, true);
        int updateItems = 0, feedbackItems = 0;
        while (items.next()) {
            const auto& item = items.getItem();
            if (item.itemID == updateId && item.isEnabled && item.text.contains("Check for updates")) ++updateItems;
            if (item.itemID == feedbackId && item.isEnabled && item.text.contains("Report a problem")) ++feedbackItems;
        }
        expect(updateItems == 1); expect(feedbackItems == 1); expect(!editor->isShowing());
       #if HM_UPDATE_PROBE_CONNECT
        const auto endpoint = processor.endpoint();
        editor->openUpdateWebsite(); editor->openFeedbackWebsite();
        expect(processor.endpoint() == endpoint);
       #else
        const auto degree = processor.getTranslationDegree(); const auto mode = processor.getListenMode();
        editor->applyOption(updateId); editor->applyOption(feedbackId);
        expect(processor.getTranslationDegree() == degree && processor.getListenMode() == mode);
       #endif
        if (cycle == 0) {
            for (int scale : {1, 2}) {
                juce::Image image(juce::Image::ARGB, editor->getWidth()*scale, editor->getHeight()*scale, true);
                juce::Graphics graphics(image);
                graphics.addTransform(juce::AffineTransform::scale(static_cast<float>(scale)));
                editor->paintEntireComponent(graphics, true);
                juce::FileOutputStream stream(output.getChildFile("support-editor-" + juce::String(scale) + ".png"));
                expect(stream.openedOk() && stream.setPosition(0) && stream.truncate().wasOk() &&
                       juce::PNGImageFormat().writeImageToStream(image, stream));
            }
        }
        const auto start = juce::Time::getMillisecondCounterHiRes(); editor.reset();
        const auto elapsed = juce::Time::getMillisecondCounterHiRes() - start;
        maximumCloseMs = juce::jmax(maximumCloseMs, elapsed); expect(elapsed < 250.0);
    }
    auto report = new juce::DynamicObject();
    report->setProperty("schema", "honestmix.support-editor-probe.v2");
    report->setProperty("compiledProductVersion", JucePlugin_VersionString);
    report->setProperty("passed", failures == 0); report->setProperty("assertions", assertions);
    report->setProperty("failed", failures); report->setProperty("editorCycles", 16);
    report->setProperty("maximumCloseMs", maximumCloseMs);
    report->setProperty("physicalAudioOpened", false); report->setProperty("browserOpened", false);
    report->setProperty("httpUsed", false); report->setProperty("headlessActionsSuppressed", true);
    report->setProperty("scope", "current hidden editor menus/actions/render/destruction; not live menu clicks, standalone device path, OS DPI, actual browser, email delivery, DAW or network verification");
    const auto json = juce::JSON::toString(juce::var(report), true);
    const auto saved = output.getChildFile("report.json").replaceWithText(json);
    std::cout << json << '\n';
    return failures == 0 && saved ? 0 : 1;
}
