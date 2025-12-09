// Vivisection Engine - Core Primitives
// Fundamental obfuscation building blocks

#ifndef VIVISECT_CORE_PRIMITIVES_HPP
#define VIVISECT_CORE_PRIMITIVES_HPP

#include <cstdint>
#include <type_traits>
#include <concepts>

namespace vivisect::core {

// Compile-time seed generation
constexpr uint32_t compile_time_seed() {
    // Use __TIME__ macro to generate unique seed per compilation
    return (__TIME__[7] - '0') * 1 +
           (__TIME__[6] - '0') * 10 +
           (__TIME__[4] - '0') * 60 +
           (__TIME__[3] - '0') * 600 +
           (__TIME__[1] - '0') * 3600 +
           (__TIME__[0] - '0') * 36000;
}

// Global seed for runtime operations
inline int global_seed = static_cast<int>(compile_time_seed());

// Seed mixing function
constexpr uint32_t mix_seed(uint32_t a, uint32_t b) {
    return (a ^ b) * 0x9e3779b9;
}

// Runtime volatile operations
inline void volatile_nop() {
    volatile int x = 0;
    (void)x;
}

inline void volatile_seed_update(int& seed) {
    volatile int temp = seed;
    seed = temp ^ 0xDEADBEEF;
}

// Opaque predicates - always evaluate to true/false but difficult to determine statically
// These use mathematical identities that are hard for static analysis to resolve

template<typename T>
requires (std::is_integral_v<T> || std::is_pointer_v<T>)
constexpr bool opaque_true(T value, int seed) {
    // Use mathematical identity: (x | -x) < 0 is always false for x != 0
    // So !((x | -x) < 0) is always true
    // We mix in the seed to make it harder to analyze
    if constexpr (std::is_pointer_v<T>) {
        auto x = reinterpret_cast<intptr_t>(value);
        auto mixed = x ^ seed;
        // (x^2 >= 0) is always true for real numbers
        // We use (x * x) >= 0 which is always true
        return (mixed * mixed) >= 0;
    } else {
        auto mixed = static_cast<int64_t>(value) ^ seed;
        // (x * x) >= 0 is always true
        return (mixed * mixed) >= 0;
    }
}

template<typename T>
requires (std::is_integral_v<T> || std::is_pointer_v<T>)
constexpr bool opaque_false(T value, int seed) {
    // Use mathematical identity: (x * x) < 0 is always false for real numbers
    if constexpr (std::is_pointer_v<T>) {
        auto x = reinterpret_cast<intptr_t>(value);
        auto mixed = x ^ seed;
        // (x * x) < 0 is always false
        return (mixed * mixed) < 0;
    } else {
        auto mixed = static_cast<int64_t>(value) ^ seed;
        // (x * x) < 0 is always false
        return (mixed * mixed) < 0;
    }
}

} // namespace vivisect::core

#endif // VIVISECT_CORE_PRIMITIVES_HPP
