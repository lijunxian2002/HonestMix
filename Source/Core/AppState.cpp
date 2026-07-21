#include "Core/AppState.h"

const juce::Identifier AppState::tagRoot        { "HonestMix" };
const juce::Identifier AppState::propDegree     { "degree" };
const juce::Identifier AppState::propProfile    { "profile" };
const juce::Identifier AppState::propOnboarded  { "onboarded" };
const juce::Identifier AppState::propListen     { "listen" };
const juce::Identifier AppState::propCurve      { "curve" };
const juce::Identifier AppState::propInterface  { "interface" };
const juce::Identifier AppState::propMixHabit   { "mixHabit" };

AppState::AppState() : root_ (tagRoot)
{
    // 默认值与数据中心 setStateInformation 的缺省保持一致
    root_.setProperty (propDegree,    100.0f,                          nullptr);
    root_.setProperty (propProfile,   "Audio-Technica ATH-M50x",       nullptr);
    root_.setProperty (propOnboarded, false,                           nullptr);
    root_.setProperty (propListen,    0,                               nullptr);
    root_.setProperty (propCurve,     0,                               nullptr);
    root_.setProperty (propInterface, 0,                               nullptr);
    root_.setProperty (propMixHabit,  60,                              nullptr);
}

//==============================================================================
void AppState::setTranslationDegree (float v)
{
    root_.setProperty (propDegree, juce::jlimit (0.0f, 200.0f, v), nullptr);
    broadcaster_.sendChangeMessage();
}

float AppState::getTranslationDegree() const noexcept
{
    return (float) root_.getProperty (propDegree, 100.0f);
}

void AppState::setProfileName (const juce::String& name)
{
    root_.setProperty (propProfile, name, nullptr);
    broadcaster_.sendChangeMessage();
}

juce::String AppState::getProfileName() const
{
    return root_.getProperty (propProfile, "Audio-Technica ATH-M50x").toString();
}

void AppState::setOnboarded (bool v) noexcept { root_.setProperty (propOnboarded, v, nullptr); }
bool AppState::isOnboarded() const noexcept   { return (bool) root_.getProperty (propOnboarded, false); }

void AppState::setListenMode (int v) noexcept { root_.setProperty (propListen, v, nullptr); }
int  AppState::getListenMode() const noexcept { return (int) root_.getProperty (propListen, 0); }

void AppState::setCurveIndex (int v) noexcept { root_.setProperty (propCurve, v, nullptr); }
int  AppState::getCurveIndex() const noexcept { return (int) root_.getProperty (propCurve, 0); }

void AppState::setInterfaceIndex (int v) noexcept { root_.setProperty (propInterface, v, nullptr); }
int  AppState::getInterfaceIndex() const noexcept { return (int) root_.getProperty (propInterface, 0); }

void AppState::setMixHabitMinutes (int v) noexcept { root_.setProperty (propMixHabit, juce::jlimit (15, 480, v), nullptr); }
int  AppState::getMixHabitMinutes() const noexcept { return (int) root_.getProperty (propMixHabit, 60); }

//==============================================================================
juce::ValueTree AppState::serialize() const
{
    return root_.createCopy();
}

void AppState::deserialize (const juce::ValueTree& state)
{
    if (state.isValid() && state.hasType (tagRoot))
    {
        root_.copyPropertiesAndChildrenFrom (state, nullptr);
        broadcaster_.sendChangeMessage();
    }
}
