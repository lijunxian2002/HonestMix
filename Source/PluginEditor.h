#pragma once

//==============================================================================
/**
 * HonestMix BETA2 主编辑器 —— 主界面装配 + 覆盖层调度
 *
 * 拼接说明（数据中心 PluginEditor 1641 行单体 → 骨架模块化）：
 *   8 个自绘组件已拆出到 Source/UI/（Seal/SnakeKnob/Setup/Transition/
 *   Bpm/Feedback/Check/Share），数据访问全部收口到 HeadphoneDatabase。
 *   本类只保留：主界面 4 元素（信息行/旋杆/BPM/诚）、内联搜索、
 *   覆盖层显隐调度、1 小时检查计时、反馈提交。
 *
 * 主界面只有 4 个元素：信息行（耳机/曲线/声卡）、旋杆（0~200）、
 * BPM、诚印章 —— 无标题 / 无版本号 / 无开关 / 无说明文字。
 */
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "Core/Design.h"
#include "Net/FeedbackClient.h"
#include "UI/SealComponent.h"
#include "UI/SnakeKnob.h"
#include "UI/SetupCard.h"
#include "UI/TransitionCard.h"
#include "UI/BpmPanel.h"
#include "UI/FeedbackCard.h"
#include "UI/CheckCard.h"
#include "UI/ShareCard.h"

class HonestMixAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit HonestMixAudioProcessorEditor (HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    void toggleBPM();
    void submitFeedback (int bass, int treble);
    void closeAllOverlays();
    void fitWindow();              // 按可见内容自适应窗口高度（替代原 setExpanded/collapseIfIdle）
    void refreshShareInfo();
    void openSearch (int field);   // 0=耳机 1=曲线 2=声卡
    void closeSearch();

    HonestMixAudioProcessor& processorRef_;

    // ── 动态画布（屏宽 28% × 黄金比；基准见 Core/Design.h）──
    int PW_ = 420, PHC_ = 300, PHE_ = 645;
    float scale_ = 1.0f;

    // ── 全局 LNF（持有实例，寿命覆盖所有子组件）──
    HonestLookAndFeel honestLNF_;
    SnakeKnobLNF      knobLNF_;

    // 所有 UI 挂在 content_ 下
    juce::Component content_;

    // ── 主界面（信息行、旋杆、BPM、诚 + 内联搜索）──
    juce::Label hpLbl_, cvLbl_, ifLbl_;
    juce::Label hpVal_, cvVal_, ifVal_;
    juce::Slider knob_;
    juce::Label  knobVal_;
    juce::Label bpmLbl_;
    SealComponent seal_;

    // 内联搜索（点信息行展开）
    juce::TextEditor searchBox_;
    std::vector<int> searchResults_;
    int searchSel_ = -1;
    int searchField_ = -1; // 0=耳机 1=曲线 2=声卡, -1=隐藏

    // ── 浮动层 ──
    BpmPanel       bpmPanel_;
    TransitionCard transitionCard_;
    FeedbackCard   feedbackCard_;
    CheckCard      checkCard_;
    ShareCard      shareCard_;
    SetupCard      setupCard_;

    // ── 反馈服务 ──
    FeedbackClient feedbackClient_;

    int   tick_ = 0;
    float lastHostBpm_ = 0.0f;
    float lastDegreeShown_ = -1.0f;

    // ── 声卡/曲线选择（随 AppState 持久化）──
    int interfaceIdx_ = 0, curveIdx_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
