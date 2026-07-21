#include "AudioInterfaceBrowser.h"
#include "../../Core/Settings.h"
#include "../LookAndFeel.h"

//==============================================================================
AudioInterfaceBrowser::AudioInterfaceBrowser()
    : listBox("interfaces", this) // 'this' 作为 ListBoxModel
{
    // 搜索框
    searchBox.setMultiLine(false);
    searchBox.setReturnKeyStartsNewLine(false);
    searchBox.setScrollbarsShown(false);
    searchBox.setSelectAllWhenFocused(true);
    searchBox.setTextToShowWhenEmpty(TRANS("搜索…"), HonestMixLookAndFeel::textSubtle);
    searchBox.onTextChange = [this]() { refreshFilter(); };
    addAndMakeVisible(searchBox);

    // 列表
    listBox.setRowHeight(28);
    addAndMakeVisible(listBox);

    // 关闭按钮
    closeButton.setButtonText(TRANS("收起"));
    closeButton.onClick = [this]() { hide(); };
    addAndMakeVisible(closeButton);

    // 初始显示全部
    refreshFilter();
}

//==============================================================================
void AudioInterfaceBrowser::show()
{
    setVisible(true);
    searchBox.grabKeyboardFocus();
    if (auto* parent = getParentComponent())
    {
        auto bounds = parent->getLocalBounds();
        setBounds(bounds.reduced(40));
    }
    toFront(true);
}

void AudioInterfaceBrowser::hide()
{
    setVisible(false);
}

//==============================================================================
void AudioInterfaceBrowser::paint(juce::Graphics& g)
{
    // 叠加层背景
    g.setColour(HonestMixLookAndFeel::overlayBg);
    g.fillAll();

    // 面板背景
    auto panelBounds = getLocalBounds().reduced(40);
    HonestMixLookAndFeel::drawCardBackground(g, panelBounds);

    // 标题
    auto titleBounds = panelBounds.removeFromTop(30).reduced(14, 0);
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText(TRANS("选择音频接口"), titleBounds.toFloat(),
               juce::Justification::centredLeft, true);
}

void AudioInterfaceBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(40);

    // 标题区域
    auto header = bounds.removeFromTop(30);

    // 搜索框
    searchBox.setBounds(bounds.removeFromTop(28).reduced(0, 2));

    // 列表
    listBox.setBounds(bounds.removeFromTop(240));

    // 关闭按钮
    closeButton.setBounds(bounds.removeFromTop(24)
                          .withSizeKeepingCentre(80, 20));
}

void AudioInterfaceBrowser::visibilityChanged()
{
    if (isVisible())
    {
        refreshFilter();
        searchBox.grabKeyboardFocus();
    }
}

//==============================================================================
void AudioInterfaceBrowser::refreshFilter()
{
    searchText = searchBox.getText().trim().toLowerCase();
    filteredIndices.clear();

    const auto& presets = Settings::getInterfacePresets();
    for (int i = 0; i < presets.size(); ++i)
    {
        if (searchText.isEmpty()
            || presets[i].name.toLowerCase().contains(searchText))
        {
            filteredIndices.add(i);
        }
    }

    listBox.updateContent();
}

void AudioInterfaceBrowser::onItemClicked(int index)
{
    if (juce::isPositiveAndBelow(index, filteredIndices.size()))
    {
        int realIndex = filteredIndices[index];
        const auto& name = Settings::getInterfaceName(realIndex);

        if (onInterfaceSelected)
            onInterfaceSelected(realIndex, name);

        hide();
    }
}

//==============================================================================
// ——— ListBoxModel 实现 ———

int AudioInterfaceBrowser::getNumRows()
{
    return filteredIndices.size();
}

void AudioInterfaceBrowser::paintListBoxItem(int row, juce::Graphics& g,
                                              int width, int height,
                                              bool rowIsSelected)
{
    if (!juce::isPositiveAndBelow(row, filteredIndices.size()))
        return;

    int realIdx = filteredIndices[row];
    const auto& presets = Settings::getInterfacePresets();
    const auto& entry = presets[realIdx];

    auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(4);

    if (rowIsSelected)
    {
        g.setColour(HonestMixLookAndFeel::optionBg);
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    }

    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText(entry.name, bounds.toFloat(),
               juce::Justification::centredLeft, true);

    if (entry.isPopular)
    {
        auto popBounds = bounds.removeFromRight(30);
        g.setColour(HonestMixLookAndFeel::textSubtle);
        g.setFont(juce::FontOptions(7.0f));
        g.drawText(TRANS("热门"), popBounds.toFloat(),
                   juce::Justification::centred, true);
    }
}

void AudioInterfaceBrowser::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    onItemClicked(row);
}

juce::String AudioInterfaceBrowser::getTooltipForRow(int row)
{
    if (juce::isPositiveAndBelow(row, filteredIndices.size()))
    {
        int realIdx = filteredIndices[row];
        return Settings::getInterfaceName(realIdx);
    }
    return {};
}
