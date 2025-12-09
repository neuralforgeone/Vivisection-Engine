// Vivisection Engine - Virtual Machine Engine Module
// Custom VM for executing obfuscated instruction sequences

#ifndef VIVISECT_MODULES_VM_ENGINE_HPP
#define VIVISECT_MODULES_VM_ENGINE_HPP

#include <cstdint>
#include <array>
#include <functional>
#include <cstring>
#include "../core/primitives.hpp"

namespace vivisect::modules {

// VM opcodes - instruction types for the virtual machine
enum class VMOpcode : uint8_t {
    // Arithmetic operations
    ADD,        // registers[dest] = registers[src1] + registers[src2]
    SUB,        // registers[dest] = registers[src1] - registers[src2]
    MUL,        // registers[dest] = registers[src1] * registers[src2]
    DIV,        // registers[dest] = registers[src1] / registers[src2]
    
    // Bitwise operations
    XOR,        // registers[dest] = registers[src1] ^ registers[src2]
    AND,        // registers[dest] = registers[src1] & registers[src2]
    OR,         // registers[dest] = registers[src1] | registers[src2]
    NOT,        // registers[dest] = ~registers[src1]
    SHL,        // registers[dest] = registers[src1] << registers[src2]
    SHR,        // registers[dest] = registers[src1] >> registers[src2]
    
    // Memory operations
    LOAD,       // registers[dest] = memory[registers[src1]]
    STORE,      // memory[registers[dest]] = registers[src1]
    LOAD_IMM,   // registers[dest] = immediate
    
    // Control flow operations
    JUMP,       // pc = immediate
    JUMP_IF_ZERO,   // if (registers[src1] == 0) pc = immediate
    JUMP_IF_NOT_ZERO, // if (registers[src1] != 0) pc = immediate
    CALL,       // push pc, pc = immediate
    RET,        // pc = pop
    
    // Special operations
    MANGLE_KEY, // Obfuscate a value using seed
    JUNK_OP,    // No-op with side effects to prevent optimization
    NOP         // No operation
};

// VM instruction structure
struct VMInstruction {
    VMOpcode opcode;
    uint8_t dest_reg;   // Destination register (0-7)
    uint8_t src1_reg;   // Source register 1 (0-7)
    uint8_t src2_reg;   // Source register 2 (0-7)
    uint32_t immediate; // Immediate value for certain operations
    
    constexpr VMInstruction(VMOpcode op = VMOpcode::NOP, 
                           uint8_t dest = 0, 
                           uint8_t src1 = 0, 
                           uint8_t src2 = 0, 
                           uint32_t imm = 0)
        : opcode(op), dest_reg(dest), src1_reg(src1), src2_reg(src2), immediate(imm) {}
};

// VM state structure - represents the virtual machine's execution state
struct VMState {
    uint32_t registers[8];      // General purpose registers
    uint32_t pc;                // Program counter
    uint32_t flags;             // Status flags (zero, carry, etc.)
    int& global_seed;           // Reference to global seed for obfuscation
    uint32_t memory[256];       // Simple memory space
    uint32_t call_stack[32];    // Call stack for CALL/RET
    uint32_t stack_ptr;         // Stack pointer
    
    VMState(int& seed) : pc(0), flags(0), global_seed(seed), stack_ptr(0) {
        for (auto& reg : registers) reg = 0;
        for (auto& mem : memory) mem = 0;
        for (auto& stack : call_stack) stack = 0;
    }
    
    // Helper to check if register index is valid
    bool is_valid_register(uint8_t reg) const {
        return reg < 8;
    }
    
    // Helper to check if memory address is valid
    bool is_valid_memory(uint32_t addr) const {
        return addr < 256;
    }
};

// VM handler function type
using VMHandler = std::function<void(VMState&, const VMInstruction&)>;

// VM Engine - executes virtual machine instructions
class VMEngine {
public:
    VMEngine(int& seed_ref) : state_(seed_ref), mutation_counter_(0) {
        initialize_handlers();
    }
    
    // Execute a sequence of VM instructions
    void execute(const VMInstruction* bytecode, size_t length) {
        if (!bytecode || length == 0) return;
        
        state_.pc = 0;
        
        while (state_.pc < length) {
            const VMInstruction& inst = bytecode[state_.pc];
            
            // Get handler for this opcode
            size_t handler_index = static_cast<size_t>(inst.opcode);
            if (handler_index < handler_table_.size() && handler_table_[handler_index]) {
                handler_table_[handler_index](state_, inst);
            }
            
            // Advance program counter (unless instruction modified it)
            if (inst.opcode != VMOpcode::JUMP && 
                inst.opcode != VMOpcode::JUMP_IF_ZERO &&
                inst.opcode != VMOpcode::JUMP_IF_NOT_ZERO &&
                inst.opcode != VMOpcode::CALL &&
                inst.opcode != VMOpcode::RET) {
                state_.pc++;
            }
            
            // Periodically mutate handlers for anti-analysis
            if (++mutation_counter_ % 100 == 0) {
                mutate_handlers();
            }
        }
    }
    
    // Execute bytecode from array
    template<size_t N>
    void execute(const std::array<VMInstruction, N>& bytecode) {
        execute(bytecode.data(), N);
    }
    
    // Register a custom handler for an opcode
    void register_handler(VMOpcode op, VMHandler handler) {
        size_t index = static_cast<size_t>(op);
        if (index < handler_table_.size()) {
            handler_table_[index] = handler;
        }
    }
    
    // Mutate handler table to prevent static analysis
    void mutate_handlers() {
        // Swap handlers randomly based on seed
        uint32_t seed = static_cast<uint32_t>(state_.global_seed);
        
        for (size_t i = 0; i < 5; i++) {
            seed = seed * 1103515245 + 12345;
            size_t idx1 = (seed >> 16) % handler_table_.size();
            
            seed = seed * 1103515245 + 12345;
            size_t idx2 = (seed >> 16) % handler_table_.size();
            
            // Only swap if both handlers exist
            if (handler_table_[idx1] && handler_table_[idx2]) {
                std::swap(handler_table_[idx1], handler_table_[idx2]);
            }
        }
        
        // Update global seed
        vivisect::core::volatile_seed_update(state_.global_seed);
    }
    
    // Get current VM state (for testing/debugging)
    const VMState& get_state() const { return state_; }
    VMState& get_state() { return state_; }
    
private:
    VMState state_;
    std::array<VMHandler, 32> handler_table_;
    uint32_t mutation_counter_;
    
    // Initialize default handlers for all opcodes
    void initialize_handlers() {
        // Arithmetic operations
        register_handler(VMOpcode::ADD, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] + s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::SUB, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] - s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::MUL, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] * s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::DIV, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                if (s.registers[i.src2_reg] != 0) {
                    s.registers[i.dest_reg] = s.registers[i.src1_reg] / s.registers[i.src2_reg];
                    s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
                }
            }
        });
        
        // Bitwise operations
        register_handler(VMOpcode::XOR, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] ^ s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::AND, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] & s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::OR, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] | s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::NOT, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg)) {
                s.registers[i.dest_reg] = ~s.registers[i.src1_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::SHL, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] << s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        register_handler(VMOpcode::SHR, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg) && s.is_valid_register(i.src2_reg)) {
                s.registers[i.dest_reg] = s.registers[i.src1_reg] >> s.registers[i.src2_reg];
                s.flags = (s.registers[i.dest_reg] == 0) ? 1 : 0;
            }
        });
        
        // Memory operations
        register_handler(VMOpcode::LOAD, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg)) {
                uint32_t addr = s.registers[i.src1_reg];
                if (s.is_valid_memory(addr)) {
                    s.registers[i.dest_reg] = s.memory[addr];
                }
            }
        });
        
        register_handler(VMOpcode::STORE, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg)) {
                uint32_t addr = s.registers[i.dest_reg];
                if (s.is_valid_memory(addr)) {
                    s.memory[addr] = s.registers[i.src1_reg];
                }
            }
        });
        
        register_handler(VMOpcode::LOAD_IMM, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg)) {
                s.registers[i.dest_reg] = i.immediate;
            }
        });
        
        // Control flow operations
        register_handler(VMOpcode::JUMP, [](VMState& s, const VMInstruction& i) {
            s.pc = i.immediate;
        });
        
        register_handler(VMOpcode::JUMP_IF_ZERO, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.src1_reg)) {
                if (s.registers[i.src1_reg] == 0) {
                    s.pc = i.immediate;
                } else {
                    s.pc++;
                }
            }
        });
        
        register_handler(VMOpcode::JUMP_IF_NOT_ZERO, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.src1_reg)) {
                if (s.registers[i.src1_reg] != 0) {
                    s.pc = i.immediate;
                } else {
                    s.pc++;
                }
            }
        });
        
        register_handler(VMOpcode::CALL, [](VMState& s, const VMInstruction& i) {
            if (s.stack_ptr < 32) {
                s.call_stack[s.stack_ptr++] = s.pc + 1;
                s.pc = i.immediate;
            }
        });
        
        register_handler(VMOpcode::RET, [](VMState& s, const VMInstruction& i) {
            if (s.stack_ptr > 0) {
                s.pc = s.call_stack[--s.stack_ptr];
            }
        });
        
        // Special operations
        register_handler(VMOpcode::MANGLE_KEY, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg)) {
                // Obfuscate value using global seed
                uint32_t value = s.registers[i.src1_reg];
                uint32_t seed = static_cast<uint32_t>(s.global_seed);
                s.registers[i.dest_reg] = vivisect::core::mix_seed(value, seed);
            }
        });
        
        register_handler(VMOpcode::JUNK_OP, [](VMState& s, const VMInstruction& i) {
            // Perform meaningless operations to waste analysis time
            volatile uint32_t temp = s.registers[0];
            temp = (temp * 0x9e3779b9) ^ 0xDEADBEEF;
            temp = (temp << 13) | (temp >> 19);
            (void)temp;
            vivisect::core::volatile_nop();
        });
        
        register_handler(VMOpcode::NOP, [](VMState& s, const VMInstruction& i) {
            // Do nothing
            vivisect::core::volatile_nop();
        });
    }
};

// Compile-time bytecode generation helper
template<size_t N>
struct VMBytecode {
    std::array<VMInstruction, N> instructions;
    
    constexpr VMBytecode(const VMInstruction (&insts)[N]) {
        for (size_t i = 0; i < N; i++) {
            instructions[i] = insts[i];
        }
    }
};

// Helper function to create bytecode at compile-time
template<typename... Instructions>
constexpr auto make_bytecode(Instructions... insts) {
    constexpr size_t N = sizeof...(Instructions);
    VMInstruction arr[N] = { insts... };
    return VMBytecode<N>(arr);
}

} // namespace vivisect::modules

#endif // VIVISECT_MODULES_VM_ENGINE_HPP
