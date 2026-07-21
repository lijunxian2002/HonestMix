#include "PluginEditor.h"

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef_ (p), setupCard_ (p.getDatabase())
{
    // 动态画布：屏宽 28% + 基准等比（常量集中在 Core/Design.h）
    auto& display = juce::Desktop::getInstance().getDisplays().getMainDisplay();
    const int screenW = display.logicalBounds.getWidth();
    PW_   = screenW * hm::kScreenWidthPercent / 100;
    scale_ = (float) PW_ / (float) hm::kRefWidth;
    PHC_  = (int) ((float) hm::kRefHeightNormal * scale_);
    PHE_  = (int) ((float) hm::kRefHeightExpand * scale_);

    addAndMakeVisible (content_);
    content_.setSize (PHE_, PHE_); // 逻辑画布（resized 时校正为 PW_ 宽）
    content_.setLookAndFeel (&honestLNF_);
    // 统一事件入口：content_ 及所有子组件的点击都汇聚到本类 mouseDown
    // （修复：原来只有 4 个 Label 挂 listener，背景/结果行点击被 content_ 吞掉）
    content_.addMouseListener (this, true);

    auto& db = processorRef_.getDatabase();

    // ── 主界面信息行（无标题、无版本号、无开关、无标签）──
    auto mkInfoLbl = [] (juce::Label& l, const char* t)
    {
        l.setText (juce::String::fromUTF8 (t), juce::dontSendNotification);
        l.setFont (juce::Font (juce::FontOptions (7.0f)));
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, hm::textLabel());
    };
    auto mkInfoVal = [] (juce::Label& l)
    {
        l.setFont (juce::Font (juce::FontOptions (11.0f)));
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, hm::textSec());
        l.setMinimumHorizontalScale (0.55f); // 长型号名自动缩窄
    };

    curveIdx_     = processorRef_.getAppState().getCurveIndex();
    interfaceIdx_ = processorRef_.getAppState().getInterfaceIndex();
    auto& curves = HeadphoneDatabase::getCurveNames();
    auto& ifs    = HeadphoneDatabase::getInterfaceNames();
    curveIdx_     = juce::jlimit (0, curves.size() - 1, curveIdx_);
    interfaceIdx_ = juce::jlimit (0, ifs.size() - 1, interfaceIdx_);

    mkInfoLbl (hpLbl_, u8"耳机"); mkInfoVal (hpVal_);
    mkInfoLbl (cvLbl_, u8"曲线"); mkInfoVal (cvVal_);
    cvVal_.setText (curves[curveIdx_], juce::dontSendNotification);
    mkInfoLbl (ifLbl_, u8"声卡"); mkInfoVal (ifVal_);
    ifVal_.setText (ifs[interfaceIdx_], juce::dontSendNotification);

    hpVal_.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    cvVal_.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    ifVal_.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    content_.addAndMakeVisible (hpLbl_); content_.addAndMakeVisible (hpVal_);
    content_.addAndMakeVisible (cvLbl_); content_.addAndMakeVisible (cvVal_);
    content_.addAndMakeVisible (ifLbl_); content_.addAndMakeVisible (ifVal_);

    // ── 旋杆（直写处理器翻译度，无 DAW 自动化）──
    knob_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob_.setLookAndFeel (&knobLNF_);
    knob_.setRotaryParameters (juce::MathConstants<float>::pi * 0.75f,
                               juce::MathConstants<float>::pi * 2.25f, true);
    knob_.onValueChange = [this]
    {
        processorRef_.setTranslationDegree ((float) knob_.getValue());
        knobVal_.setText (juce::String ((int) processorRef_.getTranslationDegree()),
                          juce::dontSendNotification);
    };
    knob_.setRange (0.0, 200.0, 1.0);
    knob_.setValue (processorRef_.getTranslationDegree(), juce::dontSendNotification);
    content_.addAndMakeVisible (knob_);

    knobVal_.setJustificationType (juce::Justification::centred);
    knobVal_.setColour (juce::Label::textColourId, hm::textSec());
    content_.addAndMakeVisible (knobVal_);

    bpmLbl_.setText ("BPM", juce::dontSendNotification);
    bpmLbl_.setJustificationType (juce::Justification::centred);
    bpmLbl_.setColour (juce::Label::textColourId, hm::textSec());
    bpmLbl_.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    content_.addAndMakeVisible (bpmLbl_);

    // ── 内联搜索框（点信息行时展开）──
    searchBox_.setTextToShowWhenEmpty (juce::String::fromUTF8 (u8"搜索型号..."), hm::textDim());
    searchBox_.setFont (juce::Font (juce::FontOptions (18.0f)));
    searchBox_.setJustification (juce::Justification::centredLeft);
    searchBox_.setIndents (16, 0);
    searchBox_.setColour (juce::TextEditor::textColourId,       hm::textSec());
    searchBox_.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    searchBox_.setColour (juce::TextEditor::outlineColourId,    juce::Colours::transparentBlack);
    searchBox_.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    searchBox_.setColour (juce::TextEditor::highlightColourId,  hm::whiteA (0.15f));
    searchBox_.onTextChange = [this]
    {
        const juce::String f = searchBox_.getText().toLowerCase();
        searchResults_.clear();
        searchSel_ = -1;
        if (searchField_ == 0)
        {
            auto& db = processorRef_.getDatabase();
            if (f.isNotEmpty())
                db.search (f, searchResults_, 64);
            else
                for (int i = 0, n = juce::jmin (50, db.getNumProfiles()); i < n; ++i)
                    searchResults_.push_back (i);   // 清空时回填前 50
        }
        repaint();
    };
    content_.addChildComponent (searchBox_);

    // ── 宠物「诚」──
    seal_.onClick = [this]
    {
        const bool show = ! feedbackCard_.isVisible();
        closeAllOverlays();
        if (show)
        {
            feedbackCard_.setVisible (true);
            fitWindow();
            feedbackCard_.captureBackdrop();
        }
        else fitWindow();
    };
    seal_.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    content_.addAndMakeVisible (seal_);

    // ── BPM 面板 ──
    content_.addChildComponent (bpmPanel_);
    bpmPanel_.onClose = [this] { bpmPanel_.setVisible (false); fitWindow(); };

    // ── 过渡卡（首次引导完成后出现一次，之后永不打扰）──
    content_.addChildComponent (transitionCard_);
    transitionCard_.onStart = [this] (int dw)
    {
        processorRef_.setOnboarded();
        processorRef_.setTranslationDegree ((float) dw);
        knob_.setValue (processorRef_.getTranslationDegree(), juce::dontSendNotification);
        knobVal_.setText (juce::String (dw), juce::dontSendNotification);
        transitionCard_.setVisible (false);
        fitWindow();
    };
    transitionCard_.onDismiss = [this] { transitionCard_.setVisible (false); fitWindow(); };

    // ── 反馈弹窗 ──
    content_.addChildComponent (feedbackCard_);
    feedbackCard_.onSubmit = [this] (int bass, int treble)
    {
        feedbackCard_.setVisible (false);
        fitWindow();
        submitFeedback (bass, treble);
    };
    feedbackCard_.onClose = [this] { feedbackCard_.setVisible (false); fitWindow(); };

    // ── 1 小时检查 ──
    content_.addChildComponent (checkCard_);
    checkCard_.onAction = [this] (int opt)
    {
        checkCard_.setVisible (false);
        switch (opt)
        {
            case 0: processorRef_.setListenMode (ListenMono);    break;
            case 1: processorRef_.setListenMode (ListeniPhone);  break;
            case 2: processorRef_.setListenMode (ListenCar);     break;
            case 3: processorRef_.setListenMode (ListenNormal);  break;
        }
        fitWindow();
    };
    checkCard_.onClose = [this] { checkCard_.setVisible (false); fitWindow(); };

    // ── 分享卡 ──
    content_.addChildComponent (shareCard_);
    shareCard_.onClose = [this] { shareCard_.setVisible (false); fitWindow(); };

    // ── 设置向导（首次：向导 → 过渡卡 → 主界面）──
    content_.addChildComponent (setupCard_);
    setupCard_.onPickProfile = [this] (int idx)
    {
        processorRef_.suspendProcessing (true);
        processorRef_.setProfileIndex (idx);
        processorRef_.suspendProcessing (false);
        auto& engine = processorRef_.getCorrectionEngine();
        hpVal_.setText (HeadphoneDatabase::shortName (
                            processorRef_.getDatabase().getProfileName (engine.getCurrentProfile())),
                        juce::dontSendNotification);
    };
    setupCard_.onFinish = [this] (int habitMinutes, int curveIdx)
    {
        processorRef_.getAppState().setMixHabitMinutes (habitMinutes);
        curveIdx_ = juce::jlimit (0, HeadphoneDatabase::getCurveNames().size() - 1, curveIdx);
        processorRef_.getAppState().setCurveIndex (curveIdx_);
        cvVal_.setText (HeadphoneDatabase::getCurveNames()[curveIdx_], juce::dontSendNotification);
        setupCard_.setVisible (false);
        transitionCard_.setVisible (true);
        fitWindow();
        transitionCard_.captureBackdrop();
    };
    setupCard_.setVisible (false);

    // ── 恢复耳机设置（全新实例时引擎默认在 0 号，须对齐 AppState 全名）──
    {
        processorRef_.suspendProcessing (true);
        processorRef_.setProfileByName (processorRef_.getProfileName());
        processorRef_.suspendProcessing (false);
        hpVal_.setText (HeadphoneDatabase::shortName (
                            db.getProfileName (processorRef_.getCorrectionEngine().getCurrentProfile())),
                        juce::dontSendNotification);
    }

    refreshShareInfo();
    setSize (PW_, PHC_);
    startTimerHz (20);
}

HonestMixAudioProcessorEditor::~HonestMixAudioProcessorEditor()
{
    knob_.setLookAndFeel (nullptr);
    content_.setLookAndFeel (nullptr);
}

//==============================================================================
void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (hm::bg());

    // 内联搜索结果绘制（不透明面板；打开时主界面元素已隐藏，不再撞车）
    if (searchField_ >= 0)
    {
        auto p = [this] (int v) { return (int) ((float) v * scale_); };
        const int x = p (12), y = p (80), w = PW_ - p (24), h = p (300);
        g.setColour (hm::bg());
        g.fillRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 12.0f);
        g.setColour (hm::borderStrong());
        g.drawRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 12.0f, 1.0f);

        auto& db = processorRef_.getDatabase();
        int iy = y + p (60);
        for (int i = 0; i < (int) searchResults_.size() && i < 12; ++i)
        {
            const int idx = searchResults_[(size_t) i];
            juce::String name;
            if      (searchField_ == 0) name = db.getProfileName (idx);
            else if (searchField_ == 1) name = HeadphoneDatabase::getCurveNames()[idx];
            else                        name = HeadphoneDatabase::getInterfaceNames()[idx];

            auto r = juce::Rectangle<int> (x + p (16), iy, w - p (32), p (36));
            const bool sel = searchSel_ == idx;
            if (sel)
            {
                g.setColour (hm::accent().withAlpha (0.08f));
                g.fillRoundedRectangle (r.toFloat(), 8.0f);
            }
            g.setFont ((float) p (16));
            g.setColour (sel ? hm::textMain() : hm::textSec());
            g.drawText (name, r.getX() + p (12), r.getY(), r.getWidth() - p (24), r.getHeight(),
                        juce::Justification::centredLeft);
            iy += p (40);
        }

        // 收起提示
        g.setFont ((float) p (9));
        g.setColour (hm::textDim());
        g.drawText (juce::String::fromUTF8 (u8"点击条目选择 · 点击空白收起"),
                    x, y + h - p (22), w, p (14), juce::Justification::centred);
    }
}

void HonestMixAudioProcessorEditor::resized()
{
    content_.setBounds (0, 0, PW_, PHE_);

    // ── Apple 舒适间距布局（基准 420 × scale_）──
    auto p = [this] (int v) { return (int) ((float) v * scale_); };

    hpLbl_.setFont (juce::FontOptions (p (15)));
    hpVal_.setFont (juce::FontOptions (p (18)));
    cvLbl_.setFont (juce::FontOptions (p (15)));
    cvVal_.setFont (juce::FontOptions (p (18)));
    ifLbl_.setFont (juce::FontOptions (p (15)));
    ifVal_.setFont (juce::FontOptions (p (18)));
    bpmLbl_.setFont (juce::FontOptions (p (17)));
    knobVal_.setFont (hm::mono ((float) p (44)));

    // 信息行: y16 标签 / y32 数值
    const int iy = p (16);
    hpLbl_.setBounds (0,         iy,           p (160), p (16));
    hpVal_.setBounds (0,         iy + p (18),  p (160), p (24));
    cvLbl_.setBounds (p (170),   iy,           p (80),  p (16));
    cvVal_.setBounds (p (170),   iy + p (18),  p (80),  p (24));
    ifLbl_.setBounds (p (260),   iy,           p (160), p (16));
    ifVal_.setBounds (p (260),   iy + p (18),  p (160), p (24));

    // 旋杆 — 留足呼吸
    const int ks = p (100);
    knob_.setBounds ((PW_ - ks) / 2, p (68), ks, ks);
    knobVal_.setBounds ((PW_ - p (120)) / 2, p (180), p (120), p (48));

    // BPM 左下 · 诚 右下（同 y、同尺寸、同边距 —— 和谐对称）
    bpmLbl_.setBounds (p (24), p (244), p (80), p (56));
    seal_.setBounds (PW_ - p (104), p (244), p (80), p (56));

    bpmPanel_.setBounds (p (16), p (16), p (388), p (570));

    // 搜索框（信息行下方）
    searchBox_.setBounds (p (12), p (80), PW_ - p (24), p (40));

    // 全屏遮罩层（随窗口实际大小，卡片在其内居中）
    for (auto* c : { (juce::Component*) &transitionCard_, (juce::Component*) &feedbackCard_,
                     (juce::Component*) &checkCard_, (juce::Component*) &shareCard_,
                     (juce::Component*) &setupCard_ })
        c->setBounds (getLocalBounds());

    // 未 onboarding → 显示引导页
    if (! processorRef_.isOnboarded() && ! setupCard_.isVisible() && ! transitionCard_.isVisible())
    {
        setupCard_.setVisible (true);
        fitWindow();
        setupCard_.captureBackdrop();
    }
}

void HonestMixAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    auto* c = e.eventComponent;

    // 输入框内的点击不触发任何面板动作
    if (c == &searchBox_)
        return;

    // 信息行 → 打开对应下拉
    if      (c == &hpVal_)  { openSearch (0); return; }
    else if (c == &cvVal_)  { openSearch (1); return; }
    else if (c == &ifVal_)  { openSearch (2); return; }
    else if (c == &bpmLbl_) { toggleBPM(); return; }

    // 下拉打开：命中结果行则选择并关闭
    if (searchField_ >= 0)
    {
        auto ep = e.getEventRelativeTo (this).getPosition();
        auto p = [this] (int v) { return (int) ((float) v * scale_); };
        int sy = p (80) + p (60);
        auto& db = processorRef_.getDatabase();

        for (int i = 0; i < (int) searchResults_.size() && i < 12; ++i)
        {
            auto r = juce::Rectangle<int> (p (28), sy, PW_ - p (56), p (36));
            if (r.contains (ep))
            {
                const int idx = searchResults_[(size_t) i];
                if (searchField_ == 0)
                {
                    processorRef_.suspendProcessing (true);
                    processorRef_.setProfileIndex (idx);
                    processorRef_.suspendProcessing (false);
                    hpVal_.setText (HeadphoneDatabase::shortName (db.getProfileName (idx)),
                                    juce::dontSendNotification);
                }
                else if (searchField_ == 1) // 曲线
                {
                    curveIdx_ = juce::jlimit (0, HeadphoneDatabase::getCurveNames().size() - 1, idx);
                    processorRef_.getAppState().setCurveIndex (curveIdx_);
                    cvVal_.setText (HeadphoneDatabase::getCurveNames()[curveIdx_],
                                    juce::dontSendNotification);
                }
                else if (searchField_ == 2) // 声卡
                {
                    interfaceIdx_ = juce::jlimit (0, HeadphoneDatabase::getInterfaceNames().size() - 1, idx);
                    processorRef_.getAppState().setInterfaceIndex (interfaceIdx_);
                    ifVal_.setText (HeadphoneDatabase::getInterfaceNames()[interfaceIdx_],
                                    juce::dontSendNotification);
                }
                refreshShareInfo();
                closeSearch();
                return;
            }
            sy += p (40);
        }
    }

    // 点击其它区域 → 关闭下拉
    closeSearch();
}

void HonestMixAudioProcessorEditor::openSearch (int field)
{
    searchField_ = field;
    searchSel_   = -1;
    searchBox_.clear();
    searchResults_.clear();

    // 隐藏被面板覆盖的主界面元素（修复：下拉与主界面撞车）
    knob_.setVisible (false);
    knobVal_.setVisible (false);
    bpmLbl_.setVisible (false);
    seal_.setVisible (false);

    auto& db = processorRef_.getDatabase();
    if (field == 0)
    {
        // 耳机：先展示前 50，输入即搜 1603 库
        const int n = juce::jmin (50, db.getNumProfiles());
        for (int i = 0; i < n; ++i)
            searchResults_.push_back (i);
        searchBox_.setVisible (true);
        searchBox_.grabKeyboardFocus();
    }
    else
    {
        // 曲线 / 声卡：固定表全量列出
        const int n = (field == 1 ? HeadphoneDatabase::getCurveNames().size()
                                  : HeadphoneDatabase::getInterfaceNames().size());
        for (int i = 0; i < n; ++i)
            searchResults_.push_back (i);
        searchBox_.setVisible (false);
    }

    fitWindow();   // 展开窗口，保证下拉在可视区内
    repaint();
}

void HonestMixAudioProcessorEditor::closeSearch()
{
    if (searchField_ < 0 && ! searchBox_.isVisible())
        return;
    searchField_ = -1;
    searchBox_.setVisible (false);
    searchResults_.clear();

    // 恢复主界面元素
    knob_.setVisible (true);
    knobVal_.setVisible (true);
    bpmLbl_.setVisible (true);
    seal_.setVisible (true);

    fitWindow();
    repaint();
}

//==============================================================================
// 按可见内容自适应窗口高度（不再一刀切拉到 645）
void HonestMixAudioProcessorEditor::fitWindow()
{
    auto p = [this] (int v) { return (int) ((float) v * scale_); };
    int h = PHC_;
    if (setupCard_.isVisible())      h = juce::jmax (h, 460 + 24);
    if (transitionCard_.isVisible()) h = juce::jmax (h, 400 + 24);
    if (feedbackCard_.isVisible())   h = juce::jmax (h, 408 + 24);
    if (checkCard_.isVisible())      h = juce::jmax (h, 310 + 24);
    if (shareCard_.isVisible())      h = juce::jmax (h, 354 + 24);
    if (bpmPanel_.isVisible())       h = juce::jmax (h, p (602));
    if (searchField_ >= 0)           h = juce::jmax (h, p (392));
    if (getWidth() != PW_ || getHeight() != h)
        setSize (PW_, h);
}

void HonestMixAudioProcessorEditor::toggleBPM()
{
    const bool show = ! bpmPanel_.isVisible();
    if (show)
    {
        const float hb = processorRef_.getHostBpm();
        if (hb > 0.0f) bpmPanel_.setBpm ((int) (hb + 0.5f));
    }
    bpmPanel_.setVisible (show);
    fitWindow();
}

void HonestMixAudioProcessorEditor::submitFeedback (int bass, int treble)
{
    auto& engine = processorRef_.getCorrectionEngine();
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("headphone", juce::var (engine.getProfileName (engine.getCurrentProfile())));
    obj->setProperty ("interface", juce::var (HeadphoneDatabase::getInterfaceNames()[interfaceIdx_]));
    obj->setProperty ("degree",    (double) processorRef_.getTranslationDegree());
    static const char* bassMap[]   = { "okay", "too_much", "too_little" };
    static const char* trebleMap[] = { "okay", "too_bright", "too_dark" };
    obj->setProperty ("bass_fb",   juce::var (bassMap [juce::jlimit (0, 2, bass)]));
    obj->setProperty ("treble_fb", juce::var (trebleMap[juce::jlimit (0, 2, treble)]));
    obj->setProperty ("comment",   juce::var (""));

    feedbackClient_.sendFeedback (juce::var (obj)); // 后台线程发送
}

void HonestMixAudioProcessorEditor::refreshShareInfo()
{
    auto& engine = processorRef_.getCorrectionEngine();
    const juce::String full (engine.getProfileName (engine.getCurrentProfile()));
    shareCard_.setInfo (full, HeadphoneDatabase::brandOf (full),
                        (int) processorRef_.getTranslationDegree());
}

void HonestMixAudioProcessorEditor::closeAllOverlays()
{
    feedbackCard_.setVisible (false);
    checkCard_.setVisible (false);
    shareCard_.setVisible (false);
}

void HonestMixAudioProcessorEditor::timerCallback()
{
    // 旋杆数值 — 只在变化时刷新
    const float cur = processorRef_.getTranslationDegree();
    if (std::abs (cur - lastDegreeShown_) > 0.5f)
    {
        lastDegreeShown_ = cur;
        knobVal_.setText (juce::String ((int) cur), juce::dontSendNotification);
        refreshShareInfo();
    }

    seal_.setDotOn (cur > 0.0f);

    // 宿主 BPM 自动跟随
    const float hb = processorRef_.getHostBpm();
    if (hb > 0.0f && std::abs (hb - lastHostBpm_) > 0.5f)
    {
        lastHostBpm_ = hb;
        if (bpmPanel_.isVisible())
            bpmPanel_.setBpm ((int) (hb + 0.5f));
    }

    // 换监听提醒：按向导选择的混音时长（分钟）触发，循环提醒
    if (! setupCard_.isVisible() && ! transitionCard_.isVisible() && ! checkCard_.isVisible())
    {
        const int target = processorRef_.getAppState().getMixHabitMinutes() * 60 * 20; // 20Hz
        if (++tick_ >= target)
        {
            tick_ = 0;
            checkCard_.setVisible (true);
            fitWindow();
            checkCard_.captureBackdrop();
        }
    }
}
