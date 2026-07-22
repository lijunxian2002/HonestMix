#include "PluginEditor.h"

//==============================================================================
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor (HonestMixAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef_ (p)
{
    setSize (400, 300);
}

void HonestMixAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (43, 45, 50));
}

void HonestMixAudioProcessorEditor::resized()
{
}
