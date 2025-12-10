#pragma once
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <mutex>
namespace vivisect::error {
enum class ErrorCode : uint32_t {
    SUCCESS = 0,
    API_RESOLUTION_FAILED = 1000,
    WRAPPER_INIT_FAILED = 1001,
    MODULE_LOAD_FAILED = 1002,
    PEB_ACCESS_FAILED = 1003,
    MODULE_NOT_FOUND = 1004,
    EXPORT_NOT_FOUND = 1005,
    VM_EXECUTION_ERROR = 2000,
    DECRYPTION_FAILED = 2001,
    DEBUGGER_DETECTED = 2002,
    VM_INVALID_OPCODE = 2003,
    VM_INVALID_REGISTER = 2004,
    VM_STACK_OVERFLOW = 2005,
    VM_STACK_UNDERFLOW = 2006,
    STRING_DECRYPT_FAILED = 2007,
    INVALID_PARAMETER = 3000,
    INCOMPATIBLE_MODULES = 3001,
    FEATURE_UNAVAILABLE = 3002,
    INVALID_PROFILE = 3003,
    INVALID_COMPLEXITY_LEVEL = 3004
};
struct Error {
    ErrorCode code;
    const char* message;
    const char* file;
    int line;
    void log() const {
        std::cerr << "[VIVISECT ERROR] "
                  << "Code: " << static_cast<uint32_t>(code)
                  << " | Message: " << (message ? message : "Unknown error")
                  << " | File: " << (file ? file : "Unknown")
                  << " | Line: " << line
                  << std::endl;
    }
    [[noreturn]] void throw_exception() const {
        throw std::runtime_error(
            std::string("Vivisection Engine Error [") +
            std::to_string(static_cast<uint32_t>(code)) + "]: " +
            (message ? message : "Unknown error") + " at " +
            (file ? file : "Unknown") + ":" + std::to_string(line)
        );
    }
    const char* category() const {
        uint32_t code_value = static_cast<uint32_t>(code);
        if (code_value >= 1000 && code_value < 2000) {
            return "Initialization";
        } else if (code_value >= 2000 && code_value < 3000) {
            return "Runtime";
        } else if (code_value >= 3000 && code_value < 4000) {
            return "Configuration";
        }
        return "Unknown";
    }
};
using ErrorHandler = std::function<void(const Error&)>;
using RecoveryStrategy = std::function<bool(const Error&)>;
class ErrorManager {
private:
    static ErrorHandler global_handler_;
    static std::mutex handler_mutex_;
    static RecoveryStrategy recovery_strategies_[10];
    static std::mutex recovery_mutex_;
public:
    static void set_error_handler(ErrorHandler handler) {
        std::lock_guard<std::mutex> lock(handler_mutex_);
        global_handler_ = handler;
    }
    static ErrorHandler get_error_handler() {
        std::lock_guard<std::mutex> lock(handler_mutex_);
        return global_handler_;
    }
    static void register_recovery_strategy(ErrorCode code, RecoveryStrategy strategy) {
        std::lock_guard<std::mutex> lock(recovery_mutex_);
        uint32_t index = static_cast<uint32_t>(code) / 1000;
        if (index < 10) {
            recovery_strategies_[index] = strategy;
        }
    }
    static bool attempt_recovery(const Error& error) {
        std::lock_guard<std::mutex> lock(recovery_mutex_);
        uint32_t index = static_cast<uint32_t>(error.code) / 1000;
        if (index < 10 && recovery_strategies_[index]) {
            return recovery_strategies_[index](error);
        }
        return false;
    }
    static void report(const Error& error) {
        ErrorHandler handler;
        {
            std::lock_guard<std::mutex> lock(handler_mutex_);
            handler = global_handler_;
        }
        if (handler) {
            handler(error);
        } else {
            error.log();
        }
    }
};
inline ErrorHandler ErrorManager::global_handler_ = nullptr;
inline std::mutex ErrorManager::handler_mutex_;
inline RecoveryStrategy ErrorManager::recovery_strategies_[10] = {};
inline std::mutex ErrorManager::recovery_mutex_;
inline void report_error(ErrorCode code, const char* message, const char* file, int line) {
    Error error{code, message, file, line};
    ErrorManager::report(error);
}
inline void set_error_handler(ErrorHandler handler) {
    ErrorManager::set_error_handler(handler);
}
inline void register_recovery_strategy(ErrorCode code, RecoveryStrategy strategy) {
    ErrorManager::register_recovery_strategy(code, strategy);
}
inline bool attempt_recovery(const Error& error) {
    return ErrorManager::attempt_recovery(error);
}
#define VIVISECT_ERROR(code, msg) \
    vivisect::error::report_error(code, msg, __FILE__, __LINE__)
#define VIVISECT_ERROR_WITH_RECOVERY(code, msg) \
    do { \
        vivisect::error::Error _err{code, msg, __FILE__, __LINE__}; \
        if (!vivisect::error::attempt_recovery(_err)) { \
            vivisect::error::ErrorManager::report(_err); \
        } \
    } while(0)
inline bool api_resolution_recovery(const Error& error) {
    error.log();
    return true;
}
inline bool wrapper_init_recovery(const Error& error) {
    error.log();
    return true;
}
inline bool vm_execution_recovery(const Error& error) {
    error.log();
    return false; 
}
inline bool decryption_failure_recovery(const Error& error) {
    error.log();
    return true;
}
inline bool config_error_recovery(const Error& error) {
    error.log();
    return true;
}
inline void register_default_recovery_strategies() {
    register_recovery_strategy(ErrorCode::API_RESOLUTION_FAILED, api_resolution_recovery);
    register_recovery_strategy(ErrorCode::WRAPPER_INIT_FAILED, wrapper_init_recovery);
    register_recovery_strategy(ErrorCode::VM_EXECUTION_ERROR, vm_execution_recovery);
    register_recovery_strategy(ErrorCode::DECRYPTION_FAILED, decryption_failure_recovery);
    register_recovery_strategy(ErrorCode::INVALID_PARAMETER, config_error_recovery);
}
} 