#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <functional>

namespace hm::updates {
inline constexpr const char* websiteOrigin = "https://honestmix-studio-20261016.lijunxian2002549234.chatgpt.site";
inline bool canonicalVersion(const juce::String& text) {
    if (text.isEmpty() || text.length() > 17 || !text.containsOnly("0123456789.") ||
        text.retainCharacters(".").length() != 2 || text.startsWithChar('.') || text.endsWithChar('.')) return false;
    const auto fields = juce::StringArray::fromTokens(text, ".", "");
    if (fields.size() != 3) return false;
    for (const auto& field : fields) {
        if (field.isEmpty() || field.length() > 5 ||
            (field.length() > 1 && field.startsWithChar('0')) || field.getIntValue() > 65535) return false;
    }
    return true;
}

inline juce::String currentPlatform() {
   #if JUCE_WINDOWS && JUCE_64BIT && JUCE_INTEL
    return "windows-x64";
   #elif JUCE_MAC && (defined(__aarch64__) || defined(__arm64__))
    return "macos-arm64";
   #else
    return {};
   #endif
}

// The update destination remains compiled and cannot come from a session,
// transport error, manifest, settings file, clipboard, or remote peer.
inline juce::String makeWebsiteLink(const juce::String& product, const juce::String& platform,
                                    const juce::String& version, const juce::String& channel = "preview") {
    if ((product != "connect" && product != "headphone") ||
        (platform != "windows-x64" && platform != "macos-arm64") ||
        (channel != "preview" && channel != "stable") || !canonicalVersion(version)) return {};
    return juce::String(websiteOrigin) + "/index.html?product=" + product + "&platform=" + platform +
           "&version=" + version + "&channel=" + channel + "#downloads";
}

inline bool feedbackContextSupported(const juce::String& product, const juce::String& platform) {
    return (product == "connect" && (platform == "windows-x64" || platform == "macos-arm64")) ||
           (product == "headphone" && platform == "windows-x64");
}

inline juce::String makeFeedbackLink(const juce::String& product, const juce::String& platform,
                                     const juce::String& version) {
    if (!feedbackContextSupported(product, platform) || !canonicalVersion(version)) return {};
    return juce::String(websiteOrigin) + "/feedback.html?product=" + product + "&platform=" + platform +
           "&version=" + version;
}

// Both helpers are called by explicit UI actions only. They open a fixed web
// page in the user's browser and contain no HTTP/upload, timer, thread, cache,
// installer, logs, session data, automatic report, or telemetry lifecycle.
inline bool openWebsite(const juce::String& product, const juce::String& version,
                        const std::function<bool(const juce::String&)>& launch =
                            [](const juce::String& url) { return juce::URL(url).launchInDefaultBrowser(); }) {
    const auto url = makeWebsiteLink(product, currentPlatform(), version);
    return url.isNotEmpty() && launch(url);
}
inline bool openFeedbackWebsite(const juce::String& product, const juce::String& version,
                                const std::function<bool(const juce::String&)>& launch =
                                    [](const juce::String& url) { return juce::URL(url).launchInDefaultBrowser(); }) {
    const auto url = makeFeedbackLink(product, currentPlatform(), version);
    return url.isNotEmpty() && launch(url);
}
}
