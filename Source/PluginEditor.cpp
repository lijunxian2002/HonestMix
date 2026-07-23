#include "PluginEditor.h"
#include "Core/Design.h"

static constexpr int kDesignW = 600, kDesignH = 328;
static constexpr int kMinW = 360, kMinH = 196;
static constexpr int kMaxW = 900, kMaxH = 492;  // 1.5×

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), proc_ (p)
{
    setResizable (true, true);
    setResizeLimits (kMinW, kMinH, kMaxW, kMaxH);
    setSize (kDesignW, kDesignH);

    // ── 挂载所有组件 ──
    addAndMakeVisible (topBar_);
    addAndMakeVisible (bottomRow_);
    addAndMakeVisible (tooltip_);
    addAndMakeVisible (infoRow_);
    addAndMakeVisible (fader_);
    addAndMakeVisible (cockpit_);

    // Cockpit 内部子组件
    vuPanel_     .setComponentID ("VUPanel");
    breath_      .setComponentID ("Breath");
    curveGrid_   .setComponentID ("CurveGrid");
    curveLabels_ .setComponentID ("CurveLabels");
    curveCanvas_ .setComponentID ("CurveCanvas");
    starField_   .setComponentID ("StarField");
    cockpit_.addAndMakeVisible (vuPanel_);
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
    const float s = (float) getWidth() / (float) kDesignW;

    auto b = getLocalBounds();
    b.removeFromTop (juce::roundToInt (22 * s));
    b.removeFromBottom (juce::roundToInt (16 * s));
    b.removeFromLeft  (juce::roundToInt (22 * s));
    b.removeFromRight (juce::roundToInt (20 * s));

    topBar_.setBounds (b.removeFromTop (juce::roundToInt (19 * s)));
    b.removeFromTop (juce::roundToInt (12 * s));

    const int infoH = juce::roundToInt (50 * s), bottomH = juce::roundToInt (28 * s);
    const int gap  = juce::roundToInt (10 * s);
    const int mainGridH = b.getHeight() - infoH - gap - bottomH - gap;

    auto mainBounds = b.removeFromTop (mainGridH);
    int faderW = juce::roundToInt ((120 + 18) * s);
    auto cockpitBounds = mainBounds.removeFromLeft (mainBounds.getWidth() - faderW);
    mainBounds.removeFromLeft (juce::roundToInt (18 * s));
    cockpit_.setBounds (cockpitBounds);
    fader_.setBounds (mainBounds);

    b.removeFromTop (gap);
    infoRow_.setBounds (b.removeFromTop (infoH));
    b.removeFromTop (gap);
    bottomRow_.setBounds (b.removeFromTop (bottomH));

    auto ov = getLocalBounds();
    overlayBPM_.setBounds (ov); overlayMonitor_.setBounds (ov); overlayFeedback_.setBounds (ov);

    // Cockpit 内部布局（由本 Editor 跨级控制，确保子组件一定能拿到 bounds）
    {
        auto cb = cockpit_.getLocalBounds().reduced (juce::roundToInt (8 * s));
        vuPanel_.setBounds (cb.removeFromTop (juce::roundToInt (22 * s)));
        cb.removeFromTop (juce::roundToInt (6 * s));
        curveGrid_.setBounds (cb); curveLabels_.setBounds (cb);
        curveCanvas_.setBounds (cb.reduced (1)); starField_.setBounds (cb.expanded (10));
        breath_.setBounds (cb.getRight() - juce::roundToInt(20*s), cb.getY() + juce::roundToInt(10*s), 3, 3);
    }
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
