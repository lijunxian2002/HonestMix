// 组件 1 · TopBar — 顶栏（品牌名 + 状态灯 + 底边线）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class TopBar : public juce::Component {
public:
    void setStatusDotColour (juce::Colour c) { dotCol_ = c; repaint(); }
    void paint (juce::Graphics&) override;
private:
    juce::Colour dotCol_ = juce::Colours::transparentBlack;
};

// 组件 2 · BottomRow — 底部按钮行（BPM / MONITOR / 诚·反馈）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
class BottomRow : public juce::Component {
public:
    std::function<void()> onBPM, onMonitor, onFeedback;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::Rectangle<int> btnBPM_, btnMonitor_, btnFeedback_;
};

// 组件 3 · Tooltip — 仪表盘浮层提示（Hz + dB）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class Tooltip : public juce::Component {
public:
    void show (int x, int y, float freqHz, float db, int mode);
    void hide();
    void paint (juce::Graphics&) override;
private:
    bool visible_ = false;
    float freq_ = 0, db_ = 0; int mode_ = 0;
    int tx_ = 0, ty_ = 0;
};

// 组件 4 · Breath — 仪表盘呼吸灯（右上角 3px 圆点 + 脉冲动画）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class Breath : public juce::Component, private juce::Timer {
public:
    Breath();
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override;
    float phase_ = 0;
};

// 组件 5 · InfoRow — 信息行（3 个 Chip）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>
class InfoRow : public juce::Component {
public:
    std::function<void(int)> onProfileSelected;   // 耳机 index
    std::function<void(int)> onCurveSelected;     // 曲线 index
    std::function<void(int)> onInterfaceSelected; // 声卡 index
    void setNames (const juce::String& hp, const juce::String& cv, const juce::String& iface);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::String hpName_, cvName_, ifName_;
    juce::Rectangle<int> hpChip_, cvChip_, ifChip_;
};

// 组件 6 · VUPanel — VU 电平条（暗底 + 频谱色填充）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class VUPanel : public juce::Component {
public:
    void setLevel (float v) { level_ = juce::jlimit(0.f,1.f,v); repaint(); }
    void paint (juce::Graphics&) override;
private:
    float level_ = 0;
};

// 组件 7 · OverlayBPM — BPM 助手弹窗
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Core/BPMData.h"
class OverlayBPM : public juce::Component {
public:
    std::function<void()> onClose;
    OverlayBPM();
    void setBPM (int bpm);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    BPMData bpmData_;
    juce::Rectangle<int> closeRect_;
};

// 组件 8 · OverlayMonitor — 监听检查弹窗
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class OverlayMonitor : public juce::Component {
public:
    std::function<void(int)> onAction; // 0单声道/1iPhone/2车/3继续
    std::function<void()>    onClose;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
};

// 组件 9 · OverlayFeedback — 翻译度反馈弹窗
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class OverlayFeedback : public juce::Component {
public:
    std::function<void(int,int)> onSubmit; // bass, treble
    std::function<void()>       onClose;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    int bass_=0, treble_=0;
    juce::Rectangle<int> submitRect_, closeRect_;
};

// 组件 10 · Fader — 垂直推子
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class Fader : public juce::Component {
public:
    std::function<void(float)> onValueChanged; // degree 0~200
    Fader();
    void setValue (float v);
    float getValue() const noexcept { return value_; }
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
private:
    void updateFromMouse (int mouseY);
    float value_ = 0;
    static constexpr int kTrackW = 24, kCapW = 34, kCapH = 22;
};

// 组件 11 · Cockpit — 驾驶舱容器
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class Cockpit : public juce::Component {
public:
    Cockpit();
    void resized() override;
    void paint (juce::Graphics&) override;
    // 子组件由外部注入（addAndMakeVisible），本类只负责内部布局
};

// 组件 12 · CurveGrid — 仪表盘网格线
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class CurveGrid : public juce::Component {
public:
    void paint (juce::Graphics&) override;
};

// 组件 12b · CurveLabels — 仪表盘坐标标签
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class CurveLabels : public juce::Component {
public:
    void paint (juce::Graphics&) override;
};

// 组件 13a · CurveCanvas — 曲线画布（3 组曲线 + 交互）
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class CurveCanvas : public juce::Component {
public:
    std::function<void(float,float)> onHover;
    std::function<void()> onLeave;
    void setTranslationDegree (float v) { degree_ = v; repaint(); }
    void setRawCurve (const float* pts, int n);  // FIR→FFT→15 dB 点，由外部注入
    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    float degree_ = 0;
    int viewMode_ = 0;
    juce::Array<float> rawYs_;
};

// 组件 13b · StarField — 星空粒子背景
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class StarField : public juce::Component, private juce::Timer {
public:
    StarField();
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override;
    struct Star { float x, y, r, bright, phase; };
    juce::Array<Star> stars_;
};
