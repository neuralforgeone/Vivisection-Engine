// Vivisection Engine - Control Flow Obfuscation Module
// Control flow flattening and state machine generation

#ifndef VIVISECT_MODULES_CONTROL_FLOW_HPP
#define VIVISECT_MODULES_CONTROL_FLOW_HPP

namespace vivisect::modules {

// Dispatch strategies for control flow flattening
enum class DispatchStrategy {
    SWITCH_BASED,      // Traditional switch-case dispatcher
    COMPUTED_GOTO,     // Computed goto (GCC/Clang)
    FUNCTION_POINTER,  // Function pointer table
    HYBRID             // Combination of strategies
};

template<DispatchStrategy Strategy = DispatchStrategy::HYBRID>
class ControlFlowFlattener {
public:
    // Control flow flattening logic
};

#define VIVISECT_FLATTEN_BEGIN(name)
#define VIVISECT_FLATTEN_STATE(id)
#define VIVISECT_FLATTEN_END()

} // namespace vivisect::modules

#endif // VIVISECT_MODULES_CONTROL_FLOW_HPP
