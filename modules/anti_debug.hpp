// Vivisection Engine - Anti-Debug Module
// Anti-debugging and anti-analysis techniques

#ifndef VIVISECT_MODULES_ANTI_DEBUG_HPP
#define VIVISECT_MODULES_ANTI_DEBUG_HPP

namespace vivisect::modules {

// Debugger response actions
enum class DebuggerResponse {
    IGNORE,
    EXIT,
    CRASH,
    CUSTOM
};

class AntiDebug {
public:
    static bool is_debugger_present() {
        return false;
    }
    
    static void respond(DebuggerResponse response) {
        // Response handling
    }
};

#define VIVISECT_ANTI_DEBUG(response)

} // namespace vivisect::modules

#endif // VIVISECT_MODULES_ANTI_DEBUG_HPP
