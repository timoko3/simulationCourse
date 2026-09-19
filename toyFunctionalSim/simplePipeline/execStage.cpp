#include "execStage.hpp"

namespace simulator{

void
ExecStage::execInstr(CpuState& state, Memory& memory, Instruction instr){
    switch (instr.instrKind){
        case InstrKind::I_ADD:
            execAdd(state, instr);
            break;

        default:
            break;
    }
    
    state.pc++;
}

inline void
ExecStage::execAdd(CpuState& state, Instruction instr){
    Register rs = state.getReg(instr.src1);
    Register rt = state.getReg(instr.src2);

    state.setReg(instr.dst, rs + rt);

}

}