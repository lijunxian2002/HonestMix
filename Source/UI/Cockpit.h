#pragma once
// Cockpit — 驾驶舱外壳（无搜索，搜索转 PluginEditor 内联输入框）
#include <juce_gui_basics/juce_gui_basics.h>

class Cockpit : public juce::Component {
public:
    Cockpit() = default;
    void paint (juce::Graphics&) override;
    void resized() override {}
    void setBorderColor(juce::Colour c){borderCol_=c;repaint();}
private:
    juce::Colour borderCol_=juce::Colour(80,78,74).withAlpha(0.5f);
};
