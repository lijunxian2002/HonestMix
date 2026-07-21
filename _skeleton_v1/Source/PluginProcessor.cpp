#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// 参数 ID 和名称
namespace ParamIDs
{
    static constexpr auto wetDryMix          = "wetDryMix";
    static constexpr auto correctionEnabled  = "correctionEnabled";
    static constexpr auto headphoneModel     = "headphoneModel";
    static constexpr auto audioInterface     = "audioInterface";
    static constexpr auto mixingHabit        = "mixingHabit";
    static constexpr auto bpmValue           = "bpmValue";
}

//==============================================================================
HonestMixAudioProcessor::HonestMixAudioProcessor()
    : AudioProcessor(BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "HonestMixParams", createParameterLayout())
{
    // 创建状态管理
    auto stateTree = juce::ValueTree("HonestMixState");
    appState = std::make_unique<AppState>(stateTree);

    // 创建耳机数据库
    headphoneDB = std::make_unique<HeadphoneDatabase>();
    headphoneDB->loadBuiltInCurves();

    // 创建 DSP 模块
    correctionProcessor = std::make_unique<CorrectionProcessor>();
    wetDryMixer = std::make_unique<WetDryMixer>();

    // 初始同步
    syncParametersToState();
}

HonestMixAudioProcessor::~HonestMixAudioProcessor() = default;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
HonestMixAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    params.add(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::wetDryMix, "干湿比",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f, "%"));

    params.add(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::correctionEnabled, "校正",
        true));

    params.add(std::make_unique<juce::AudioParameterInt>(
        ParamIDs::headphoneModel, "耳机型号", -1, 20, -1));

    params.add(std::make_unique<juce::AudioParameterInt>(
        ParamIDs::audioInterface, "声卡型号", -1, 20, -1));

    params.add(std::make_unique<juce::AudioParameterInt>(
        ParamIDs::mixingHabit, "混音习惯", -1, 10, -1));

    params.add(std::make_unique<juce::AudioParameterInt>(
        ParamIDs::bpmValue, "BPM", 20, 300, 120));

    return params;
}

//==============================================================================
void HonestMixAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels      = static_cast<juce::uint32>(getTotalNumOutputChannels());

    correctionProcessor->prepare(spec);
    wetDryMixer->prepare(spec);

    needsConfigRefresh = true;
}

void HonestMixAudioProcessor::releaseResources()
{
    correctionProcessor->reset();
    wetDryMixer->reset();
}

void HonestMixAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 同步状态
    if (needsConfigRefresh)
    {
        syncParametersToState();
        syncStateToDSP();
        needsConfigRefresh = false;
    }

    // 湿信号 Buffer（校正后）
    juce::AudioBuffer<float> wetBuffer(buffer.getNumChannels(),
                                        buffer.getNumSamples());
    wetBuffer.makeCopyOf(buffer);

    // 应用校正
    correctionProcessor->process(wetBuffer);

    // 干湿混合
    wetDryMixer->process(buffer, wetBuffer, buffer);
}

//==============================================================================
juce::AudioProcessorEditor* HonestMixAudioProcessor::createEditor()
{
    return new HonestMixAudioProcessorEditor(*this);
}

//==============================================================================
void HonestMixAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // 保存 APVTS 状态
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HonestMixAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // 恢复 APVTS 状态
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
        needsConfigRefresh = true;
    }
}

//==============================================================================
bool HonestMixAudioProcessor::isConfigured() const
{
    return appState->isSetupComplete();
}

void HonestMixAudioProcessor::resetConfiguration()
{
    appState->reset();
    needsConfigRefresh = true;
}

//==============================================================================
void HonestMixAudioProcessor::syncParametersToState()
{
    appState->setWetDryMix(
        apvts.getRawParameterValue(ParamIDs::wetDryMix)->load());

    appState->setCorrectionEnabled(
        apvts.getRawParameterValue(ParamIDs::correctionEnabled)->load() > 0.5f);

    appState->setHeadphoneModel(
        static_cast<int>(apvts.getRawParameterValue(ParamIDs::headphoneModel)->load()));

    appState->setAudioInterface(
        static_cast<int>(apvts.getRawParameterValue(ParamIDs::audioInterface)->load()));

    appState->setMixingHabit(
        static_cast<int>(apvts.getRawParameterValue(ParamIDs::mixingHabit)->load()));

    appState->setBPM(
        static_cast<int>(apvts.getRawParameterValue(ParamIDs::bpmValue)->load()));
}

void HonestMixAudioProcessor::syncStateToDSP()
{
    correctionProcessor->setEnabled(appState->isCorrectionEnabled());
    wetDryMixer->setMixPercent(appState->getWetDryMix());

    // 加载当前耳机的校正曲线
    auto hpIdx = appState->getHeadphoneModel();
    const auto& presets = Settings::getHeadphonePresets();
    auto hpName = (hpIdx >= 0 && hpIdx < presets.size()) ? presets[hpIdx].curveId
                               : juce::String{};

    if (hpName.isNotEmpty() && headphoneDB->hasCurve(hpName))
    {
        auto* curve = headphoneDB->getCurveById(hpName);
        if (curve != nullptr)
        {
            correctionProcessor->loadCurve(*curve, getSampleRate());
        }
    }
    else
    {
        correctionProcessor->clearCurve();
    }
}

//==============================================================================
// 插件入口宏
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HonestMixAudioProcessor();
}
