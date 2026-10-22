#pragma once
#include "MeasuredHeadphone.h"
#include "ResponsePlotModel.h"
namespace hm::prediction {
inline double db(std::complex<double> response)noexcept {
 const double magnitude=std::abs(response);
 return magnitude>0?20*std::log10(magnitude):-std::numeric_limits<double>::infinity();
}
// Predicted acoustic magnitude = fixed measured headphone magnitude * the
// ACTUAL complex DSP transfer. This is not an inverted correction curve, a dB
// interpolation, or a claim to have measured the customer's physical unit.
inline double evaluate(const CorrectionResponseSnapshot& state,double hz)noexcept {
 if(!state.audioReady||!state.activeIR||!state.validFrequency(hz))return std::numeric_limits<double>::quiet_NaN();
 const auto measured=measurement::raw(state.activeProfile);const double original=measured.relativeDb(hz);
 return std::isfinite(original)?original+db(state.evaluateCorrectionResponse(hz)):original;
}
struct Basis : CorrectionResponseBasis {
 std::vector<double> original,target;
 int measuredProfile=-1;
 double evaluate(std::size_t i,double degree,double blend,bool bypassed)const noexcept {
  if(i>=original.size())return std::numeric_limits<double>::quiet_NaN();
  return original[i]+db(CorrectionResponseBasis::evaluate(i,degree,blend,bypassed));
 }
};
inline Basis build(const CorrectionResponseSnapshot& state){
 Basis result;static_cast<CorrectionResponseBasis&>(result)=hm::plot::buildBasis(state);result.measuredProfile=state.activeProfile;
 const auto raw=measurement::raw(state.activeProfile),target=measurement::target();
 result.original.reserve(result.frequencies.size());result.target.reserve(result.frequencies.size());
 for(double hz:result.frequencies){result.original.push_back(raw.relativeDb(hz));result.target.push_back(target.relativeDb(hz));}
 return result;
}
}
