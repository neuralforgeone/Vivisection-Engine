#ifndef VIVISECT_CORE_PRIMITIVES_HPP
#define VIVISECT_CORE_PRIMITIVES_HPP
#include <cstdint>
#include <type_traits>
#include <concepts>
namespace vivisect::core {
constexpr uint32_t compile_time_seed() {
    return (__TIME__[7] - '0') * 1 +
           (__TIME__[6] - '0') * 10 +
           (__TIME__[4] - '0') * 60 +
           (__TIME__[3] - '0') * 600 +
           (__TIME__[1] - '0') * 3600 +
           (__TIME__[0] - '0') * 36000;
}
inline int global_seed = static_cast<int>(compile_time_seed());
constexpr uint32_t mix_seed(uint32_t a, uint32_t b) {
    return (a ^ b) * 0x9e3779b9;
}
inline void volatile_nop() {
    volatile int x = 0;
    (void)x;
}
inline void volatile_seed_update(int& seed) {
    volatile int temp = seed;
    seed = temp ^ 0xDEADBEEF;
}
template<typename T>
requires (std::is_integral_v<T> || std::is_pointer_v<T>)
constexpr bool opaque_true(T value, int seed) {
    if constexpr (std::is_pointer_v<T>) {
        auto x = reinterpret_cast<intptr_t>(value);
        auto mixed = x ^ seed;
        return (mixed * mixed) >= 0;
    } else {
        auto mixed = static_cast<int64_t>(value) ^ seed;
        return (mixed * mixed) >= 0;
    }
}
template<typename T>
requires (std::is_integral_v<T> || std::is_pointer_v<T>)
constexpr bool opaque_false(T value, int seed) {
    if constexpr (std::is_pointer_v<T>) {
        auto x = reinterpret_cast<intptr_t>(value);
        auto mixed = x ^ seed;
        return (mixed * mixed) < 0;
    } else {
        auto mixed = static_cast<int64_t>(value) ^ seed;
        return (mixed * mixed) < 0;
    }
}
} 
#endif 