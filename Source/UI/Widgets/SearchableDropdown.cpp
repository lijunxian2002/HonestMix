#include "SearchableDropdown.h"

class DropdownListModel : public juce::ListBoxModel {
public:
    juce::StringArray* items = nullptr;
    std::function<void(int)> onClick;
    int getNumRows() override { return items ? items->size() : 0; }
    void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override {
        if (selected) g.fillAll(juce::Colours::white.withAlpha(0.06f));
        g.setColour(juce::Colour(235, 235, 230).withAlpha(0.62f));
        g.setFont(juce::FontOptions(8.0f));
        g.drawText((*items)[row], 16, 0, w - 16, h, juce::Justification::centredLeft);
    }
    void listBoxItemClicked(int row, const juce::MouseEvent&) override { if (onClick) onClick(row); }
};

SearchableDropdown::SearchableDropdown() {
    search_.setMultiLine(false);
    search_.setFont(juce::FontOptions(12.0f));
    search_.setColour(juce::TextEditor::textColourId, juce::Colours::white.withAlpha(0.8f));
    search_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(30, 32, 40));
    search_.setColour(juce::TextEditor::outlineColourId, juce::Colours::white.withAlpha(0.1f));
    search_.onTextChange = [this] { updateResults(); };
    addAndMakeVisible(search_);

    auto* model = new DropdownListModel();
    model->items = &filtered_;
    model->onClick = [this](int idx) {
        if (onSelect && idx >= 0 && idx < filtered_.size())
            onSelect(allItems_.indexOf(filtered_[idx]));
    };
    listBox_.setModel(model);
    addAndMakeVisible(listBox_);
}

void SearchableDropdown::showAt (juce::Component& parent, juce::Rectangle<int> targetBounds) {
    setSize (260, 230);  // HTML: max-width 260, list max-height 200 + search 30
    // 定位到 target 下方，超出屏幕则翻到上方
    auto pos = targetBounds.getBottomLeft();
    if (pos.y + 320 > parent.getParentComponent()->getHeight())
        pos.y = targetBounds.getY() - 320 - 4;
    setTopLeftPosition (pos.x, juce::jmax (4, pos.y));
    // 挂到最顶层，避免被任何兄弟组件裁剪
    if (auto* top = parent.getTopLevelComponent())
        top->addAndMakeVisible (this);
    else
        parent.addAndMakeVisible (this);
    toFront (true);
    search_.grabKeyboardFocus();
}

void SearchableDropdown::hide() { setVisible (false); if (onDismiss) onDismiss(); }

void SearchableDropdown::mouseDown (const juce::MouseEvent& e) {
    if (! getLocalBounds().contains (e.getPosition()))
        hide();
}

bool SearchableDropdown::keyPressed (const juce::KeyPress& k) {
    if (k == juce::KeyPress::escapeKey) { hide(); return true; }
    return false;
}

void SearchableDropdown::setItems(const juce::StringArray& items) {
    allItems_ = items;
    filtered_ = allItems_;
    listBox_.updateContent();
}

void SearchableDropdown::updateResults() {
    auto query = search_.getText().toLowerCase();
    filtered_.clear();
    if (query.isEmpty()) { filtered_ = allItems_; }
    else { for (auto& item : allItems_) { if (item.toLowerCase().contains(query)) filtered_.add(item); } }
    listBox_.updateContent();
}

void SearchableDropdown::resized() {
    auto b = getLocalBounds();
    search_.setBounds(b.removeFromTop(30).reduced(8, 4));
    listBox_.setBounds(b.reduced(8, 4));
}

void SearchableDropdown::paint(juce::Graphics& g) {
    g.setColour(juce::Colour::fromRGBA(18, 20, 26, 250));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}
