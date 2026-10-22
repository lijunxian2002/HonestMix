#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class HoverButton : public juce::Component {
public:
    HoverButton();
    void setText(const juce::String& t) { text_ = t; repaint(); }
    void setAccent(bool a) { isAccent_ = a; repaint(); }
    std::function<void()> onClick;

    void paint(juce::Graphics&) override;
    void mouseEnter(const juce::MouseEvent&) override { isHovered_ = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { isHovered_ = false; repaint(); }
    void mouseDown(const juce::MouseEvent&) override { isPressed_ = true; repaint(); }
    void mouseUp(const juce::MouseEvent&) override;

private:
    juce::String text_;
    bool isHovered_ = false, isPressed_ = false, isAccent_ = false;
};
