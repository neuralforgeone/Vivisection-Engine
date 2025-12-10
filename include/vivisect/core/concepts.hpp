#ifndef VIVISECT_CORE_CONCEPTS_HPP
#define VIVISECT_CORE_CONCEPTS_HPP
#include <concepts>
#include <type_traits>
namespace vivisect::core {
template<typename T>
concept ObfuscatableInteger = std::is_integral_v<T> && !std::is_const_v<T>;
template<typename T>
concept ObfuscatablePointer = std::is_pointer_v<T> && !std::is_const_v<T>;
template<typename T>
concept OpaqueValue = std::is_integral_v<T> || std::is_pointer_v<T>;
template<typename T>
concept CipherAlgorithm = requires(T cipher, const uint8_t* data, size_t len) {
    { cipher.encrypt(data, len) } -> std::same_as<void>;
    { cipher.decrypt(data, len) } -> std::same_as<void>;
};
} 
#endif 