// Vivisection Engine - Control Flow Obfuscation Module
// Multi-layered control flow flattening with various dispatcher strategies

#ifndef VIVISECT_MODULES_CONTROL_FLOW_HPP
#define VIVISECT_MODULES_CONTROL_FLOW_HPP

#include <cstdint>
#include <functional>
#include <array>
#include "../core/primitives.hpp"
#include "../core/random.hpp"

namespace vivisect::modules {

// Dispatcher strategy enumeration
enum class DispatchStrategy {
    SWITCH_BASED,      // Traditional switch-case dispatcher
    COMPUTED_GOTO,     // Computed goto (GCC/Clang)
    FUNCTION_POINTER,  // Function pointer table
    HYBRID             // Combination of strategies
};

// Control flow flattener template class
template<DispatchStrategy Strategy = DispatchStrategy::HYBRID>
class ControlFlowFlattener {
public:
    // State machine generation
    template<typename... States>
    static void flatten(States&&... states) {
        // This is a compile-time interface for state machine generation
        // The actual flattening is done through macros for practical use
    }
    
    // Bogus flow injection
    static void inject_bogus_paths(int complexity_level) {
        // Inject fake control flow paths based on complexity
        volatile int dummy = 0;
        
        for (int i = 0; i < complexity_level; ++i) {
            // Create opaque predicates that always evaluate to false
            if (vivisect::core::opaque_false(i, vivisect::core::global_seed)) {
                // This path is never taken but appears legitimate
                dummy = dummy * 2 + i;
                if (dummy > 1000) {
                    dummy = 0;
                }
            }
            
            // Add some volatile operations to prevent optimization
            vivisect::core::volatile_nop();
        }
    }
    
    // Opaque predicate integration
    static void add_opaque_branches(int count) {
        volatile int branch_taken = 0;
        
        for (int i = 0; i < count; ++i) {
            // Add branches that always go one way but are hard to analyze
            if (vivisect::core::opaque_true(i, vivisect::core::global_seed)) {
                branch_taken++;
            } else {
                // Dead code path
                branch_taken--;
            }
            
            vivisect::core::volatile_seed_update(vivisect::core::global_seed);
        }
    }
    
    // Generate unique state ID based on compile-time context
    template<int Line, int Counter>
    static constexpr uint32_t generate_state_id() {
        return vivisect::core::CompileTimeRandom<
            vivisect::core::mix_seed(Line, Counter)
        >::next();
    }
};

// Switch-based dispatcher implementation
template<>
class ControlFlowFlattener<DispatchStrategy::SWITCH_BASED> {
public:
    template<typename... States>
    static void flatten(States&&... states) {
        // Switch-based implementation
    }
    
    static void inject_bogus_paths(int complexity_level) {
        volatile int dummy = 0;
        for (int i = 0; i < complexity_level; ++i) {
            if (vivisect::core::opaque_false(i, vivisect::core::global_seed)) {
                dummy = dummy * 2 + i;
            }
            vivisect::core::volatile_nop();
        }
    }
    
    static void add_opaque_branches(int count) {
        volatile int branch_taken = 0;
        for (int i = 0; i < count; ++i) {
            if (vivisect::core::opaque_true(i, vivisect::core::global_seed)) {
                branch_taken++;
            }
            vivisect::core::volatile_seed_update(vivisect::core::global_seed);
        }
    }
};

// Computed goto dispatcher (GCC/Clang only)
#if defined(__GNUC__) || defined(__clang__)
template<>
class ControlFlowFlattener<DispatchStrategy::COMPUTED_GOTO> {
public:
    template<typename... States>
    static void flatten(States&&... states) {
        // Computed goto implementation
    }
    
    static void inject_bogus_paths(int complexity_level) {
        volatile int dummy = 0;
        for (int i = 0; i < complexity_level; ++i) {
            if (vivisect::core::opaque_false(i, vivisect::core::global_seed)) {
                dummy = dummy * 2 + i;
            }
            vivisect::core::volatile_nop();
        }
    }
    
    static void add_opaque_branches(int count) {
        volatile int branch_taken = 0;
        for (int i = 0; i < count; ++i) {
            if (vivisect::core::opaque_true(i, vivisect::core::global_seed)) {
                branch_taken++;
            }
            vivisect::core::volatile_seed_update(vivisect::core::global_seed);
        }
    }
};
#endif

// Function pointer table dispatcher
template<>
class ControlFlowFlattener<DispatchStrategy::FUNCTION_POINTER> {
public:
    template<typename... States>
    static void flatten(States&&... states) {
        // Function pointer table implementation
    }
    
    static void inject_bogus_paths(int complexity_level) {
        volatile int dummy = 0;
        for (int i = 0; i < complexity_level; ++i) {
            if (vivisect::core::opaque_false(i, vivisect::core::global_seed)) {
                dummy = dummy * 2 + i;
            }
            vivisect::core::volatile_nop();
        }
    }
    
    static void add_opaque_branches(int count) {
        volatile int branch_taken = 0;
        for (int i = 0; i < count; ++i) {
            if (vivisect::core::opaque_true(i, vivisect::core::global_seed)) {
                branch_taken++;
            }
            vivisect::core::volatile_seed_update(vivisect::core::global_seed);
        }
    }
};

// Internal state machine implementation details
namespace detail {
    // State transition function type
    using StateFunc = void(*)();
    
    // State machine context
    struct StateMachineContext {
        uint32_t current_state;
        uint32_t next_state;
        bool running;
        int& seed_ref;
        
        StateMachineContext(int& seed) 
            : current_state(0), next_state(0), running(true), seed_ref(seed) {}
    };
    
    // Helper for generating unique state IDs
    template<int UniqueId>
    constexpr uint32_t make_state_id() {
        return vivisect::core::CompileTimeRandom<
            vivisect::core::mix_seed(UniqueId, __LINE__)
        >::next();
    }
}

// Convenience macros for control flow flattening
// These macros create a state machine that obfuscates the control flow

// Begin a flattened control flow block
#define VIVISECT_FLATTEN_BEGIN(name) \
    { \
        vivisect::modules::detail::StateMachineContext name##_ctx(vivisect::core::global_seed); \
        constexpr uint32_t name##_initial_state = vivisect::modules::detail::make_state_id<__COUNTER__>(); \
        name##_ctx.current_state = name##_initial_state; \
        name##_ctx.running = true; \
        \
        while (name##_ctx.running) { \
            /* Inject bogus paths before dispatch */ \
            vivisect::modules::ControlFlowFlattener<>::inject_bogus_paths(2); \
            \
            /* State dispatcher using switch */ \
            switch (name##_ctx.current_state) {

// Define a state in the flattened control flow
#define VIVISECT_FLATTEN_STATE(id) \
            case vivisect::modules::detail::make_state_id<__COUNTER__>(): \
                /* Add opaque branches */ \
                vivisect::modules::ControlFlowFlattener<>::add_opaque_branches(1);

// Transition to next state
#define VIVISECT_FLATTEN_NEXT(name, next_id) \
                name##_ctx.current_state = vivisect::modules::detail::make_state_id<next_id>(); \
                break;

// End the flattened control flow block
#define VIVISECT_FLATTEN_END(name) \
            default: \
                /* Exit state machine */ \
                name##_ctx.running = false; \
                break; \
            } \
            \
            /* Update seed for next iteration */ \
            vivisect::core::volatile_seed_update(name##_ctx.seed_ref); \
        } \
    }

// Simplified macro for single-block flattening
#define VIVISECT_FLATTEN_BLOCK(code) \
    { \
        vivisect::modules::detail::StateMachineContext block_ctx(vivisect::core::global_seed); \
        constexpr uint32_t block_initial_state = vivisect::modules::detail::make_state_id<__COUNTER__>(); \
        block_ctx.current_state = block_initial_state; \
        block_ctx.running = true; \
        \
        while (block_ctx.running) { \
            vivisect::modules::ControlFlowFlattener<>::inject_bogus_paths(1); \
            \
            switch (block_ctx.current_state) { \
            case block_initial_state: \
                vivisect::modules::ControlFlowFlattener<>::add_opaque_branches(1); \
                code \
                block_ctx.running = false; \
                break; \
            default: \
                block_ctx.running = false; \
                break; \
            } \
            \
            vivisect::core::volatile_seed_update(block_ctx.seed_ref); \
        } \
    }

} // namespace vivisect::modules

#endif // VIVISECT_MODULES_CONTROL_FLOW_HPP
