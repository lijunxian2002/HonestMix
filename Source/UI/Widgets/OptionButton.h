#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class OptionButton : public juce::Component {
public:
    OptionButton();
    void setText(const juce::String& t) { text_ = t; repaint(); }
    void setSelected(bool s) { selected_ = s; repaint(); }
    bool isSelected() const { return selected_; }
    std::function<void()> onClick;

    void paint(juce::Graphics&) override;
    void mouseEnter(const juce::MouseEvent&) override { isHovered_ = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { isHovered_ = false; repaint(); }
    void mouseDown(const juce::MouseEvent&) override;

private:
    juce::String text_;
    bool selected_ = false, isHovered_ = false;
};
