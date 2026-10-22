#pragma once
#include "PluginProcessor.h"
#include "UI/Skin.h"
#include "UI/TempoModel.h"
#include "UI/HeadphonePrediction.h"
#include <array>
#include <future>
class MechKey final : public juce::TextButton {
public:
    explicit MechKey(const juce::String& s) : juce::TextButton(s) {}
    void paintButton(juce::Graphics&,bool,bool) override;
};
class MechFader final : public juce::Slider {
public:
    MechFader();
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
    bool dragging() const noexcept { return dragging_; }
    float capY() const noexcept;
private:
    double startValue_ = 100;
    double lastDragY_ = 0;
    bool dragging_ = false;
};
class HonestMixAudioProcessorEditor final : public juce::AudioProcessorEditor,
    private juce::Timer, private juce::ListBoxModel {
public:
    explicit HonestMixAudioProcessorEditor(HonestMixAudioProcessor&);
    ~HonestMixAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    static juce::Point<int> designSize() { return {1000,480}; }
    void paintSurfaceAtScale(juce::Graphics&,int,juce::Rectangle<int>&);
    juce::Image renderSurface(int,juce::Rectangle<int>&);
    void refreshFromProcessor();
    static juce::Rectangle<float> plotBounds() { return {72,118,640,224}; }
    bool isTempoView() const noexcept { return view_ == View::tempo; }
    double displayedTempo() const noexcept;
    double hoveredHz() const noexcept { return hoverHz_; }
    double displayedResponseDb(double hz)const noexcept {return responseReady_?hm::prediction::evaluate(responseState_,hz):std::numeric_limits<double>::quiet_NaN();}
    bool predictionReady()const noexcept {return responseReady_;}
    const std::vector<juce::Point<float>>& plottedPrediction()const noexcept {return curve_;}
    void advanceAnimation(double seconds);
    struct AnimationSnapshot {
        float graphOpacity;
        bool graphHovered, reducedMotion;
        juce::Point<float> follower;
        double elapsedSeconds;
    };
    AnimationSnapshot animationSnapshot() const noexcept {return {graphOpacity_,graphHovered_,reducedMotion_,chase_,animationTime_};}
    juce::PopupMenu makeOptionsMenu() const;
    void applyOption(int id);
private:
    enum class View { normal, browse, tempo };
    struct Surface final : juce::Component, juce::TooltipClient {
        explicit Surface(HonestMixAudioProcessorEditor& e):owner(e){}
        void paint(juce::Graphics& g) override { owner.paintSurface(g); }
        void mouseEnter(const juce::MouseEvent& e) override { owner.pointerMove(e.position); }
        void mouseMove(const juce::MouseEvent& e) override { owner.pointerMove(e.position); }
        void mouseExit(const juce::MouseEvent&) override { owner.pointerExit(); }
        void mouseDown(const juce::MouseEvent& e) override { owner.surfaceClick(e.position,e.mods); }
        juce::String getTooltip() override {
            if(owner.view_==View::normal&&juce::Rectangle<int>(908,438,64,28).contains(getMouseXYRelative()))
                return juce::String::fromUTF8("Updates and feedback / 更新与问题反馈");
            if(owner.view_==View::normal&&owner.heldOutputPeak_>1&&juce::Rectangle<int>(738,442,160,20).contains(getMouseXYRelative()))
                return juce::String::fromUTF8("Output sample peak above 0 dBFS; held 1.5 s. / 输出采样峰值超零，保持1.5秒。");
            return {};
        }
        HonestMixAudioProcessorEditor& owner;
    } surface_{*this};
    struct Look final : juce::LookAndFeel_V4 {
        juce::Font getTextButtonFont(juce::TextButton&,int) override {return hm::skin::uiFont(13);}
        juce::Font getLabelFont(juce::Label&) override {return hm::skin::uiFont(15);}
        juce::Font getComboBoxFont(juce::ComboBox&) override {return hm::skin::uiFont(13);}
        juce::Font getPopupMenuFont() override {return juce::Font("Microsoft YaHei UI",14.0f,juce::Font::plain);}
        juce::Font getAlertWindowFont() override {return juce::Font("Microsoft YaHei UI",14.0f,juce::Font::plain);}
        juce::Font getAlertWindowTitleFont() override {return juce::Font("Microsoft YaHei UI",18.0f,juce::Font::plain);}
        juce::Font getAlertWindowMessageFont() override {return juce::Font("Microsoft YaHei UI",15.0f,juce::Font::plain);}
        void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    } look_;
    void timerCallback() override;
    void paintSurface(juce::Graphics&);
    void paintStars(juce::Graphics&);
    void paintPlot(juce::Graphics&);
    void paintTempo(juce::Graphics&);
    void setView(View);
    void pointerMove(juce::Point<float>);
    void pointerExit();
    void surfaceClick(juce::Point<float>,juce::ModifierKeys);
    void showListen();
    void openUpdateWebsite();
    void openFeedbackWebsite();
    void saveTempoPreferences();
    void restoreTempoPreferences();
    void filterProfiles();
    void selectProfile();
    void rebuildResponse();
    int getNumRows() override {return (int)results_.size();}
    void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
    void selectedRowsChanged(int) override;
    void listBoxItemDoubleClicked(int,const juce::MouseEvent&) override {selectProfile();}
    void returnKeyPressed(int) override {selectProfile();}
    HonestMixAudioProcessor& proc_;
    MechFader degree_;
    MechKey profile_{"Select headphone"}, compare_{"COMPARE"}, bpm_{"TEMPO"};
    MechKey back_{"BACK"}, apply_{"USE HEADPHONE"}, source_{"HOST"};
    MechKey straight_{"STRAIGHT"}, dotted_{"DOTTED"}, triplet_{"TRIPLET"};
    juce::TextEditor search_, tempoInput_;
    juce::ListBox list_;
    juce::TooltipWindow tooltip_{this,650};
    View view_=View::normal;
    std::vector<int> results_;
    std::vector<juce::Point<float>> curve_,targetCurve_;
    bool curveBelowScale_=false,curveAboveScale_=false;
    CorrectionResponseSnapshot responseState_;
    hm::prediction::Basis basis_;
    std::future<hm::prediction::Basis> basisJob_;
    std::uint64_t basisGeneration_=~std::uint64_t(0),basisIncoming_=~std::uint64_t(0);
    juce::String activeName_, selectionName_;
    juce::String manualText_{"120"};
    float previousDegree_=100, lastDegree_=-1, appliedDegree_=100;
    float heldOutputPeak_=0;
    double outputPeakHoldUntil_=0;
    std::uint32_t outputPeakResetGeneration_=0;
    double lastRate_=0, lastTick_=0, animationTime_=0, refreshAccumulator_=0, manualTempo_=120;
    double trailSampleAccumulator_=0;
    float graphOpacity_=0;
    int lastProfile_=-1;
    int lastListenMode_=-1;
    bool hostSync_=true, pointerInside_=false, graphHovered_=false, reducedMotion_=false, responseReady_=false;
    bool processorGesture_=false;
    bool lastBypassed_=false;
    bool lastAudioReady_=false;
    double lastProfileBlend_=-1;
    juce::String responseError_;
    juce::String updateNotice_;
    double updateNoticeUntil_=0;
    bool tempoSourceChosen_=false;
    std::uint64_t tempoPreferencesGeneration_=~std::uint64_t(0);
    float renderScaleOverride_=0;
    double basisRequestRate_=0;
    double hoverHz_=0;
    juce::Point<float> pointer_{390,220}, chase_{390,220}, parallax_;
    std::array<juce::Point<float>,5> starTrail_{};
    hm::tempo::Feel feel_=hm::tempo::Feel::straight;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HonestMixAudioProcessorEditor)
};
