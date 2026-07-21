#include "MainUI.h"
#include "Knob.h"
#include "ToggleSwitch.h"
#include "LookAndFeel.h"

//==============================================================================
MainUI::MainUI(AppState& state)
    : appState(state)
{
    // WET 旋钮
    wetKnob = std::make_unique<Knob>();
    wetKnob->setLabelText("WET");
    wetKnob->setValueSuffix("%");
    wetKnob->setValue(appState.getWetDryMix());
    wetKnob->onValueChange = [this]() { onWetDryChanged(); };
    addAndMakeVisible(wetKnob.get());

    // 校正开关
    correctionSwitch = std::make_unique<ToggleSwitch>();
    correctionSwitch->setLabelText(TRANS("校正"));
    correctionSwitch->setToggleState(appState.isCorrectionEnabled());
    correctionSwitch->onStateChanged = [this](bool on) { onCorrectionToggled(on); };
    addAndMakeVisible(correctionSwitch.get());

    // BPM 按钮
    bpmButton.setText(TRANS("BPM 助手"), juce::dontSendNotification);
    bpmButton.setJustificationType(juce::Justification::centred);
    bpmButton.setFont(juce::FontOptions(9.0f));
    bpmButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    bpmButton.setColour(juce::Label::textColourId, HonestMixLookAndFeel::textMuted);
    bpmButton.addMouseListener(this, false);
    addAndMakeVisible(bpmButton);

    // 监听状态变更
    appState.getChangeBroadcaster().addChangeListener(this);

    refreshDeviceInfo();
}

MainUI::~MainUI()
{
    appState.getChangeBroadcaster().removeChangeListener(this);
}

//==============================================================================
void MainUI::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 卡片背景
    HonestMixLookAndFeel::drawCardBackground(g, bounds);

    // === 标题栏 ===
    auto titleBar = bounds.removeFromTop(32).reduced(14, 0);

    g.setColour(HonestMixLookAndFeel::textPrimary);
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("HonestMix", titleBar.toFloat(),
               juce::Justification::centredLeft, true);

    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(8.0f));
    g.drawText("v0.1.0", titleBar.toFloat(),
               juce::Justification::centredRight, true);

    // === 信息栏 ===
    auto infoBar = bounds.removeFromTop(36).reduced(4, 0);

    // 分隔线
    g.setColour(HonestMixLookAndFeel::divider);
    g.fillRect(0, infoBar.getY() - 2, getWidth(), 1);
    g.fillRect(0, infoBar.getBottom() + 2, getWidth(), 1);

    // 三列信息
    int infoColW = infoBar.getWidth() / 3;
    for (int i = 0; i < 3; ++i)
    {
        auto col = infoBar.removeFromLeft(infoColW);

        juce::String label, value;
        switch (i)
        {
        case 0: label = "耳机"; value = headphoneName; break;
        case 1: label = "曲线"; value = curveTarget;   break;
        case 2: label = "声卡"; value = interfaceName; break;
        }

        g.setColour(HonestMixLookAndFeel::textSubtle);
        g.setFont(juce::FontOptions(7.0f));
        g.drawText(label, col.removeFromTop(12).reduced(2, 0).toFloat(),
                   juce::Justification::centred, true);

        g.setColour(HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(value, col.reduced(2, 0).toFloat(),
                   juce::Justification::centred, true);
    }
}

void MainUI::resized()
{
    auto bounds = getLocalBounds().reduced(14);

    // 标题栏
    titleBar = bounds.removeFromTop(32);

    // 信息栏
    infoBar = bounds.removeFromTop(44);

    // 旋钮区域（居中占据主要空间）
    knobArea = bounds.removeFromTop(140).reduced(20, 0);
    wetKnob->setBounds(knobArea);

    // 底部栏
    bottomBar = bounds.removeFromTop(36);
    int halfW = bottomBar.getWidth() / 2;
    correctionSwitch->setBounds(bottomBar.removeFromLeft(halfW).reduced(4, 4));
    bpmButton.setBounds(bottomBar.reduced(4, 4));
}

//==============================================================================
void MainUI::changeListenerCallback(juce::ChangeBroadcaster*)
{
    refreshDeviceInfo();
    repaint();
}

void MainUI::onWetDryChanged()
{
    float val = static_cast<float>(wetKnob->getValue());
    appState.setWetDryMix(val);
}

void MainUI::onCorrectionToggled(bool on)
{
    appState.setCorrectionEnabled(on);
}

void MainUI::refreshDeviceInfo()
{
    headphoneName = appState.getHeadphoneModelName();
    if (headphoneName.isEmpty() && appState.getHeadphoneModel() >= 0)
        headphoneName = Settings::getHeadphoneName(appState.getHeadphoneModel());

    curveTarget = {}; // 由工程师注入目标曲线名称

    interfaceName = appState.getAudioInterfaceName();
    if (interfaceName.isEmpty() && appState.getAudioInterface() >= 0)
        interfaceName = Settings::getInterfaceName(appState.getAudioInterface());
}
