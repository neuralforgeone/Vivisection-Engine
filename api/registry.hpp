// Vivisection Engine - Registry API Wrapper
// Stealthy Windows Registry APIs

#ifndef VIVISECT_API_REGISTRY_HPP
#define VIVISECT_API_REGISTRY_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <windows.h>
#include "resolver.hpp"

namespace vivisect::api {

class RegistryAPI {
public:
    RegistryAPI() : initialized_(false) {
        resolve_functions();
    }
    
    bool is_initialized() const { return initialized_; }
    
    // Key operations
    LSTATUS create_key_ex(HKEY key, LPCSTR subkey, DWORD reserved, LPSTR class_name, DWORD options,
                         REGSAM sam, LPSECURITY_ATTRIBUTES sa, PHKEY result, LPDWORD disposition) {
        if (!create_key_ex_) return ERROR_PROC_NOT_FOUND;
        return create_key_ex_(key, subkey, reserved, class_name, options, sam, sa, result, disposition);
    }
    
    LSTATUS open_key_ex(HKEY key, LPCSTR subkey, DWORD options, REGSAM sam, PHKEY result) {
        if (!open_key_ex_) return ERROR_PROC_NOT_FOUND;
        return open_key_ex_(key, subkey, options, sam, result);
    }
    
    LSTATUS close_key(HKEY key) {
        if (!close_key_) return ERROR_PROC_NOT_FOUND;
        return close_key_(key);
    }
    
    // Value operations
    LSTATUS set_value_ex(HKEY key, LPCSTR value_name, DWORD reserved, DWORD type, const BYTE* data, DWORD size) {
        if (!set_value_ex_) return ERROR_PROC_NOT_FOUND;
        return set_value_ex_(key, value_name, reserved, type, data, size);
    }
    
    LSTATUS query_value_ex(HKEY key, LPCSTR value_name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size) {
        if (!query_value_ex_) return ERROR_PROC_NOT_FOUND;
        return query_value_ex_(key, value_name, reserved, type, data, size);
    }
    
    // Enumeration
    LSTATUS enum_key_ex(HKEY key, DWORD index, LPSTR name, LPDWORD name_len, LPDWORD reserved,
                       LPSTR class_name, LPDWORD class_len, PFILETIME last_write) {
        if (!enum_key_ex_) return ERROR_PROC_NOT_FOUND;
        return enum_key_ex_(key, index, name, name_len, reserved, class_name, class_len, last_write);
    }
    
    LSTATUS enum_value(HKEY key, DWORD index, LPSTR value_name, LPDWORD value_name_len, LPDWORD reserved,
                      LPDWORD type, LPBYTE data, LPDWORD data_len) {
        if (!enum_value_) return ERROR_PROC_NOT_FOUND;
        return enum_value_(key, index, value_name, value_name_len, reserved, type, data, data_len);
    }
    
private:
    void resolve_functions() {
        HMODULE advapi32 = APIResolver::find_module("advapi32.dll");
        if (!advapi32) {
            initialized_ = false;
            return;
        }
        
        create_key_ex_ = reinterpret_cast<RegCreateKeyExA_t>(
            APIResolver::find_export(advapi32, "RegCreateKeyExA"));
        open_key_ex_ = reinterpret_cast<RegOpenKeyExA_t>(
            APIResolver::find_export(advapi32, "RegOpenKeyExA"));
        close_key_ = reinterpret_cast<RegCloseKey_t>(
            APIResolver::find_export(advapi32, "RegCloseKey"));
        set_value_ex_ = reinterpret_cast<RegSetValueExA_t>(
            APIResolver::find_export(advapi32, "RegSetValueExA"));
        query_value_ex_ = reinterpret_cast<RegQueryValueExA_t>(
            APIResolver::find_export(advapi32, "RegQueryValueExA"));
        enum_key_ex_ = reinterpret_cast<RegEnumKeyExA_t>(
            APIResolver::find_export(advapi32, "RegEnumKeyExA"));
        enum_value_ = reinterpret_cast<RegEnumValueA_t>(
            APIResolver::find_export(advapi32, "RegEnumValueA"));
        
        // Check if all critical functions were resolved
        initialized_ = (create_key_ex_ != nullptr && 
                       open_key_ex_ != nullptr &&
                       close_key_ != nullptr &&
                       set_value_ex_ != nullptr &&
                       query_value_ex_ != nullptr &&
                       enum_key_ex_ != nullptr &&
                       enum_value_ != nullptr);
    }
    
    // Function pointer types
    using RegCreateKeyExA_t = LSTATUS(WINAPI*)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, 
                                                LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
    using RegOpenKeyExA_t = LSTATUS(WINAPI*)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
    using RegCloseKey_t = LSTATUS(WINAPI*)(HKEY);
    using RegSetValueExA_t = LSTATUS(WINAPI*)(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD);
    using RegQueryValueExA_t = LSTATUS(WINAPI*)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
    using RegEnumKeyExA_t = LSTATUS(WINAPI*)(HKEY, DWORD, LPSTR, LPDWORD, LPDWORD, 
                                             LPSTR, LPDWORD, PFILETIME);
    using RegEnumValueA_t = LSTATUS(WINAPI*)(HKEY, DWORD, LPSTR, LPDWORD, LPDWORD, 
                                             LPDWORD, LPBYTE, LPDWORD);
    
    // Function pointers
    RegCreateKeyExA_t create_key_ex_ = nullptr;
    RegOpenKeyExA_t open_key_ex_ = nullptr;
    RegCloseKey_t close_key_ = nullptr;
    RegSetValueExA_t set_value_ex_ = nullptr;
    RegQueryValueExA_t query_value_ex_ = nullptr;
    RegEnumKeyExA_t enum_key_ex_ = nullptr;
    RegEnumValueA_t enum_value_ = nullptr;
    
    bool initialized_;
};

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_REGISTRY_HPP
