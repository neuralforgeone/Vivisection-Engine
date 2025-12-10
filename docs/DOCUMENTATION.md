# Vivisection Engine – Complete Technical Documentation

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Core Primitives](#core-primitives)
3. [String Encryption](#string-encryption)
4. [MBA Transformations](#mba-transformations)
5. [Control Flow Flattening](#control-flow-flattening)
6. [Virtual Machine Engine](#virtual-machine-engine)
7. [Anti-Debug Module](#anti-debug-module)
8. [Junk Code Generation](#junk-code-generation)
9. [API Resolution System](#api-resolution-system)
10. [API Wrapper Classes](#api-wrapper-classes)
11. [Main Function Protection](#main-function-protection)
12. [Configuration System](#configuration-system)
13. [Error Handling](#error-handling)
14. [Performance Characteristics](#performance-characteristics)
15. [Platform Support](#platform-support)
16. [Build Instructions](#build-instructions)
17. [Integration Guide](#integration-guide)
18. [Troubleshooting](#troubleshooting)

---

## Architecture Overview

Vivisection Engine is organized into five layers:

**1. Core Layer** (`core/`)
- Compile-time primitives
- Random number generation
- Opaque predicates
- Type concepts
- Configuration structures

**2. Module Layer** (`modules/`)
- String encryption
- MBA transformations
- Control flow flattening
- VM engine
- Anti-debug
- Junk code generation

**3. API Layer** (`api/`)
- PEB-based API resolution
- Wrapper classes for Windows APIs
- Hash-based symbol lookup

**4. Integration Layer** (`integration/`)
- High-level macros
- Main function protection
- Convenience interfaces

**5. Configuration Layer** (`core/config.hpp`)
- Global configuration
- Per-function control
- Preset profiles

All components are header-only. No linking required.

---

## Core Primitives

Location: `include/vivisect/core/primitives.hpp`

### Opaque Predicates

Conditionals whose outcome is known at compile-time but obscured at runtime.


**Functions:**
```cpp
template<typename T>
constexpr bool opaque_true(T value, int seed);

template<typename T>
constexpr bool opaque_false(T value, int seed);
```

Always evaluates to true/false respectively, but analysis tools cannot determine this statically.

**Usage:**
```cpp
if (opaque_true(&some_var, __LINE__)) {
    // Real code path
} else {
    // Dead code path (never executed)
}
```

### Compile-Time Seeds

```cpp
constexpr uint32_t compile_time_seed();
constexpr uint32_t mix_seed(uint32_t a, uint32_t b);
```

Generates unique seeds based on `__LINE__`, `__COUNTER__`, and `__TIME__`.

**Macro:**
```cpp
#define VIVISECT_UNIQUE_SEED (__LINE__ ^ __COUNTER__ ^ compile_time_seed())
```

Use this to generate unique values per call site.

### Volatile Operations

```cpp
void volatile_nop();
void volatile_seed_update(int& seed);
```

Prevents compiler from optimizing away junk code.

---

## String Encryption

Location: `include/vivisect/modules/string_crypt.hpp`

### Cipher Implementations

**XTEA Cipher:**
```cpp
class XTEACipher {
    static constexpr void encrypt(uint32_t* data, const uint32_t* key);
    static constexpr void decrypt(uint32_t* data, const uint32_t* key);
};
```

**AES-Like Cipher:**
```cpp
class AESLikeCipher {
    static constexpr void encrypt(uint32_t* data, const uint32_t* key);
    static constexpr void decrypt(uint32_t* data, const uint32_t* key);
};
```

### EncryptedString Template

```cpp
template<size_t N, typename Cipher = XTEACipher>
class EncryptedString {
public:
    constexpr EncryptedString(const char (&str)[N]);
    std::string decrypt() const;
    const char* c_str() const;
};
```


**Encryption Process:**
1. String encrypted at compile-time
2. Unique key generated per string (based on location)
3. Stored as encrypted uint32_t array
4. Decrypted on stack when accessed
5. Minimal plaintext lifetime

**Macro:**
```cpp
#define VIVISECT_STR(str) \
    vivisect::modules::EncryptedString<sizeof(str)>(str).decrypt()
```

**Example:**
```cpp
std::string api_key = VIVISECT_STR("sk_live_abc123");
// String never appears in plaintext in binary
```

**PE Section Distribution (Windows):**

When enabled, encrypted fragments are distributed across multiple PE sections to further obscure data.

```cpp
vivisect::config::current_profile.distribute_across_sections = true;
```

---

## MBA Transformations

Location: `include/vivisect/modules/mba.hpp`

Mixed Boolean-Arithmetic expressions replace simple operations with mathematically equivalent but complex formulas.

### Supported Operations

```cpp
template<typename T>
class MBA {
    static constexpr T add(T a, T b, int variant = 0);
    static constexpr T sub(T a, T b, int variant = 0);
    static constexpr T xor_op(T a, T b, int variant = 0);
    static constexpr T and_op(T a, T b, int variant = 0);
    static constexpr T or_op(T a, T b, int variant = 0);
    static constexpr T not_op(T a, int variant = 0);
    static constexpr T chain(T value, int depth);
};
```

### Formula Variants

**ADD variants:**
- `(a ^ b) + 2 * (a & b)`
- `(a | b) + (a & b)`
- `2 * (a | b) - (a ^ b)`
- `(a - ~b) - 1`

**SUB variants:**
- `(a ^ b) - 2 * (~a & b)`
- `(a & ~b) - (~a & b)`
- `a + (~b + 1)`

**XOR variants:**
- `(a | b) - (a & b)`
- `(a + b) - 2 * (a & b)`


### Macros

```cpp
#define VIVISECT_MBA_ADD(a, b)
#define VIVISECT_MBA_SUB(a, b)
#define VIVISECT_MBA_XOR(a, b)
#define VIVISECT_MBA_AND(a, b)
#define VIVISECT_MBA_OR(a, b)
#define VIVISECT_MBA_NOT(a)
```

**Example:**
```cpp
int x = 10, y = 20;
int sum = VIVISECT_MBA_ADD(x, y);  // Computes 30 via complex formula
```

### Chaining

```cpp
int value = 42;
int obfuscated = MBA::chain(value, 5);  // Apply 5 layers of MBA
```

Chains multiple MBA operations to increase complexity.

**Resistance to Symbolic Execution:**

MBA expressions are designed to resist simplification by symbolic execution engines. Multiple variants prevent pattern matching.

---

## Control Flow Flattening

Location: `include/vivisect/modules/control_flow.hpp`

Transforms structured control flow into state machine-based execution.

### Dispatcher Strategies

```cpp
enum class DispatchStrategy {
    SWITCH_BASED,      // switch(state) { case 0: ... }
    COMPUTED_GOTO,     // goto *jump_table[state] (GCC/Clang)
    FUNCTION_POINTER,  // func_table[state]()
    HYBRID             // Mix of strategies
};
```

### ControlFlowFlattener Template

```cpp
template<DispatchStrategy Strategy = DispatchStrategy::HYBRID>
class ControlFlowFlattener {
public:
    template<typename... States>
    static void flatten(States&&... states);
    
    static void inject_bogus_paths(int complexity_level);
    static void add_opaque_branches(int count);
};
```

### Macros

```cpp
VIVISECT_FLATTEN_BEGIN(name)
VIVISECT_FLATTEN_STATE(id) { /* code */ }
VIVISECT_FLATTEN_END()
```


**Example:**
```cpp
void protected_function() {
    VIVISECT_FLATTEN_BEGIN(my_func)
    
    VIVISECT_FLATTEN_STATE(0) {
        // State 0 code
    }
    
    VIVISECT_FLATTEN_STATE(1) {
        // State 1 code
    }
    
    VIVISECT_FLATTEN_STATE(2) {
        // State 2 code
    }
    
    VIVISECT_FLATTEN_END()
}
```

**Transformation:**

Direct control flow → State machine with dispatcher

```
Original:           Flattened:
A → B → C          dispatcher:
                     switch(state):
                       case 0: A; state=1; break;
                       case 1: B; state=2; break;
                       case 2: C; state=3; break;
```

**Bogus Paths:**

Inject fake control flow paths that appear legitimate but are never executed.

```cpp
ControlFlowFlattener<>::inject_bogus_paths(5);
```

**Opaque Branches:**

Add branches guarded by opaque predicates.

```cpp
ControlFlowFlattener<>::add_opaque_branches(3);
```

---

## Virtual Machine Engine

Location: `include/vivisect/modules/vm_engine.hpp`

Custom VM for executing obfuscated instruction sequences.

### VM State

```cpp
struct VMState {
    uint32_t registers[8];
    uint32_t pc;              // Program counter
    uint32_t flags;
    int& global_seed;
};
```

### Opcodes

```cpp
enum class VMOpcode {
    ADD, SUB, XOR, AND, OR, NOT,
    LOAD, STORE,
    JUMP, CALL,
    MANGLE_KEY,
    JUNK_OP
};
```


### VMEngine Class

```cpp
class VMEngine {
public:
    VMEngine(int& seed_ref);
    
    void execute(const VMOpcode* bytecode, size_t length);
    void register_handler(VMOpcode op, VMHandler handler);
    void mutate_handlers();
};
```

**Handler Type:**
```cpp
using VMHandler = void(*)(VMState&);
```

### Bytecode Generation

```cpp
template<VMOpcode... Ops>
constexpr auto make_bytecode();
```

**Example:**
```cpp
constexpr auto bytecode = make_bytecode<
    VMOpcode::LOAD,
    VMOpcode::ADD,
    VMOpcode::STORE
>();
```

### Usage

```cpp
int seed = 12345;
VMEngine vm(seed);

VMOpcode bytecode[] = {
    VMOpcode::LOAD,
    VMOpcode::ADD,
    VMOpcode::XOR,
    VMOpcode::STORE
};

vm.execute(bytecode, 4);
```

### Handler Mutation

Runtime mutation of handler table to prevent static analysis.

```cpp
vm.mutate_handlers();
```

After mutation, same bytecode produces same result but through different handler implementations.

**Anti-Devirtualization:**

- Dynamic dispatch prevents pattern matching
- Handler mutation breaks static handler identification
- MBA-obfuscated handlers resist symbolic execution

---

## Anti-Debug Module

Location: `include/vivisect/modules/anti_debug.hpp`

### Detection Methods

```cpp
class AntiDebug {
public:
    static bool is_debugger_present();
    static bool check_remote_debugger();
    static bool timing_check();
    static bool exception_check();
    static bool hardware_breakpoint_check();
};
```


**Detection Techniques:**

1. **IsDebuggerPresent API** (Windows)
2. **CheckRemoteDebuggerPresent API** (Windows)
3. **PEB flags** (BeingDebugged, NtGlobalFlag)
4. **Timing checks** (RDTSC instruction)
5. **Exception-based** (SEH probing)
6. **Hardware breakpoints** (Debug registers DR0-DR7)
7. **ptrace detection** (Linux)

### Response Actions

```cpp
enum class DebuggerResponse {
    IGNORE,   // Detect but continue
    EXIT,     // Terminate process
    CRASH,    // Intentional crash
    CUSTOM    // User-defined handler
};
```

### Response Handling

```cpp
static void respond(DebuggerResponse response);
static void set_custom_handler(void(*handler)());
```

### Continuous Monitoring

```cpp
static void start_monitoring(int interval_ms);
static void stop_monitoring();
```

Spawns background thread that periodically checks for debuggers.

### Macro

```cpp
#define VIVISECT_ANTI_DEBUG(response) \
    if (vivisect::modules::AntiDebug::is_debugger_present()) { \
        vivisect::modules::AntiDebug::respond(response); \
    }
```

**Example:**
```cpp
VIVISECT_ANTI_DEBUG(vivisect::modules::DebuggerResponse::EXIT);
```

---

## Junk Code Generation

Location: `include/vivisect/modules/junk_code.hpp`

Inserts meaningless but realistic-looking code to increase analysis difficulty.

### Patterns

```cpp
enum class JunkPattern {
    ARITHMETIC,     // x = (x * 3 + 7) ^ 0x5A
    BITWISE,        // x = (x << 3) | (x >> 5)
    MEMORY,         // volatile int tmp = stack[i]
    CONTROL_FLOW,   // if (opaque_true) { junk }
    MIXED           // Combination
};
```


### JunkCodeGenerator Class

```cpp
class JunkCodeGenerator {
public:
    template<JunkPattern Pattern = JunkPattern::MIXED>
    static void insert(int complexity);
    
    static void insert_realistic_dead_code();
    static void insert_with_opaque_predicate();
};
```

### Macro

```cpp
#define VIVISECT_JUNK(level) \
    vivisect::modules::JunkCodeGenerator::insert<>(level)
```

**Example:**
```cpp
VIVISECT_JUNK(3);  // Insert junk code with complexity 3
```

**Characteristics:**

- Uses volatile operations to prevent optimization
- Mimics legitimate code patterns
- Integrated with opaque predicates
- Configurable density

**Density Control:**
```cpp
vivisect::config::current_profile.junk_code_density = 5;
```

Higher density = more junk code = larger binary + slower execution.

---

## API Resolution System

Location: `include/vivisect/api/resolver.hpp`

Resolves Windows APIs dynamically without leaving IAT traces.

### PEB Structures

```cpp
struct PEB;
struct LDR_DATA_TABLE_ENTRY;
```

Windows Process Environment Block structures for traversing loaded modules.

### APIResolver Class

```cpp
class APIResolver {
public:
    // Module resolution
    static HMODULE find_module(uint32_t name_hash);
    static HMODULE find_module(const char* name);
    
    // Function resolution
    static void* find_export(HMODULE module, uint32_t name_hash);
    static void* find_export(HMODULE module, const char* name);
    
    // Hash computation
    static constexpr uint32_t hash(const char* str);
    
private:
    static PEB* get_peb();
};
```


### Resolution Process

1. Access PEB via segment register (FS/GS)
2. Traverse InLoadOrderModuleList
3. Compare hashed module names
4. Parse export table
5. Compare hashed function names
6. Return function address

**No GetProcAddress or LoadLibrary calls.**

### Hash-Based Resolution

```cpp
constexpr uint32_t kernel32_hash = APIResolver::hash("kernel32.dll");
constexpr uint32_t getpid_hash = APIResolver::hash("GetCurrentProcessId");

HMODULE kernel32 = APIResolver::find_module(kernel32_hash);
void* func = APIResolver::find_export(kernel32, getpid_hash);
```

Hashes computed at compile-time. No strings in binary.

### Macro

```cpp
#define VIVISECT_API(dll, func) \
    vivisect::api::APIResolver::find_export( \
        vivisect::api::APIResolver::find_module(VIVISECT_STR(dll).c_str()), \
        VIVISECT_STR(func).c_str())
```

**Example:**
```cpp
auto CreateProcessA = (CreateProcessA_t)VIVISECT_API("kernel32.dll", "CreateProcessA");
```

---

## API Wrapper Classes

Location: `include/vivisect/api/`

Pre-built wrappers that automatically resolve APIs stealthily.

### ProcessAPI

Location: `include/vivisect/api/process.hpp`

```cpp
class ProcessAPI {
public:
    ProcessAPI();
    bool is_initialized() const;
    
    // Process operations
    HANDLE open_process(DWORD access, BOOL inherit, DWORD pid);
    BOOL terminate_process(HANDLE process, UINT exit_code);
    DWORD get_current_process_id();
    
    // Memory operations
    LPVOID virtual_alloc_ex(HANDLE process, LPVOID address, SIZE_T size, DWORD type, DWORD protect);
    BOOL write_process_memory(HANDLE process, LPVOID address, LPCVOID buffer, SIZE_T size, SIZE_T* written);
    BOOL read_process_memory(HANDLE process, LPCVOID address, LPVOID buffer, SIZE_T size, SIZE_T* read);
    
    // Thread operations
    HANDLE create_remote_thread(HANDLE process, LPSECURITY_ATTRIBUTES sa, SIZE_T stack, 
                                LPTHREAD_START_ROUTINE start, LPVOID param, DWORD flags, LPDWORD tid);
    DWORD suspend_thread(HANDLE thread);
};
```


### CryptoAPI

Location: `include/vivisect/api/crypto.hpp`

```cpp
class CryptoAPI {
public:
    CryptoAPI();
    bool is_initialized() const;
    
    // Context management
    BOOL acquire_context(HCRYPTPROV* prov, LPCSTR container, LPCSTR provider, DWORD type, DWORD flags);
    BOOL release_context(HCRYPTPROV prov, DWORD flags);
    
    // Hashing
    BOOL create_hash(HCRYPTPROV prov, ALG_ID alg, HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash);
    BOOL hash_data(HCRYPTHASH hash, const BYTE* data, DWORD len, DWORD flags);
    
    // Encryption
    BOOL derive_key(HCRYPTPROV prov, ALG_ID alg, HCRYPTHASH hash, DWORD flags, HCRYPTKEY* key);
    BOOL encrypt(HCRYPTKEY key, HCRYPTHASH hash, BOOL final, DWORD flags, BYTE* data, DWORD* len, DWORD buf_len);
    BOOL decrypt(HCRYPTKEY key, HCRYPTHASH hash, BOOL final, DWORD flags, BYTE* data, DWORD* len);
    
    // Random generation
    BOOL gen_random(HCRYPTPROV prov, DWORD len, BYTE* buffer);
};
```

### NetworkAPI

Location: `include/vivisect/api/network.hpp`

```cpp
class NetworkAPI {
public:
    NetworkAPI();
    bool is_initialized() const;
    
    // Internet operations
    HINTERNET internet_open(LPCSTR agent, DWORD access_type, LPCSTR proxy, LPCSTR proxy_bypass, DWORD flags);
    HINTERNET internet_connect(HINTERNET internet, LPCSTR server, INTERNET_PORT port, 
                               LPCSTR username, LPCSTR password, DWORD service, DWORD flags, DWORD_PTR context);
    BOOL internet_close_handle(HINTERNET handle);
    
    // HTTP operations
    HINTERNET http_open_request(HINTERNET connect, LPCSTR verb, LPCSTR object, LPCSTR version,
                                LPCSTR referrer, LPCSTR* accept_types, DWORD flags, DWORD_PTR context);
    BOOL http_send_request(HINTERNET request, LPCSTR headers, DWORD headers_len, LPVOID optional, DWORD optional_len);
    
    // Data transfer
    BOOL internet_read_file(HINTERNET file, LPVOID buffer, DWORD bytes_to_read, LPDWORD bytes_read);
    BOOL internet_write_file(HINTERNET file, LPCVOID buffer, DWORD bytes_to_write, LPDWORD bytes_written);
};
```


### RegistryAPI

Location: `include/vivisect/api/registry.hpp`

```cpp
class RegistryAPI {
public:
    RegistryAPI();
    bool is_initialized() const;
    
    // Key operations
    LSTATUS create_key_ex(HKEY key, LPCSTR subkey, DWORD reserved, LPSTR class_name, DWORD options,
                         REGSAM sam, LPSECURITY_ATTRIBUTES sa, PHKEY result, LPDWORD disposition);
    LSTATUS open_key_ex(HKEY key, LPCSTR subkey, DWORD options, REGSAM sam, PHKEY result);
    LSTATUS close_key(HKEY key);
    
    // Value operations
    LSTATUS set_value_ex(HKEY key, LPCSTR value_name, DWORD reserved, DWORD type, const BYTE* data, DWORD size);
    LSTATUS query_value_ex(HKEY key, LPCSTR value_name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size);
    
    // Enumeration
    LSTATUS enum_key_ex(HKEY key, DWORD index, LPSTR name, LPDWORD name_len, LPDWORD reserved,
                       LPSTR class_name, LPDWORD class_len, PFILETIME last_write);
};
```

### Usage Pattern

```cpp
vivisect::api::ProcessAPI proc;

if (!proc.is_initialized()) {
    // Handle initialization failure
    return;
}

DWORD pid = proc.get_current_process_id();
```

All functions resolved stealthily on construction. No IAT entries.

---

## Main Function Protection

Location: `include/vivisect/integration/main_protect.hpp`

Wraps program entry point with comprehensive protection.

### VIVISECT_MAIN Macro

```cpp
#define VIVISECT_MAIN(body) \
    int main(int argc, char** argv) { \
        /* Prologue */ \
        /* VM execution */ \
        /* Anti-debug checks */ \
        /* Control flow flattening wrapper */ \
        body \
        /* Epilogue */ \
        /* Cleanup */ \
        return 0; \
    }
```


**Protection Layers Applied:**

1. VM engine initialization and execution
2. Anti-debug checks at entry
3. Control flow flattening wrapper
4. Junk code in prologue/epilogue
5. Exception handling (SEH on Windows)
6. Opaque predicate gating

**Example:**
```cpp
#include <vivisect/vivisect.hpp>

VIVISECT_MAIN({
    auto secret = VIVISECT_STR("Protected");
    return 0;
})
```

**Configuration:**

Control which protections are applied:

```cpp
auto& cfg = vivisect::config::current_profile;
cfg.enable_vm_execution = true;
cfg.enable_anti_debug = true;
cfg.enable_control_flow_flattening = true;
cfg.enable_junk_code = true;
```

---

## Configuration System

Location: `include/vivisect/core/config.hpp`

### ObfuscationProfile Structure

```cpp
struct ObfuscationProfile {
    // Control flow
    bool enable_control_flow_flattening = true;
    DispatchStrategy dispatch_strategy = DispatchStrategy::HYBRID;
    int bogus_flow_complexity = 5;
    
    // String encryption
    bool enable_string_encryption = true;
    bool distribute_across_sections = true;
    
    // VM
    bool enable_vm_execution = true;
    int vm_handler_count = 32;
    bool mutate_vm_handlers = true;
    
    // Anti-debug
    bool enable_anti_debug = true;
    DebuggerResponse debugger_response = DebuggerResponse::EXIT;
    
    // Junk code
    bool enable_junk_code = true;
    int junk_code_density = 3;
    
    // MBA
    bool enable_mba = true;
    int mba_complexity = 5;
    
    // Performance
    bool enable_performance_monitoring = false;
};
```


### Preset Profiles

```cpp
constexpr ObfuscationProfile MINIMAL_PROTECTION = {
    .enable_control_flow_flattening = false,
    .enable_vm_execution = false,
    .junk_code_density = 1,
    .mba_complexity = 2,
    // ... minimal settings
};

constexpr ObfuscationProfile BALANCED_PROTECTION = {
    .enable_control_flow_flattening = true,
    .enable_vm_execution = false,
    .junk_code_density = 3,
    .mba_complexity = 5,
    // ... balanced settings
};

constexpr ObfuscationProfile MAXIMUM_PROTECTION = {
    .enable_control_flow_flattening = true,
    .enable_vm_execution = true,
    .junk_code_density = 7,
    .mba_complexity = 10,
    // ... maximum settings
};
```

### Global Configuration

```cpp
extern ObfuscationProfile current_profile;
```

**Usage:**
```cpp
// Use preset
vivisect::config::current_profile = vivisect::config::MAXIMUM_PROTECTION;

// Or customize
auto& cfg = vivisect::config::current_profile;
cfg.enable_vm_execution = true;
cfg.junk_code_density = 5;
```

### Per-Function Configuration

```cpp
void fast_function() {
    auto saved = vivisect::config::current_profile;
    vivisect::config::current_profile = vivisect::config::MINIMAL_PROTECTION;
    
    // Fast code
    
    vivisect::config::current_profile = saved;
}

void secure_function() {
    auto saved = vivisect::config::current_profile;
    vivisect::config::current_profile = vivisect::config::MAXIMUM_PROTECTION;
    
    // Secure code
    
    vivisect::config::current_profile = saved;
}
```

---

## Error Handling

Location: `include/vivisect/error/error.hpp`

### ErrorCode Enum

```cpp
enum class ErrorCode {
    SUCCESS,
    API_RESOLUTION_FAILED,
    WRAPPER_INIT_FAILED,
    VM_EXECUTION_ERROR,
    INVALID_CONFIGURATION,
    PLATFORM_NOT_SUPPORTED
};
```


### Error Structure

```cpp
struct Error {
    ErrorCode code;
    const char* message;
    const char* file;
    int line;
};
```

### Error Handling

```cpp
using ErrorHandler = void(*)(const Error&);

void set_error_handler(ErrorHandler handler);
void log_error(const Error& error);
```

### Macro

```cpp
#define VIVISECT_ERROR(code, msg) \
    vivisect::error::log_error({code, msg, __FILE__, __LINE__})
```

**Example:**
```cpp
if (!proc.is_initialized()) {
    VIVISECT_ERROR(ErrorCode::WRAPPER_INIT_FAILED, "ProcessAPI init failed");
}
```

---

## Performance Characteristics

### Overhead by Technique

| Technique | Overhead | Binary Size Impact |
|-----------|----------|-------------------|
| String Encryption | Low (one-time) | +10-20% |
| MBA (complexity 5) | Low-Medium | +5-10% |
| Junk Code (density 3) | Low | +15-30% |
| API Resolution | Low (one-time) | Minimal |
| Anti-Debug | Low | Minimal |
| Control Flow Flattening | Medium-High (2-3x) | +30-50% |
| VM Engine | High (100-300x) | +20-40% |

### Profile Comparison

| Profile | Runtime Overhead | Binary Size | Protection Level |
|---------|-----------------|-------------|------------------|
| MINIMAL | 5-10% | +10-20% | Basic |
| BALANCED | 20-40% | +30-50% | Good |
| MAXIMUM | 100-300% | +100-200% | Maximum |

### Optimization Guidelines

**Hot Paths:**
- Use MINIMAL profile
- Avoid VM engine
- Limit control flow flattening
- Cache decrypted strings

**Security-Critical:**
- Use MAXIMUM profile
- Apply all techniques
- Accept performance cost

**Typical Application:**
- Use BALANCED profile
- Selective MAXIMUM for critical functions
- Profile and measure actual impact


---

## Platform Support

### Windows

**Status:** Full support

**Features:**
- All obfuscation modules
- PEB-based API resolution
- All API wrappers
- SEH exception handling
- PE section manipulation
- Full anti-debug capabilities

**Requirements:**
- Windows 10 or later
- Windows SDK 10.0+
- MSVC 2019+ / Clang-CL

### Linux

**Status:** Core obfuscation only

**Supported:**
- String encryption
- MBA transformations
- Control flow flattening (including computed goto)
- VM engine
- Junk code
- Configuration system
- Basic anti-debug (ptrace detection, timing)

**Not Supported:**
- PEB-based API resolution
- Windows API wrappers
- SEH exception handling
- PE section manipulation
- Windows-specific anti-debug

**Requirements:**
- GCC 10+ or Clang 10+
- C++20 support

### macOS

**Status:** Core obfuscation only (same as Linux)

**Requirements:**
- Clang 10+
- C++20 support
- macOS 11+

### Conditional Compilation

```cpp
#ifdef _WIN32
    // Windows-specific code
    vivisect::api::ProcessAPI proc;
#endif

#ifdef __linux__
    // Linux-specific code
#endif

#ifdef __APPLE__
    // macOS-specific code
#endif
```

---

## Build Instructions

### Windows (MSVC)

**Command Line:**
```cmd
cl /std:c++20 /O2 /EHsc /I path\to\include main.cpp
```

**Visual Studio:**
1. Add `include/` to Additional Include Directories
2. Set C++ Language Standard to ISO C++20
3. Build


### Linux (GCC)

```bash
g++ -std=c++20 -O2 -I include/ main.cpp -o myapp
```

### Linux (Clang)

```bash
clang++ -std=c++20 -O2 -I include/ main.cpp -o myapp
```

### macOS

```bash
clang++ -std=c++20 -O2 -I include/ main.cpp -o myapp
```

### CMake

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(path/to/VivisectionEngine/include)

add_executable(myapp main.cpp)
```

### Compiler Flags

**Recommended:**
- `/O2` or `-O2` (optimization)
- `/EHsc` (exception handling, MSVC)
- `-std=c++20` (C++20 mode)

**Optional:**
- `/GL` or `-flto` (link-time optimization)
- `/arch:AVX2` or `-mavx2` (SIMD)

---

## Integration Guide

### Basic Integration

**Step 1:** Include header
```cpp
#include <vivisect/vivisect.hpp>
```

**Step 2:** Apply protection
```cpp
int main() {
    VIVISECT_ANTI_DEBUG(vivisect::modules::DebuggerResponse::EXIT);
    
    auto secret = VIVISECT_STR("API Key");
    int x = VIVISECT_MBA_ADD(10, 20);
    
    VIVISECT_JUNK(3);
    return 0;
}
```

### Advanced Integration

**Protected Entry Point:**
```cpp
VIVISECT_MAIN({
    // Your application code
    return 0;
})
```

**Selective Protection:**
```cpp
void critical_function() {
    auto saved = vivisect::config::current_profile;
    vivisect::config::current_profile = vivisect::config::MAXIMUM_PROTECTION;
    
    VIVISECT_FLATTEN_BEGIN(critical)
    
    VIVISECT_FLATTEN_STATE(0) {
        auto key = VIVISECT_STR("license_key");
        // Validation logic
    }
    
    VIVISECT_FLATTEN_END()
    
    vivisect::config::current_profile = saved;
}
```


### Layered Protection Strategy

```cpp
void highly_protected() {
    // Layer 1: Anti-debug
    VIVISECT_ANTI_DEBUG(vivisect::modules::DebuggerResponse::EXIT);
    
    // Layer 2: Junk code
    VIVISECT_JUNK(5);
    
    // Layer 3: Control flow flattening
    VIVISECT_FLATTEN_BEGIN(secure)
    
    VIVISECT_FLATTEN_STATE(0) {
        // Layer 4: String encryption
        auto key = VIVISECT_STR("secret");
        
        // Layer 5: MBA operations
        int hash = 0;
        for (char c : key) {
            hash = VIVISECT_MBA_ADD(hash, static_cast<int>(c));
            hash = VIVISECT_MBA_XOR(hash, 0x5A);
        }
    }
    
    VIVISECT_FLATTEN_STATE(1) {
        // Layer 6: VM execution
        using namespace vivisect::modules;
        int seed = 12345;
        VMEngine vm(seed);
        
        VMOpcode bytecode[] = {
            VMOpcode::LOAD,
            VMOpcode::MANGLE_KEY,
            VMOpcode::STORE
        };
        
        vm.execute(bytecode, 3);
    }
    
    VIVISECT_FLATTEN_END()
}
```

### Conditional Protection

```cpp
#ifdef RELEASE_BUILD
    #define PROTECT_STRING(s) VIVISECT_STR(s)
    #define PROTECT_ADD(a, b) VIVISECT_MBA_ADD(a, b)
#else
    #define PROTECT_STRING(s) std::string(s)
    #define PROTECT_ADD(a, b) ((a) + (b))
#endif
```

Use protection only in release builds for easier debugging.

---

## Troubleshooting

### Compilation Errors

**Error: C++20 not enabled**

Solution:
- MSVC: `/std:c++20`
- GCC/Clang: `-std=c++20`

**Error: Cannot find vivisect.hpp**

Solution: Add include path
- MSVC: `/I path\to\include`
- GCC/Clang: `-I path/to/include`

**Error: Computed goto not supported**

Solution: Use different dispatcher or MSVC doesn't support computed goto
```cpp
#ifdef _MSC_VER
    using Strategy = DispatchStrategy::SWITCH_BASED;
#else
    using Strategy = DispatchStrategy::COMPUTED_GOTO;
#endif
```


### Runtime Issues

**Application crashes immediately**

Possible causes:
1. Anti-debug triggering under debugger
2. Stack canary corruption
3. VM bytecode error

Solution:
```cpp
// Temporarily disable anti-debug
vivisect::config::current_profile.enable_anti_debug = false;

// Or use IGNORE response
VIVISECT_ANTI_DEBUG(vivisect::modules::DebuggerResponse::IGNORE);
```

**Strings appear corrupted**

Cause: String lifetime issue

Solution:
```cpp
// Bad: Temporary destroyed
const char* get_key() {
    return VIVISECT_STR("key").c_str();  // Dangling pointer
}

// Good: Return std::string
std::string get_key() {
    return VIVISECT_STR("key");
}
```

**API wrappers fail to initialize**

Causes:
1. Not on Windows
2. Antivirus blocking PEB access
3. API not available

Solution:
```cpp
vivisect::api::ProcessAPI proc;
if (!proc.is_initialized()) {
    // Handle failure
    VIVISECT_ERROR(ErrorCode::WRAPPER_INIT_FAILED, "ProcessAPI failed");
}
```

### Performance Issues

**Application too slow**

Solution: Reduce protection level
```cpp
// Use MINIMAL for hot paths
auto saved = vivisect::config::current_profile;
vivisect::config::current_profile = vivisect::config::MINIMAL_PROTECTION;

// Performance-critical code

vivisect::config::current_profile = saved;
```

**Binary too large**

Solution: Reduce junk code and disable section distribution
```cpp
auto& cfg = vivisect::config::current_profile;
cfg.junk_code_density = 1;
cfg.distribute_across_sections = false;
```

### Debugging Protected Code

**Cannot step through obfuscated code**

Solution: Use conditional compilation
```cpp
#ifdef _DEBUG
    // Unprotected debug build
    void my_function() {
        // Direct code
    }
#else
    // Protected release build
    void my_function() {
        VIVISECT_FLATTEN_BEGIN(my_function)
        // Obfuscated code
        VIVISECT_FLATTEN_END()
    }
#endif
```


---

## Advanced Topics

### Custom VM Handlers

```cpp
void custom_handler(vivisect::modules::VMState& state) {
    // Custom operation
    state.registers[0] = state.registers[0] * 2 + 1;
    state.pc++;
}

int seed = 12345;
VMEngine vm(seed);
vm.register_handler(VMOpcode::MANGLE_KEY, custom_handler);
```

### Custom Dispatcher Strategy

Implement your own control flow dispatcher by specializing the template.

### Extending API Wrappers

```cpp
class CustomAPI {
public:
    CustomAPI() {
        // Resolve your APIs
        my_func_ = (MyFunc_t)vivisect::api::APIResolver::find_export(
            vivisect::api::APIResolver::find_module("mydll.dll"),
            "MyFunction"
        );
    }
    
    bool is_initialized() const {
        return my_func_ != nullptr;
    }
    
private:
    using MyFunc_t = int(*)(int);
    MyFunc_t my_func_ = nullptr;
};
```

### Custom Obfuscation Module

Create your own module following the pattern:

```cpp
namespace vivisect::modules {

class MyModule {
public:
    static void my_obfuscation() {
        // Implementation
    }
};

} // namespace vivisect::modules

#define VIVISECT_MY_OBFUSCATION() \
    vivisect::modules::MyModule::my_obfuscation()
```

---

## Security Considerations

### What Vivisection Engine Provides

- Static analysis resistance
- Dynamic debugging disruption
- Signature evasion
- Control flow obfuscation
- Data flow obfuscation
- Import hiding

### What It Does Not Provide

- Cryptographic security guarantees
- Protection against determined expert analysis
- Runtime integrity verification
- Tamper detection
- Code signing

### Threat Model

**Effective Against:**
- Automated analysis tools
- Script kiddies
- Signature-based detection
- Casual reverse engineering
- Pattern matching

**Less Effective Against:**
- Expert manual analysis
- Dedicated reverse engineers with time
- Hardware-based debugging
- Memory dumping
- Dynamic instrumentation frameworks (Frida, etc.)


### Defense in Depth

Vivisection Engine should be one layer in a comprehensive security strategy:

1. **Code obfuscation** (Vivisection Engine)
2. **Packing/compression** (UPX, MPRESS, custom)
3. **Anti-tampering** (checksums, integrity verification)
4. **Server-side validation** (license checks, authentication)
5. **Code signing** (legitimate software signature)
6. **Runtime protection** (anti-cheat, DRM)

### Best Practices

**Do:**
- Apply maximum protection to security-critical functions
- Use minimal protection for performance-critical code
- Test thoroughly with protection enabled
- Profile performance impact
- Combine multiple techniques
- Update protection regularly

**Don't:**
- Rely solely on obfuscation for security
- Apply maximum protection everywhere
- Assume protection is unbreakable
- Ignore performance impact
- Skip testing protected builds
- Use obfuscation as substitute for proper security design

---

## API Reference Quick Guide

### Macros

```cpp
// String encryption
VIVISECT_STR(str)

// MBA operations
VIVISECT_MBA_ADD(a, b)
VIVISECT_MBA_SUB(a, b)
VIVISECT_MBA_XOR(a, b)
VIVISECT_MBA_AND(a, b)
VIVISECT_MBA_OR(a, b)
VIVISECT_MBA_NOT(a)

// Control flow
VIVISECT_FLATTEN_BEGIN(name)
VIVISECT_FLATTEN_STATE(id)
VIVISECT_FLATTEN_END()

// Junk code
VIVISECT_JUNK(level)

// Anti-debug
VIVISECT_ANTI_DEBUG(response)

// Main protection
VIVISECT_MAIN(body)

// API resolution
VIVISECT_API(dll, func)

// Error handling
VIVISECT_ERROR(code, msg)

// Unique seed
VIVISECT_UNIQUE_SEED
```

### Classes

```cpp
// Core
vivisect::core::CompileTimeRandom<Seed>
vivisect::core::ObfuscationProfile

// Modules
vivisect::modules::EncryptedString<N, Cipher>
vivisect::modules::MBA
vivisect::modules::ControlFlowFlattener<Strategy>
vivisect::modules::VMEngine
vivisect::modules::AntiDebug
vivisect::modules::JunkCodeGenerator

// API
vivisect::api::APIResolver
vivisect::api::ProcessAPI
vivisect::api::CryptoAPI
vivisect::api::NetworkAPI
vivisect::api::RegistryAPI
```


### Enums

```cpp
// Dispatcher strategies
vivisect::modules::DispatchStrategy::{
    SWITCH_BASED,
    COMPUTED_GOTO,
    FUNCTION_POINTER,
    HYBRID
}

// Junk patterns
vivisect::modules::JunkPattern::{
    ARITHMETIC,
    BITWISE,
    MEMORY,
    CONTROL_FLOW,
    MIXED
}

// Debugger responses
vivisect::modules::DebuggerResponse::{
    IGNORE,
    EXIT,
    CRASH,
    CUSTOM
}

// VM opcodes
vivisect::modules::VMOpcode::{
    ADD, SUB, XOR, AND, OR, NOT,
    LOAD, STORE,
    JUMP, CALL,
    MANGLE_KEY,
    JUNK_OP
}

// Error codes
vivisect::error::ErrorCode::{
    SUCCESS,
    API_RESOLUTION_FAILED,
    WRAPPER_INIT_FAILED,
    VM_EXECUTION_ERROR,
    INVALID_CONFIGURATION,
    PLATFORM_NOT_SUPPORTED
}
```

---

## Complete Usage Example

```cpp
#include <vivisect/vivisect.hpp>
#include <iostream>

// License validation with maximum protection
bool validate_license(const std::string& key) {
    auto saved = vivisect::config::current_profile;
    vivisect::config::current_profile = vivisect::config::MAXIMUM_PROTECTION;
    
    VIVISECT_ANTI_DEBUG(vivisect::modules::DebuggerResponse::EXIT);
    
    bool valid = false;
    
    VIVISECT_FLATTEN_BEGIN(license_check)
    
    VIVISECT_FLATTEN_STATE(0) {
        auto expected = VIVISECT_STR("XXXX-YYYY-ZZZZ-WWWW");
        VIVISECT_JUNK(5);
        
        int match = 0;
        for (size_t i = 0; i < key.length() && i < expected.length(); i++) {
            if (key[i] == expected[i]) {
                match = VIVISECT_MBA_ADD(match, 1);
            }
        }
        
        valid = (match == static_cast<int>(expected.length()));
    }
    
    VIVISECT_FLATTEN_STATE(1) {
        if (vivisect::modules::AntiDebug::timing_check()) {
            valid = false;
        }
        VIVISECT_JUNK(3);
    }
    
    VIVISECT_FLATTEN_END()
    
    vivisect::config::current_profile = saved;
    return valid;
}

// Protected main
VIVISECT_MAIN({
    std::cout << "Protected Application" << std::endl;
    
    std::string license = "XXXX-YYYY-ZZZZ-WWWW";
    
    if (validate_license(license)) {
        std::cout << "License valid" << std::endl;
        
        // Application logic
        auto api_key = VIVISECT_STR("sk_live_abc123");
        int value = VIVISECT_MBA_ADD(100, 23);
        
        VIVISECT_JUNK(2);
        
        std::cout << "Running..." << std::endl;
    } else {
        std::cout << "Invalid license" << std::endl;
        return 1;
    }
    
    return 0;
})
```


---

## Verification

### Check String Encryption

```bash
# Windows
strings myapp.exe | grep "secret"

# Linux/macOS
strings myapp | grep "secret"
```

Should not find encrypted strings.

### Check Control Flow

Disassemble and look for state machine patterns:

```bash
# Windows
dumpbin /disasm myapp.exe > disasm.txt

# Linux
objdump -d myapp > disasm.txt

# macOS
otool -tV myapp > disasm.txt
```

Look for dispatcher loops and state variables.

### Check API Resolution

Examine import table:

```bash
# Windows
dumpbin /imports myapp.exe

# Linux
readelf -d myapp

# macOS
otool -L myapp
```

Protected APIs should not appear in IAT.

### Test Anti-Debug

Run under debugger:

```bash
# Should exit or crash if anti-debug works
gdb ./myapp
# or
x64dbg myapp.exe
```

---

## Performance Profiling

### Measure Overhead

```cpp
#include <chrono>

void benchmark() {
    using namespace std::chrono;
    
    // Unprotected
    auto start = high_resolution_clock::now();
    int result1 = 10 + 20;
    auto end = high_resolution_clock::now();
    auto time1 = duration_cast<nanoseconds>(end - start).count();
    
    // Protected
    start = high_resolution_clock::now();
    int result2 = VIVISECT_MBA_ADD(10, 20);
    end = high_resolution_clock::now();
    auto time2 = duration_cast<nanoseconds>(end - start).count();
    
    std::cout << "Overhead: " << (time2 - time1) << " ns" << std::endl;
}
```

### Profile Application

Use profiling tools to identify hot paths:

**Windows:**
- Visual Studio Profiler
- Intel VTune

**Linux:**
- perf
- Valgrind (callgrind)

**macOS:**
- Instruments

Apply minimal protection to hot paths identified by profiler.


---

## Comparison with Commercial Tools

### VMProtect/Themida

Vivisection Engine provides similar protection but as source-level obfuscation.

**Advantages:**
- No external tool required
- Compile-time obfuscation
- Full source code control
- No licensing restrictions
- Cross-platform core features

**Disadvantages:**
- Requires source code modification
- No GUI configuration
- Less mature than commercial tools

---

## FAQ

**Q: Is this production-ready?**

A: Yes. All modules are implemented and tested. Use appropriate protection levels for your use case.

**Q: What's the performance impact?**

A: Depends on protection level. MINIMAL: 5-10%, BALANCED: 20-40%, MAXIMUM: 100-300%. Profile your specific application.

**Q: Can this be defeated?**

A: Yes, by determined experts with time. Obfuscation raises cost, not impossibility. Use as part of defense-in-depth strategy.

**Q: Does it work with existing code?**

A: Yes. Header-only, no linking required. Apply protection incrementally.

**Q: What about false positives from antivirus?**

A: Possible. Obfuscated code may trigger heuristics. Code signing and reputation help. Test with target antivirus products.

**Q: Can I use this commercially?**

A: Check license. Typically yes for proprietary applications.

**Q: Does it support ARM?**

A: Core features yes. Windows-specific features require x86/x64 Windows.

**Q: How do I debug protected code?**

A: Use conditional compilation to disable protection in debug builds. Or use IGNORE response for anti-debug.

**Q: What's the binary size increase?**

A: MINIMAL: +10-20%, BALANCED: +30-50%, MAXIMUM: +100-200%. Use external packer if size is critical.

**Q: Can I extend it?**

A: Yes. Create custom modules following existing patterns. All source available.


---

## Glossary

**Obfuscation:** Transformation of code to make it harder to understand while preserving functionality.

**Control Flow Flattening:** Converting structured control flow into state machine-based execution.

**MBA (Mixed Boolean-Arithmetic):** Replacing simple operations with complex equivalent mathematical expressions.

**Opaque Predicate:** Conditional whose outcome is known at compile-time but obscured at runtime.

**VM (Virtual Machine):** Simulated CPU that executes custom instruction handlers.

**PEB (Process Environment Block):** Windows structure containing process information.

**IAT (Import Address Table):** PE structure containing addresses of imported functions.

**Junk Code:** Meaningless instructions inserted to increase complexity.

**Dispatcher:** Code that routes execution to different states in flattened control flow.

**Handler:** Function that implements VM instruction behavior.

**Bytecode:** Sequence of VM instructions.

**Stealth API Resolution:** Dynamic API loading without IAT traces.

**PE Section:** Segment of Windows executable with specific attributes.

**Entropy:** Measure of randomness in code or data.

**Dead Code:** Code that never executes but appears legitimate.

**Symbolic Execution:** Analysis technique that explores execution paths symbolically.

**Devirtualization:** Process of recovering original code from VM-protected code.

---

## Version History

**1.0.0** (Current)
- Initial release
- All core modules implemented
- Windows full support
- Linux/macOS core support
- Comprehensive test suite
- Complete documentation

---

## License

Copyright (c) 2025

See LICENSE file for details.

---

## Support

For issues, questions, or contributions:

1. Check this documentation
2. Review test results in project root
3. Examine example code in `examples/`
4. Check source code comments
5. Profile your specific use case

---

## Acknowledgments

Vivisection Engine builds upon research in:
- Code obfuscation techniques
- Virtual machine protection
- Control flow analysis
- Symbolic execution resistance
- Anti-debugging methods

---

**End of Documentation**

For the latest updates and additional resources, see the project repository.
