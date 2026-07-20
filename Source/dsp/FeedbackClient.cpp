#include "dsp/FeedbackClient.h"

FeedbackClient::FeedbackClient() {}

void FeedbackClient::setConfig (const juce::String& apiUrl, const juce::String& apiKey)
{
    url = apiUrl;
    key = apiKey;
}

bool FeedbackClient::sendFeedback (const juce::var& data)
{
    if (! isConfigured())
        return false;

    auto json = juce::JSON::toString (data);
    if (json.isEmpty())
        return false;

    // 用 createInputStream 发送 POST
    juce::URL target (url);
    auto opts = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inPostData)
                    .withExtraHeaders (juce::String ("apikey: " + key + "\r\nAuthorization: Bearer " + key + "\r\nContent-Type: application/json"))
                    .withConnectionTimeoutMs (3000);

    target = target.withPOSTData (json);

    auto stream = target.createInputStream (opts);
    return stream != nullptr;
}
