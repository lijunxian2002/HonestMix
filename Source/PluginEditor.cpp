#include "PluginEditor.h"
#include "Core/Design.h"

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), proc_ (p)
{
    setSize (600, 328);

    // ── 挂载所有组件 ──
    addAndMakeVisible (topBar_);
    addAndMakeVisible (bottomRow_);
    addAndMakeVisible (tooltip_);
    addAndMakeVisible (infoRow_);
    addAndMakeVisible (vuPanel_);
    addAndMakeVisible (fader_);
    addAndMakeVisible (cockpit_);

    cockpit_.addAndMakeVisible (breath_);
    cockpit_.addAndMakeVisible (curveGrid_);
    cockpit_.addAndMakeVisible (curveLabels_);
    cockpit_.addAndMakeVisible (curveCanvas_);
    cockpit_.addAndMakeVisible (starField_);

    addChildComponent (overlayBPM_);
    addChildComponent (overlayMonitor_);
    addChildComponent (overlayFeedback_);

    // ── 透明度：Tooltip 在最顶但默认透明 ──
    tooltip_.setInterceptsMouseClicks (false, false);

    // ── 信号连接 ──
    fader_.onValueChanged = [this](float v) {
        proc_.setTranslationDegree (v);
        topBar_.setStatusDotColour (hm::spectrumColor (v / 200.f));
        vuPanel_.setLevel (v / 200.f);
        curveCanvas_.setTranslationDegree (v);
    };
    fader_.setValue (proc_.getTranslationDegree());

    infoRow_.setNames (
        HeadphoneDatabase::shortName (proc_.getProfileName()),
        HeadphoneDatabase::getCurveNames()[curveIdx_],
        HeadphoneDatabase::getInterfaceNames()[interfaceIdx_]);

    infoRow_.onProfileSelected = [this](int) { /* TODO: 下拉选耳机 */ };
    infoRow_.onCurveSelected   = [this](int) { /* TODO */ };
    infoRow_.onInterfaceSelected = [this](int) { /* TODO */ };

    bottomRow_.onBPM      = [this] { toggleOverlay (overlayBPM_); };
    bottomRow_.onMonitor  = [this] { toggleOverlay (overlayMonitor_); };
    bottomRow_.onFeedback = [this] { toggleOverlay (overlayFeedback_); };

    overlayBPM_.onClose = [this] { overlayBPM_.setVisible (false); };
    overlayMonitor_.onAction = [this](int o) {
        overlayMonitor_.setVisible (false);
        switch (o) { case 0:proc_.setListenMode(ListenMono);break;case 1:proc_.setListenMode(ListeniPhone);break;case 2:proc_.setListenMode(ListenCar);break;default:proc_.setListenMode(ListenNormal);break; }
    };
    overlayFeedback_.onClose = [this] { overlayFeedback_.setVisible (false); };

    curveCanvas_.onHover = [this](float fx, float fy) {
        auto r = curveCanvas_.getBounds();
        float freq = 20 * std::pow (1000, fx / r.getWidth());
        float db = 12 - fy / r.getHeight() * 24;
        tooltip_.show (r.getX() + (int)fx, r.getY() + (int)fy - 24, freq, db, 0);
    };
    curveCanvas_.onLeave = [this] { tooltip_.hide(); };

    startTimerHz (20);
}

void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (26, 26, 24));
}

void HonestMixAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();
    b.removeFromTop (22); b.removeFromBottom (16);
    b.removeFromLeft (22); b.removeFromRight (20);

    // TopBar: h=19, margin-bottom=12
    topBar_.setBounds (b.removeFromTop (19));
    b.removeFromTop (12);

    // 预留给 InfoRow + BottomRow
    const int infoH = 50, bottomH = 28, gap = 10;
    const int mainGridH = b.getHeight() - infoH - gap - bottomH - gap;

    // MainGrid: Cockpit(左) + Fader(右=120), gap=18
    auto mainBounds = b.removeFromTop (mainGridH);
    auto cockpitBounds = mainBounds.removeFromLeft (mainBounds.getWidth() - 120 - 18);
    mainBounds.removeFromLeft (18); // gap
    cockpit_.setBounds (cockpitBounds);
    fader_.setBounds (mainBounds);  // 余下全给推子

    b.removeFromTop (gap);

    // InfoRow
    infoRow_.setBounds (b.removeFromTop (infoH));
    b.removeFromTop (gap);

    // BottomRow
    bottomRow_.setBounds (b.removeFromTop (bottomH));

    // Overlays（覆盖全窗口）
    auto ov = getLocalBounds();
    overlayBPM_.setBounds (ov);
    overlayMonitor_.setBounds (ov);
    overlayFeedback_.setBounds (ov);

    // Tooltip 的 bounds 由 show() 动态控制，这里不设

    // ── Cockpit 内部 ──
    auto cb = cockpit_.getLocalBounds().reduced (8);
    vuPanel_.setBounds (cb.removeFromTop (22));
    cb.removeFromTop (6);
    curveGrid_.setBounds (cb);
    curveLabels_.setBounds (cb);
    curveCanvas_.setBounds (cb.reduced (1));
    starField_.setBounds (cb.expanded (10));
    breath_.setBounds (cb.getRight() - 20, cb.getY() + 10, 3, 3);
}

void HonestMixAudioProcessorEditor::toggleOverlay (juce::Component& overlay, juce::Image*)
{
    const bool show = ! overlay.isVisible();
    overlayBPM_.setVisible (false);
    overlayMonitor_.setVisible (false);
    overlayFeedback_.setVisible (false);
    if (show) overlay.setVisible (true);
}

void HonestMixAudioProcessorEditor::timerCallback()
{
    float cur = proc_.getTranslationDegree();
    if (std::abs (cur - lastDegree_) > 0.5f) { lastDegree_ = cur; }
    float hb = proc_.getHostBpm();
    if (hb > 0 && std::abs (hb - lastHostBpm_) > 0.5f) { lastHostBpm_ = hb; overlayBPM_.setBPM ((int)(hb+0.5f)); }
}
