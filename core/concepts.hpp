// Vivisection Engine - C++20 Concepts for Type Safety

#ifndef VIVISECT_CORE_CONCEPTS_HPP
#define VIVISECT_CORE_CONCEPTS_HPP

#include <concepts>
#include <type_traits>

namespace vivisect::core {

// Obfuscatable integer types
template<typename T>
concept ObfuscatableInteger = std::is_integral_v<T> && !std::is_const_v<T>;

// Obfuscatable pointer types
template<typename T>
concept ObfuscatablePointer = std::is_pointer_v<T> && !std::is_const_v<T>;

// Opaque value types (for opaque predicates)
template<typename T>
concept OpaqueValue = std::is_integral_v<T> || std::is_pointer_v<T>;

// Cipher algorithm concept
template<typename T>
concept CipherAlgorithm = requires(T cipher, const uint8_t* data, size_t len) {
    { cipher.encrypt(data, len) } -> std::same_as<void>;
    { cipher.decrypt(data, len) } -> std::same_as<void>;
};

} // namespace vivisect::core

#endif // VIVISECT_CORE_CONCEPTS_HPP
