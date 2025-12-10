#ifndef VIVISECT_API_RESOLVER_HPP
#define VIVISECT_API_RESOLVER_HPP
#ifdef VIVISECT_PLATFORM_WINDOWS
#include <cstdint>
#include <windows.h>
#include "../error/error.hpp"
namespace vivisect::api {
struct UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
};
struct LIST_ENTRY {
    LIST_ENTRY* Flink;
    LIST_ENTRY* Blink;
};
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
struct PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
};
struct PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    BOOLEAN SpareBool;
    PVOID Mutant;
    PVOID ImageBaseAddress;
    PEB_LDR_DATA* Ldr;
};
struct TEB {
    PVOID Reserved1[12];
    PEB* ProcessEnvironmentBlock;
};
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
    static PEB* get_peb() {
        PEB* peb = nullptr;
        try {
#ifdef _WIN64
            peb = reinterpret_cast<PEB*>(__readgsqword(0x60));
#else
            peb = reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif
        } catch (...) {
            VIVISECT_ERROR(error::ErrorCode::PEB_ACCESS_FAILED, "Failed to access PEB");
            return nullptr;
        }
        if (!peb) {
            VIVISECT_ERROR(error::ErrorCode::PEB_ACCESS_FAILED, "PEB pointer is null");
        }
        return peb;
    }
    static HMODULE find_module(uint32_t name_hash) {
        PEB* peb = get_peb();
        if (!peb || !peb->Ldr) {
            VIVISECT_ERROR(error::ErrorCode::MODULE_NOT_FOUND, "PEB or Ldr is null");
            return nullptr;
        }
        LIST_ENTRY* head = &peb->Ldr->InLoadOrderModuleList;
        LIST_ENTRY* current = head->Flink;
        while (current != head && current != nullptr) {
            LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(
                current, 
                LDR_DATA_TABLE_ENTRY, 
                InLoadOrderLinks
            );
            if (entry->BaseDllName.Buffer && entry->BaseDllName.Length > 0) {
                char narrow_name[256] = {0};
                int len = entry->BaseDllName.Length / sizeof(WCHAR);
                if (len >= 256) len = 255;
                for (int i = 0; i < len; i++) {
                    wchar_t wc = entry->BaseDllName.Buffer[i];
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
        VIVISECT_ERROR(error::ErrorCode::MODULE_NOT_FOUND, "Module not found by hash");
        return nullptr;
    }
    static HMODULE find_module(const char* name) {
        uint32_t name_hash = hash_string_lower(name);
        HMODULE result = find_module(name_hash);
        if (!result) {
            result = LoadLibraryA(name);
        }
        return result;
    }
    static void* find_export(HMODULE module, uint32_t name_hash) {
        if (!module) {
            VIVISECT_ERROR(error::ErrorCode::EXPORT_NOT_FOUND, "Module handle is null");
            return nullptr;
        }
        BYTE* base = reinterpret_cast<BYTE*>(module);
        IMAGE_DOS_HEADER* dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            VIVISECT_ERROR(error::ErrorCode::EXPORT_NOT_FOUND, "Invalid DOS signature");
            return nullptr;
        }
        IMAGE_NT_HEADERS* nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(
            base + dos_header->e_lfanew
        );
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
            VIVISECT_ERROR(error::ErrorCode::EXPORT_NOT_FOUND, "Invalid NT signature");
            return nullptr;
        }
        DWORD export_rva = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (export_rva == 0) {
            VIVISECT_ERROR(error::ErrorCode::EXPORT_NOT_FOUND, "No export directory");
            return nullptr;
        }
        IMAGE_EXPORT_DIRECTORY* export_dir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
            base + export_rva
        );
        DWORD* functions = reinterpret_cast<DWORD*>(base + export_dir->AddressOfFunctions);
        DWORD* names = reinterpret_cast<DWORD*>(base + export_dir->AddressOfNames);
        WORD* ordinals = reinterpret_cast<WORD*>(base + export_dir->AddressOfNameOrdinals);
        for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
            const char* func_name = reinterpret_cast<const char*>(base + names[i]);
            if (hash(func_name) == name_hash) {
                WORD ordinal = ordinals[i];
                DWORD func_rva = functions[ordinal];
                DWORD export_dir_start = export_rva;
                DWORD export_dir_end = export_rva + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
                if (func_rva >= export_dir_start && func_rva < export_dir_end) {
                    return nullptr;
                }
                return reinterpret_cast<void*>(base + func_rva);
            }
        }
        VIVISECT_ERROR(error::ErrorCode::EXPORT_NOT_FOUND, "Export not found by hash");
        return nullptr;
    }
    static void* find_export(HMODULE module, const char* name) {
        uint32_t name_hash = hash(name);
        void* result = find_export(module, name_hash);
        if (!result) {
            result = reinterpret_cast<void*>(GetProcAddress(module, name));
        }
        return result;
    }
    static constexpr uint32_t hash(const char* str) {
        uint32_t hash = 0x811c9dc5; 
        while (*str) {
            hash ^= static_cast<uint32_t>(*str++);
            hash *= 0x01000193; 
        }
        return hash;
    }
private:
    static uint32_t hash_string_lower(const char* str) {
        uint32_t hash = 0x811c9dc5;
        while (*str) {
            char c = *str++;
            if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 'a';
            }
            hash ^= static_cast<uint32_t>(c);
            hash *= 0x01000193;
        }
        return hash;
    }
};
#define VIVISECT_API(dll, func) \
    vivisect::api::APIResolver::find_export( \
        vivisect::api::APIResolver::find_module(dll), \
        func)
} 
#endif 
#endif 