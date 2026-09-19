#include "execStage.hpp"
#include "syscall.hpp"

namespace simulator{

void
ExecStage::execInstr(CpuState& state, Memory& memory, Instruction instr){
    switch (instr.instrKind){
        case InstrKind::I_ADD:
            execAdd(state, instr);
            break;
        case InstrKind::I_LI:
            execLi(state, instr);
            break;
        case InstrKind::I_SYSCALL:
            execSyscall(state, instr);
            break;
        default:
            break;
    }

    state.pc += sizeof(Word);
}

inline void
ExecStage::execAdd(CpuState& state, Instruction instr){
    Register rs = state.getReg(instr.src1);
    Register rt = state.getReg(instr.src2);

    state.setReg(instr.dst, rs + rt);
}

inline void
ExecStage::execLi(CpuState& state, Instruction instr){
    state.setReg(instr.dst, instr.src1);
}

inline void
ExecStage::execSyscall(CpuState& state, Instruction instr){
    throw SimSyscall{state.getReg(SYSCALL_NUM_REG), instr.src1};
}

}
