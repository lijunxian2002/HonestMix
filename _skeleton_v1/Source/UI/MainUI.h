#pragma once

#include <JuceHeader.h>
#include "../Core/AppState.h"

class Knob;
class ToggleSwitch;

//==============================================================================
/**
 * 主界面 (Main UI)
 *
 * 混音开始后显示的主操作面板。
 * 复刻 HTML 中的 .main-ui 样式：
 *   - 顶部：设备名称 + 版本号
 *   - 信息栏：耳机型号 / 目标曲线 / 声卡型号
 *   - 中央：WET 旋钮
 *   - 底部：校正开关 + BPM 助手入口
 */
class MainUI : public juce::Component,
               private juce::ChangeListener
{
public:
    //==============================================================================
    explicit MainUI(AppState& appState);
    ~MainUI() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    AppState& appState;

    // ——— 子组件 ———
    std::unique_ptr<Knob>          wetKnob;
    std::unique_ptr<ToggleSwitch>  correctionSwitch;
    juce::Label                    bpmButton;

    // 设备信息文字（缓存）
    juce::String headphoneName;
    juce::String curveTarget;
    juce::String interfaceName;

    // ——— 布局缓存 ———
    juce::Rectangle<int> titleBar;
    juce::Rectangle<int> infoBar;
    juce::Rectangle<int> knobArea;
    juce::Rectangle<int> bottomBar;

    // ——— 回调 ———
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void onWetDryChanged();
    void onCorrectionToggled(bool on);

    void refreshDeviceInfo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainUI)
};
