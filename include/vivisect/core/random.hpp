#ifndef VIVISECT_CORE_RANDOM_HPP
#define VIVISECT_CORE_RANDOM_HPP
#include <cstdint>
#include "primitives.hpp"
namespace vivisect::core {
template<uint32_t Seed>
class CompileTimeRandom {
public:
    static constexpr uint32_t next() {
        return (Seed * 1103515245 + 12345) & 0x7fffffff;
    }
    static constexpr uint32_t range(uint32_t min, uint32_t max) {
        return min + (next() % (max - min + 1));
    }
    static constexpr bool coin_flip() {
        return (next() & 1) == 1;
    }
};
#define VIVISECT_UNIQUE_SEED (__LINE__ ^ __COUNTER__ ^ vivisect::core::compile_time_seed())
} 
#endif 