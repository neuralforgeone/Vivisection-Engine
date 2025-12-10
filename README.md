Vivisection Engine – C++20 Obfuscation Framework

Vivisection Engine is a header-only C++20 obfuscation framework focused on making static and dynamic reverse engineering significantly harder through layered compile-time and runtime transformations.

Target use case: protecting native Windows and cross-platform C++ binaries against decompilers, debuggers, and signature-based analysis.

--------------------------------------------------

Core Obfuscation Techniques

1. Main Function Virtualization (VIVISECT_MAIN)

Wraps the program entry point inside a protected execution container:
- Virtual Machine execution layer
- Flattened control flow
- Runtime anti-debug checks
- Junk code insertion
- Opaque predicate gating

This hides the real entry logic and disrupts linear disassembly and graph recovery.

--------------------------------------------------

2. Control Flow Flattening

Transforms structured control flow into dispatcher-driven state machines:
- Switch-based dispatcher
- Computed-goto dispatcher
- Function-pointer table dispatcher
- Hybrid mixed dispatcher

State variables are mangled with MBA and runtime-mutable keys.

--------------------------------------------------

3. Virtual Machine Engine

Custom VM with:
- Configurable instruction set
- Dynamic instruction dispatch
- Runtime handler table mutation
- Encrypted bytecode stream
- MBA-obfuscated handlers

Designed to defeat pattern-based VM devirtualizers.

--------------------------------------------------

4. Compile-Time String Encryption

All protected strings are:
- Encrypted at compile time
- Assigned unique per-string keys
- Decrypted only at point of use
- Never stored statically as plaintext

Supported ciphers:
- XTEA
- AES-like lightweight block cipher

Usage example:
auto s = VIVISECT_STR("Secret API Key");

--------------------------------------------------

5. Mixed Boolean–Arithmetic (MBA)

Rewrites basic operations into non-linear algebraic equivalents:
- ADD, SUB, XOR, AND, OR, NOT
- Resistant to symbolic simplification
- Used across VM, control flow, and API resolution

--------------------------------------------------

6. Junk Code Generation

Injects realistic dead code:
- Volatile stack operations
- Fake data dependencies
- Randomized call chains
- Opaque predicate-guarded branches

Goal: increase entropy and destroy recognizable patterns.

--------------------------------------------------

7. Anti-Debug Module

Runtime debugger detection via:
- Timing checks
- Exception-based probes
- Hardware breakpoint detection

Configurable responses:
- Exit process
- Crash
- Silent bypass
- Custom callback

--------------------------------------------------

Stealth Windows API Resolution

No static imports for protected APIs.

Resolution method:
- PEB traversal
- Export table parsing
- Compile-time hashed symbol lookup
- No GetProcAddress / LoadLibrary traces in import table

Macro example:
VIVISECT_STEALTH_API("kernel32.dll", "CreateProcessA")

--------------------------------------------------

API Wrapper Classes (Stealth by Default)

All wrappers resolve APIs using the stealth resolver automatically.

Included Wrappers:
- ProcessAPI
- CryptographyAPI
- NetworkingAPI
- RegistryAPI

Example:
vivisect::api::ProcessAPI proc;
DWORD pid = proc.get_current_process_id();

No plaintext DLL or function names remain in the binary.

--------------------------------------------------

Configuration System

Global and per-function control.

Preset Profiles:
- MINIMAL
- BALANCED
- MAXIMUM

Manual Control Example:
auto& cfg = vivisect::config::current_profile;
cfg.enable_vm_execution = true;
cfg.enable_control_flow_flattening = true;
cfg.junk_code_density = 5;
cfg.mba_complexity = 7;

--------------------------------------------------

Usage

Basic Protection Example:

#include <vivisect/vivisect.hpp>

int main() {
    VIVISECT_ANTI_DEBUG(vivisect::modules::DebuggerResponse::EXIT);

    auto secret = VIVISECT_STR("Hidden Value");
    int x = 10, y = 20;
    int z = VIVISECT_MBA_ADD(x, y);

    VIVISECT_JUNK(3);
    return 0;
}

--------------------------------------------------

Protected Entry Point Example:

#include <vivisect/vivisect.hpp>

VIVISECT_MAIN({
    auto s = VIVISECT_STR("Protected main");
    return 0;
})

--------------------------------------------------

Build Requirements

- C++20 compiler
- MSVC 2019+ / GCC 10+ / Clang 10+
- Windows SDK 10+ (Windows builds)

Header-only. No external libraries.

MSVC Example:
cl /std:c++20 /O2 main.cpp

--------------------------------------------------

Platform Support

- Windows: Full feature set
- Linux / macOS: Core obfuscation only (no PEB/API stealth)

--------------------------------------------------

Binary Size Considerations

- Heavy VM + control-flow flattening + junk may inflate output size significantly.
- This framework prioritizes reverse-engineering resistance over size.
- External packers (UPX, MPRESS, etc.) may be applied post-build if needed.

--------------------------------------------------

Project Layout

include/vivisect/
- core/        // primitives, MBA, config
- modules/     // VM, anti-debug, junk, CF flattening
- api/         // stealth API resolution + wrappers
- integration/ // high-level macros
- vivisect.hpp // master include

--------------------------------------------------

Security Model

Vivisection Engine provides:
- Static analysis resistance
- Dynamic debugging disruption
- Signature evasion
- Control-flow and data-flow obfuscation

It is not a cryptographic security system. It raises cost, not guarantees impossibility.

--------------------------------------------------
