#include "MeasuredHeadphone.h"
#include "measured_data.h"
#include <cmath>
#include <algorithm>
namespace hm::measurement {
namespace {
double interpolate(const double* f,const double* db,std::size_t size,double hz)noexcept {
 if(hz==f[0])return db[0];if(hz==f[size-1])return db[size-1];
 const auto* upper=std::upper_bound(f,f+size,hz);const auto at=(std::size_t)(upper-f);const double t=std::log(hz/f[at-1])/std::log(f[at]/f[at-1]);return db[at-1]+(db[at]-db[at-1])*t;
}
Curve view(const double* values)noexcept {return {data::frequencies,values,(std::size_t)data::points,interpolate(data::frequencies,values,data::points,1000.)};}
}
bool Curve::supports(double hz)const noexcept{return frequencies&&decibels&&count>=2&&std::isfinite(hz)&&hz>=frequencies[0]&&hz<=frequencies[count-1];}
double Curve::relativeDb(double hz)const noexcept{return supports(hz)?interpolate(frequencies,decibels,count,hz)-referenceAt1k:std::numeric_limits<double>::quiet_NaN();}
int modelCount()noexcept{return data::profiles;}
const char* modelName(int profile)noexcept{return profile>=0&&profile<data::profiles?data::names[profile]:nullptr;}
Curve raw(int profile)noexcept{return profile>=0&&profile<data::profiles?view(data::raw[profile]):Curve{};}
Curve target()noexcept{return view(data::target);}
}
