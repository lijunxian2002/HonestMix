#include "Net/FeedbackClient.h"
#include <cstdlib>

FeedbackClient::FeedbackClient()
{
    // 尝试从环境变量读取 Supabase 配置
    const char* envUrl = std::getenv ("HONESTMIX_SUPABASE_URL");
    const char* envKey = std::getenv ("HONESTMIX_SUPABASE_KEY");
    if (envUrl && envKey)
        setConfig (juce::String (envUrl), juce::String (envKey));
}

FeedbackClient::~FeedbackClient()
{
    pool_.removeAllJobs (true, 5000);
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

    const juce::String u = url, k = key;
    pool_.addJob ([u, k, json]
    {
        juce::URL target (u);
        auto opts = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inPostData)
                        .withExtraHeaders ("apikey: " + k + "\r\nAuthorization: Bearer " + k
                                           + "\r\nContent-Type: application/json")
                        .withConnectionTimeoutMs (5000);

        target = target.withPOSTData (json);
        auto stream = target.createInputStream (opts); // 后台线程阻塞，不影响 UI
    });
    return true;
}
