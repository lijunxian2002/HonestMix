#pragma once
#include <cmath>
#include <limits>
namespace hm::tempo {
enum class Feel { straight, dotted, triplet };
inline bool valid(double bpm) noexcept { return std::isfinite(bpm) && bpm >= 1.0 && bpm <= 1000.0; }
inline double quarter(double bpm) noexcept { return valid(bpm) ? 60000.0 / bpm : std::numeric_limits<double>::quiet_NaN(); }
inline double note(double bpm, int denominator, Feel feel = Feel::straight) noexcept {
    if (denominator <= 0) return std::numeric_limits<double>::quiet_NaN();
    return quarter(bpm) * 4.0 / denominator * (feel == Feel::dotted ? 1.5 : feel == Feel::triplet ? 2.0 / 3.0 : 1.0);
}
struct Pair { double low, high; };
struct Times { double preShort, preMedium, preLong; Pair room, plate, hall, fast, medium, slow; };
inline Times calculate(double bpm) noexcept {
    const double q = quarter(bpm);
    return {q/32, q/16, q/8, {q/2,q}, {2*q,4*q}, {4*q,8*q}, {q/16,q/8}, {q/4,q/2}, {q,2*q}};
}
}
