#pragma once

//==============================================================================
/**
 * HonestMix 反馈客户端（数据中心原样移植，仅改 include 路径）
 * 使用 Supabase REST API 上传用户反馈（后台线程，不阻塞 UI）
 *
 * 数据表结构 (Supabase):
 *   id:         int8 (auto)      created_at: timestamptz (auto)
 *   headphone:  text   耳机型号   interface:  text  声卡型号
 *   degree:     float4 翻译度     bass_fb / treble_fb / comment: text
 *
 * 配置：环境变量 HONESTMIX_SUPABASE_URL / HONESTMIX_SUPABASE_KEY
 */
#include <juce_core/juce_core.h>

class FeedbackClient
{
public:
    FeedbackClient();
    ~FeedbackClient();

    /** 设置 Supabase 项目信息 */
    void setConfig (const juce::String& apiUrl, const juce::String& apiKey);

    /** 发送一条反馈（排队到后台线程，立即返回；返回值 = 是否已入队） */
    bool sendFeedback (const juce::var& data);

    /** 返回是否已配置 */
    bool isConfigured() const { return url.isNotEmpty() && key.isNotEmpty(); }

private:
    juce::String url;
    juce::String key;
    juce::ThreadPool pool_ { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackClient)
};
