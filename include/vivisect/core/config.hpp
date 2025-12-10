#ifndef VIVISECT_CORE_CONFIG_HPP
#define VIVISECT_CORE_CONFIG_HPP
#include <string>
#include <optional>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
namespace vivisect::config {
namespace modules {
    enum class DebuggerResponse;
}
struct ObfuscationProfile {
    bool enable_control_flow_flattening = true;
    int bogus_flow_complexity = 5;
    int opaque_predicate_count = 3;
    bool enable_string_encryption = true;
    bool distribute_across_sections = true;
    int encryption_rounds = 1;
    bool enable_vm_execution = true;
    int vm_handler_count = 32;
    bool mutate_vm_handlers = true;
    int vm_mutation_frequency = 100;
    bool enable_anti_debug = true;
    bool enable_timing_checks = true;
    bool enable_exception_checks = true;
    bool enable_hardware_breakpoint_checks = true;
    bool enable_junk_code = true;
    int junk_code_density = 3;
    bool use_realistic_junk = true;
    bool enable_mba = true;
    int mba_complexity = 5;
    int mba_chain_depth = 2;
    bool enable_performance_monitoring = false;
    bool validate() const {
        if (bogus_flow_complexity < 0 || bogus_flow_complexity > 20) return false;
        if (opaque_predicate_count < 0 || opaque_predicate_count > 50) return false;
        if (encryption_rounds < 1 || encryption_rounds > 10) return false;
        if (vm_handler_count < 8 || vm_handler_count > 256) return false;
        if (vm_mutation_frequency < 1) return false;
        if (junk_code_density < 0 || junk_code_density > 10) return false;
        if (mba_complexity < 0 || mba_complexity > 20) return false;
        if (mba_chain_depth < 1 || mba_chain_depth > 10) return false;
        return true;
    }
    std::string serialize() const {
        std::ostringstream oss;
        oss << "enable_control_flow_flattening=" << enable_control_flow_flattening << "\n";
        oss << "bogus_flow_complexity=" << bogus_flow_complexity << "\n";
        oss << "opaque_predicate_count=" << opaque_predicate_count << "\n";
        oss << "enable_string_encryption=" << enable_string_encryption << "\n";
        oss << "distribute_across_sections=" << distribute_across_sections << "\n";
        oss << "encryption_rounds=" << encryption_rounds << "\n";
        oss << "enable_vm_execution=" << enable_vm_execution << "\n";
        oss << "vm_handler_count=" << vm_handler_count << "\n";
        oss << "mutate_vm_handlers=" << mutate_vm_handlers << "\n";
        oss << "vm_mutation_frequency=" << vm_mutation_frequency << "\n";
        oss << "enable_anti_debug=" << enable_anti_debug << "\n";
        oss << "enable_timing_checks=" << enable_timing_checks << "\n";
        oss << "enable_exception_checks=" << enable_exception_checks << "\n";
        oss << "enable_hardware_breakpoint_checks=" << enable_hardware_breakpoint_checks << "\n";
        oss << "enable_junk_code=" << enable_junk_code << "\n";
        oss << "junk_code_density=" << junk_code_density << "\n";
        oss << "use_realistic_junk=" << use_realistic_junk << "\n";
        oss << "enable_mba=" << enable_mba << "\n";
        oss << "mba_complexity=" << mba_complexity << "\n";
        oss << "mba_chain_depth=" << mba_chain_depth << "\n";
        oss << "enable_performance_monitoring=" << enable_performance_monitoring << "\n";
        return oss.str();
    }
    static ObfuscationProfile deserialize(const std::string& data) {
        ObfuscationProfile profile;
        std::istringstream iss(data);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            auto parse_bool = [](const std::string& s) { return s == "1" || s == "true"; };
            auto parse_int = [](const std::string& s) { return std::stoi(s); };
            if (key == "enable_control_flow_flattening") profile.enable_control_flow_flattening = parse_bool(value);
            else if (key == "bogus_flow_complexity") profile.bogus_flow_complexity = parse_int(value);
            else if (key == "opaque_predicate_count") profile.opaque_predicate_count = parse_int(value);
            else if (key == "enable_string_encryption") profile.enable_string_encryption = parse_bool(value);
            else if (key == "distribute_across_sections") profile.distribute_across_sections = parse_bool(value);
            else if (key == "encryption_rounds") profile.encryption_rounds = parse_int(value);
            else if (key == "enable_vm_execution") profile.enable_vm_execution = parse_bool(value);
            else if (key == "vm_handler_count") profile.vm_handler_count = parse_int(value);
            else if (key == "mutate_vm_handlers") profile.mutate_vm_handlers = parse_bool(value);
            else if (key == "vm_mutation_frequency") profile.vm_mutation_frequency = parse_int(value);
            else if (key == "enable_anti_debug") profile.enable_anti_debug = parse_bool(value);
            else if (key == "enable_timing_checks") profile.enable_timing_checks = parse_bool(value);
            else if (key == "enable_exception_checks") profile.enable_exception_checks = parse_bool(value);
            else if (key == "enable_hardware_breakpoint_checks") profile.enable_hardware_breakpoint_checks = parse_bool(value);
            else if (key == "enable_junk_code") profile.enable_junk_code = parse_bool(value);
            else if (key == "junk_code_density") profile.junk_code_density = parse_int(value);
            else if (key == "use_realistic_junk") profile.use_realistic_junk = parse_bool(value);
            else if (key == "enable_mba") profile.enable_mba = parse_bool(value);
            else if (key == "mba_complexity") profile.mba_complexity = parse_int(value);
            else if (key == "mba_chain_depth") profile.mba_chain_depth = parse_int(value);
            else if (key == "enable_performance_monitoring") profile.enable_performance_monitoring = parse_bool(value);
        }
        return profile;
    }
};
constexpr ObfuscationProfile MINIMAL_PROTECTION = {
    .enable_control_flow_flattening = true,
    .bogus_flow_complexity = 2,
    .opaque_predicate_count = 1,
    .enable_string_encryption = true,
    .distribute_across_sections = false,
    .encryption_rounds = 1,
    .enable_vm_execution = false,
    .vm_handler_count = 16,
    .mutate_vm_handlers = false,
    .vm_mutation_frequency = 1000,
    .enable_anti_debug = true,
    .enable_timing_checks = true,
    .enable_exception_checks = false,
    .enable_hardware_breakpoint_checks = false,
    .enable_junk_code = false,
    .junk_code_density = 1,
    .use_realistic_junk = false,
    .enable_mba = false,
    .mba_complexity = 2,
    .mba_chain_depth = 1,
    .enable_performance_monitoring = false
};
constexpr ObfuscationProfile BALANCED_PROTECTION = {
    .enable_control_flow_flattening = true,
    .bogus_flow_complexity = 5,
    .opaque_predicate_count = 3,
    .enable_string_encryption = true,
    .distribute_across_sections = true,
    .encryption_rounds = 2,
    .enable_vm_execution = true,
    .vm_handler_count = 32,
    .mutate_vm_handlers = true,
    .vm_mutation_frequency = 100,
    .enable_anti_debug = true,
    .enable_timing_checks = true,
    .enable_exception_checks = true,
    .enable_hardware_breakpoint_checks = true,
    .enable_junk_code = true,
    .junk_code_density = 3,
    .use_realistic_junk = true,
    .enable_mba = true,
    .mba_complexity = 5,
    .mba_chain_depth = 2,
    .enable_performance_monitoring = false
};
constexpr ObfuscationProfile MAXIMUM_PROTECTION = {
    .enable_control_flow_flattening = true,
    .bogus_flow_complexity = 10,
    .opaque_predicate_count = 8,
    .enable_string_encryption = true,
    .distribute_across_sections = true,
    .encryption_rounds = 3,
    .enable_vm_execution = true,
    .vm_handler_count = 64,
    .mutate_vm_handlers = true,
    .vm_mutation_frequency = 50,
    .enable_anti_debug = true,
    .enable_timing_checks = true,
    .enable_exception_checks = true,
    .enable_hardware_breakpoint_checks = true,
    .enable_junk_code = true,
    .junk_code_density = 7,
    .use_realistic_junk = true,
    .enable_mba = true,
    .mba_complexity = 10,
    .mba_chain_depth = 4,
    .enable_performance_monitoring = false
};
enum class ProfileType {
    MINIMAL,
    BALANCED,
    MAXIMUM,
    CUSTOM
};
class ConfigurationManager {
public:
    static ConfigurationManager& instance() {
        static ConfigurationManager instance;
        return instance;
    }
    void select_profile(ProfileType type) {
        switch (type) {
            case ProfileType::MINIMAL:
                current_profile_ = MINIMAL_PROTECTION;
                current_type_ = ProfileType::MINIMAL;
                break;
            case ProfileType::BALANCED:
                current_profile_ = BALANCED_PROTECTION;
                current_type_ = ProfileType::BALANCED;
                break;
            case ProfileType::MAXIMUM:
                current_profile_ = MAXIMUM_PROTECTION;
                current_type_ = ProfileType::MAXIMUM;
                break;
            case ProfileType::CUSTOM:
                current_type_ = ProfileType::CUSTOM;
                break;
        }
    }
    const ObfuscationProfile& get_profile() const {
        return current_profile_;
    }
    ObfuscationProfile& get_mutable_profile() {
        current_type_ = ProfileType::CUSTOM;
        return current_profile_;
    }
    void set_profile(const ObfuscationProfile& profile) {
        if (!profile.validate()) {
            throw std::invalid_argument("Invalid obfuscation profile configuration");
        }
        current_profile_ = profile;
        current_type_ = ProfileType::CUSTOM;
    }
    ProfileType get_profile_type() const {
        return current_type_;
    }
    void set_control_flow_flattening(bool enabled) { current_profile_.enable_control_flow_flattening = enabled; current_type_ = ProfileType::CUSTOM; }
    void set_bogus_flow_complexity(int complexity) { current_profile_.bogus_flow_complexity = complexity; current_type_ = ProfileType::CUSTOM; }
    void set_string_encryption(bool enabled) { current_profile_.enable_string_encryption = enabled; current_type_ = ProfileType::CUSTOM; }
    void set_vm_execution(bool enabled) { current_profile_.enable_vm_execution = enabled; current_type_ = ProfileType::CUSTOM; }
    void set_anti_debug(bool enabled) { current_profile_.enable_anti_debug = enabled; current_type_ = ProfileType::CUSTOM; }
    void set_junk_code(bool enabled) { current_profile_.enable_junk_code = enabled; current_type_ = ProfileType::CUSTOM; }
    void set_mba(bool enabled) { current_profile_.enable_mba = enabled; current_type_ = ProfileType::CUSTOM; }
    bool validate_current() const {
        return current_profile_.validate();
    }
    std::string serialize_current() const {
        return current_profile_.serialize();
    }
    void load_from_string(const std::string& data) {
        ObfuscationProfile profile = ObfuscationProfile::deserialize(data);
        if (!profile.validate()) {
            throw std::invalid_argument("Deserialized profile failed validation");
        }
        current_profile_ = profile;
        current_type_ = ProfileType::CUSTOM;
    }
private:
    ConfigurationManager() : current_profile_(BALANCED_PROTECTION), current_type_(ProfileType::BALANCED) {}
    ObfuscationProfile current_profile_;
    ProfileType current_type_;
};
struct FunctionObfuscationConfig {
    std::optional<bool> enable_control_flow_flattening;
    std::optional<bool> enable_string_encryption;
    std::optional<bool> enable_vm_execution;
    std::optional<bool> enable_anti_debug;
    std::optional<bool> enable_junk_code;
    std::optional<bool> enable_mba;
    std::optional<int> complexity_override;
    ObfuscationProfile merge_with_global(const ObfuscationProfile& global) const {
        ObfuscationProfile result = global;
        if (enable_control_flow_flattening.has_value()) {
            result.enable_control_flow_flattening = enable_control_flow_flattening.value();
        }
        if (enable_string_encryption.has_value()) {
            result.enable_string_encryption = enable_string_encryption.value();
        }
        if (enable_vm_execution.has_value()) {
            result.enable_vm_execution = enable_vm_execution.value();
        }
        if (enable_anti_debug.has_value()) {
            result.enable_anti_debug = enable_anti_debug.value();
        }
        if (enable_junk_code.has_value()) {
            result.enable_junk_code = enable_junk_code.value();
        }
        if (enable_mba.has_value()) {
            result.enable_mba = enable_mba.value();
        }
        if (complexity_override.has_value()) {
            result.bogus_flow_complexity = complexity_override.value();
            result.mba_complexity = complexity_override.value();
        }
        return result;
    }
};
class FunctionConfigRegistry {
public:
    static FunctionConfigRegistry& instance() {
        static FunctionConfigRegistry instance;
        return instance;
    }
    void register_function(const std::string& function_name, const FunctionObfuscationConfig& config) {
        function_configs_[function_name] = config;
    }
    std::optional<FunctionObfuscationConfig> get_function_config(const std::string& function_name) const {
        auto it = function_configs_.find(function_name);
        if (it != function_configs_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    ObfuscationProfile get_effective_profile(const std::string& function_name) const {
        const auto& global = ConfigurationManager::instance().get_profile();
        auto func_config = get_function_config(function_name);
        if (func_config.has_value()) {
            return func_config.value().merge_with_global(global);
        }
        return global;
    }
    void clear() {
        function_configs_.clear();
    }
private:
    FunctionConfigRegistry() = default;
    std::unordered_map<std::string, FunctionObfuscationConfig> function_configs_;
};
#define VIVISECT_FUNCTION_CONFIG(func_name, ...) \
    namespace { \
        struct FunctionConfigRegistrar_##func_name { \
            FunctionConfigRegistrar_##func_name() { \
                vivisect::config::FunctionObfuscationConfig config __VA_ARGS__; \
                vivisect::config::FunctionConfigRegistry::instance().register_function(#func_name, config); \
            } \
        }; \
        static FunctionConfigRegistrar_##func_name registrar_##func_name; \
    }
inline ObfuscationProfile current_profile = BALANCED_PROTECTION;
} 
#endif 