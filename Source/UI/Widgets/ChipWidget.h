#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class ChipWidget : public juce::Component {
public:
    ChipWidget();
    void setLabel(const juce::String& l) { label_ = l; repaint(); }
    void setValue(const juce::String& v) { value_ = v; repaint(); }
    void setHoverable(bool h) { hoverable_ = h; }
    std::function<void()> onOpen;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    juce::String label_, value_;
    bool hoverable_ = true, isHovered_ = false;
    juce::Rectangle<int> labelRect_, valueRect_;
};
