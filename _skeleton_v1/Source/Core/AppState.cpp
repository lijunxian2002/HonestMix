#include "AppState.h"

//==============================================================================
AppState::AppState(juce::ValueTree rootState)
    : root(rootState)
{
    jassert(root.hasType(tagRoot)); // 必须由 PluginProcessor 创建正确的 root
}

//==============================================================================
// ——— 阶段管理 ———

AppPhase AppState::getCurrentPhase() const noexcept
{
    return static_cast<AppPhase>(getIntProp(propPhase, 0));
}

void AppState::setPhase(AppPhase newPhase)
{
    if (getCurrentPhase() != newPhase)
    {
        setIntProp(propPhase, static_cast<int>(newPhase));
        broadcaster.sendChangeMessage();
    }
}

void AppState::reset()
{
    root.removeAllChildren(nullptr);
    root.removeAllProperties(nullptr);
    setIntProp(propPhase, static_cast<int>(AppPhase::Unconfigured));
    hourlyCheckHandled = false;
    broadcaster.sendChangeMessage();
}

//==============================================================================
// ——— 设置数据 ———

void AppState::setHeadphoneModel(int index)       { setIntProp(propHeadphone, index); }
int   AppState::getHeadphoneModel() const noexcept { return getIntProp(propHeadphone, -1); }

void AppState::setHeadphoneModelName(const juce::String& name) { setStrProp(propHeadphoneName, name); }
juce::String AppState::getHeadphoneModelName() const           { return getStrProp(propHeadphoneName, {}); }

void AppState::setAudioInterface(int index)       { setIntProp(propInterface, index); }
int   AppState::getAudioInterface() const noexcept { return getIntProp(propInterface, -1); }

void AppState::setAudioInterfaceName(const juce::String& name) { setStrProp(propInterfaceName, name); }
juce::String AppState::getAudioInterfaceName() const           { return getStrProp(propInterfaceName, {}); }

void AppState::setMixingHabit(int index)     { setIntProp(propHabit, index); }
int   AppState::getMixingHabit() const noexcept { return getIntProp(propHabit, -1); }

void AppState::setCheckOtherEnvironment(int index) { setIntProp(propCheckEnv, index); }
int   AppState::getCheckOtherEnvironment() const noexcept { return getIntProp(propCheckEnv, -1); }

bool AppState::isSetupComplete() const noexcept
{
    return getHeadphoneModel() >= 0
        && getAudioInterface() >= 0
        && getMixingHabit() >= 0;
}

//==============================================================================
// ——— 运行时数据 ———

void  AppState::setWetDryMix(float percent)       { setFloatProp(propWetDry, jlimit(0.0f, 100.0f, percent)); }
float AppState::getWetDryMix() const noexcept      { return getFloatProp(propWetDry, 50.0f); }

void  AppState::setCorrectionEnabled(bool enabled) { setIntProp(propCorrect, enabled ? 1 : 0); }
bool  AppState::isCorrectionEnabled() const noexcept { return getIntProp(propCorrect, 1) != 0; }

void  AppState::setBPM(int bpm)                    { setIntProp(propBPM, jlimit(20, 300, bpm)); }
int   AppState::getBPM() const noexcept            { return getIntProp(propBPM, 120); }

//==============================================================================
// ——— 混音计时 ———

void AppState::resetMixTimer()
{
    mixStartTime = juce::Time::getCurrentTime();
    hourlyCheckHandled = false;
}

double AppState::getMixElapsedMinutes() const
{
    return (juce::Time::getCurrentTime() - mixStartTime).inMinutes();
}

bool AppState::shouldShowHourlyCheck() const
{
    return !hourlyCheckHandled
        && getCurrentPhase() == AppPhase::Active
        && getMixElapsedMinutes() >= 60.0;
}

void AppState::dismissHourlyCheck()
{
    hourlyCheckHandled = true;
}

bool AppState::isHourlyCheckDismissed() const noexcept
{
    return hourlyCheckHandled;
}

//==============================================================================
// ——— 持久化 ———

juce::ValueTree AppState::serialize() const
{
    return root.createCopy();
}

void AppState::deserialize(const juce::ValueTree& state)
{
    root.copyPropertiesAndChildrenFrom(state, nullptr);
    hourlyCheckHandled = false;
    broadcaster.sendChangeMessage();
}

//==============================================================================
// ——— 辅助方法 ———

int AppState::getIntProp(const char* propName, int defaultVal) const
{
    return root.getProperty(propName, defaultVal);
}

void AppState::setIntProp(const char* propName, int value)
{
    if (root.getProperty(propName, -1) != value)
    {
        root.setProperty(propName, value, nullptr);
        broadcaster.sendChangeMessage();
    }
}

float AppState::getFloatProp(const char* propName, float defaultVal) const
{
    return root.getProperty(propName, defaultVal);
}

void AppState::setFloatProp(const char* propName, float value)
{
    root.setProperty(propName, value, nullptr);
    broadcaster.sendChangeMessage();
}

juce::String AppState::getStrProp(const char* propName, const juce::String& defaultVal) const
{
    return root.getProperty(propName, defaultVal);
}

void AppState::setStrProp(const char* propName, const juce::String& value)
{
    root.setProperty(propName, value, nullptr);
    broadcaster.sendChangeMessage();
}
