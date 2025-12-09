// Vivisection Engine - Configuration System

#ifndef VIVISECT_CORE_CONFIG_HPP
#define VIVISECT_CORE_CONFIG_HPP

namespace vivisect::config {

// Forward declarations for module types
namespace modules {
    enum class DispatchStrategy;
    enum class DebuggerResponse;
}

// Obfuscation profile structure
struct ObfuscationProfile {
    // Control flow settings
    bool enable_control_flow_flattening = true;
    int bogus_flow_complexity = 5;
    
    // String encryption settings
    bool enable_string_encryption = true;
    bool distribute_across_sections = true;
    
    // VM settings
    bool enable_vm_execution = true;
    int vm_handler_count = 32;
    bool mutate_vm_handlers = true;
    
    // Anti-debug settings
    bool enable_anti_debug = true;
    
    // Junk code settings
    bool enable_junk_code = true;
    int junk_code_density = 3;
    
    // MBA settings
    bool enable_mba = true;
    int mba_complexity = 5;
    
    // Performance settings
    bool enable_performance_monitoring = false;
};

// Preset profiles
constexpr ObfuscationProfile MINIMAL_PROTECTION = {
    .enable_control_flow_flattening = true,
    .bogus_flow_complexity = 2,
    .enable_string_encryption = true,
    .distribute_across_sections = false,
    .enable_vm_execution = false,
    .vm_handler_count = 16,
    .mutate_vm_handlers = false,
    .enable_anti_debug = true,
    .enable_junk_code = false,
    .junk_code_density = 1,
    .enable_mba = false,
    .mba_complexity = 2,
    .enable_performance_monitoring = false
};

constexpr ObfuscationProfile BALANCED_PROTECTION = {
    .enable_control_flow_flattening = true,
    .bogus_flow_complexity = 5,
    .enable_string_encryption = true,
    .distribute_across_sections = true,
    .enable_vm_execution = true,
    .vm_handler_count = 32,
    .mutate_vm_handlers = true,
    .enable_anti_debug = true,
    .enable_junk_code = true,
    .junk_code_density = 3,
    .enable_mba = true,
    .mba_complexity = 5,
    .enable_performance_monitoring = false
};

constexpr ObfuscationProfile MAXIMUM_PROTECTION = {
    .enable_control_flow_flattening = true,
    .bogus_flow_complexity = 10,
    .enable_string_encryption = true,
    .distribute_across_sections = true,
    .enable_vm_execution = true,
    .vm_handler_count = 64,
    .mutate_vm_handlers = true,
    .enable_anti_debug = true,
    .enable_junk_code = true,
    .junk_code_density = 7,
    .enable_mba = true,
    .mba_complexity = 10,
    .enable_performance_monitoring = false
};

// Global configuration (can be modified at runtime)
inline ObfuscationProfile current_profile = BALANCED_PROTECTION;

} // namespace vivisect::config

#endif // VIVISECT_CORE_CONFIG_HPP
