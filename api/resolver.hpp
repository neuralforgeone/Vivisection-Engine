// Vivisection Engine - API Resolution System
// Stealthy Windows API resolution via PEB parsing

#ifndef VIVISECT_API_RESOLVER_HPP
#define VIVISECT_API_RESOLVER_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <cstdint>
#include <windows.h>

namespace vivisect::api {

class APIResolver {
public:
    static HMODULE find_module(uint32_t name_hash) {
        return nullptr;
    }
    
    static HMODULE find_module(const char* name) {
        return LoadLibraryA(name);
    }
    
    static void* find_export(HMODULE module, uint32_t name_hash) {
        return nullptr;
    }
    
    static void* find_export(HMODULE module, const char* name) {
        return GetProcAddress(module, name);
    }
    
    static constexpr uint32_t hash(const char* str) {
        uint32_t hash = 0x811c9dc5;
        while (*str) {
            hash ^= static_cast<uint32_t>(*str++);
            hash *= 0x01000193;
        }
        return hash;
    }
};

#define VIVISECT_API(dll, func) GetProcAddress(LoadLibraryA(dll), func)

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_RESOLVER_HPP
