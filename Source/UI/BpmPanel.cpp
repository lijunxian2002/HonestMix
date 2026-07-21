#include "UI/BpmPanel.h"
#include "Core/Design.h"

BpmPanel::BpmPanel()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);

    input_.setMultiLine (false);
    input_.setFont (hm::mono (24.0f));
    input_.setText ("117", juce::dontSendNotification);
    input_.setJustification (juce::Justification::centredLeft);
    input_.setIndents (12, 0);
    input_.setInputRestrictions (3, "0123456789");
    input_.setColour (juce::TextEditor::textColourId,       hm::textSec());
    input_.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    input_.setColour (juce::TextEditor::outlineColourId,    juce::Colours::transparentBlack);
    input_.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    input_.setColour (juce::TextEditor::highlightColourId,  hm::whiteA (0.15f));
    input_.setColour (juce::CaretComponent::caretColourId,  hm::whiteA (0.5f));
    input_.onTextChange = [this]
    {
        const int v = input_.getText().getIntValue();
        if (v >= 40 && v <= 240) { bpm_ = v; repaint(); }
    };
    input_.onReturnKey = [this] { input_.giveAwayKeyboardFocus(); };
    addAndMakeVisible (input_);
}

void BpmPanel::setBpm (int bpm)
{
    bpm_ = juce::jlimit (40, 240, bpm);
    if (! input_.hasKeyboardFocus (false))
        input_.setText (juce::String (bpm_), juce::dontSendNotification);
    repaint();
}

void BpmPanel::resized()
{
    input_.setBounds (21, 45, 240, 45);
}

void BpmPanel::visibilityChanged()
{
    if (isVisible()) startTimerHz (30); else stopTimer();
}

void BpmPanel::timerCallback()
{
    if (tapFlashMs_ > 0 && juce::Time::getCurrentTime().toMilliseconds() - tapFlashMs_ > 150)
    {
        tapFlashMs_ = 0;
        repaint();
    }
}

void BpmPanel::handleTap()
{
    auto now = juce::Time::getCurrentTime();
    taps_.add (now);
    if (taps_.size() > 8) taps_.remove (0);
    if (taps_.size() >= 3)
    {
        const auto ms = (taps_.getLast().toMilliseconds() - taps_.getFirst().toMilliseconds()) / (taps_.size() - 1);
        if (ms > 0) setBpm (juce::jlimit (40, 240, (int) (60000.0 / (double) ms + 0.5)));
    }
    tapFlashMs_ = now.toMilliseconds();
    repaint();
}

void BpmPanel::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();
    if (tapRect_.contains (p))        handleTap();
    else if (closeRect_.contains (p) && onClose) onClose();
}

void BpmPanel::paint (juce::Graphics& g)
{
    hm::drawCard (g, getLocalBounds().toFloat(), 10.0f, hm::panelBg());
    const auto mouse = getMouseXYRelative();
    const double ms = 60000.0 / bpm_;

    // 碰撞矩形（paint 内赋值，与数据中心一致）
    tapRect_   = { 270, 45, 105, 45 };
    closeRect_ = { 330, 10, 52, 24 };

    g.setFont (16.0f);
    g.setColour (hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"BPM 助手"), 21, 12, 180, 21, juce::Justification::centredLeft);

    // 关闭按钮：描边胶囊（修复：原 textDim 0.06 肉眼不可见）
    const bool closeHov = closeRect_.contains (mouse);
    g.setColour (hm::whiteA (closeHov ? 0.10f : 0.05f));
    g.fillRoundedRectangle (closeRect_.toFloat(), 6.0f);
    g.setColour (hm::whiteA (closeHov ? 0.18f : 0.10f));
    g.drawRoundedRectangle (closeRect_.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setFont (10.0f);
    g.setColour (closeHov ? hm::textMain() : hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"收起"), closeRect_, juce::Justification::centred);

    // 输入框底 + 按速度按钮
    g.setColour (hm::innerBg());
    g.fillRoundedRectangle (input_.getBounds().toFloat(), 6.0f);
    g.setColour (hm::border());
    g.drawRoundedRectangle (input_.getBounds().toFloat().reduced (0.5f), 6.0f, 1.0f);

    const bool flash = tapFlashMs_ > 0;
    g.setColour (hm::whiteA (flash ? 0.14f : (tapRect_.contains (mouse) ? 0.08f : 0.05f)));
    g.fillRoundedRectangle (tapRect_.toFloat(), 6.0f);
    g.setColour (hm::border());
    g.drawRoundedRectangle (tapRect_.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setFont (10.0f);
    g.setColour (flash ? hm::textMain() : hm::textLabel());
    g.drawText (juce::String::fromUTF8 (u8"按速度"), tapRect_, juce::Justification::centred);

    // 双卡片：BPM / 每拍
    auto miniCard = [&] (juce::Rectangle<int> r, const char* label, const juce::String& val)
    {
        g.setColour (hm::innerBg());
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (hm::border());
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
        g.setFont (10.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (label), r.getX() + 12, r.getY() + 8, r.getWidth() - 24, 14, juce::Justification::centredLeft);
        g.setFont (hm::mono (19.0f));
        g.setColour (hm::textSec());
        g.drawText (val, r.getX() + 12, r.getY() + 25, r.getWidth() - 24, 24, juce::Justification::centredLeft);
    };
    miniCard ({ 21, 102, 172, 54 }, "BPM", juce::String (bpm_));
    miniCard ({ 203, 102, 172, 54 }, "Beat", juce::String ((int) (ms + 0.5)) + " ms");

    // 表格区
    auto sectTitle = [&] (int y, const char* t)
    {
        g.setFont (11.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (t), 21, y, 354, 18, juce::Justification::centredLeft);
    };
    auto row2 = [&] (int y, const char* l, const juce::String& v1, const char* h1,
                                    const juce::String& v2, const char* h2)
    {
        g.setFont (10.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (l), 21, y, 70, 18, juce::Justification::centredLeft);
        g.setFont (hm::mono (11.0f));
        g.setColour (hm::textSec());
        g.drawText (v1, 91, y, 76, 18, juce::Justification::centredLeft);
        g.setFont (9.0f);
        g.setColour (hm::textDim());
        g.drawText (juce::String::fromUTF8 (h1), 171, y + 1, 56, 16, juce::Justification::centredLeft);
        if (v2.isNotEmpty())
        {
            g.setColour (hm::whiteA (0.10f));
            g.drawText ("|", 231, y, 14, 18, juce::Justification::centred);
            g.setFont (hm::mono (11.0f));
            g.setColour (hm::textSec());
            g.drawText (v2, 249, y, 76, 18, juce::Justification::centredLeft);
            g.setFont (9.0f);
            g.setColour (hm::textDim());
            g.drawText (juce::String::fromUTF8 (h2), 329, y + 1, 50, 16, juce::Justification::centredLeft);
        }
    };
    auto msFmt  = [] (double v) { return v >= 1000.0 ? juce::String (v / 1000.0, 2) + " s"
                                                     : juce::String ((int) (v + 0.5)) + " ms"; };

    sectTitle (168, u8"预延迟 PREDELAY");
    row2 (189, u8"短", msFmt (ms / 32), u8"贴脸", {}, "");
    row2 (209, u8"中", msFmt (ms / 16), u8"清晰", {}, "");
    row2 (229, u8"长", msFmt (ms / 8),  u8"空间", {}, "");

    sectTitle (258, u8"混响时间 REVERB");
    row2 (279, u8"房间 ROOM",  msFmt (ms * 0.5), u8"干练", msFmt (ms * 1.0), u8"自然");
    row2 (299, u8"板式 PLATE", msFmt (ms * 2.0), u8"明亮", msFmt (ms * 4.0), u8"饱满");
    row2 (319, u8"大厅 HALL",  msFmt (ms * 4.0), u8"辽阔", msFmt (ms * 8.0), u8"宏大");

    sectTitle (348, u8"压缩释放 COMPRESSOR");
    row2 (369, u8"快速", msFmt (ms / 16), u8"灵活", msFmt (ms / 8), u8"紧实");
    row2 (389, u8"中速", msFmt (ms / 4),  u8"自然", msFmt (ms / 2), u8"流畅");
    row2 (409, u8"慢速", msFmt (ms),      u8"平滑", msFmt (ms * 2), u8"沉稳");

    sectTitle (438, u8"延迟时间 DELAY");
    row2 (459, "1/2",  msFmt (ms),      u8"宽厚", {}, "");
    row2 (479, "1/4",  msFmt (ms / 2),  u8"回荡", {}, "");
    row2 (499, "1/8",  msFmt (ms / 4),  u8"律动", {}, "");
    row2 (519, "1/16", msFmt (ms / 8),  u8"点缀", {}, "");
    row2 (539, "1/64", msFmt (ms / 32), u8"镶边", {}, "");
}
