#include "UI/Cockpit.h"

void Cockpit::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    // HTML: background rgba(0,0,0,0.42) + inset shadow inset 0 5px 18px
    g.setColour(juce::Colour::fromRGBA(0,0,0,107));
    g.fillRoundedRectangle(b.toFloat(),10.f);
    // inset shadow — 顶部暗条
    juce::ColourGradient insetGrad(juce::Colour::fromRGBA(0,0,0,153),0,0,
                                     juce::Colour::fromRGBA(0,0,0,0),0,(float)b.getHeight()*0.25f,false);
    g.setGradientFill(insetGrad);g.fillRoundedRectangle(b.toFloat(),10.f);
    // HTML box-shadow 多层: 0 0 0 1px → 0 0 0 5px → 0 0 0 6px
    g.setColour(juce::Colour::fromRGBA(55,60,70,51));       // rgba(55,60,70,0.2)
    g.drawRoundedRectangle(b.toFloat().reduced(0.5f),10.f,1.f);
    g.setColour(juce::Colour::fromRGBA(12,13,16,204));      // rgba(12,13,16,0.8)
    g.drawRoundedRectangle(b.toFloat().reduced(5.0f),10.f,1.f);
    g.setColour(juce::Colour::fromRGBA(40,45,52,25));       // rgba(40,45,52,0.1)
    g.drawRoundedRectangle(b.toFloat().reduced(6.0f),10.f,1.f);
    // 主边框 1.5px + 动态色（减薄，避免黑边过重）
    g.setColour(juce::Colour::fromRGBA(16,18,22,180));
    g.drawRoundedRectangle(b.toFloat().reduced(1.5f),10.f,1.5f);
    g.setColour(borderCol_);  // 随fader变化
    g.drawRoundedRectangle(b.toFloat().reduced(1.5f),10.f,2.f);
    // inset 内衬线
    g.setColour(juce::Colours::white.withAlpha(0.008f));
    g.drawRoundedRectangle(b.toFloat().reduced(4.5f),7.f,1.f);
    // inset 0 2px 0 highlight
    g.setColour(juce::Colours::white.withAlpha(0.006f));
    g.fillRect((float)b.getX()+2.f,(float)b.getY()+2.f,(float)b.getWidth()-4.f,1.f);
    // drop shadow
    juce::DropShadow sd(juce::Colours::black.withAlpha(0.42f),21,{0,8});
    sd.drawForRectangle(g,b);
}
