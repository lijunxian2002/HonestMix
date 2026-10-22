#pragma once
#include <cstddef>
#include <limits>
namespace hm::measurement {
struct Curve {
 const double* frequencies=nullptr;
 const double* decibels=nullptr;
 std::size_t count=0;
 double referenceAt1k=0;
 bool supports(double hz)const noexcept;
 double relativeDb(double hz)const noexcept;
};
int modelCount()noexcept;
const char* modelName(int profile)noexcept;
Curve raw(int profile)noexcept;
Curve target()noexcept;
}
