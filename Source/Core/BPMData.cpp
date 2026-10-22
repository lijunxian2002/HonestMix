#include "Core/BPMData.h"
#include <cmath>
BPMData::BPMData(double bpm){setBPM(bpm);}
void BPMData::setBPM(double bpm){bpm_=std::isfinite(bpm)&&bpm>=1&&bpm<=1000?bpm:std::numeric_limits<double>::quiet_NaN();}
double BPMData::safeBeatMs()const noexcept{return 60000.0/bpm_;}
double BPMData::beatMs()const noexcept{return safeBeatMs();}
double BPMData::twoBeatMs()const noexcept{return beatMs()*2;}
double BPMData::preShort()const noexcept{return beatMs()/32;}
double BPMData::preMedium()const noexcept{return beatMs()/16;}
double BPMData::preLong()const noexcept{return beatMs()/8;}
BPMData::Pair BPMData::roomReverb()const noexcept{auto q=beatMs();return {q/2,q};}
BPMData::Pair BPMData::plateReverb()const noexcept{auto q=beatMs();return {q*2,q*4};}
BPMData::Pair BPMData::hallReverb()const noexcept{auto q=beatMs();return {q*4,q*8};}
BPMData::Pair BPMData::compFast()const noexcept{auto q=beatMs();return {q/16,q/8};}
BPMData::Pair BPMData::compMedium()const noexcept{auto q=beatMs();return {q/4,q/2};}
BPMData::Pair BPMData::compSlow()const noexcept{auto q=beatMs();return {q,q*2};}
double BPMData::delayHalf()const noexcept{return beatMs()*2;}
double BPMData::delayQuarter()const noexcept{return beatMs();}
double BPMData::delayEighth()const noexcept{return beatMs()/2;}
double BPMData::delay16th()const noexcept{return beatMs()/4;}
double BPMData::delay64th()const noexcept{return beatMs()/16;}
juce::String BPMData::fmtMs(double ms){if(!std::isfinite(ms))return "--";return ms>=1000?juce::String(ms/1000,3)+" s":juce::String(ms,2)+" ms";}
