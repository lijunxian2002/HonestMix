#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class SearchableDropdown : public juce::Component {
public:
    SearchableDropdown();
    void showAt (juce::Component& parent, juce::Rectangle<int> targetBounds);
    void hide();
    void setItems (const juce::StringArray& items);
    std::function<void(int)> onSelect;
    std::function<void()> onDismiss;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    void updateResults();
    juce::TextEditor search_;
    juce::ListBox listBox_;
    juce::StringArray allItems_, filtered_;
};
