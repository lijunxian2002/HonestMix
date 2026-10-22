#pragma once
#include "dsp/CorrectionResponse.h"
#include <algorithm>
#include <stdexcept>

namespace hm::plot {
namespace detail {
// Message/background threads only. Double precision radix-2 FFT samples the
// actual immutable host-rate IR. No float FFT, angle/phase interpolation, dB
// interpolation or log-grid peak estimation is involved.
inline void fft(std::vector<std::complex<double>>& x){
 const auto n=x.size();for(std::size_t i=1,j=0;i<n;++i){auto bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(x[i],x[j]);}
 for(std::size_t length=2;length<=n;length<<=1){const double a=-6.283185307179586476925286766559/(double)length;const std::complex<double> step(std::cos(a),std::sin(a));for(std::size_t base=0;base<n;base+=length){std::complex<double> phase(1,0);for(std::size_t j=0;j<length/2;++j){const auto even=x[base+j],odd=x[base+j+length/2]*phase;x[base+j]=even+odd;x[base+j+length/2]=even-odd;phase*=step;}}}
}
inline std::vector<std::complex<double>> transform(const HostRateIR& model,std::size_t n){
 if(model.coefficients.size()>n)throw std::invalid_argument("IR exceeds bounded plot FFT length");std::vector<std::complex<double>> bins(n);for(std::size_t i=0;i<model.coefficients.size();++i)bins[i]=model.coefficients[i];fft(bins);return bins;
}
}
// <=1Hz actual FFT-bin spacing, plus exact 20Hz and upper-bound endpoints.
// For 44.1/48/96/192k: N=65536/65536/131072/262144 respectively.
// Each temporary FFT <=16MiB (N<=1048576); the two responses are transformed
// sequentially, never retaining two full FFT arrays. Returned audible-range
// vectors contain at most 40002 points at conventional audio sample rates.
inline CorrectionResponseBasis buildBasis(const CorrectionResponseSnapshot& snapshot){
 CorrectionResponseBasis result;result.generation=snapshot.generation;result.incomingGeneration=snapshot.incomingGeneration;result.sampleRate=snapshot.sampleRate;
 if(!snapshot.activeIR||!std::isfinite(snapshot.sampleRate)||snapshot.sampleRate<=40)return result;
 if(snapshot.activeIR->sampleRate!=snapshot.sampleRate||(snapshot.incomingIR&&snapshot.incomingIR->sampleRate!=snapshot.sampleRate))throw std::invalid_argument("Plot IR sample rate mismatch");
 constexpr std::size_t maximumFFT=1u<<20;std::size_t n=1;while((double)n<snapshot.sampleRate&&n<maximumFFT)n<<=1;
 if((double)n<snapshot.sampleRate)throw std::invalid_argument("Sample rate exceeds <=1Hz bounded plot FFT support");
 const double spacing=snapshot.sampleRate/(double)n,upper=std::min(20000.0,std::nextafter(snapshot.sampleRate*.5,0.0));
 const auto first=(std::size_t)std::ceil(20.0/spacing),last=(std::size_t)std::floor(upper/spacing);
 std::vector<std::size_t> indices;const auto endpoint=std::numeric_limits<std::size_t>::max();
 result.frequencies.push_back(20.0);indices.push_back(endpoint);
 for(std::size_t bin=first;bin<=last;++bin){const double hz=(double)bin*spacing;if(hz>20.0&&hz<upper){result.frequencies.push_back(hz);indices.push_back(bin);}}
 if(upper>20.0){result.frequencies.push_back(upper);indices.push_back(endpoint);}
 auto evaluate=[&](const HostRateIR& ir,std::vector<std::complex<double>>& out){auto bins=detail::transform(ir,n);out.reserve(indices.size());for(std::size_t i=0;i<indices.size();++i)out.push_back(indices[i]==endpoint?ir.evaluate(result.frequencies[i]):bins[indices[i]]);};
 evaluate(*snapshot.activeIR,result.activeWet);if(snapshot.incomingIR)evaluate(*snapshot.incomingIR,result.incomingWet);
 result.dry.reserve(indices.size());for(double hz:result.frequencies)result.dry.push_back(std::polar(1.0,-6.283185307179586476925286766559*hz*snapshot.latencySamples/snapshot.sampleRate));
 return result;
}
}
