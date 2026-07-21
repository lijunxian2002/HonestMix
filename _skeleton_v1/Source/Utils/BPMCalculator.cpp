#include "BPMCalculator.h"

//==============================================================================
BPMCalculator::BPMCalculator(int bpm)
    : bpm(juce::jlimit(20, 300, bpm)) {}

//==============================================================================
void BPMCalculator::setBPM(int newBPM)
{
    bpm = juce::jlimit(20, 300, newBPM);
}

//==============================================================================
double BPMCalculator::getBeatDurationMs() const noexcept
{
    return 60000.0 / static_cast<double>(bpm);
}

double BPMCalculator::getTwoBeatsMs() const noexcept
{
    return getBeatDurationMs() * 2.0;
}

double BPMCalculator::getQuarterMs() const noexcept
{
    return getBeatDurationMs();
}

//==============================================================================
double BPMCalculator::getDelayMs(double noteRatio) const noexcept
{
    return getBeatDurationMs() * noteRatio;
}

//==============================================================================
double BPMCalculator::getRoomReverbMs() const noexcept
{
    // Room reverb ≈ 1/2 beat
    return getBeatDurationMs() * 0.5;
}

double BPMCalculator::getPlateReverbMs() const noexcept
{
    // Plate reverb ≈ 2 beats
    return getBeatDurationMs() * 2.0;
}

double BPMCalculator::getHallReverbMs() const noexcept
{
    // Hall reverb ≈ 4 beats
    return getBeatDurationMs() * 4.0;
}

//==============================================================================
double BPMCalculator::getFastReleaseMs() const noexcept
{
    return getBeatDurationMs() * 0.25;  // 1/16 note
}

double BPMCalculator::getMediumReleaseMs() const noexcept
{
    return getBeatDurationMs() * 0.5;   // 1/8 note
}

double BPMCalculator::getSlowReleaseMs() const noexcept
{
    return getBeatDurationMs() * 1.0;   // 1/4 note
}

//==============================================================================
juce::String BPMCalculator::formatMs(double ms, int decimalPlaces)
{
    return juce::String(ms, decimalPlaces) + " ms";
}
