#include "Core/AppState.h"

const juce::Identifier AppState::tagRoot        { "HonestMix" };
const juce::Identifier AppState::propDegree     { "degree" };
const juce::Identifier AppState::propProfile    { "profile" };
const juce::Identifier AppState::propOnboarded  { "onboarded" };
const juce::Identifier AppState::propListen     { "listen" };
const juce::Identifier AppState::propCurve      { "curve" };
const juce::Identifier AppState::propInterface  { "interface" };
const juce::Identifier AppState::propMixHabit   { "mixHabit" };

AppState::AppState() : state_ (tagRoot)
{
    // 默认值与数据中心 setStateInformation 的缺省保持一致
    state_.setProperty (propDegree,    100.0f,                          nullptr);
    state_.setProperty (propProfile,   "Audio-Technica ATH-M50x",       nullptr);
    state_.setProperty (propOnboarded, false,                           nullptr);
    state_.setProperty (propListen,    0,                               nullptr);
    state_.setProperty (propCurve,     0,                               nullptr);
    state_.setProperty (propInterface, 0,                               nullptr);
    state_.setProperty (propMixHabit,  60,                              nullptr);

    // 默认值设完后才挂监听，避免构造阶段触发回调
    state_.addListener (this);
}

//==============================================================================
void AppState::setTranslationDegree (float v)
{
    state_.setProperty (propDegree, juce::jlimit (0.0f, 200.0f, v), nullptr);
}

float AppState::getTranslationDegree() const noexcept
{
    return (float) state_.getProperty (propDegree, 100.0f);
}

void AppState::setProfileName (const juce::String& name)
{
    state_.setProperty (propProfile, name, nullptr);
}

juce::String AppState::getProfileName() const
{
    return state_.getProperty (propProfile, "Audio-Technica ATH-M50x").toString();
}

void AppState::setOnboarded (bool v) noexcept  { state_.setProperty (propOnboarded, v, nullptr); }
bool AppState::isOnboarded() const noexcept    { return (bool) state_.getProperty (propOnboarded, false); }

void AppState::setListenMode (int v) noexcept  { state_.setProperty (propListen, v, nullptr); }
int  AppState::getListenMode() const noexcept  { return (int) state_.getProperty (propListen, 0); }

void AppState::setCurveIndex (int v) noexcept  { state_.setProperty (propCurve, v, nullptr); }
int  AppState::getCurveIndex() const noexcept  { return (int) state_.getProperty (propCurve, 0); }

void AppState::setInterfaceIndex (int v) noexcept { state_.setProperty (propInterface, v, nullptr); }
int  AppState::getInterfaceIndex() const noexcept  { return (int) state_.getProperty (propInterface, 0); }

void AppState::setMixHabitMinutes (int v) noexcept { state_.setProperty (propMixHabit, juce::jlimit (15, 480, v), nullptr); }
int  AppState::getMixHabitMinutes() const noexcept  { return (int) state_.getProperty (propMixHabit, 60); }

//==============================================================================
juce::ValueTree AppState::serialize() const
{
    return state_.createCopy();
}

void AppState::deserialize (const juce::ValueTree& state)
{
    if (state.isValid() && state.hasType (tagRoot))
    {
        // 临时移除自身监听，避免批量属性更新时频繁回调
        state_.removeListener (this);
        state_.copyPropertiesAndChildrenFrom (state, nullptr);
        state_.addListener (this);
    }
}

//==============================================================================
void AppState::valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& id)
{
    if (id == propDegree)
    {
        if (onTranslationDegreeChanged)
            onTranslationDegreeChanged (getTranslationDegree());
    }
    else if (id == propCurve)
    {
        if (onCurveIndexChanged)
            onCurveIndexChanged (getCurveIndex());
    }
    else if (id == propInterface)
    {
        if (onInterfaceIndexChanged)
            onInterfaceIndexChanged (getInterfaceIndex());
    }
    else if (id == propProfile)
    {
        if (onProfileNameChanged)
            onProfileNameChanged (getProfileName());
    }
    else if (id == propOnboarded)
    {
        if (onOnboardedChanged)
            onOnboardedChanged (isOnboarded());
    }
    else if (id == propListen)
    {
        if (onListenModeChanged)
            onListenModeChanged (getListenMode());
    }
    else if (id == propMixHabit)
    {
        if (onMixHabitMinutesChanged)
            onMixHabitMinutesChanged (getMixHabitMinutes());
    }
}
