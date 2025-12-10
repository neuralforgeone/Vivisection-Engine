#ifndef VIVISECT_MODULES_VM_ENGINE_HPP
#define VIVISECT_MODULES_VM_ENGINE_HPP
#include <cstdint>
#include <array>
#include <functional>
#include <cstring>
#include "../core/primitives.hpp"
#include "../error/error.hpp"
namespace vivisect::modules {
enum class VMOpcode : uint8_t {
    ADD,        
    SUB,        
    MUL,        
    DIV,        
    XOR,        
    AND,        
    OR,         
    NOT,        
    SHL,        
    SHR,        
    LOAD,       
    STORE,      
    LOAD_IMM,   
    JUMP,       
    JUMP_IF_ZERO,   
    JUMP_IF_NOT_ZERO, 
    CALL,       
    RET,        
    MANGLE_KEY, 
    JUNK_OP,    
    NOP         
};
struct VMInstruction {
    VMOpcode opcode;
    uint8_t dest_reg;   
    uint8_t src1_reg;   
    uint8_t src2_reg;   
    uint32_t immediate; 
    constexpr VMInstruction(VMOpcode op = VMOpcode::NOP, 
                           uint8_t dest = 0, 
                           uint8_t src1 = 0, 
                           uint8_t src2 = 0, 
                           uint32_t imm = 0)
        : opcode(op), dest_reg(dest), src1_reg(src1), src2_reg(src2), immediate(imm) {}
};
struct VMState {
    uint32_t registers[8];      
    uint32_t pc;                
    uint32_t flags;             
    int& global_seed;           
    uint32_t memory[256];       
    uint32_t call_stack[32];    
    uint32_t stack_ptr;         
    VMState(int& seed) : pc(0), flags(0), global_seed(seed), stack_ptr(0) {
        for (auto& reg : registers) reg = 0;
        for (auto& mem : memory) mem = 0;
        for (auto& stack : call_stack) stack = 0;
    }
    bool is_valid_register(uint8_t reg) const {
        return reg < 8;
    }
    bool is_valid_memory(uint32_t addr) const {
        return addr < 256;
    }
};
using VMHandler = std::function<void(VMState&, const VMInstruction&)>;
class VMEngine {
public:
    VMEngine(int& seed_ref) : state_(seed_ref), mutation_counter_(0) {
        initialize_handlers();
    }
    void execute(const VMInstruction* bytecode, size_t length) {
        if (!bytecode || length == 0) {
            VIVISECT_ERROR(error::ErrorCode::VM_EXECUTION_ERROR, "VM: Invalid bytecode or length");
            return;
        }
        state_.pc = 0;
        while (state_.pc < length) {
            const VMInstruction& inst = bytecode[state_.pc];
            if (!state_.is_valid_register(inst.dest_reg) || 
                !state_.is_valid_register(inst.src1_reg) || 
                !state_.is_valid_register(inst.src2_reg)) {
                VIVISECT_ERROR(error::ErrorCode::VM_INVALID_REGISTER, "VM: Invalid register index");
                return;
            }
            size_t handler_index = static_cast<size_t>(inst.opcode);
            if (handler_index >= handler_table_.size() || !handler_table_[handler_index]) {
                VIVISECT_ERROR(error::ErrorCode::VM_INVALID_OPCODE, "VM: Invalid or unregistered opcode");
                return;
            }
            try {
                handler_table_[handler_index](state_, inst);
            } catch (const std::exception&) {
                VIVISECT_ERROR(error::ErrorCode::VM_EXECUTION_ERROR, "VM: Handler execution failed");
                return;
            }
            if (inst.opcode != VMOpcode::JUMP && 
                inst.opcode != VMOpcode::JUMP_IF_ZERO &&
                inst.opcode != VMOpcode::JUMP_IF_NOT_ZERO &&
                inst.opcode != VMOpcode::CALL &&
                inst.opcode != VMOpcode::RET) {
                state_.pc++;
            }
            if (++mutation_counter_ % 100 == 0) {
                mutate_handlers();
            }
        }
    }
    template<size_t N>
    void execute(const std::array<VMInstruction, N>& bytecode) {
        execute(bytecode.data(), N);
    }
    void register_handler(VMOpcode op, VMHandler handler) {
        size_t index = static_cast<size_t>(op);
        if (index < handler_table_.size()) {
            handler_table_[index] = handler;
        }
    }
    void mutate_handlers() {
        uint32_t seed = static_cast<uint32_t>(state_.global_seed);
        for (size_t i = 0; i < 5; i++) {
            seed = seed * 1103515245 + 12345;
            size_t idx1 = (seed >> 16) % handler_table_.size();
            seed = seed * 1103515245 + 12345;
            size_t idx2 = (seed >> 16) % handler_table_.size();
            if (handler_table_[idx1] && handler_table_[idx2]) {
                std::swap(handler_table_[idx1], handler_table_[idx2]);
            }
        }
        vivisect::core::volatile_seed_update(state_.global_seed);
    }
    const VMState& get_state() const { return state_; }
    VMState& get_state() { return state_; }
private:
    VMState state_;
    std::array<VMHandler, 32> handler_table_;
    uint32_t mutation_counter_;
    void initialize_handlers() {
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
        register_handler(VMOpcode::MANGLE_KEY, [](VMState& s, const VMInstruction& i) {
            if (s.is_valid_register(i.dest_reg) && s.is_valid_register(i.src1_reg)) {
                uint32_t value = s.registers[i.src1_reg];
                uint32_t seed = static_cast<uint32_t>(s.global_seed);
                s.registers[i.dest_reg] = vivisect::core::mix_seed(value, seed);
            }
        });
        register_handler(VMOpcode::JUNK_OP, [](VMState& s, const VMInstruction& i) {
            volatile uint32_t temp = s.registers[0];
            temp = (temp * 0x9e3779b9) ^ 0xDEADBEEF;
            temp = (temp << 13) | (temp >> 19);
            (void)temp;
            vivisect::core::volatile_nop();
        });
        register_handler(VMOpcode::NOP, [](VMState& s, const VMInstruction& i) {
            vivisect::core::volatile_nop();
        });
    }
};
template<size_t N>
struct VMBytecode {
    std::array<VMInstruction, N> instructions;
    constexpr VMBytecode(const VMInstruction (&insts)[N]) {
        for (size_t i = 0; i < N; i++) {
            instructions[i] = insts[i];
        }
    }
};
template<typename... Instructions>
constexpr auto make_bytecode(Instructions... insts) {
    constexpr size_t N = sizeof...(Instructions);
    VMInstruction arr[N] = { insts... };
    return VMBytecode<N>(arr);
}
} 
#endif 