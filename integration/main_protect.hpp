// Vivisection Engine - Main Function Protection
// Comprehensive obfuscation wrapper for main entry point

#ifndef VIVISECT_INTEGRATION_MAIN_PROTECT_HPP
#define VIVISECT_INTEGRATION_MAIN_PROTECT_HPP

#include "../modules/vm_engine.hpp"
#include "../modules/anti_debug.hpp"
#include "../modules/control_flow.hpp"
#include "../modules/junk_code.hpp"
#include "../core/config.hpp"

namespace vivisect::integration {

#define VIVISECT_MAIN(body) \
    int main(int argc, char** argv) { \
        body \
        return 0; \
    }

} // namespace vivisect::integration

#endif // VIVISECT_INTEGRATION_MAIN_PROTECT_HPP
