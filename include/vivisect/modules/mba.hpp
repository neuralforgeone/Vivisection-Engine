#ifndef VIVISECT_MODULES_MBA_HPP
#define VIVISECT_MODULES_MBA_HPP
#include <cstdint>
#include <type_traits>
#include "../core/concepts.hpp"
#include "../core/random.hpp"
namespace vivisect::modules {
template<typename T>
concept MBAOperand = std::is_integral_v<T>;
class MBA {
public:
    template<MBAOperand T>
    static constexpr T add(T a, T b, int variant = 0) {
        switch (variant % 5) {
            case 0:
                return (a ^ b) + 2 * (a & b);
            case 1:
                return (a | b) + (a & b);
            case 2:
                return 2 * (a | b) - (a ^ b);
            case 3:
                return ((a ^ b) + ((a & b) << 1));
            case 4:
                return (a - ~b - 1);
            default:
                return a + b;
        }
    }
    template<MBAOperand T>
    static constexpr T sub(T a, T b, int variant = 0) {
        switch (variant % 5) {
            case 0:
                return (a ^ b) - 2 * (~a & b);
            case 1:
                return (a & ~b) - (~a & b);
            case 2:
                return 2 * (a & ~b) - (a ^ b);
            case 3:
                return ((a ^ b) - ((~a & b) << 1));
            case 4:
                return (a + ~b + 1);
            default:
                return a - b;
        }
    }
    template<MBAOperand T>
    static constexpr T xor_op(T a, T b, int variant = 0) {
        switch (variant % 5) {
            case 0:
                return (a | b) - (a & b);
            case 1:
                return (a + b) - 2 * (a & b);
            case 2:
                return (~a & b) | (a & ~b);
            case 3:
                return ((a | b) & ~(a & b));
            case 4:
                return (a - b) + 2 * (~a & b);
            default:
                return a ^ b;
        }
    }
    template<MBAOperand T>
    static constexpr T and_op(T a, T b, int variant = 0) {
        switch (variant % 5) {
            case 0:
                return (a + b) - (a | b);
            case 1:
                return (a + b) - (a ^ b);
            case 2:
                return ~(~a | ~b);
            case 3:
                return ((a | b) - (a ^ b));
            case 4:
                return (a & b);
            default:
                return a & b;
        }
    }
    template<MBAOperand T>
    static constexpr T or_op(T a, T b, int variant = 0) {
        switch (variant % 5) {
            case 0:
                return (a + b) - (a & b);
            case 1:
                return (a ^ b) + (a & b);
            case 2:
                return ~(~a & ~b);
            case 3:
                return ((a & b) + (a ^ b));
            case 4:
                return (a + b);
            default:
                return a | b;
        }
    }
    template<MBAOperand T>
    static constexpr T not_op(T a, int variant = 0) {
        switch (variant % 5) {
            case 0:
                return ~a;
            case 1:
                return -a - 1;
            case 2:
                return (a ^ static_cast<T>(-1));
            case 3:
                return -(a + 1);
            case 4:
                return (0 - a - 1);
            default:
                return ~a;
        }
    }
    template<MBAOperand T>
    static constexpr T chain(T value, int depth) {
        if (depth <= 0) {
            return value;
        }
        constexpr uint32_t seed = __COUNTER__;
        constexpr uint32_t op_select = core::CompileTimeRandom<seed>::next();
        switch (op_select % 4) {
            case 0: {
                constexpr T temp_val = static_cast<T>(core::CompileTimeRandom<seed + 1>::next());
                T intermediate = add(value, temp_val, op_select);
                return chain(sub(intermediate, temp_val, op_select + 1), depth - 1);
            }
            case 1: {
                constexpr T temp_val = static_cast<T>(core::CompileTimeRandom<seed + 2>::next());
                T intermediate = xor_op(value, temp_val, op_select);
                return chain(xor_op(intermediate, temp_val, op_select + 1), depth - 1);
            }
            case 2: {
                T intermediate = not_op(value, op_select);
                return chain(not_op(intermediate, op_select + 1), depth - 1);
            }
            case 3: {
                constexpr T mask = static_cast<T>(core::CompileTimeRandom<seed + 3>::next());
                T part1 = and_op(value, mask, op_select);
                T part2 = and_op(value, not_op(mask, op_select), op_select + 1);
                return chain(or_op(part1, part2, op_select + 2), depth - 1);
            }
            default:
                return chain(value, depth - 1);
        }
    }
};
#define VIVISECT_MBA_ADD(a, b) \
    vivisect::modules::MBA::add((a), (b), VIVISECT_UNIQUE_SEED)
#define VIVISECT_MBA_SUB(a, b) \
    vivisect::modules::MBA::sub((a), (b), VIVISECT_UNIQUE_SEED)
#define VIVISECT_MBA_XOR(a, b) \
    vivisect::modules::MBA::xor_op((a), (b), VIVISECT_UNIQUE_SEED)
#define VIVISECT_MBA_AND(a, b) \
    vivisect::modules::MBA::and_op((a), (b), VIVISECT_UNIQUE_SEED)
#define VIVISECT_MBA_OR(a, b) \
    vivisect::modules::MBA::or_op((a), (b), VIVISECT_UNIQUE_SEED)
#define VIVISECT_MBA_NOT(a) \
    vivisect::modules::MBA::not_op((a), VIVISECT_UNIQUE_SEED)
#define VIVISECT_MBA_CHAIN(value, depth) \
    vivisect::modules::MBA::chain((value), (depth))
} 
#endif 