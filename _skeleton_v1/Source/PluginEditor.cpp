#include "PluginEditor.h"
#include "UI/MainComponent.h"
#include "UI/LookAndFeel.h"

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor(
    HonestMixAudioProcessor& proc)
    : AudioProcessorEditor(&proc), processor(proc)
{
    // 最小工作尺寸（宿主可缩放）
    setSize(800, 600);
    setResizable(true, true);
    setResizeLimits(600, 400, 1920, 1200);

    // 创建根 UI 组件
    mainComponent = std::make_unique<MainComponent>(processor.getAppState());
    addAndMakeVisible(mainComponent.get());

    // 应用 LookAndFeel
    setLookAndFeel(&HonestMixLookAndFeel::getInstance());
}

HonestMixAudioProcessorEditor::~HonestMixAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void HonestMixAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(HonestMixLookAndFeel::bgDark);
}

void HonestMixAudioProcessorEditor::resized()
{
    // MainComponent 撑满整个编辑器
    if (mainComponent != nullptr)
        mainComponent->setBounds(getLocalBounds());
}
