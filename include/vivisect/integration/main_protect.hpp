#ifndef VIVISECT_INTEGRATION_MAIN_PROTECT_HPP
#define VIVISECT_INTEGRATION_MAIN_PROTECT_HPP
#include "../modules/vm_engine.hpp"
#include "../modules/anti_debug.hpp"
#include "../modules/control_flow.hpp"
#include "../modules/junk_code.hpp"
#include "../core/config.hpp"
#include <functional>
#include <exception>
namespace vivisect::integration {
struct MainProtectionConfig {
    bool enable_vm_prologue = true;
    bool enable_anti_debug = true;
    bool enable_control_flow = true;
    bool enable_junk_code = true;
    bool enable_exception_handling = true;
    modules::DebuggerResponse debugger_response = modules::DebuggerResponse::EXIT_PROCESS;
    int junk_code_density = 3;
    std::function<void()> custom_prologue = nullptr;
    std::function<void()> custom_epilogue = nullptr;
    MainProtectionConfig() {
        const auto& profile = config::current_profile;
        enable_vm_prologue = profile.enable_vm_execution;
        enable_anti_debug = profile.enable_anti_debug;
        enable_control_flow = profile.enable_control_flow_flattening;
        enable_junk_code = profile.enable_junk_code;
        junk_code_density = profile.junk_code_density;
    }
};
inline MainProtectionConfig main_protection_config;
inline void execute_prologue() {
    if (main_protection_config.custom_prologue) {
        main_protection_config.custom_prologue();
    }
    if (main_protection_config.enable_junk_code) {
        modules::JunkCodeGenerator::insert_with_density(main_protection_config.junk_code_density);
    }
    if (main_protection_config.enable_vm_prologue) {
        modules::VMEngine vm(vivisect::core::global_seed);
        modules::VMInstruction init_bytecode[] = {
            modules::VMInstruction(modules::VMOpcode::LOAD_IMM, 0, 0, 0, 0xDEADBEEF),
            modules::VMInstruction(modules::VMOpcode::LOAD_IMM, 1, 0, 0, 0xCAFEBABE),
            modules::VMInstruction(modules::VMOpcode::XOR, 2, 0, 1, 0),
            modules::VMInstruction(modules::VMOpcode::MANGLE_KEY, 3, 2, 0, 0),
            modules::VMInstruction(modules::VMOpcode::JUNK_OP, 0, 0, 0, 0),
            modules::VMInstruction(modules::VMOpcode::NOP, 0, 0, 0, 0)
        };
        vm.execute(init_bytecode, 6);
    }
    if (main_protection_config.enable_anti_debug) {
        VIVISECT_ANTI_DEBUG(main_protection_config.debugger_response);
    }
    if (main_protection_config.enable_junk_code) {
        modules::JunkCodeGenerator::insert_realistic_dead_code();
    }
}
inline void execute_epilogue() {
    if (main_protection_config.enable_junk_code) {
        modules::JunkCodeGenerator::insert_with_opaque_predicate();
    }
    if (main_protection_config.enable_vm_prologue) {
        modules::VMEngine vm(vivisect::core::global_seed);
        modules::VMInstruction cleanup_bytecode[] = {
            modules::VMInstruction(modules::VMOpcode::LOAD_IMM, 0, 0, 0, 0x12345678),
            modules::VMInstruction(modules::VMOpcode::NOT, 1, 0, 0, 0),
            modules::VMInstruction(modules::VMOpcode::XOR, 2, 0, 1, 0),
            modules::VMInstruction(modules::VMOpcode::JUNK_OP, 0, 0, 0, 0),
            modules::VMInstruction(modules::VMOpcode::MANGLE_KEY, 3, 2, 0, 0)
        };
        vm.execute(cleanup_bytecode, 5);
    }
    if (main_protection_config.custom_epilogue) {
        main_protection_config.custom_epilogue();
    }
    if (main_protection_config.enable_junk_code) {
        modules::JunkCodeGenerator::insert<modules::JunkPattern::MIXED>(2);
    }
}
} 
#define VIVISECT_MAIN(body) \
    int main(int argc, char** argv) { \
        int __vivisect_result = 0; \
        \
         \
        if (vivisect::integration::main_protection_config.enable_exception_handling) { \
            try { \
                 \
                vivisect::integration::execute_prologue(); \
                \
                 \
                if (vivisect::integration::main_protection_config.enable_control_flow) { \
                    volatile int __state = 0; \
                    volatile bool __done = false; \
                    \
                    while (!__done) { \
                        switch (__state) { \
                            case 0: \
                                 \
                                { body } \
                                __state = 1; \
                                break; \
                            case 1: \
                                 \
                                vivisect::integration::execute_epilogue(); \
                                __done = true; \
                                break; \
                            default: \
                                __done = true; \
                                break; \
                        } \
                    } \
                } else { \
                     \
                    { body } \
                    vivisect::integration::execute_epilogue(); \
                } \
            } \
            catch (const std::exception& e) { \
                 \
                vivisect::core::volatile_nop(); \
                __vivisect_result = 1; \
            } \
            catch (...) { \
                 \
                vivisect::core::volatile_nop(); \
                __vivisect_result = 2; \
            } \
        } else { \
             \
            vivisect::integration::execute_prologue(); \
            \
            if (vivisect::integration::main_protection_config.enable_control_flow) { \
                volatile int __state = 0; \
                volatile bool __done = false; \
                \
                while (!__done) { \
                    switch (__state) { \
                        case 0: \
                            { body } \
                            __state = 1; \
                            break; \
                        case 1: \
                            vivisect::integration::execute_epilogue(); \
                            __done = true; \
                            break; \
                        default: \
                            __done = true; \
                            break; \
                    } \
                } \
            } else { \
                { body } \
                vivisect::integration::execute_epilogue(); \
            } \
        } \
        \
        return __vivisect_result; \
    }
#define VIVISECT_MAIN_SIMPLE(body) \
    int main(int argc, char** argv) { \
        vivisect::integration::execute_prologue(); \
        { body } \
        vivisect::integration::execute_epilogue(); \
        return 0; \
    }
#define VIVISECT_CONFIGURE_MAIN_PROTECTION(config_lambda) \
    namespace { \
        struct __MainProtectionConfigurator { \
            __MainProtectionConfigurator() { \
                config_lambda(vivisect::integration::main_protection_config); \
            } \
        } __main_protection_configurator_instance; \
    }
#endif 