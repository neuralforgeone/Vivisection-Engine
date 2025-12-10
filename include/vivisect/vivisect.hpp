#ifndef VIVISECT_HPP
#define VIVISECT_HPP
#define VIVISECT_VERSION_MAJOR 1
#define VIVISECT_VERSION_MINOR 0
#define VIVISECT_VERSION_PATCH 0
#if defined(_WIN32) || defined(_WIN64)
    #define VIVISECT_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define VIVISECT_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define VIVISECT_PLATFORM_MACOS
#endif
#if defined(_MSC_VER)
    #define VIVISECT_COMPILER_MSVC
    #if _MSC_VER < 1929
        #error "Vivisection Engine requires MSVC 2019 16.10 or later for C++20 support"
    #endif
#elif defined(__clang__)
    #define VIVISECT_COMPILER_CLANG
    #if __clang_major__ < 10
        #error "Vivisection Engine requires Clang 10 or later for C++20 support"
    #endif
#elif defined(__GNUC__)
    #define VIVISECT_COMPILER_GCC
    #if __GNUC__ < 10
        #error "Vivisection Engine requires GCC 10 or later for C++20 support"
    #endif
#endif
#if defined(_MSVC_LANG)
    #if _MSVC_LANG < 202002L
        #error "Vivisection Engine requires C++20 or later"
    #endif
#elif __cplusplus < 202002L
    #error "Vivisection Engine requires C++20 or later"
#endif
#ifdef VIVISECT_IMPLEMENTATION
    #define VIVISECT_INLINE
#else
    #define VIVISECT_INLINE inline
#endif
#include "core/primitives.hpp"
#include "core/random.hpp"
#include "core/concepts.hpp"
#include "core/config.hpp"
#include "error/error.hpp"
#include "modules/string_crypt.hpp"
#include "modules/mba.hpp"
#include "modules/control_flow.hpp"
#include "modules/vm_engine.hpp"
#include "modules/anti_debug.hpp"
#include "modules/junk_code.hpp"
#ifdef VIVISECT_PLATFORM_WINDOWS
    #include "api/resolver.hpp"
    #include "api/process.hpp"
    #include "api/crypto.hpp"
    #include "api/network.hpp"
    #include "api/registry.hpp"
#endif
#include "integration/macros.hpp"
#include "integration/main_protect.hpp"
namespace vivisect {
    namespace core {}
    namespace modules {}
    namespace api {}
    namespace integration {}
    namespace config {}
    namespace error {}
}
#endif 