#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace headphoneIR {
// 128 input samples on either side, including the negative-time pre-ring.
// The fixed integer output delay depends only on sample rate, never profile.
constexpr int radius=128;
inline int delay(double rate) noexcept {return rate==44100.0?0:(int)std::ceil(radius*rate/44100.0);}
inline juce::AudioBuffer<float> convert(const float* source,int size,double rate){
 if(rate==44100.0){juce::AudioBuffer<float> result(1,size);result.copyFrom(0,0,source,size);return result;}
 const double ratio=rate/44100.0,cutoff=juce::jmin(1.0,ratio),pi=juce::MathConstants<double>::pi;
 const int d=delay(rate),length=d+(int)std::ceil((size-1+radius)*ratio)+1;
 juce::AudioBuffer<float> result(1,length);
 for(int n=0;n<length;++n){
  const double x=(n-d)/ratio;double sum=0;
  const int first=juce::jmax(0,(int)std::ceil(x-radius)),last=juce::jmin(size-1,(int)std::floor(x+radius));
  for(int k=first;k<=last;++k){const double t=x-k,a=pi*cutoff*t;
   const double sinc=std::abs(a)<1e-12?1.0:std::sin(a)/a;
   const double window=.42+.5*std::cos(pi*t/radius)+.08*std::cos(2*pi*t/radius);
   sum+=(double)source[k]*cutoff*sinc*window;
  }
  // This is the analytical change in sample density, not gain normalisation.
  result.setSample(0,n,(float)(sum/ratio));
 }
 return result;
}
}
