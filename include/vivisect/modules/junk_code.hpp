#ifndef VIVISECT_MODULES_JUNK_CODE_HPP
#define VIVISECT_MODULES_JUNK_CODE_HPP
#include "../core/primitives.hpp"
#include "../core/random.hpp"
#include <cstdint>
#include <cstring>
namespace vivisect::modules {
enum class JunkPattern {
    ARITHMETIC,     
    BITWISE,        
    MEMORY,         
    CONTROL_FLOW,   
    MIXED           
};
class JunkCodeGenerator {
private:
    static void insert_arithmetic(int complexity) {
        volatile int x = vivisect::core::global_seed;
        volatile int y = complexity;
        volatile int z = 0;
        for (int i = 0; i < complexity; ++i) {
            z = x + y;
            x = z - y;
            y = x * 2;
            z = y / 2;
            x = z % 7;
            y = x + z;
            z = (x * y) + (z - x);
            x = (y + z) * (x - 1);
            y = (z * 2) - (x / 2);
        }
        vivisect::core::global_seed ^= static_cast<int>(z);
    }
    static void insert_bitwise(int complexity) {
        volatile uint32_t x = static_cast<uint32_t>(vivisect::core::global_seed);
        volatile uint32_t y = static_cast<uint32_t>(complexity);
        volatile uint32_t z = 0;
        for (int i = 0; i < complexity; ++i) {
            z = x ^ y;
            x = z | y;
            y = x & z;
            z = ~x;
            x = y << 2;
            y = z >> 1;
            z = x ^ (y | z);
            x = (y & 0xFF) | (z << 8);
            y = (x >> 4) ^ (z & 0xF0F0F0F0);
            z = ((x | y) & (x ^ y)) | (~x & y);
        }
        vivisect::core::global_seed ^= static_cast<int>(z);
    }
    static void insert_memory(int complexity) {
        if (complexity > 50) {
            complexity = 50;
        }
        volatile char buffer1[64];
        volatile char buffer2[64];
        for (int i = 0; i < 64; ++i) {
            buffer1[i] = static_cast<char>((vivisect::core::global_seed + i) & 0xFF);
            buffer2[i] = static_cast<char>((vivisect::core::global_seed - i) & 0xFF);
        }
        volatile int checksum = 0;
        for (int i = 0; i < complexity; ++i) {
            int idx1 = (vivisect::core::global_seed + i) % 64;
            int idx2 = (vivisect::core::global_seed + i + 1) % 64;
            if (idx1 < 0) idx1 = -idx1;
            if (idx2 < 0) idx2 = -idx2;
            idx1 = idx1 % 64;
            idx2 = idx2 % 64;
            char temp = buffer1[idx1];
            buffer1[idx1] = buffer2[idx2];
            buffer2[idx2] = temp;
            checksum += static_cast<int>(buffer1[idx1]) + static_cast<int>(buffer2[idx2]);
        }
        vivisect::core::global_seed ^= checksum;
    }
    static void insert_control_flow(int complexity) {
        volatile int x = vivisect::core::global_seed;
        volatile int y = complexity;
        volatile int result = 0;
        for (int i = 0; i < complexity; ++i) {
            if (x > 0) {
                if (y > 0) {
                    result = x + y;
                } else {
                    result = x - y;
                }
            } else {
                if (y > 0) {
                    result = y - x;
                } else {
                    result = -(x + y);
                }
            }
            switch (result % 5) {
                case 0:
                    x = result + 1;
                    break;
                case 1:
                    x = result - 1;
                    break;
                case 2:
                    x = result * 2;
                    break;
                case 3:
                    x = result / 2;
                    break;
                default:
                    x = result;
                    break;
            }
            y = (x > y) ? (x - y) : (y - x);
            result = (y > 10) ? (y * 2) : (y + 10);
        }
        vivisect::core::global_seed ^= result;
    }
    static void insert_mixed(int complexity) {
        int per_pattern = complexity / 4;
        int remainder = complexity % 4;
        insert_arithmetic(per_pattern + (remainder > 0 ? 1 : 0));
        insert_bitwise(per_pattern + (remainder > 1 ? 1 : 0));
        insert_memory(per_pattern + (remainder > 2 ? 1 : 0));
        insert_control_flow(per_pattern);
    }
public:
    template<JunkPattern Pattern = JunkPattern::MIXED>
    static void insert(int complexity) {
        if (complexity <= 0) {
            return;
        }
        if (complexity > 100) {
            complexity = 100;
        }
        if constexpr (Pattern == JunkPattern::ARITHMETIC) {
            insert_arithmetic(complexity);
        } else if constexpr (Pattern == JunkPattern::BITWISE) {
            insert_bitwise(complexity);
        } else if constexpr (Pattern == JunkPattern::MEMORY) {
            insert_memory(complexity);
        } else if constexpr (Pattern == JunkPattern::CONTROL_FLOW) {
            insert_control_flow(complexity);
        } else {
            insert_mixed(complexity);
        }
    }
    static void insert_realistic_dead_code() {
        volatile int seed = vivisect::core::global_seed;
        if (vivisect::core::opaque_false(seed, seed)) {
            volatile int x = seed * 2;
            volatile int y = x + seed;
            volatile int z = y - x;
            vivisect::core::volatile_nop();
            for (int i = 0; i < 10; ++i) {
                x = (x * y) + z;
                y = (y ^ x) - z;
                z = (z + x) & y;
            }
            vivisect::core::global_seed = static_cast<int>(z);
        }
        if (vivisect::core::opaque_true(seed, seed)) {
            vivisect::core::volatile_seed_update(vivisect::core::global_seed);
        } else {
            volatile int dummy = seed * seed;
            vivisect::core::global_seed = static_cast<int>(dummy);
        }
    }
    static void insert_with_opaque_predicate() {
        volatile int seed = vivisect::core::global_seed;
        if (vivisect::core::opaque_false(seed, 0xDEADBEEF)) {
            insert_arithmetic(5);
        } else if (vivisect::core::opaque_false(seed, 0xCAFEBABE)) {
            insert_bitwise(5);
        } else {
            vivisect::core::volatile_nop();
        }
        int result = vivisect::core::opaque_true(seed, seed) ? seed : (seed * 2);
        vivisect::core::global_seed ^= result;
    }
    static void insert_with_density(int density) {
        if (density <= 0) {
            return;
        }
        if (density > 10) {
            density = 10;
        }
        for (int i = 0; i < density; ++i) {
            switch (i % 5) {
                case 0:
                    insert<JunkPattern::ARITHMETIC>(density);
                    break;
                case 1:
                    insert<JunkPattern::BITWISE>(density);
                    break;
                case 2:
                    insert<JunkPattern::MEMORY>(density);
                    break;
                case 3:
                    insert<JunkPattern::CONTROL_FLOW>(density);
                    break;
                case 4:
                    insert_realistic_dead_code();
                    break;
            }
        }
        insert_with_opaque_predicate();
    }
};
#define VIVISECT_JUNK(level) \
    vivisect::modules::JunkCodeGenerator::insert<>(level)
#define VIVISECT_JUNK_DENSITY(density) \
    vivisect::modules::JunkCodeGenerator::insert_with_density(density)
#define VIVISECT_JUNK_DEAD_CODE() \
    vivisect::modules::JunkCodeGenerator::insert_realistic_dead_code()
#define VIVISECT_JUNK_OPAQUE() \
    vivisect::modules::JunkCodeGenerator::insert_with_opaque_predicate()
} 
#endif 