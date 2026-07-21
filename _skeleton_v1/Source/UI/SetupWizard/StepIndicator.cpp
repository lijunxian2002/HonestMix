#include "StepIndicator.h"
#include "../LookAndFeel.h"

//==============================================================================
StepIndicator::StepIndicator() {}

void StepIndicator::setCurrentStep(int step)
{
    currentStep = juce::jlimit(1, totalSteps, step);
    repaint();
}

void StepIndicator::setTotalSteps(int total)
{
    totalSteps = juce::jmax(1, total);
    repaint();
}

//==============================================================================
void StepIndicator::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto dotSpacing = bounds.getWidth() / static_cast<float>(totalSteps);
    auto centreY = bounds.getCentreY();

    for (int i = 0; i < totalSteps; ++i)
    {
        bool done = (i + 1) <= currentStep;

        g.setColour(done ? HonestMixLookAndFeel::textMuted
                         : HonestMixLookAndFeel::divider);

        auto dotX = dotSpacing * (i + 0.5f) - dotDiameter * 0.5f;
        g.fillEllipse(dotX, centreY - dotDiameter * 0.5f,
                      dotDiameter, dotDiameter);
    }
}

void StepIndicator::resized()
{
    // 自动适应父级宽度
}
