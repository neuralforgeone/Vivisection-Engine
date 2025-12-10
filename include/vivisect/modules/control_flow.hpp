#ifndef VIVISECT_MODULES_CONTROL_FLOW_HPP
#define VIVISECT_MODULES_CONTROL_FLOW_HPP
#include <cstdint>
#include <functional>
#include <array>
#include "../core/primitives.hpp"
#include "../core/random.hpp"
namespace vivisect::modules {
enum class DispatchStrategy {
    SWITCH_BASED,      
    COMPUTED_GOTO,     
    FUNCTION_POINTER,  
    HYBRID             
};
template<DispatchStrategy Strategy = DispatchStrategy::HYBRID>
class ControlFlowFlattener {
public:
    template<typename... States>
    static void flatten(States&&... states) {
    }
    static void inject_bogus_paths(int complexity_level) {
        volatile int dummy = 0;
        for (int i = 0; i < complexity_level; ++i) {
            if (vivisect::core::opaque_false(i, vivisect::core::global_seed)) {
                dummy = dummy * 2 + i;
                if (dummy > 1000) {
                    dummy = 0;
                }
            }
            vivisect::core::volatile_nop();
        }
    }
    static void add_opaque_branches(int count) {
        volatile int branch_taken = 0;
        for (int i = 0; i < count; ++i) {
            if (vivisect::core::opaque_true(i, vivisect::core::global_seed)) {
                branch_taken++;
            } else {
                branch_taken--;
            }
            vivisect::core::volatile_seed_update(vivisect::core::global_seed);
        }
    }
    template<int Line, int Counter>
    static constexpr uint32_t generate_state_id() {
        return vivisect::core::CompileTimeRandom<
            vivisect::core::mix_seed(Line, Counter)
        >::next();
    }
};
template<>
class ControlFlowFlattener<DispatchStrategy::SWITCH_BASED> {
public:
    template<typename... States>
    static void flatten(States&&... states) {
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
#if defined(__GNUC__) || defined(__clang__)
template<>
class ControlFlowFlattener<DispatchStrategy::COMPUTED_GOTO> {
public:
    template<typename... States>
    static void flatten(States&&... states) {
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
template<>
class ControlFlowFlattener<DispatchStrategy::FUNCTION_POINTER> {
public:
    template<typename... States>
    static void flatten(States&&... states) {
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
namespace detail {
    using StateFunc = void(*)();
    struct StateMachineContext {
        uint32_t current_state;
        uint32_t next_state;
        bool running;
        int& seed_ref;
        StateMachineContext(int& seed) 
            : current_state(0), next_state(0), running(true), seed_ref(seed) {}
    };
    template<int UniqueId>
    constexpr uint32_t make_state_id() {
        return vivisect::core::CompileTimeRandom<
            vivisect::core::mix_seed(UniqueId, __LINE__)
        >::next();
    }
}
#define VIVISECT_FLATTEN_BEGIN(name) \
    { \
        vivisect::modules::detail::StateMachineContext name##_ctx(vivisect::core::global_seed); \
        constexpr uint32_t name##_initial_state = vivisect::modules::detail::make_state_id<__COUNTER__>(); \
        name##_ctx.current_state = name##_initial_state; \
        name##_ctx.running = true; \
        \
        while (name##_ctx.running) { \
             \
            vivisect::modules::ControlFlowFlattener<>::inject_bogus_paths(2); \
            \
             \
            switch (name##_ctx.current_state) {
#define VIVISECT_FLATTEN_STATE(id) \
            case vivisect::modules::detail::make_state_id<__COUNTER__>(): \
                 \
                vivisect::modules::ControlFlowFlattener<>::add_opaque_branches(1);
#define VIVISECT_FLATTEN_NEXT(name, next_id) \
                name##_ctx.current_state = vivisect::modules::detail::make_state_id<next_id>(); \
                break;
#define VIVISECT_FLATTEN_END(name) \
            default: \
                 \
                name##_ctx.running = false; \
                break; \
            } \
            \
             \
            vivisect::core::volatile_seed_update(name##_ctx.seed_ref); \
        } \
    }
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
} 
#endif 