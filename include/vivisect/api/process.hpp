#ifndef VIVISECT_API_PROCESS_HPP
#define VIVISECT_API_PROCESS_HPP
#ifdef VIVISECT_PLATFORM_WINDOWS
#include <windows.h>
#include "resolver.hpp"
#include "../error/error.hpp"
namespace vivisect::api {
class ProcessAPI {
public:
    ProcessAPI() : initialized_(false) {
        resolve_functions();
    }
    bool is_initialized() const { return initialized_; }
    HANDLE open_process(DWORD access, BOOL inherit, DWORD pid) {
        if (!open_process_) return nullptr;
        return open_process_(access, inherit, pid);
    }
    BOOL terminate_process(HANDLE process, UINT exit_code) {
        if (!terminate_process_) return FALSE;
        return terminate_process_(process, exit_code);
    }
    DWORD get_current_process_id() {
        if (!get_current_process_id_) return 0;
        return get_current_process_id_();
    }
    LPVOID virtual_alloc_ex(HANDLE process, LPVOID address, SIZE_T size, DWORD type, DWORD protect) {
        if (!virtual_alloc_ex_) return nullptr;
        return virtual_alloc_ex_(process, address, size, type, protect);
    }
    BOOL write_process_memory(HANDLE process, LPVOID address, LPCVOID buffer, SIZE_T size, SIZE_T* written) {
        if (!write_process_memory_) return FALSE;
        return write_process_memory_(process, address, buffer, size, written);
    }
    BOOL read_process_memory(HANDLE process, LPCVOID address, LPVOID buffer, SIZE_T size, SIZE_T* read) {
        if (!read_process_memory_) return FALSE;
        return read_process_memory_(process, address, buffer, size, read);
    }
    HANDLE create_remote_thread(HANDLE process, LPSECURITY_ATTRIBUTES sa, SIZE_T stack, 
                                LPTHREAD_START_ROUTINE start, LPVOID param, DWORD flags, LPDWORD tid) {
        if (!create_remote_thread_) return nullptr;
        return create_remote_thread_(process, sa, stack, start, param, flags, tid);
    }
    DWORD suspend_thread(HANDLE thread) {
        if (!suspend_thread_) return static_cast<DWORD>(-1);
        return suspend_thread_(thread);
    }
private:
    void resolve_functions() {
        HMODULE kernel32 = APIResolver::find_module("kernel32.dll");
        if (!kernel32) {
            VIVISECT_ERROR(error::ErrorCode::WRAPPER_INIT_FAILED, "ProcessAPI: Failed to find kernel32.dll");
            initialized_ = false;
            return;
        }
        open_process_ = reinterpret_cast<OpenProcess_t>(
            APIResolver::find_export(kernel32, "OpenProcess"));
        terminate_process_ = reinterpret_cast<TerminateProcess_t>(
            APIResolver::find_export(kernel32, "TerminateProcess"));
        get_current_process_id_ = reinterpret_cast<GetCurrentProcessId_t>(
            APIResolver::find_export(kernel32, "GetCurrentProcessId"));
        virtual_alloc_ex_ = reinterpret_cast<VirtualAllocEx_t>(
            APIResolver::find_export(kernel32, "VirtualAllocEx"));
        write_process_memory_ = reinterpret_cast<WriteProcessMemory_t>(
            APIResolver::find_export(kernel32, "WriteProcessMemory"));
        read_process_memory_ = reinterpret_cast<ReadProcessMemory_t>(
            APIResolver::find_export(kernel32, "ReadProcessMemory"));
        create_remote_thread_ = reinterpret_cast<CreateRemoteThread_t>(
            APIResolver::find_export(kernel32, "CreateRemoteThread"));
        suspend_thread_ = reinterpret_cast<SuspendThread_t>(
            APIResolver::find_export(kernel32, "SuspendThread"));
        initialized_ = (open_process_ != nullptr && 
                       terminate_process_ != nullptr &&
                       get_current_process_id_ != nullptr &&
                       virtual_alloc_ex_ != nullptr &&
                       write_process_memory_ != nullptr &&
                       read_process_memory_ != nullptr &&
                       create_remote_thread_ != nullptr &&
                       suspend_thread_ != nullptr);
        if (!initialized_) {
            VIVISECT_ERROR(error::ErrorCode::WRAPPER_INIT_FAILED, "ProcessAPI: Failed to resolve all required functions");
        }
    }
    using OpenProcess_t = HANDLE(WINAPI*)(DWORD, BOOL, DWORD);
    using TerminateProcess_t = BOOL(WINAPI*)(HANDLE, UINT);
    using GetCurrentProcessId_t = DWORD(WINAPI*)();
    using VirtualAllocEx_t = LPVOID(WINAPI*)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
    using WriteProcessMemory_t = BOOL(WINAPI*)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
    using ReadProcessMemory_t = BOOL(WINAPI*)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T*);
    using CreateRemoteThread_t = HANDLE(WINAPI*)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, 
                                                  LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
    using SuspendThread_t = DWORD(WINAPI*)(HANDLE);
    OpenProcess_t open_process_ = nullptr;
    TerminateProcess_t terminate_process_ = nullptr;
    GetCurrentProcessId_t get_current_process_id_ = nullptr;
    VirtualAllocEx_t virtual_alloc_ex_ = nullptr;
    WriteProcessMemory_t write_process_memory_ = nullptr;
    ReadProcessMemory_t read_process_memory_ = nullptr;
    CreateRemoteThread_t create_remote_thread_ = nullptr;
    SuspendThread_t suspend_thread_ = nullptr;
    bool initialized_;
};
} 
#endif 
#endif 