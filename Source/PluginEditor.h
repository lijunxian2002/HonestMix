#pragma once
// 组装版 PluginEditor — 14 组件全部挂载、布局、信号连接
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/Components.h"

class HonestMixAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit HonestMixAudioProcessorEditor (HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void toggleOverlay (juce::Component& overlay, juce::Image* backdrop=nullptr);

    HonestMixAudioProcessor& proc_;

    TopBar           topBar_;
    BottomRow        bottomRow_;
    Tooltip          tooltip_;
    Breath           breath_;
    InfoRow          infoRow_;
    VUPanel          vuPanel_;
    OverlayBPM       overlayBPM_;
    OverlayMonitor   overlayMonitor_;
    OverlayFeedback  overlayFeedback_;
    Fader            fader_;
    Cockpit          cockpit_;
    CurveGrid        curveGrid_;
    CurveLabels      curveLabels_;
    CurveCanvas      curveCanvas_;
    StarField        starField_;

    float lastHostBpm_ = 0;
    float lastDegree_ = -1;
    int   curveIdx_ = 0, interfaceIdx_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestMixAudioProcessorEditor)
};
