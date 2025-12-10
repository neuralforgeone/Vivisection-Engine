#ifndef VIVISECT_MODULES_STRING_CRYPT_HPP
#define VIVISECT_MODULES_STRING_CRYPT_HPP
#include <cstdint>
#include <array>
#include <string>
#include <cstring>
#include "../core/primitives.hpp"
#include "../core/random.hpp"
#include "../core/concepts.hpp"
#include "../error/error.hpp"
namespace vivisect::modules {
class XTEACipher {
public:
    static constexpr uint32_t DELTA = 0x9E3779B9;
    static constexpr uint32_t ROUNDS = 32;
    static constexpr void encrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        uint32_t sum = 0;
        for (uint32_t i = 0; i < ROUNDS; ++i) {
            v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
            sum += DELTA;
            v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
        }
    }
    static constexpr void decrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        uint32_t sum = DELTA * ROUNDS;
        for (uint32_t i = 0; i < ROUNDS; ++i) {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
            sum -= DELTA;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        }
    }
    static constexpr void encrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            encrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
    static constexpr void decrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            decrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
};
class AESLikeCipher {
public:
    static constexpr uint32_t ROUNDS = 8;
    static constexpr uint32_t rotate_left(uint32_t x, int bits) {
        return (x << bits) | (x >> (32 - bits));
    }
    static constexpr uint32_t rotate_right(uint32_t x, int bits) {
        return (x >> bits) | (x << (32 - bits));
    }
    static constexpr uint32_t round_function(uint32_t x, uint32_t k) {
        x ^= k;
        x = rotate_left(x, 7);
        x ^= 0x9E3779B9; 
        x = rotate_left(x, 13);
        x ^= k;
        return x;
    }
    static constexpr void encrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        for (uint32_t round = 0; round < ROUNDS; ++round) {
            uint32_t temp = v0;
            v0 = v1 ^ round_function(v0, key[round % 4]);
            v1 = temp;
        }
        uint32_t temp = v0;
        v0 = v1;
        v1 = temp;
    }
    static constexpr void decrypt(uint32_t& v0, uint32_t& v1, const uint32_t* key) {
        uint32_t temp = v0;
        v0 = v1;
        v1 = temp;
        for (uint32_t round = ROUNDS; round > 0; --round) {
            uint32_t temp = v1;
            v1 = v0 ^ round_function(v1, key[(round - 1) % 4]);
            v0 = temp;
        }
    }
    static constexpr void encrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            encrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
    static constexpr void decrypt_buffer(uint32_t* data, size_t num_blocks, const uint32_t* key) {
        for (size_t i = 0; i < num_blocks; ++i) {
            decrypt(data[i * 2], data[i * 2 + 1], key);
        }
    }
};
template<size_t N, typename Cipher = XTEACipher>
class EncryptedString {
public:
    constexpr EncryptedString(const char (&str)[N]) : original_length_(N - 1) {
        key_[0] = core::compile_time_seed() ^ __LINE__;
        key_[1] = core::mix_seed(key_[0], __COUNTER__);
        key_[2] = core::mix_seed(key_[1], N);
        key_[3] = core::mix_seed(key_[2], 0xDEADBEEF);
        for (size_t i = 0; i < N; ++i) {
            char_buffer_[i] = str[i];
        }
        for (size_t i = N; i < buffer_size_; ++i) {
            char_buffer_[i] = 0;
        }
        uint32_t temp_data[num_blocks_ * 2] = {};
        for (size_t i = 0; i < buffer_size_; ++i) {
            size_t word_idx = i / 4;
            size_t byte_idx = i % 4;
            temp_data[word_idx] |= (static_cast<uint32_t>(static_cast<uint8_t>(char_buffer_[i])) << (byte_idx * 8));
        }
        Cipher::encrypt_buffer(temp_data, num_blocks_, key_);
        for (size_t i = 0; i < num_blocks_ * 2; ++i) {
            encrypted_data_[i] = temp_data[i];
        }
    }
    std::string decrypt() const {
        try {
            char temp_buffer[buffer_size_];
            uint32_t temp_data[num_blocks_ * 2];
            for (size_t i = 0; i < num_blocks_ * 2; ++i) {
                temp_data[i] = encrypted_data_[i];
            }
            Cipher::decrypt_buffer(temp_data, num_blocks_, key_);
            const char* char_ptr = reinterpret_cast<const char*>(temp_data);
            for (size_t i = 0; i < buffer_size_; ++i) {
                temp_buffer[i] = char_ptr[i];
            }
            std::string result(temp_buffer, original_length_);
            for (size_t i = 0; i < buffer_size_; ++i) {
                temp_buffer[i] = 0;
            }
            for (size_t i = 0; i < num_blocks_ * 2; ++i) {
                temp_data[i] = 0;
            }
            return result;
        } catch (const std::exception&) {
            VIVISECT_ERROR(error::ErrorCode::STRING_DECRYPT_FAILED, "String decryption failed");
            return std::string(); 
        }
    }
    const char* c_str() const {
        thread_local char temp_buffer[buffer_size_];
        uint32_t temp_data[num_blocks_ * 2];
        for (size_t i = 0; i < num_blocks_ * 2; ++i) {
            temp_data[i] = encrypted_data_[i];
        }
        Cipher::decrypt_buffer(temp_data, num_blocks_, key_);
        const char* char_ptr = reinterpret_cast<const char*>(temp_data);
        for (size_t i = 0; i < buffer_size_; ++i) {
            temp_buffer[i] = char_ptr[i];
        }
        temp_buffer[original_length_] = '\0';
        return temp_buffer;
    }
    constexpr size_t length() const {
        return original_length_;
    }
private:
    static constexpr size_t buffer_size_ = ((N + 7) / 8) * 8;
    static constexpr size_t num_blocks_ = buffer_size_ / 8;
    uint32_t encrypted_data_[num_blocks_ * 2];
    uint32_t key_[4];
    size_t original_length_;
    char char_buffer_[buffer_size_] = {};
};
#ifdef _WIN32
#define VIVISECT_STR_SECTION(str, section_name) \
    []() -> std::string { \
        __declspec(allocate(section_name)) \
        static constexpr vivisect::modules::EncryptedString<sizeof(str)> encrypted(str); \
        return encrypted.decrypt(); \
    }()
#define VIVISECT_STR_TEXT(str) VIVISECT_STR_SECTION(str, ".text")
#define VIVISECT_STR_DATA(str) VIVISECT_STR_SECTION(str, ".data")
#define VIVISECT_STR_RDATA(str) VIVISECT_STR_SECTION(str, ".rdata")
#else
#define VIVISECT_STR_SECTION(str, section_name) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).decrypt()
#define VIVISECT_STR_TEXT(str) VIVISECT_STR(str)
#define VIVISECT_STR_DATA(str) VIVISECT_STR(str)
#define VIVISECT_STR_RDATA(str) VIVISECT_STR(str)
#endif
#define VIVISECT_STR(str) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).decrypt()
#define VIVISECT_STR_XTEA(str) \
    vivisect::modules::EncryptedString<sizeof(str), vivisect::modules::XTEACipher>(str).decrypt()
#define VIVISECT_STR_AES(str) \
    vivisect::modules::EncryptedString<sizeof(str), vivisect::modules::AESLikeCipher>(str).decrypt()
#define VIVISECT_CSTR(str) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).c_str()
} 
#endif 