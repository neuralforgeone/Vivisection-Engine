// Vivisection Engine - Network API Wrapper
// Stealthy networking APIs

#ifndef VIVISECT_API_NETWORK_HPP
#define VIVISECT_API_NETWORK_HPP

#ifdef VIVISECT_PLATFORM_WINDOWS

#include <windows.h>
#include <wininet.h>
#include "resolver.hpp"

namespace vivisect::api {

class NetworkAPI {
public:
    NetworkAPI() : initialized_(false) {
        resolve_functions();
    }
    
    bool is_initialized() const { return initialized_; }
    
    // Internet operations
    HINTERNET internet_open(LPCSTR agent, DWORD access_type, LPCSTR proxy, LPCSTR proxy_bypass, DWORD flags) {
        if (!internet_open_) return nullptr;
        return internet_open_(agent, access_type, proxy, proxy_bypass, flags);
    }
    
    HINTERNET internet_connect(HINTERNET internet, LPCSTR server, INTERNET_PORT port, 
                               LPCSTR username, LPCSTR password, DWORD service, DWORD flags, DWORD_PTR context) {
        if (!internet_connect_) return nullptr;
        return internet_connect_(internet, server, port, username, password, service, flags, context);
    }
    
    BOOL internet_close_handle(HINTERNET handle) {
        if (!internet_close_handle_) return FALSE;
        return internet_close_handle_(handle);
    }
    
    // HTTP operations
    HINTERNET http_open_request(HINTERNET connect, LPCSTR verb, LPCSTR object, LPCSTR version,
                                LPCSTR referrer, LPCSTR* accept_types, DWORD flags, DWORD_PTR context) {
        if (!http_open_request_) return nullptr;
        return http_open_request_(connect, verb, object, version, referrer, accept_types, flags, context);
    }
    
    BOOL http_send_request(HINTERNET request, LPCSTR headers, DWORD headers_len, LPVOID optional, DWORD optional_len) {
        if (!http_send_request_) return FALSE;
        return http_send_request_(request, headers, headers_len, optional, optional_len);
    }
    
    BOOL http_query_info(HINTERNET request, DWORD info_level, LPVOID buffer, LPDWORD buffer_len, LPDWORD index) {
        if (!http_query_info_) return FALSE;
        return http_query_info_(request, info_level, buffer, buffer_len, index);
    }
    
    // Data transfer
    BOOL internet_read_file(HINTERNET file, LPVOID buffer, DWORD bytes_to_read, LPDWORD bytes_read) {
        if (!internet_read_file_) return FALSE;
        return internet_read_file_(file, buffer, bytes_to_read, bytes_read);
    }
    
    BOOL internet_write_file(HINTERNET file, LPCVOID buffer, DWORD bytes_to_write, LPDWORD bytes_written) {
        if (!internet_write_file_) return FALSE;
        return internet_write_file_(file, buffer, bytes_to_write, bytes_written);
    }
    
private:
    void resolve_functions() {
        HMODULE wininet = APIResolver::find_module("wininet.dll");
        if (!wininet) {
            initialized_ = false;
            return;
        }
        
        internet_open_ = reinterpret_cast<InternetOpenA_t>(
            APIResolver::find_export(wininet, "InternetOpenA"));
        internet_connect_ = reinterpret_cast<InternetConnectA_t>(
            APIResolver::find_export(wininet, "InternetConnectA"));
        internet_close_handle_ = reinterpret_cast<InternetCloseHandle_t>(
            APIResolver::find_export(wininet, "InternetCloseHandle"));
        http_open_request_ = reinterpret_cast<HttpOpenRequestA_t>(
            APIResolver::find_export(wininet, "HttpOpenRequestA"));
        http_send_request_ = reinterpret_cast<HttpSendRequestA_t>(
            APIResolver::find_export(wininet, "HttpSendRequestA"));
        http_query_info_ = reinterpret_cast<HttpQueryInfoA_t>(
            APIResolver::find_export(wininet, "HttpQueryInfoA"));
        internet_read_file_ = reinterpret_cast<InternetReadFile_t>(
            APIResolver::find_export(wininet, "InternetReadFile"));
        internet_write_file_ = reinterpret_cast<InternetWriteFile_t>(
            APIResolver::find_export(wininet, "InternetWriteFile"));
        
        // Check if all critical functions were resolved
        initialized_ = (internet_open_ != nullptr && 
                       internet_connect_ != nullptr &&
                       internet_close_handle_ != nullptr &&
                       http_open_request_ != nullptr &&
                       http_send_request_ != nullptr &&
                       http_query_info_ != nullptr &&
                       internet_read_file_ != nullptr &&
                       internet_write_file_ != nullptr);
    }
    
    // Function pointer types
    using InternetOpenA_t = HINTERNET(WINAPI*)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD);
    using InternetConnectA_t = HINTERNET(WINAPI*)(HINTERNET, LPCSTR, INTERNET_PORT, 
                                                   LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR);
    using InternetCloseHandle_t = BOOL(WINAPI*)(HINTERNET);
    using HttpOpenRequestA_t = HINTERNET(WINAPI*)(HINTERNET, LPCSTR, LPCSTR, LPCSTR,
                                                   LPCSTR, LPCSTR*, DWORD, DWORD_PTR);
    using HttpSendRequestA_t = BOOL(WINAPI*)(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD);
    using HttpQueryInfoA_t = BOOL(WINAPI*)(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD);
    using InternetReadFile_t = BOOL(WINAPI*)(HINTERNET, LPVOID, DWORD, LPDWORD);
    using InternetWriteFile_t = BOOL(WINAPI*)(HINTERNET, LPCVOID, DWORD, LPDWORD);
    
    // Function pointers
    InternetOpenA_t internet_open_ = nullptr;
    InternetConnectA_t internet_connect_ = nullptr;
    InternetCloseHandle_t internet_close_handle_ = nullptr;
    HttpOpenRequestA_t http_open_request_ = nullptr;
    HttpSendRequestA_t http_send_request_ = nullptr;
    HttpQueryInfoA_t http_query_info_ = nullptr;
    InternetReadFile_t internet_read_file_ = nullptr;
    InternetWriteFile_t internet_write_file_ = nullptr;
    
    bool initialized_;
};

} // namespace vivisect::api

#endif // VIVISECT_PLATFORM_WINDOWS

#endif // VIVISECT_API_NETWORK_HPP
