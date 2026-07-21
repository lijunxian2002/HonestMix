#include "PluginProcessor.h"
#include "PluginEditor.h"

HonestMixAudioProcessor::HonestMixAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

//==============================================================================
void HonestMixAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = 2;

    correctionEngine_.prepare (spec);
    correctionEngine_.setEnabled (true);
    setLatencySamples (correctionEngine_.getLatencySamples());
    mixer_.prepare (2, samplesPerBlock);

    filterL_.prepare (spec);
    filterR_.prepare (spec);
    updateSimulationFilters();
}

void HonestMixAudioProcessor::releaseResources()
{
    correctionEngine_.reset();
}

//==============================================================================
void HonestMixAudioProcessor::setListenMode (ListenMode m)
{
    listenMode_.store ((int) m);
    updateSimulationFilters();
}

void HonestMixAudioProcessor::updateSimulationFilters()
{
    auto bypass = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate_, sampleRate_ * 0.49f);
    auto iphone = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate_, 400.0f);
    auto car    = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate_, 100.0f, 0.7f, 6.0f);

    switch (getListenMode())
    {
    case ListenNormal: *filterL_.state = bypass; *filterR_.state = bypass; break;
    case ListeniPhone: *filterL_.state = iphone; *filterR_.state = iphone; break;
    case ListenCar:    *filterL_.state = car;    *filterR_.state = car;    break;
    default:           *filterL_.state = bypass; *filterR_.state = bypass; break;
    }
}

//==============================================================================
void HonestMixAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 宿主 BPM 跟随
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm(); bpm.hasValue() && *bpm > 0.0)
                hostBpm_.store ((float) *bpm);

    // 信号链：预存干 → FIR 卷积（始终 ON）→ 翻译度混合
    mixer_.captureDry (buffer);
    correctionEngine_.process (buffer);
    mixer_.mixInPlace (buffer);

    // 监听模式模拟（矫正之后叠加）
    const int numSamples = buffer.getNumSamples();
    const auto mode = getListenMode();

    if (mode == ListenMono)
    {
        auto* ch0 = buffer.getWritePointer (0);
        auto* ch1 = buffer.getWritePointer (1);
        for (int s = 0; s < numSamples; ++s)
        {
            float avg = (ch0[s] + ch1[s]) * 0.5f;
            ch0[s] = avg;
            ch1[s] = avg;
        }
    }
    else if (mode == ListeniPhone || mode == ListenCar)
    {
        juce::dsp::AudioBlock<float> block (buffer);
        auto ctx = juce::dsp::ProcessContextReplacing<float> (block);
        filterL_.process (ctx);
    }
}

//==============================================================================
juce::AudioProcessorEditor* HonestMixAudioProcessor::createEditor()
{
    return new HonestMixAudioProcessorEditor (*this);
}

//==============================================================================
void HonestMixAudioProcessor::setTranslationDegree (float v)
{
    mixer_.setTranslation (v);
    appState_.setTranslationDegree (v);
}

void HonestMixAudioProcessor::setProfileIndex (int idx)
{
    correctionEngine_.setProfile (idx);
    appState_.setProfileName (correctionEngine_.getProfileName (correctionEngine_.getCurrentProfile()));
}

void HonestMixAudioProcessor::setProfileByName (const juce::String& name)
{
    correctionEngine_.setProfileByName (name);
    appState_.setProfileName (correctionEngine_.getProfileName (correctionEngine_.getCurrentProfile()));
}

//==============================================================================
void HonestMixAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // 运行值先回流到 AppState（双保险：正常路径 UI 写入时已同步）
    appState_.setTranslationDegree (mixer_.getTranslation());
    appState_.setListenMode (listenMode_.load());

    if (auto xml = appState_.serialize().createXml())
        copyXmlToBinary (*xml, destData);
}

void HonestMixAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        appState_.deserialize (juce::ValueTree::fromXml (*xml));

        // 状态 → 运行模块（引擎未 prepare 时会延迟加载，安全）
        mixer_.setTranslation (appState_.getTranslationDegree());
        listenMode_.store (appState_.getListenMode());
        updateSimulationFilters();
        correctionEngine_.setProfileByName (appState_.getProfileName());
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HonestMixAudioProcessor();
}
