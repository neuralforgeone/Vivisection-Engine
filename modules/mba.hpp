// Vivisection Engine - Mixed Boolean-Arithmetic Module
// MBA transformations for obfuscating arithmetic operations

#ifndef VIVISECT_MODULES_MBA_HPP
#define VIVISECT_MODULES_MBA_HPP

#include <concepts>
#include "../core/concepts.hpp"

namespace vivisect::modules {

class MBA {
public:
    template<std::integral T>
    static constexpr T add(T a, T b, int variant = 0) {
        return a + b;
    }
    
    template<std::integral T>
    static constexpr T xor_op(T a, T b, int variant = 0) {
        return a ^ b;
    }
};

#define VIVISECT_MBA_ADD(a, b) ((a) + (b))
#define VIVISECT_MBA_XOR(a, b) ((a) ^ (b))

} // namespace vivisect::modules

#endif // VIVISECT_MODULES_MBA_HPP
