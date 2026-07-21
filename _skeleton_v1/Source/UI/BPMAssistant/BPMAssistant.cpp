#include "BPMAssistant.h"
#include "TimeReferenceTable.h"
#include "TapTempo.h"
#include "../LookAndFeel.h"
#include "../../Utils/BPMCalculator.h"

//==============================================================================
BPMAssistant::BPMAssistant(AppState& state)
    : appState(state)
{
    tapTempo = std::make_unique<TapTempo>();

    // BPM 输入
    bpmInput.setMultiLine(false);
    bpmInput.setReturnKeyStartsNewLine(false);
    bpmInput.setSelectAllWhenFocused(true);
    bpmInput.setText(juce::String(appState.getBPM()));
    bpmInput.onTextChange = [this]() { onBPMInputChanged(); };
    addAndMakeVisible(bpmInput);

    // 按速度按钮
    tapButton.setButtonText(TRANS("按速度"));
    tapButton.onClick = [this]() { onTapTempo(); };
    addAndMakeVisible(tapButton);

    // 关闭按钮
    closeButton.setButtonText(TRANS("收起"));
    closeButton.onClick = [this]()
    {
        setVisible(false);
    };
    addAndMakeVisible(closeButton);

    // 标签
    bpmValueLabel.setText(juce::String(appState.getBPM()),
                          juce::dontSendNotification);
    bpmValueLabel.setFont(juce::FontOptions(14.0f));
    addAndMakeVisible(bpmValueLabel);

    BPMCalculator calc(appState.getBPM());
    beatDurationLabel.setText(BPMCalculator::formatMs(calc.getBeatDurationMs(), 0),
                              juce::dontSendNotification);
    beatDurationLabel.setFont(juce::FontOptions(14.0f));
    addAndMakeVisible(beatDurationLabel);

    // 4 个时间参考表
    preDelayTable = std::make_unique<TimeReferenceTable>(
        TimeReferenceTable::Type::PreDelay, "💡 预延迟 PREDELAY");
    addAndMakeVisible(preDelayTable.get());

    reverbTable = std::make_unique<TimeReferenceTable>(
        TimeReferenceTable::Type::Reverb, "💡 混响时间 REVERB TIME");
    addAndMakeVisible(reverbTable.get());

    releaseTable = std::make_unique<TimeReferenceTable>(
        TimeReferenceTable::Type::Release, "💡 压缩释放 COMPRESSOR RELEASE");
    addAndMakeVisible(releaseTable.get());

    delayTable = std::make_unique<TimeReferenceTable>(
        TimeReferenceTable::Type::Delay, "💡 延迟时间 DELAY TIME");
    addAndMakeVisible(delayTable.get());

    // 初始数据
    refreshData();
}

BPMAssistant::~BPMAssistant() = default;

//==============================================================================
void BPMAssistant::refreshData()
{
    setBPM(appState.getBPM());
}

void BPMAssistant::setBPM(int bpm)
{
    int clamped = juce::jlimit(20, 300, bpm);
    appState.setBPM(clamped);

    preDelayTable->setBPM(clamped);
    reverbTable->setBPM(clamped);
    releaseTable->setBPM(clamped);
    delayTable->setBPM(clamped);

    updateDisplay();
}

//==============================================================================
void BPMAssistant::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 面板背景
    HonestMixLookAndFeel::drawCardBackground(g, bounds);

    // 标题栏
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText(TRANS("BPM 助手"),
               titleArea.toFloat(),
               juce::Justification::centredLeft, true);

    // 底部提示
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(7.0f));
    auto foot = bounds.removeFromBottom(16).reduced(14, 0);
    g.drawText(TRANS("BPM 跟随宿主 · 也可手动输入 · 当前 ")
               + juce::String(appState.getBPM()) + " BPM",
               foot.toFloat(),
               juce::Justification::centred, true);
}

void BPMAssistant::resized()
{
    auto bounds = getLocalBounds().reduced(14);

    // 标题栏 + 关闭按钮
    titleArea = bounds.removeFromTop(28);
    closeButton.setBounds(titleArea.removeFromRight(40)
                          .withSizeKeepingCentre(40, 20));

    bounds.removeFromTop(4);

    // 输入行
    inputArea = bounds.removeFromTop(32);
    tapButton.setBounds(inputArea.removeFromRight(60));
    inputArea.removeFromRight(6);
    bpmInput.setBounds(inputArea);

    bounds.removeFromTop(6);

    // 概览（两列）
    summaryArea = bounds.removeFromTop(48);
    int halfW = summaryArea.getWidth() / 2;
    bpmValueLabel.setBounds(summaryArea.removeFromLeft(halfW).reduced(4, 0));
    beatDurationLabel.setBounds(summaryArea.reduced(4, 0));

    bounds.removeFromTop(6);

    // 4 个表
    tablesArea = bounds;
    int tableH = tablesArea.getHeight() / 4;

    preDelayTable->setBounds(tablesArea.removeFromTop(tableH));
    tablesArea.removeFromTop(2);
    reverbTable->setBounds(tablesArea.removeFromTop(tableH));
    tablesArea.removeFromTop(2);
    releaseTable->setBounds(tablesArea.removeFromTop(tableH));
    tablesArea.removeFromTop(2);
    delayTable->setBounds(tablesArea);
}

//==============================================================================
void BPMAssistant::onTapTempo()
{
    int bpm = tapTempo->registerTap();
    if (bpm > 0)
    {
        bpmInput.setText(juce::String(bpm), juce::sendNotification);
        setBPM(bpm);
    }
}

void BPMAssistant::onBPMInputChanged()
{
    int bpm = bpmInput.getText().getIntValue();
    if (bpm >= 20 && bpm <= 300)
        setBPM(bpm);
}

void BPMAssistant::updateDisplay()
{
    BPMCalculator calc(appState.getBPM());
    bpmValueLabel.setText(juce::String(calc.getBPM()), juce::dontSendNotification);
    beatDurationLabel.setText(BPMCalculator::formatMs(calc.getBeatDurationMs(), 0),
                              juce::dontSendNotification);
}
