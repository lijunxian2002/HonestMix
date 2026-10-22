#pragma once
// 13b StarField — 逐行对标 13-CurveCanvas+StarField.html
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class StarField : public juce::Component, private juce::Timer {
public:
    StarField();
    ~StarField() override = default;
    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    void startHeart();
    void endHeart();

    struct Star {
        float x, y; float toX, toY; float homeX, homeY;
        float size; juce::int64 phaseOffset; float baseOpacity;
        float opacity = 0; float scale = 1.0f;
        bool lerping = false;  // 仅心形进出时启用位置过渡
    };
    juce::Array<Star> stars_;       // 45

    // 追星
    float chaseCurX_=50, chaseCurY_=50, chaseHomeX_=50, chaseHomeY_=50;
    float chaseVX_=0, chaseVY_=0;

    juce::Image bgImage_;

    // 鼠标
    float mx_=50, my_=50;
    bool mouseInside_ = false;

    // 视差
    float parallaxDX_=0, parallaxDY_=0;
    float curDX_=0, curDY_=0;       // 平滑后的视差

    // 心跳
    bool heartMode_ = false;
    std::vector<int> heartStars_;
    juce::int64 lastHeartTime_ = 0;
    juce::int64 stillStart_ = 0;
    juce::int64 t0_ = 0;
};
