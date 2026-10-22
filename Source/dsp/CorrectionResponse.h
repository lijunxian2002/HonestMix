#pragma once
#include <array>
#include <complex>
#include <cstdint>
#include <memory>
#include <vector>
#include <cmath>
#include <limits>
#include <juce_dsp/juce_dsp.h>

enum class ProfilePhase { Unprepared, Preparing, Warming, Fading, Stable };
struct HostBpmSnapshot { double bpm=0,updatedAtMs=0;bool valid=false,fresh=false,calculable=false;std::uint64_t generation=0; };
struct HostRateIR {
 int profile=-1,latencySamples=0;double sampleRate=0;std::uint64_t generation=0,requestGeneration=0,specGeneration=0;
 std::vector<float> coefficients;
 std::array<std::array<float,5>,4> monitorCoefficients{};
 std::complex<double> evaluate(double hz)const noexcept {
  if(!(sampleRate>0)||!std::isfinite(hz)||hz<0||hz>=sampleRate*.5)return {};
  const double angle=-6.283185307179586476925286766559*hz/sampleRate;
  const std::complex<double> step(std::cos(angle),std::sin(angle));std::complex<double> phase(1,0),sum{};
  for(float tap:coefficients){sum+=(double)tap*phase;phase*=step;}return sum;
 }
};
struct StereoResponseMatrix { std::complex<double> ll{},lr{},rl{},rr{}; };
struct CorrectionResponseBasis {
 std::uint64_t generation=0,incomingGeneration=0;double sampleRate=0;
 std::vector<double> frequencies;
 std::vector<std::complex<double>> activeWet,incomingWet,dry;
 std::complex<double> evaluate(std::size_t index,double appliedDegree,double profileBlend=0,bool bypassed=false)const noexcept {
  if(index>=dry.size())return {};const double t=appliedDegree*.01;
  if(bypassed)return dry[index];
  const auto wet=incomingWet.empty()?activeWet[index]:activeWet[index]*(1-profileBlend)+incomingWet[index]*profileBlend;
  return dry[index]*(1-t)+wet*t;
 }
};
// One coherent audio-side endpoint, not a claim of LTI behaviour during a
// time-varying ramp. Default graph covers correction, before the listen stage.
struct CorrectionResponseSnapshot {
 std::shared_ptr<const HostRateIR> activeIR,incomingIR;
 std::uint64_t generation=0,incomingGeneration=0,requestedGeneration=0,revision=0;
 int activeProfile=-1,requestedProfile=-1,incomingProfile=-1,latencySamples=0;
 double sampleRate=0,appliedDegree=100,targetDegree=100,profileBlend=0;
 ProfilePhase profilePhase=ProfilePhase::Unprepared;
 std::array<float,4> appliedModes{{1,0,0,0}};
 std::array<std::array<float,5>,4> monitorCoefficients{};
 bool degreeTransition=false,modeTransition=false,audioReady=false,bypassed=false;
 bool profileError=false;
 juce::String profileErrorMessage;
 bool isSteady()const noexcept{return audioReady&&profilePhase==ProfilePhase::Stable&&!degreeTransition&&!modeTransition;}
 bool validFrequency(double hz)const noexcept{return sampleRate>0&&std::isfinite(hz)&&hz>=0&&hz<sampleRate*.5;}
 std::complex<double> evaluateCorrectionResponse(double hz)const noexcept {
  if(!activeIR||!validFrequency(hz))return {};
  const auto dry=std::polar(1.0,-6.283185307179586476925286766559*hz*latencySamples/sampleRate);
  if(bypassed)return dry;
  auto wet=activeIR->evaluate(hz);if(incomingIR)wet=wet*(1-profileBlend)+incomingIR->evaluate(hz)*profileBlend;
  return dry*(1-appliedDegree*.01)+wet*(appliedDegree*.01);
 }
 CorrectionResponseBasis buildBasis(const std::vector<double>& hz)const {
  CorrectionResponseBasis result;result.generation=generation;result.incomingGeneration=incomingGeneration;result.sampleRate=sampleRate;result.frequencies=hz;
  result.activeWet.reserve(hz.size());result.dry.reserve(hz.size());if(incomingIR)result.incomingWet.reserve(hz.size());
  for(double f:hz){result.activeWet.push_back(activeIR?activeIR->evaluate(f):std::complex<double>{});result.dry.push_back(validFrequency(f)?std::polar(1.0,-6.283185307179586476925286766559*f*latencySamples/sampleRate):std::complex<double>{});if(incomingIR)result.incomingWet.push_back(incomingIR->evaluate(f));}
  return result;
 }
 StereoResponseMatrix evaluateMonitorMatrix(double hz)const noexcept {
  if(!validFrequency(hz))return {};if(bypassed)return {1.,0.,0.,1.};const auto z=std::polar(1.0,-6.283185307179586476925286766559*hz/sampleRate);
  auto biquad=[&](int m){const auto& c=monitorCoefficients[(std::size_t)m];return ((double)c[0]+(double)c[1]*z+(double)c[2]*z*z)/(1.0+(double)c[3]*z+(double)c[4]*z*z);};
  const std::complex<double> cross=appliedModes[1]*.5;
  const auto diagonal=std::complex<double>(appliedModes[0],0)+cross+(double)appliedModes[2]*biquad(2)+(double)appliedModes[3]*biquad(3);
  return {diagonal,cross,cross,diagonal};
 }
};
