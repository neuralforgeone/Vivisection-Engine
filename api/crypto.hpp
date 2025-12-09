// Vivisection Engine - Crypto API Wrapper
// Stealthy cryptography APIs

#ifndef VIVISECT_API_CRYPTO_HPP
#define VIVISECT_API_CRYPTO_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <windows.h>

namespace vivisect::api {

class CryptoAPI {
public:
    CryptoAPI() : initialized_(false) {}
    
    bool is_initialized() const { return initialized_; }
    
private:
    bool initialized_;
};

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_CRYPTO_HPP
