#ifndef VIVISECT_API_CRYPTO_HPP
#define VIVISECT_API_CRYPTO_HPP
#ifdef VIVISECT_PLATFORM_WINDOWS
#include <windows.h>
#include <wincrypt.h>
#include "resolver.hpp"
namespace vivisect::api {
class CryptoAPI {
public:
    CryptoAPI() : initialized_(false) {
        resolve_functions();
    }
    bool is_initialized() const { return initialized_; }
    BOOL acquire_context(HCRYPTPROV* prov, LPCSTR container, LPCSTR provider, DWORD type, DWORD flags) {
        if (!acquire_context_) return FALSE;
        return acquire_context_(prov, container, provider, type, flags);
    }
    BOOL release_context(HCRYPTPROV prov, DWORD flags) {
        if (!release_context_) return FALSE;
        return release_context_(prov, flags);
    }
    BOOL create_hash(HCRYPTPROV prov, ALG_ID alg, HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash) {
        if (!create_hash_) return FALSE;
        return create_hash_(prov, alg, key, flags, hash);
    }
    BOOL hash_data(HCRYPTHASH hash, const BYTE* data, DWORD len, DWORD flags) {
        if (!hash_data_) return FALSE;
        return hash_data_(hash, data, len, flags);
    }
    BOOL destroy_hash(HCRYPTHASH hash) {
        if (!destroy_hash_) return FALSE;
        return destroy_hash_(hash);
    }
    BOOL derive_key(HCRYPTPROV prov, ALG_ID alg, HCRYPTHASH hash, DWORD flags, HCRYPTKEY* key) {
        if (!derive_key_) return FALSE;
        return derive_key_(prov, alg, hash, flags, key);
    }
    BOOL encrypt(HCRYPTKEY key, HCRYPTHASH hash, BOOL final, DWORD flags, BYTE* data, DWORD* len, DWORD buf_len) {
        if (!encrypt_) return FALSE;
        return encrypt_(key, hash, final, flags, data, len, buf_len);
    }
    BOOL decrypt(HCRYPTKEY key, HCRYPTHASH hash, BOOL final, DWORD flags, BYTE* data, DWORD* len) {
        if (!decrypt_) return FALSE;
        return decrypt_(key, hash, final, flags, data, len);
    }
    BOOL destroy_key(HCRYPTKEY key) {
        if (!destroy_key_) return FALSE;
        return destroy_key_(key);
    }
    BOOL gen_random(HCRYPTPROV prov, DWORD len, BYTE* buffer) {
        if (!gen_random_) return FALSE;
        return gen_random_(prov, len, buffer);
    }
private:
    void resolve_functions() {
        HMODULE advapi32 = APIResolver::find_module("advapi32.dll");
        if (!advapi32) {
            initialized_ = false;
            return;
        }
        acquire_context_ = reinterpret_cast<CryptAcquireContextA_t>(
            APIResolver::find_export(advapi32, "CryptAcquireContextA"));
        release_context_ = reinterpret_cast<CryptReleaseContext_t>(
            APIResolver::find_export(advapi32, "CryptReleaseContext"));
        create_hash_ = reinterpret_cast<CryptCreateHash_t>(
            APIResolver::find_export(advapi32, "CryptCreateHash"));
        hash_data_ = reinterpret_cast<CryptHashData_t>(
            APIResolver::find_export(advapi32, "CryptHashData"));
        destroy_hash_ = reinterpret_cast<CryptDestroyHash_t>(
            APIResolver::find_export(advapi32, "CryptDestroyHash"));
        derive_key_ = reinterpret_cast<CryptDeriveKey_t>(
            APIResolver::find_export(advapi32, "CryptDeriveKey"));
        encrypt_ = reinterpret_cast<CryptEncrypt_t>(
            APIResolver::find_export(advapi32, "CryptEncrypt"));
        decrypt_ = reinterpret_cast<CryptDecrypt_t>(
            APIResolver::find_export(advapi32, "CryptDecrypt"));
        destroy_key_ = reinterpret_cast<CryptDestroyKey_t>(
            APIResolver::find_export(advapi32, "CryptDestroyKey"));
        gen_random_ = reinterpret_cast<CryptGenRandom_t>(
            APIResolver::find_export(advapi32, "CryptGenRandom"));
        initialized_ = (acquire_context_ != nullptr && 
                       release_context_ != nullptr &&
                       create_hash_ != nullptr &&
                       hash_data_ != nullptr &&
                       destroy_hash_ != nullptr &&
                       derive_key_ != nullptr &&
                       encrypt_ != nullptr &&
                       decrypt_ != nullptr &&
                       destroy_key_ != nullptr &&
                       gen_random_ != nullptr);
    }
    using CryptAcquireContextA_t = BOOL(WINAPI*)(HCRYPTPROV*, LPCSTR, LPCSTR, DWORD, DWORD);
    using CryptReleaseContext_t = BOOL(WINAPI*)(HCRYPTPROV, DWORD);
    using CryptCreateHash_t = BOOL(WINAPI*)(HCRYPTPROV, ALG_ID, HCRYPTKEY, DWORD, HCRYPTHASH*);
    using CryptHashData_t = BOOL(WINAPI*)(HCRYPTHASH, const BYTE*, DWORD, DWORD);
    using CryptDestroyHash_t = BOOL(WINAPI*)(HCRYPTHASH);
    using CryptDeriveKey_t = BOOL(WINAPI*)(HCRYPTPROV, ALG_ID, HCRYPTHASH, DWORD, HCRYPTKEY*);
    using CryptEncrypt_t = BOOL(WINAPI*)(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE*, DWORD*, DWORD);
    using CryptDecrypt_t = BOOL(WINAPI*)(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE*, DWORD*);
    using CryptDestroyKey_t = BOOL(WINAPI*)(HCRYPTKEY);
    using CryptGenRandom_t = BOOL(WINAPI*)(HCRYPTPROV, DWORD, BYTE*);
    CryptAcquireContextA_t acquire_context_ = nullptr;
    CryptReleaseContext_t release_context_ = nullptr;
    CryptCreateHash_t create_hash_ = nullptr;
    CryptHashData_t hash_data_ = nullptr;
    CryptDestroyHash_t destroy_hash_ = nullptr;
    CryptDeriveKey_t derive_key_ = nullptr;
    CryptEncrypt_t encrypt_ = nullptr;
    CryptDecrypt_t decrypt_ = nullptr;
    CryptDestroyKey_t destroy_key_ = nullptr;
    CryptGenRandom_t gen_random_ = nullptr;
    bool initialized_;
};
} 
#endif 
#endif 