// Vivisection Engine - String Encryption Module

#ifndef VIVISECT_MODULES_STRING_CRYPT_HPP
#define VIVISECT_MODULES_STRING_CRYPT_HPP

#include <cstdint>
#include <array>
#include <string>
#include <cstring>
#include "../core/primitives.hpp"
#include "../core/random.hpp"
#include "../core/concepts.hpp"

namespace vivisect::modules {

// ============================================================================
// XTEA Cipher Implementation
// ============================================================================

class XTEACipher {
public:
    static constexpr uint32_t DELTA = 0x9E3779B9;
    static constexpr uint32_t ROUNDS = 32;
    
    // Compile-time encryption of two 32-bit blocks
    static constexpr void encrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        uint32_t sum = 0;
        for (uint32_t i = 0; i < ROUNDS; ++i) {
            v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
            sum += DELTA;
            v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
        }
    }
    
    // Runtime decryption of two 32-bit blocks
    static constexpr void decrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        uint32_t sum = DELTA * ROUNDS;
        for (uint32_t i = 0; i < ROUNDS; ++i) {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
            sum -= DELTA;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        }
    }
    
    // Encrypt array of data (length must be multiple of 8 bytes)
    static constexpr void encrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            encrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
    
    // Decrypt array of data (length must be multiple of 8 bytes)
    static constexpr void decrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            decrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
};

// ============================================================================
// AES-Like Cipher Implementation (Simplified but Correct)
// ============================================================================

class AESLikeCipher {
public:
    static constexpr uint32_t ROUNDS = 8;
    
    // Simple rotation-based mixing (guaranteed reversible)
    static constexpr uint32_t rotate_left(uint32_t x, int bits) {
        return (x << bits) | (x >> (32 - bits));
    }
    
    static constexpr uint32_t rotate_right(uint32_t x, int bits) {
        return (x >> bits) | (x << (32 - bits));
    }
    
    // Feistel-like round function (automatically reversible)
    static constexpr uint32_t round_function(uint32_t x, uint32_t k) {
        x ^= k;
        x = rotate_left(x, 7);
        x ^= 0x9E3779B9; // Golden ratio constant
        x = rotate_left(x, 13);
        x ^= k;
        return x;
    }
    
    // Encrypt two 32-bit blocks using Feistel network
    static constexpr void encrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        for (uint32_t round = 0; round < ROUNDS; ++round) {
            // Feistel structure - automatically reversible
            uint32_t temp = v0;
            v0 = v1 ^ round_function(v0, key[round % 4]);
            v1 = temp;
        }
        
        // Final swap
        uint32_t temp = v0;
        v0 = v1;
        v1 = temp;
    }
    
    // Decrypt two 32-bit blocks (reverse Feistel network)
    static constexpr void decrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        // Reverse final swap
        uint32_t temp = v0;
        v0 = v1;
        v1 = temp;
        
        // Reverse Feistel rounds
        for (uint32_t round = ROUNDS; round > 0; --round) {
            uint32_t temp = v1;
            v1 = v0 ^ round_function(v1, key[(round - 1) % 4]);
            v0 = temp;
        }
    }
    
    // Encrypt array of data
    static constexpr void encrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            encrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
    
    // Decrypt array of data
    static constexpr void decrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            decrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
};

// ============================================================================
// Encrypted String Template
// ============================================================================

template<size_t N, typename Cipher = XTEACipher>
class EncryptedString {
public:
    // Compile-time constructor that encrypts the string
    constexpr EncryptedString(const char (&str)[N]) : original_length_(N - 1) {
        // Generate unique key based on compile-time context
        key_[0] = core::compile_time_seed() ^ __LINE__;
        key_[1] = core::mix_seed(key_[0], __COUNTER__);
        key_[2] = core::mix_seed(key_[1], N);
        key_[3] = core::mix_seed(key_[2], 0xDEADBEEF);
        
        // Copy string to buffer and pad with zeros
        for (size_t i = 0; i < N; ++i) {
            char_buffer_[i] = str[i];
        }
        for (size_t i = N; i < buffer_size_; ++i) {
            char_buffer_[i] = 0;
        }
        
        // Convert char array to uint32_t array manually (constexpr-safe)
        uint32_t temp_data[num_blocks_ * 2] = {};
        for (size_t i = 0; i < buffer_size_; ++i) {
            size_t word_idx = i / 4;
            size_t byte_idx = i % 4;
            temp_data[word_idx] |= (static_cast<uint32_t>(static_cast<uint8_t>(char_buffer_[i])) << (byte_idx * 8));
        }
        
        // Encrypt the data
        Cipher::encrypt_buffer(temp_data, num_blocks_, key_);
        
        // Copy encrypted data to storage
        for (size_t i = 0; i < num_blocks_ * 2; ++i) {
            encrypted_data_[i] = temp_data[i];
        }
    }
    
    // Runtime decryption - returns decrypted string
    std::string decrypt() const {
        // Allocate stack-based temporary storage
        char temp_buffer[buffer_size_];
        uint32_t temp_data[num_blocks_ * 2];
        
        // Copy encrypted data to temporary buffer
        for (size_t i = 0; i < num_blocks_ * 2; ++i) {
            temp_data[i] = encrypted_data_[i];
        }
        
        // Decrypt the data
        Cipher::decrypt_buffer(temp_data, num_blocks_, key_);
        
        // Convert back to char array
        const char* char_ptr = reinterpret_cast<const char*>(temp_data);
        for (size_t i = 0; i < buffer_size_; ++i) {
            temp_buffer[i] = char_ptr[i];
        }
        
        // Create string from decrypted data
        std::string result(temp_buffer, original_length_);
        
        // Clear temporary buffer (security measure)
        for (size_t i = 0; i < buffer_size_; ++i) {
            temp_buffer[i] = 0;
        }
        for (size_t i = 0; i < num_blocks_ * 2; ++i) {
            temp_data[i] = 0;
        }
        
        return result;
    }
    
    // Get C-string (allocates on stack, returns pointer)
    // WARNING: Returned pointer is only valid within the calling scope
    const char* c_str() const {
        thread_local char temp_buffer[buffer_size_];
        uint32_t temp_data[num_blocks_ * 2];
        
        // Copy encrypted data
        for (size_t i = 0; i < num_blocks_ * 2; ++i) {
            temp_data[i] = encrypted_data_[i];
        }
        
        // Decrypt
        Cipher::decrypt_buffer(temp_data, num_blocks_, key_);
        
        // Convert to char array
        const char* char_ptr = reinterpret_cast<const char*>(temp_data);
        for (size_t i = 0; i < buffer_size_; ++i) {
            temp_buffer[i] = char_ptr[i];
        }
        
        // Ensure null termination
        temp_buffer[original_length_] = '\0';
        
        return temp_buffer;
    }
    
    // Get original length
    constexpr size_t length() const {
        return original_length_;
    }
    
private:
    // Calculate buffer size (must be multiple of 8 bytes for block cipher)
    static constexpr size_t buffer_size_ = ((N + 7) / 8) * 8;
    static constexpr size_t num_blocks_ = buffer_size_ / 8;
    
    // Storage for encrypted data
    uint32_t encrypted_data_[num_blocks_ * 2];
    uint32_t key_[4];
    size_t original_length_;
    
    // Temporary buffer for compile-time encryption
    char char_buffer_[buffer_size_] = {};
};

// ============================================================================
// PE Section Distribution Support (Windows-specific)
// ============================================================================

#ifdef _WIN32

// Macro to place encrypted string in specific PE section
#define VIVISECT_STR_SECTION(str, section_name) \
    []() -> std::string { \
        __declspec(allocate(section_name)) \
        static constexpr vivisect::modules::EncryptedString<sizeof(str)> encrypted(str); \
        return encrypted.decrypt(); \
    }()

// Predefined sections for string distribution
#define VIVISECT_STR_TEXT(str) VIVISECT_STR_SECTION(str, ".text")
#define VIVISECT_STR_DATA(str) VIVISECT_STR_SECTION(str, ".data")
#define VIVISECT_STR_RDATA(str) VIVISECT_STR_SECTION(str, ".rdata")

#else

// Non-Windows platforms don't support PE sections
#define VIVISECT_STR_SECTION(str, section_name) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).decrypt()

#define VIVISECT_STR_TEXT(str) VIVISECT_STR(str)
#define VIVISECT_STR_DATA(str) VIVISECT_STR(str)
#define VIVISECT_STR_RDATA(str) VIVISECT_STR(str)

#endif

// ============================================================================
// Convenience Macros
// ============================================================================

// Basic string obfuscation macro
#define VIVISECT_STR(str) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).decrypt()

// String obfuscation with specific cipher
#define VIVISECT_STR_XTEA(str) \
    vivisect::modules::EncryptedString<sizeof(str), vivisect::modules::XTEACipher>(str).decrypt()

#define VIVISECT_STR_AES(str) \
    vivisect::modules::EncryptedString<sizeof(str), vivisect::modules::AESLikeCipher>(str).decrypt()

// C-string obfuscation (use with caution - limited lifetime)
#define VIVISECT_CSTR(str) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).c_str()

} // namespace vivisect::modules

#endif // VIVISECT_MODULES_STRING_CRYPT_HPP
