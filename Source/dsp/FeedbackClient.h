#pragma once

#include <juce_core/juce_core.h>

/**
 * HonestMix 反馈客户端
 * 使用 Supabase REST API 上传用户反馈
 *
 * 数据表结构 (Supabase):
 *   id:         int8 (auto)
 *   created_at: timestamptz (auto)
 *   headphone:  text      -- 耳机型号
 *   interface:  text      -- 声卡型号
 *   drywet:     float4    -- 当前干湿比
 *   correction: bool      -- 校正是否开启
 *   bass_fb:    text      -- 低频反馈 (too_much / okay / too_little)
 *   treble_fb:  text      -- 高频反馈
 *   comment:    text      -- 备注
 */
class FeedbackClient
{
public:
    FeedbackClient();

    /** 设置 Supabase 项目信息 */
    void setConfig (const juce::String& apiUrl, const juce::String& apiKey);

    /** 发送一条反馈 */
    bool sendFeedback (const juce::var& data);

    /** 返回是否已配置 */
    bool isConfigured() const { return url.isNotEmpty() && key.isNotEmpty(); }

private:
    juce::String url;
    juce::String key;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackClient)
};
