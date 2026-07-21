#include "TapTempo.h"

//==============================================================================
TapTempo::TapTempo() {}

//==============================================================================
int TapTempo::registerTap()
{
    auto now = juce::Time::getCurrentTime();

    // 如果有点击历史，检查超时
    if (!tapTimes.isEmpty())
    {
        auto elapsed = (now - tapTimes.getLast()).inMilliseconds();
        if (elapsed > resetTimeoutMs)
            tapTimes.clear();
    }

    tapTimes.add(now);

    // 保留最近 N 次
    while (tapTimes.size() > maxTaps)
        tapTimes.remove(0);

    // 需要至少 2 次点击才能计算
    if (tapTimes.size() < 2)
        return 0;

    // 计算平均间隔
    double totalMs = 0.0;
    for (int i = 1; i < tapTimes.size(); ++i)
    {
        double interval = (tapTimes[i] - tapTimes[i - 1]).inMilliseconds();

        // 忽略过短的间隔（防抖）
        if (interval < minIntervalMs)
            continue;

        totalMs += interval;
    }

    int validIntervals = tapTimes.size() - 1;
    if (validIntervals <= 0)
        return 0;

    double avgMs = totalMs / validIntervals;

    if (avgMs < minIntervalMs)
        return 0;

    // BPM = 60000 / 平均每拍 ms
    currentBPM = static_cast<int>(std::round(60000.0 / avgMs));
    currentBPM = juce::jlimit(20, 300, currentBPM);

    return currentBPM;
}

void TapTempo::reset()
{
    tapTimes.clear();
    currentBPM = 0;
}

float TapTempo::getConfidence() const noexcept
{
    if (tapTimes.size() < 3)
        return 0.0f;

    // 置信度随点击次数增加
    return juce::jmin(1.0f, (tapTimes.size() - 1) / static_cast<float>(maxTaps - 1));
}
