#pragma once

//==============================================================================
/**
 * 首次设置向导（全屏遮罩）—— 3 步：耳机 → 混音时长 → 目标曲线
 *
 * 步骤 1 耳机：输入即搜 1603 库 / 无输入时显示 8 副热门预设
 * 步骤 2 混音时长：30分/1h/2h/3h+ —— 决定"换监听提醒"节奏（AppState 持久化）
 * 步骤 3 目标曲线：Harman OE（默认预选）等 4 条
 *
 * 修复记录：
 *   - v0.4.0 初版"下一步"按钮与第 8 行预设重叠 20px，点击被 K701 行吞掉，
 *     导致步骤无法前进（用户体感：耳机页出现两次、曲线卡不存在）
 *   - 步骤 1/2 增加可见的"‹ 返回"按钮
 *
 * 回调：
 *   onPickProfile(int kProfiles 下标)          —— 选中耳机即驱动 FIR 切换
 *   onFinish(int habitMinutes, int curveIndex) —— 第 3 步完成 → 过渡卡
 */
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "Core/HeadphoneDatabase.h"

class SetupCard : public juce::Component
{
public:
    std::function<void (int)> onPickProfile;
    std::function<void (int, int)> onFinish;

    explicit SetupCard (const HeadphoneDatabase& db);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

    /** 混音时长选项（分钟）—— 与 UI 行一一对应 */
    static constexpr int kHabitMinutes[4] = { 30, 60, 120, 180 };

    /** 显示前由 Editor 调用：捕获主界面快照并高斯模糊（Apple 毛玻璃底） */
    void captureBackdrop();

private:
    juce::Rectangle<int> cardRect() const;
    juce::Rectangle<int> rowRect (int i) const;   // 步骤 0 行
    juce::Rectangle<int> optRect (int i) const;   // 步骤 1/2 选项行
    void rebuildResults();

    const HeadphoneDatabase& db_;
    juce::Image backdrop_;       // 模糊底图
    juce::TextEditor input_;
    juce::String filter_;
    std::vector<int> results_;   // kProfiles 下标（搜索态）
    int selProfile_ = -1;        // 选中的 kProfiles 下标
    int selHabit_   = 1;         // 混音时长（默认 1 小时）
    int selCurve_   = 0;         // 目标曲线（默认 Harman OE）
    int step_ = 0;               // 0 耳机 / 1 混音时长 / 2 目标曲线

    juce::Rectangle<int> nextRect_, backRect_;

    static constexpr int kMaxRows = 9;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetupCard)
};
