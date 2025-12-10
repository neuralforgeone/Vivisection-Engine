#pragma once
#include "../core/primitives.hpp"
#include <windows.h>
#include <winternl.h>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <intrin.h>
namespace vivisect {
namespace modules {
enum class DebuggerResponse {
    IGNORE_DEBUGGER,     
    EXIT_PROCESS,        
    CRASH_PROCESS,       
    CUSTOM_HANDLER       
};
class AntiDebug {
public:
    static bool is_debugger_present();
    static bool check_remote_debugger();
    static bool timing_check();
    static bool exception_check();
    static bool hardware_breakpoint_check();
    static void respond(DebuggerResponse response);
    static void set_custom_handler(std::function<void()> handler);
    static void start_monitoring(int interval_ms);
    static void stop_monitoring();
private:
    static std::function<void()> custom_handler_;
    static std::atomic<bool> monitoring_active_;
    static std::thread monitoring_thread_;
    static void monitoring_loop(int interval_ms);
    static bool perform_all_checks();
};
#define VIVISECT_ANTI_DEBUG(response) \
    do { \
        if (vivisect::modules::AntiDebug::is_debugger_present() || \
            vivisect::modules::AntiDebug::check_remote_debugger() || \
            vivisect::modules::AntiDebug::timing_check() || \
            vivisect::modules::AntiDebug::hardware_breakpoint_check()) { \
            vivisect::modules::AntiDebug::respond(response); \
        } \
    } while(0)
} 
} 
#ifdef VIVISECT_IMPLEMENTATION
namespace vivisect {
namespace modules {
std::function<void()> AntiDebug::custom_handler_ = nullptr;
std::atomic<bool> AntiDebug::monitoring_active_(false);
std::thread AntiDebug::monitoring_thread_;
bool AntiDebug::is_debugger_present() {
    return ::IsDebuggerPresent() != 0;
}
bool AntiDebug::check_remote_debugger() {
    BOOL debugger_present = FALSE;
    HANDLE current_process = ::GetCurrentProcess();
    if (::CheckRemoteDebuggerPresent(current_process, &debugger_present)) {
        return debugger_present != FALSE;
    }
    return false;
}
bool AntiDebug::timing_check() {
    #if defined(_M_X64) || defined(_M_IX86)
    unsigned __int64 start = __rdtsc();
    volatile int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += i;
    }
    unsigned __int64 end = __rdtsc();
    unsigned __int64 elapsed = end - start;
    return elapsed > 100000;
    #else
    LARGE_INTEGER start, end, frequency;
    ::QueryPerformanceFrequency(&frequency);
    ::QueryPerformanceCounter(&start);
    volatile int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += i;
    }
    ::QueryPerformanceCounter(&end);
    double elapsed_us = static_cast<double>(end.QuadPart - start.QuadPart) * 1000000.0 / frequency.QuadPart;
    return elapsed_us > 1000.0;
    #endif
}
bool AntiDebug::exception_check() {
    __try {
        ::RaiseException(EXCEPTION_BREAKPOINT, 0, 0, nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}
bool AntiDebug::hardware_breakpoint_check() {
    CONTEXT ctx = {};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    HANDLE current_thread = ::GetCurrentThread();
    if (::GetThreadContext(current_thread, &ctx)) {
        if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
            return true;
        }
        if ((ctx.Dr7 & 0xFF) != 0) {
            return true;
        }
    }
    return false;
}
void AntiDebug::respond(DebuggerResponse response) {
    switch (response) {
        case DebuggerResponse::IGNORE_DEBUGGER:
            break;
        case DebuggerResponse::EXIT_PROCESS:
            ::ExitProcess(1);
            break;
        case DebuggerResponse::CRASH_PROCESS:
            {
                volatile int* null_ptr = nullptr;
                *null_ptr = 42;  
            }
            break;
        case DebuggerResponse::CUSTOM_HANDLER:
            if (custom_handler_) {
                custom_handler_();
            }
            break;
    }
}
void AntiDebug::set_custom_handler(std::function<void()> handler) {
    custom_handler_ = handler;
}
void AntiDebug::start_monitoring(int interval_ms) {
    if (monitoring_active_.exchange(true)) {
        return;
    }
    monitoring_thread_ = std::thread([interval_ms]() {
        monitoring_loop(interval_ms);
    });
}
void AntiDebug::stop_monitoring() {
    if (!monitoring_active_.exchange(false)) {
        return;
    }
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
}
void AntiDebug::monitoring_loop(int interval_ms) {
    while (monitoring_active_.load()) {
        if (perform_all_checks()) {
            respond(DebuggerResponse::EXIT_PROCESS);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}
bool AntiDebug::perform_all_checks() {
    return is_debugger_present() ||
           check_remote_debugger() ||
           timing_check() ||
           hardware_breakpoint_check();
}
} 
} 
#endif 