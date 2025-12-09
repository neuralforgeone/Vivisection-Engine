// Vivisection Engine - Network API Wrapper
// Stealthy networking APIs

#ifndef VIVISECT_API_NETWORK_HPP
#define VIVISECT_API_NETWORK_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <windows.h>

namespace vivisect::api {

class NetworkAPI {
public:
    NetworkAPI() : initialized_(false) {}
    
    bool is_initialized() const { return initialized_; }
    
private:
    bool initialized_;
};

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_NETWORK_HPP
