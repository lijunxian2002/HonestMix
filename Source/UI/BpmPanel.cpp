#include "UI/BpmPanel.h"
#include "Core/Design.h"

BpmPanel::BpmPanel()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);

    input_.setMultiLine (false);
    input_.setFont (hm::mono (22.0f));
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
    input_.setBounds (20, 44, getWidth() - 112, 44);
}

void BpmPanel::captureBackdrop()
{
    if (auto* p = getParentComponent())
    {
        setVisible (false);   // 快照不含自身
        backdrop_ = hm::blurredBackdrop (*p, getBounds(), 6.0f);
        setVisible (true);
        repaint();
    }
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
    const int w = getWidth();
    const int m = 20;                      // 左右边距（均衡布局基准）
    const float cw = (float) (w - 2 * m);  // 内容宽
    auto fx = [&] (float f) { return m + (int) (cw * f); };  // 比例列定位

    // ── 毛玻璃底：面板区域模糊快照 + 半透明卡片 ──
    if (backdrop_.isValid())
    {
        juce::Path clip;
        clip.addRoundedRectangle (getLocalBounds().toFloat(), 10.0f);
        g.saveState();
        g.reduceClipRegion (clip);
        g.drawImageAt (backdrop_, 0, 0);
        g.restoreState();
    }
    hm::drawCard (g, getLocalBounds().toFloat(), 10.0f, hm::panelBg());

    const auto mouse = getMouseXYRelative();
    const double ms = 60000.0 / bpm_;

    // ── 标题 + 收起（钉右上角）──
    tapRect_   = { w - m - 68, 44, 68, 44 };
    closeRect_ = { w - m - 52, 12, 52, 24 };

    g.setFont (15.0f);
    g.setColour (hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"BPM 助手"), m, 14, 160, 20, juce::Justification::centredLeft);

    const bool closeHov = closeRect_.contains (mouse);
    g.setColour (hm::whiteA (closeHov ? 0.10f : 0.05f));
    g.fillRoundedRectangle (closeRect_.toFloat(), 6.0f);
    g.setColour (hm::whiteA (closeHov ? 0.18f : 0.10f));
    g.drawRoundedRectangle (closeRect_.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setFont (11.0f);
    g.setColour (closeHov ? hm::textMain() : hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"收起"), closeRect_, juce::Justification::centred);

    // ── 输入框 + 按速度 ──
    g.setColour (hm::innerBg());
    g.fillRoundedRectangle (input_.getBounds().toFloat(), 6.0f);
    g.setColour (hm::border());
    g.drawRoundedRectangle (input_.getBounds().toFloat().reduced (0.5f), 6.0f, 1.0f);

    const bool flash = tapFlashMs_ > 0;
    g.setColour (hm::whiteA (flash ? 0.14f : (tapRect_.contains (mouse) ? 0.08f : 0.05f)));
    g.fillRoundedRectangle (tapRect_.toFloat(), 6.0f);
    g.setColour (hm::border());
    g.drawRoundedRectangle (tapRect_.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setFont (11.0f);
    g.setColour (flash ? hm::textMain() : hm::textSec());
    g.drawText (juce::String::fromUTF8 (u8"按速度"), tapRect_, juce::Justification::centred);

    // ── 双卡片：BPM / 每拍 ──
    auto miniCard = [&] (juce::Rectangle<int> r, const juce::String& label, const juce::String& val)
    {
        g.setColour (hm::innerBg());
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (hm::border());
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
        g.setFont (11.0f);
        g.setColour (hm::textLabel());
        g.drawText (label, r.getX() + 12, r.getY() + 7, r.getWidth() - 24, 14, juce::Justification::centredLeft);
        g.setFont (hm::mono (19.0f));
        g.setColour (hm::textSec());
        g.drawText (val, r.getX() + r.getWidth() - 12, r.getY() + 24, r.getWidth() - 24, 24, juce::Justification::right);
    };
    const int cardW = (w - 2 * m - 8) / 2;
    miniCard ({ m, 100, cardW, 52 }, "BPM", juce::String (bpm_));
    miniCard ({ m + cardW + 8, 100, w - m - (m + cardW + 8), 52 }, juce::String (u8"每拍"), juce::String ((int) (ms + 0.5)) + " ms");

    // ── 表格区（比例列：标签 0 / 值1 .17 / 注1 .40 / 值2 .58 / 注2 .81）──
    auto sectTitle = [&] (int y, const char* t)
    {
        g.setFont (12.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (t), m, y, (int) cw, 16, juce::Justification::centredLeft);
    };
    auto row2 = [&] (int y, const char* l, const juce::String& v1, const char* h1,
                                    const juce::String& v2, const char* h2)
    {
        g.setFont (11.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (l), fx (0.0f), y, fx (0.16f) - fx (0.0f), 18, juce::Justification::centredLeft);
        g.setFont (hm::mono (13.0f));
        g.setColour (hm::textSec());
        g.drawText (v1, fx (0.17f), y, fx (0.38f) - fx (0.17f), 18, juce::Justification::right);
        g.setFont (10.0f);
        g.setColour (hm::textLabel());
        g.drawText (juce::String::fromUTF8 (h1), fx (0.40f), y + 1, fx (0.53f) - fx (0.40f), 16, juce::Justification::centredLeft);
        if (v2.isNotEmpty())
        {
            g.setColour (hm::whiteA (0.12f));
            g.drawText ("|", fx (0.55f), y, fx (0.58f) - fx (0.55f), 18, juce::Justification::centred);
            g.setFont (hm::mono (13.0f));
            g.setColour (hm::textSec());
            g.drawText (v2, fx (0.59f), y, fx (0.79f) - fx (0.59f), 18, juce::Justification::right);
            g.setFont (10.0f);
            g.setColour (hm::textLabel());
            g.drawText (juce::String::fromUTF8 (h2), fx (0.81f), y + 1, fx (1.0f) - fx (0.81f), 16, juce::Justification::centredLeft);
        }
    };
    auto msFmt  = [] (double v) { return v >= 1000.0 ? juce::String (v / 1000.0, 2) + " s"
                                                     : juce::String ((int) (v + 0.5)) + " ms"; };

    sectTitle (162, u8"预延迟 PREDELAY");
    row2 (182, u8"短", msFmt (ms / 32), u8"贴脸", {}, "");
    row2 (201, u8"中", msFmt (ms / 16), u8"清晰", {}, "");
    row2 (220, u8"长", msFmt (ms / 8),  u8"空间", {}, "");

    sectTitle (240, u8"混响时间 REVERB");
    row2 (260, u8"房间 ROOM",  msFmt (ms * 0.5), u8"干练", msFmt (ms * 1.0), u8"自然");
    row2 (279, u8"板式 PLATE", msFmt (ms * 2.0), u8"明亮", msFmt (ms * 4.0), u8"饱满");
    row2 (298, u8"大厅 HALL",  msFmt (ms * 4.0), u8"辽阔", msFmt (ms * 8.0), u8"宏大");

    sectTitle (318, u8"延迟时间 DELAY");
    row2 (338, "1/2",  msFmt (ms),      u8"宽厚", {}, "");
    row2 (357, "1/4",  msFmt (ms / 2),  u8"回荡", {}, "");
    row2 (376, "1/8",  msFmt (ms / 4),  u8"律动", {}, "");
    row2 (395, "1/16", msFmt (ms / 8),  u8"点缀", {}, "");
    row2 (414, "1/64", msFmt (ms / 32), u8"镶边", {}, "");
}
