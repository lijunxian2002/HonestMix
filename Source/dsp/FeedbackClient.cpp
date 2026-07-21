#include "dsp/FeedbackClient.h"
#include <cstdlib>

FeedbackClient::FeedbackClient()
{
    // 尝试从环境变量读取 Supabase 配置
    const char* envUrl  = std::getenv ("HONESTMIX_SUPABASE_URL");
    const char* envKey  = std::getenv ("HONESTMIX_SUPABASE_KEY");
    if (envUrl && envKey)
        setConfig (juce::String (envUrl), juce::String (envKey));
}

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
