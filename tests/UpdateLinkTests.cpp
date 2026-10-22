#include <JuceHeader.h>
#include "Updates/UpdateLink.h"
#include <iostream>

// Fresh 014/612 contract source. All openers are fakes: no real browser/network.
int main() {
    int assertions = 0, failed = 0;
    auto expect = [&](bool ok, const juce::String& label) {
        ++assertions;
        if (!ok) { ++failed; std::cerr << label << '\n'; }
    };
    using namespace hm::updates;
    for (const auto* version : {"0.0.0", "0.2.14", "0.6.12", "1.10.2", "65535.65535.65535"})
        expect(canonicalVersion(version), "canonical version accepted");
    for (const auto* version : {"", "1", "1.2", "1.2.3.4", ".1.2", "1.2.", "1..2", "1.2.3.",
         "01.2.3", "1.02.3", "1.2.03", "65536.0.0", "1.2.-3", "+1.2.3", "v1.2.3", "1.2.3-beta",
         "1.2.3+meta", " 1.2.3", "1.2.3\n", "1.2.3&token=private", "1.2.3%0a", "1/2/3"}) {
        expect(!canonicalVersion(version), "noncanonical version rejected");
        expect(makeFeedbackLink("connect", "windows-x64", version).isEmpty(), "bad version cannot enter feedback URL");
    }
    const juce::String origin(websiteOrigin);
    expect(makeWebsiteLink("connect", "windows-x64", "0.2.14") == origin +
        "/index.html?product=connect&platform=windows-x64&version=0.2.14&channel=preview#downloads", "existing update URL retained");
    expect(makeWebsiteLink("connect", "macos-arm64", "0.2.14", "stable") == origin +
        "/index.html?product=connect&platform=macos-arm64&version=0.2.14&channel=stable#downloads", "fixed Mac update URL retained");
    expect(makeFeedbackLink("connect", "windows-x64", "0.2.14") == origin +
        "/feedback.html?product=connect&platform=windows-x64&version=0.2.14", "exact Windows feedback URL");
    expect(makeFeedbackLink("connect", "macos-arm64", "0.2.14") == origin +
        "/feedback.html?product=connect&platform=macos-arm64&version=0.2.14", "exact Mac feedback URL");
    expect(makeFeedbackLink("headphone", "windows-x64", "0.6.12") == origin +
        "/feedback.html?product=headphone&platform=windows-x64&version=0.6.12", "exact Headphone feedback URL");
    expect(makeFeedbackLink("headphone", "macos-arm64", "0.6.12").isEmpty(), "unimplemented Headphone Mac context rejected");
    for (const auto* product : {"", "CONNECT", "site", "remote", "connect&url=https://evil.invalid", "../connect", "https://evil.invalid"})
        expect(makeFeedbackLink(product, "windows-x64", "0.2.14").isEmpty(), "bad feedback product rejected");
    for (const auto* platform : {"", "x64", "windows-x86", "windows-arm64", "macos-intel", "windows-x64#evil", "../windows-x64"})
        expect(makeFeedbackLink("connect", platform, "0.2.14").isEmpty(), "bad feedback platform rejected");
    for (const auto* channel : {"", "release", "PREVIEW", "preview?evil", "stable#evil"})
        expect(makeWebsiteLink("connect", "windows-x64", "0.2.14", channel).isEmpty(), "bad update channel rejected");

    int feedbackLaunches = 0, updateLaunches = 0;
    const auto feedbackRecorder = [&](const juce::String& url) {
        ++feedbackLaunches;
        return url == makeFeedbackLink("connect", currentPlatform(), "0.2.14") &&
               !url.contains("channel=") && !url.contains("room=") && !url.contains("token=");
    };
    const auto updateRecorder = [&](const juce::String& url) {
        ++updateLaunches;
        return url == makeWebsiteLink("connect", currentPlatform(), "0.2.14");
    };
    expect(!openFeedbackWebsite("connect", "0.2.14&bad=1", feedbackRecorder) && feedbackLaunches == 0, "invalid feedback version never launches");
    expect(!openFeedbackWebsite("malicious", "0.2.14", feedbackRecorder) && feedbackLaunches == 0, "invalid feedback product never launches");
    if (currentPlatform().isNotEmpty()) {
        expect(openFeedbackWebsite("connect", "0.2.14", feedbackRecorder) && feedbackLaunches == 1, "one explicit fake feedback launch");
        expect(openWebsite("connect", "0.2.14", updateRecorder) && updateLaunches == 1, "update action still independently works");
        expect(!openFeedbackWebsite("connect", "0.2.14", [](const juce::String&) { return false; }), "feedback opener failure propagates");
    } else {
        expect(!openFeedbackWebsite("connect", "0.2.14", feedbackRecorder) && feedbackLaunches == 0, "unknown architecture cannot be mislabeled");
    }
    auto report = new juce::DynamicObject();
    report->setProperty("schema", "honestmix.support-link-tests.v2");
    report->setProperty("compiledProductVersion", JucePlugin_VersionString);
    report->setProperty("passed", failed == 0);
    report->setProperty("assertions", assertions); report->setProperty("failed", failed);
    report->setProperty("fakeFeedbackLaunches", feedbackLaunches); report->setProperty("fakeUpdateLaunches", updateLaunches);
    report->setProperty("browserOpened", false); report->setProperty("httpUsed", false);
    report->setProperty("automaticReportOrUpdateImplemented", false);
    std::cout << juce::JSON::toString(juce::var(report), true) << '\n';
    return failed == 0 ? 0 : 1;
}
