#include "UI/SetupCard.h"
#include "Core/Design.h"

constexpr int SetupCard::kHabitMinutes[4];

SetupCard::SetupCard (const HeadphoneDatabase& db) : db_ (db)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);

    input_.setMultiLine (false);
    input_.setFont (juce::Font (juce::FontOptions (16.0f)));
    input_.setJustification (juce::Justification::centredLeft);
    input_.setIndents (12, 0);
    input_.setTextToShowWhenEmpty (juce::String::fromUTF8 (u8"输入耳机型号搜索 1603 数据库…"), hm::textDim());
    input_.setColour (juce::TextEditor::textColourId,       hm::textSec());
    input_.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    input_.setColour (juce::TextEditor::outlineColourId,    juce::Colours::transparentBlack);
    input_.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    input_.setColour (juce::TextEditor::highlightColourId,  hm::whiteA (0.15f));
    input_.setColour (juce::CaretComponent::caretColourId,  hm::whiteA (0.5f));
    input_.onTextChange = [this]
    {
        filter_ = input_.getText();
        rebuildResults();
        repaint();
    };
    addAndMakeVisible (input_);
}

//==============================================================================
// 布局：卡片 396×460 随画布水平+垂直居中；下一步按钮固定 y+404（与行不重叠）
juce::Rectangle<int> SetupCard::cardRect() const
{
    return { (getWidth() - 396) / 2, juce::jmax (12, (getHeight() - 460) / 2), 396, 460 };
}

void SetupCard::captureBackdrop()
{
    if (auto* p = getParentComponent())
    {
        setVisible (false);   // 快照不含自身
        backdrop_ = hm::blurredBackdrop (*p, getBounds(), 6.0f);
        setVisible (true);
        repaint();
    }
}

juce::Rectangle<int> SetupCard::rowRect (int i) const
{
    auto c = cardRect();
    return { c.getX() + 24, c.getY() + 120 + i * 34, c.getWidth() - 48, 30 };
}

juce::Rectangle<int> SetupCard::optRect (int i) const
{
    auto c = cardRect();
    return { c.getX() + 24, c.getY() + 72 + i * 48, c.getWidth() - 48, 40 };
}

void SetupCard::rebuildResults()
{
    results_.clear();
    selProfile_ = -1;
    if (filter_.isNotEmpty())
        db_.search (filter_, results_, kMaxRows);
}

//==============================================================================
void SetupCard::paint (juce::Graphics& g)
{
    // Apple 毛玻璃：先铺模糊底图，再压暗
    if (backdrop_.isValid())
        g.drawImageAt (backdrop_, 0, 0);
    g.setColour (hm::overlayBg());
    g.fillRect (getLocalBounds());

    auto c = cardRect();
    hm::drawCard (g, c.toFloat(), 12.0f, hm::panelBg());
    const auto mouse = getMouseXYRelative();
    const int x = c.getX(), y = c.getY(), w = c.getWidth();

    // ── 步骤 1/2 的返回键 ──
    if (step_ > 0)
    {
        backRect_ = { x + 20, y + 20, 64, 22 };
        const bool hov = backRect_.contains (mouse);
        g.setFont (13.0f);
        g.setColour (hov ? hm::textMain() : hm::textSec());
        g.drawText (juce::String::fromUTF8 (u8"‹ 返回"), backRect_, juce::Justification::centredLeft);
    }

    // ── 步骤指示（1/3 · 2/3 · 3/3）──
    g.setFont (10.0f);
    g.setColour (hm::textDim());
    g.drawText (juce::String (step_ + 1) + " / 3", x + 24, y + 24, w - 48, 16,
                juce::Justification::right);

    if (step_ == 0)
    {
        g.setFont (18.0f);
        g.setColour (hm::textMain());
        g.drawText (juce::String::fromUTF8 (u8"你在用什么耳机？"), x + 24, y + 44, w - 48, 24,
                    juce::Justification::centredLeft);

        // 输入框底盘（置顶，与结果行不重叠）
        g.setColour (hm::innerBg());
        g.fillRoundedRectangle (input_.getBounds().toFloat(), 6.0f);
        g.setColour (hm::border());
        g.drawRoundedRectangle (input_.getBounds().toFloat().reduced (0.5f), 6.0f, 1.0f);

        if (filter_.isEmpty())
        {
            // 热门预设（数据库直供全名，显示短名）
            auto& presets = HeadphoneDatabase::getPopularPresets();
            for (int i = 0; i < presets.size() && i < 8; ++i)
            {
                auto r = rowRect (i);
                const int idx = db_.findProfileByName (presets[i]);
                const bool sel = (selProfile_ == idx && idx >= 0);
                const bool hov = r.contains (mouse);
                if (sel || hov)
                {
                    g.setColour (hm::whiteA (sel ? 0.10f : 0.05f));
                    g.fillRoundedRectangle (r.toFloat(), 5.0f);
                }
                if (sel) { g.setColour (hm::accent()); g.fillEllipse (r.getX() + 6.f, r.getY() + 9.f, 12.f, 12.f); }
                else     { g.setColour (hm::borderStrong()); g.drawEllipse (r.getX() + 6.f, r.getY() + 9.f, 12.f, 12.f, 1.5f); }
                g.setFont (13.0f);
                g.setColour (sel ? hm::textMain() : (hov ? hm::textMain() : hm::textSec()));
                g.drawText (HeadphoneDatabase::shortName (presets[i]),
                            r.getX() + 26, r.getY(), r.getWidth() - 36, r.getHeight(),
                            juce::Justification::centredLeft);
            }
        }
        else
        {
            // 实时搜索结果
            for (int i = 0; i < (int) results_.size(); ++i)
            {
                auto r = rowRect (i);
                const int idx = results_[(size_t) i];
                const bool sel = (selProfile_ == idx);
                const bool hov = r.contains (mouse);
                if (sel || hov)
                {
                    g.setColour (hm::whiteA (sel ? 0.10f : 0.05f));
                    g.fillRoundedRectangle (r.toFloat(), 5.0f);
                }
                g.setFont (13.0f);
                g.setColour (sel ? hm::textMain() : (hov ? hm::textMain() : hm::textSec()));
                g.drawFittedText (juce::String (db_.getProfileName (idx)),
                                  r.getX() + 10, r.getY(), r.getWidth() - 20, r.getHeight(),
                                  juce::Justification::centredLeft, 1);
                if (sel)
                {
                    g.setColour (hm::accent());
                    g.drawText (juce::String::fromUTF8 (u8"✓"), r, juce::Justification::right);
                }
            }
            if (results_.empty())
            {
                g.setFont (12.0f);
                g.setColour (hm::textDim());
                g.drawText (juce::String::fromUTF8 (u8"无匹配型号"), rowRect (0),
                            juce::Justification::centredLeft);
            }
        }
    }
    else if (step_ == 1)
    {
        g.setFont (18.0f);
        g.setColour (hm::textMain());
        g.drawText (juce::String::fromUTF8 (u8"你通常一次混多久？"), x + 24, y + 44, w - 48, 24,
                    juce::Justification::centredLeft);

        static const char* habitName[] = { u8"30 分钟", u8"1 小时", u8"2 小时", u8"3 小时以上" };
        static const char* habitDesc[] = { u8"速战速决", u8"推荐节奏", u8"深度工作", u8"马拉松局" };
        for (int i = 0; i < 4; ++i)
        {
            auto r = optRect (i);
            const bool sel = (selHabit_ == i), hov = r.contains (mouse);
            if (sel || hov)
            {
                g.setColour (hm::whiteA (sel ? 0.10f : 0.05f));
                g.fillRoundedRectangle (r.toFloat(), 5.0f);
            }
            g.setFont (14.0f);
            g.setColour (sel ? hm::textMain() : (hov ? hm::textMain() : hm::textSec()));
            g.drawText (juce::String::fromUTF8 (habitName[i]), r.getX() + 12, r.getY() + 3,
                        r.getWidth() - 24, 20, juce::Justification::centredLeft);
            g.setFont (11.0f);
            g.setColour (hm::textLabel());
            g.drawText (juce::String::fromUTF8 (habitDesc[i]), r.getX() + 12, r.getY() + 23,
                        r.getWidth() - 24, 14, juce::Justification::centredLeft);
            if (sel)
            {
                g.setColour (hm::accent());
                g.drawText (juce::String::fromUTF8 (u8"✓"), r, juce::Justification::right);
            }
        }

        g.setFont (10.0f);
        g.setColour (hm::textDim());
        g.drawText (juce::String::fromUTF8 (u8"到点会提醒你换一次监听环境"),
                    x + 24, y + 280, w - 48, 16, juce::Justification::centredLeft);
    }
    else
    {
        g.setFont (18.0f);
        g.setColour (hm::textMain());
        g.drawText (juce::String::fromUTF8 (u8"目标曲线"), x + 24, y + 44, w - 48, 24,
                    juce::Justification::centredLeft);

        static const char* curOpts[] = { u8"Harman 2018 OE", u8"Diffuse Field", u8"Free Field", u8"Flat" };
        static const char* curDesc[] = { u8"科学盲听标准", u8"实验室参考", u8"自然声场", u8"直通" };
        for (int i = 0; i < 4; ++i)
        {
            auto r = optRect (i);
            const bool sel = (selCurve_ == i), hov = r.contains (mouse);
            if (sel || hov)
            {
                g.setColour (hm::whiteA (sel ? 0.10f : 0.05f));
                g.fillRoundedRectangle (r.toFloat(), 5.0f);
            }
            g.setFont (14.0f);
            g.setColour (sel ? hm::textMain() : (hov ? hm::textMain() : hm::textSec()));
            g.drawText (juce::String::fromUTF8 (curOpts[i]), r.getX() + 12, r.getY() + 3,
                        r.getWidth() - 24, 20, juce::Justification::centredLeft);
            g.setFont (11.0f);
            g.setColour (hm::textLabel());
            g.drawText (juce::String::fromUTF8 (curDesc[i]), r.getX() + 12, r.getY() + 23,
                        r.getWidth() - 24, 14, juce::Justification::centredLeft);
            if (sel)
            {
                g.setColour (hm::accent());
                g.drawText (juce::String::fromUTF8 (u8"✓"), r, juce::Justification::right);
            }
        }
    }

    // ── 下一步 / 开始使用（固定 y+404，不与任何行重叠）──
    const bool ok = (step_ == 0 ? selProfile_ >= 0 : true); // 步骤 1/2 均有默认选择
    nextRect_ = { x + 24, y + 404, w - 48, 36 };
    const bool hov = ok && nextRect_.contains (mouse);
    if (ok)
    {
        g.setColour (hm::accent().withAlpha (hov ? 0.7f : 0.5f));
        g.fillRoundedRectangle (nextRect_.toFloat(), 18.0f);
        g.setFont (15.0f);
        g.setColour (juce::Colours::white.withAlpha (hov ? 1.0f : 0.8f));
    }
    else
    {
        g.setColour (hm::whiteA (0.04f));
        g.fillRoundedRectangle (nextRect_.toFloat(), 18.0f);
        g.setFont (15.0f);
        g.setColour (hm::textDim());
    }
    g.drawText (juce::String::fromUTF8 (step_ == 2 ? u8"开始使用" : u8"下一步"),
                nextRect_, juce::Justification::centred);
}

//==============================================================================
void SetupCard::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();

    // 返回键
    if (step_ > 0 && backRect_.contains (p))
    {
        --step_;
        input_.setVisible (step_ == 0);
        repaint();
        return;
    }

    if (step_ == 0)
    {
        if (filter_.isEmpty())
        {
            auto& presets = HeadphoneDatabase::getPopularPresets();
            for (int i = 0; i < presets.size() && i < 8; ++i)
                if (rowRect (i).contains (p))
                {
                    selProfile_ = db_.findProfileByName (presets[i]);
                    if (selProfile_ >= 0 && onPickProfile)
                        onPickProfile (selProfile_);
                    repaint();
                    return;
                }
        }
        else
        {
            for (int i = 0; i < (int) results_.size(); ++i)
                if (rowRect (i).contains (p))
                {
                    selProfile_ = results_[(size_t) i];
                    if (onPickProfile)
                        onPickProfile (selProfile_);
                    repaint();
                    return;
                }
        }

        if (nextRect_.contains (p) && selProfile_ >= 0)
        {
            step_ = 1;
            input_.setVisible (false);
            repaint();
        }
    }
    else if (step_ == 1)
    {
        for (int i = 0; i < 4; ++i)
            if (optRect (i).contains (p))
            {
                selHabit_ = i;
                repaint();
                return;
            }
        if (nextRect_.contains (p))
        {
            step_ = 2;
            repaint();
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            if (optRect (i).contains (p))
            {
                selCurve_ = i;
                repaint();
                return;
            }
        if (nextRect_.contains (p) && onFinish)
            onFinish (kHabitMinutes[selHabit_], selCurve_);
    }
}

void SetupCard::mouseMove (const juce::MouseEvent&) { repaint(); }

void SetupCard::resized()
{
    auto c = cardRect();
    input_.setBounds (c.getX() + 24, c.getY() + 76, c.getWidth() - 48, 36);
}
