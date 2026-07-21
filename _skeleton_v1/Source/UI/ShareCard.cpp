#include "ShareCard.h"
#include "LookAndFeel.h"

//==============================================================================
ShareCard::ShareCard(AppState& state)
    : appState(state)
{
    saveButton.setButtonText(TRANS("保存卡片"));
    saveButton.onClick = []()
    {
        // TODO: 导出卡片图片
    };
    addAndMakeVisible(saveButton);

    shareButton.setButtonText(TRANS("分享"));
    shareButton.onClick = []()
    {
        // TODO: 调用系统分享
    };
    addAndMakeVisible(shareButton);
}

//==============================================================================
void ShareCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 卡片背景
    HonestMixLookAndFeel::drawCardBackground(g, bounds);

    // 标题行
    auto title = titleArea;
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("HonestMix", title.removeFromLeft(title.getWidth() / 2).toFloat(),
               juce::Justification::centredLeft, true);

    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(7.0f));
    g.drawText(TRANS("翻译度已确认"), title.toFloat(),
               juce::Justification::centredRight, true);

    // 波形区
    auto waveArea = waveformArea.reduced(0, 4);
    g.setColour(HonestMixLookAndFeel::divider);
    g.fillRoundedRectangle(waveArea.toFloat(), 5.0f);

    // 简易波形
    int numBars = 16;
    int barW = waveArea.getWidth() / numBars;
    auto waveCentre = waveArea.getCentreY();
    g.setColour(HonestMixLookAndFeel::textSubtle);
    for (int i = 0; i < numBars; ++i)
    {
        float height = 4.0f + std::sin(i * 1.2f) * 8.0f + std::cos(i * 0.5f) * 4.0f;
        g.fillRect(waveArea.getX() + i * barW + 2,
                   waveCentre - height / 2.0f,
                   barW - 3, height);
    }

    // 工程名
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText(TRANS("《 未命名工程 · 2026.07 》"),
               waveformArea.toFloat(),
               juce::Justification::centredBottom, true);

    // 信息行
    auto info = infoArea;
    int colW = info.getWidth() / 3;
    struct InfoItem { juce::String value; juce::String label; };
    InfoItem items[3] = {
        { juce::String((int)appState.getWetDryMix()) + "%", "干湿比" },
        { appState.getHeadphoneModelName(), "耳机" },
        { {}, "目标" }  // 由工程师注入目标曲线名称
    };

    for (int i = 0; i < 3; ++i)
    {
        auto col = info.removeFromLeft(colW);

        g.setColour(HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(12.0f));
        g.drawText(items[i].value, col.removeFromTop(20).toFloat(),
                   juce::Justification::centred, true);

        g.setColour(HonestMixLookAndFeel::textSubtle);
        g.setFont(juce::FontOptions(7.0f));
        g.drawText(items[i].label, col.toFloat(),
                   juce::Justification::centred, true);
    }

    // 二维码
    drawQRCode(g, qrArea);

    // 说明文字
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(9.0f));
    g.drawText(TRANS("这首混音通过了 HonestMix 翻译度验证。"),
               textArea.removeFromTop(20).toFloat(),
               juce::Justification::centredLeft, true);
    g.setColour(HonestMixLookAndFeel::textSubtle);
    g.setFont(juce::FontOptions(7.0f));
    g.drawText(TRANS("免费 · 开源 · 社区驱动"),
               textArea.toFloat(),
               juce::Justification::centredLeft, true);
}

void ShareCard::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    // 标题
    titleArea = bounds.removeFromTop(24);

    // 波形区域
    waveformArea = bounds.removeFromTop(90);

    // 分隔线
    bounds.removeFromTop(4);
    // 信息区域
    infoArea = bounds.removeFromTop(44);

    // 分隔线
    bounds.removeFromTop(4);

    // 底部区域：二维码 + 文字 + 按钮
    auto bottom = bounds.removeFromTop(90);

    // 二维码 48x48
    qrArea = bottom.removeFromLeft(60);
    qrArea = qrArea.withSizeKeepingCentre(48, 48);

    // 说明文字
    textArea = bottom.removeFromLeft(bottom.getWidth() - 80);

    // 按钮
    buttonsArea = bounds.removeFromTop(32);
    int btnW = (buttonsArea.getWidth() - 8) / 2;
    saveButton.setBounds(buttonsArea.removeFromLeft(btnW));
    buttonsArea.removeFromLeft(8);
    shareButton.setBounds(buttonsArea);
}

//==============================================================================
void ShareCard::drawQRCode(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto qrBounds = bounds.reduced(2);
    g.setColour(HonestMixLookAndFeel::divider);
    g.fillRoundedRectangle(qrBounds.toFloat(), 4.0f);

    // 模拟 5x5 二维码矩阵
    int cellSize = qrBounds.getWidth() / 7;
    bool pattern[7][7] = {
        {1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1},
        {1,0,1,0,1,0,1},
        {1,0,0,0,0,0,1},
        {1,0,1,0,1,0,1},
        {1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1}
    };

    for (int y = 0; y < 7; ++y)
    {
        for (int x = 0; x < 7; ++x)
        {
            g.setColour(pattern[y][x]
                ? HonestMixLookAndFeel::textMuted.withAlpha(0.15f)
                : HonestMixLookAndFeel::textSubtle.withAlpha(0.04f));
            g.fillRect(qrBounds.getX() + x * cellSize + 1,
                       qrBounds.getY() + y * cellSize + 1,
                       cellSize - 1, cellSize - 1);
        }
    }
}
