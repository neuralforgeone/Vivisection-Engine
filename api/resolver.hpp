// Vivisection Engine - API Resolution System
// Stealthy Windows API resolution via PEB parsing

#ifndef VIVISECT_API_RESOLVER_HPP
#define VIVISECT_API_RESOLVER_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <cstdint>
#include <windows.h>

namespace vivisect::api {

// Forward declarations for PEB structures
// These are internal Windows structures not officially documented but stable across versions

// Unicode string structure used in PEB
struct UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
};

// List entry for doubly-linked lists
struct LIST_ENTRY {
    LIST_ENTRY* Flink;
    LIST_ENTRY* Blink;
};

// Loader data table entry - represents a loaded module
struct LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
};

// PEB loader data structure
struct PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
};

// Process Environment Block (PEB)
struct PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    BOOLEAN SpareBool;
    PVOID Mutant;
    PVOID ImageBaseAddress;
    PEB_LDR_DATA* Ldr;
    // ... many more fields, but we only need Ldr
};

// Thread Environment Block (TEB) - used to access PEB
struct TEB {
    PVOID Reserved1[12];
    PEB* ProcessEnvironmentBlock;
    // ... many more fields
};

// PE format structures for export parsing
struct IMAGE_EXPORT_DIRECTORY {
    DWORD Characteristics;
    DWORD TimeDateStamp;
    WORD MajorVersion;
    WORD MinorVersion;
    DWORD Name;
    DWORD Base;
    DWORD NumberOfFunctions;
    DWORD NumberOfNames;
    DWORD AddressOfFunctions;
    DWORD AddressOfNames;
    DWORD AddressOfNameOrdinals;
};

class APIResolver {
public:
    // Get the Process Environment Block for the current process
    static PEB* get_peb() {
#ifdef _WIN64
        // On x64, TEB is at gs:[0x30], PEB is at gs:[0x60]
        return reinterpret_cast<PEB*>(__readgsqword(0x60));
#else
        // On x86, TEB is at fs:[0x18], PEB is at fs:[0x30]
        return reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif
    }
    
    // Find a loaded module by hash of its name
    static HMODULE find_module(uint32_t name_hash) {
        PEB* peb = get_peb();
        if (!peb || !peb->Ldr) {
            return nullptr;
        }
        
        // Walk the InLoadOrderModuleList
        LIST_ENTRY* head = &peb->Ldr->InLoadOrderModuleList;
        LIST_ENTRY* current = head->Flink;
        
        while (current != head && current != nullptr) {
            LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(
                current, 
                LDR_DATA_TABLE_ENTRY, 
                InLoadOrderLinks
            );
            
            if (entry->BaseDllName.Buffer && entry->BaseDllName.Length > 0) {
                // Convert wide string to narrow for hashing
                char narrow_name[256] = {0};
                int len = entry->BaseDllName.Length / sizeof(WCHAR);
                if (len >= 256) len = 255;
                
                for (int i = 0; i < len; i++) {
                    wchar_t wc = entry->BaseDllName.Buffer[i];
                    // Convert to lowercase for case-insensitive comparison
                    if (wc >= L'A' && wc <= L'Z') {
                        wc = wc - L'A' + L'a';
                    }
                    narrow_name[i] = static_cast<char>(wc);
                }
                
                if (hash(narrow_name) == name_hash) {
                    return reinterpret_cast<HMODULE>(entry->DllBase);
                }
            }
            
            current = current->Flink;
        }
        
        return nullptr;
    }
    
    // Find a loaded module by name (fallback method)
    static HMODULE find_module(const char* name) {
        // First try stealthy method via hash
        uint32_t name_hash = hash_string_lower(name);
        HMODULE result = find_module(name_hash);
        
        // If not found, fall back to LoadLibraryA
        if (!result) {
            result = LoadLibraryA(name);
        }
        
        return result;
    }
    
    // Find an exported function by hash of its name
    static void* find_export(HMODULE module, uint32_t name_hash) {
        if (!module) {
            return nullptr;
        }
        
        // Get DOS header
        BYTE* base = reinterpret_cast<BYTE*>(module);
        IMAGE_DOS_HEADER* dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            return nullptr;
        }
        
        // Get NT headers
        IMAGE_NT_HEADERS* nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(
            base + dos_header->e_lfanew
        );
        
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
            return nullptr;
        }
        
        // Get export directory
        DWORD export_rva = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (export_rva == 0) {
            return nullptr;
        }
        
        IMAGE_EXPORT_DIRECTORY* export_dir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
            base + export_rva
        );
        
        // Get export tables
        DWORD* functions = reinterpret_cast<DWORD*>(base + export_dir->AddressOfFunctions);
        DWORD* names = reinterpret_cast<DWORD*>(base + export_dir->AddressOfNames);
        WORD* ordinals = reinterpret_cast<WORD*>(base + export_dir->AddressOfNameOrdinals);
        
        // Search for the function by name hash
        for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
            const char* func_name = reinterpret_cast<const char*>(base + names[i]);
            
            if (hash(func_name) == name_hash) {
                WORD ordinal = ordinals[i];
                DWORD func_rva = functions[ordinal];
                
                // Check if this is a forwarded export
                DWORD export_dir_start = export_rva;
                DWORD export_dir_end = export_rva + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
                
                if (func_rva >= export_dir_start && func_rva < export_dir_end) {
                    // This is a forwarded export, we don't handle these stealthily
                    return nullptr;
                }
                
                return reinterpret_cast<void*>(base + func_rva);
            }
        }
        
        return nullptr;
    }
    
    // Find an exported function by name (fallback method)
    static void* find_export(HMODULE module, const char* name) {
        // First try stealthy method via hash
        uint32_t name_hash = hash(name);
        void* result = find_export(module, name_hash);
        
        // If not found, fall back to GetProcAddress
        if (!result) {
            result = reinterpret_cast<void*>(GetProcAddress(module, name));
        }
        
        return result;
    }
    
    // Compile-time hash function (FNV-1a)
    static constexpr uint32_t hash(const char* str) {
        uint32_t hash = 0x811c9dc5; // FNV offset basis
        while (*str) {
            hash ^= static_cast<uint32_t>(*str++);
            hash *= 0x01000193; // FNV prime
        }
        return hash;
    }
    
private:
    // Helper to hash a string with lowercase conversion
    static uint32_t hash_string_lower(const char* str) {
        uint32_t hash = 0x811c9dc5;
        while (*str) {
            char c = *str++;
            // Convert to lowercase
            if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 'a';
            }
            hash ^= static_cast<uint32_t>(c);
            hash *= 0x01000193;
        }
        return hash;
    }
};

// Convenience macro for API resolution
// Usage: auto func = VIVISECT_API("kernel32.dll", "VirtualAlloc");
#define VIVISECT_API(dll, func) \
    vivisect::api::APIResolver::find_export( \
        vivisect::api::APIResolver::find_module(dll), \
        func)

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_RESOLVER_HPP
