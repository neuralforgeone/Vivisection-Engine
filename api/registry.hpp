// Vivisection Engine - Registry API Wrapper
// Stealthy Windows Registry APIs

#ifndef VIVISECT_API_REGISTRY_HPP
#define VIVISECT_API_REGISTRY_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <windows.h>

namespace vivisect::api {

class RegistryAPI {
public:
    RegistryAPI() : initialized_(false) {}
    
    bool is_initialized() const { return initialized_; }
    
private:
    bool initialized_;
};

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_REGISTRY_HPP
