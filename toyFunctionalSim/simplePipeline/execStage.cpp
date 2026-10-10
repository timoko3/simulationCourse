#include "execStage.hpp"

namespace simulator{

void
ExecStage::execInstr(CpuState& state, Memory& memory, const Instruction& instr){
    switch (instr.instrKind){
        case InstrKind::I_ADD:
            execAdd(state, instr);
            break;
        case InstrKind::I_ADDI:
            execAddi(state, instr);
            break;
        case InstrKind::I_BEQ:
            execBeq(state, instr);
            break;
        case InstrKind::I_BEXT:
            execBext(state, instr);
            break;
        case InstrKind::I_LI:
            execLi(state, instr);
            break;
        case InstrKind::I_XOR:
            execXor(state, instr);
            break;
        case InstrKind::I_CLS:
            execCls(state, instr);
            break;
        case InstrKind::I_USAT:
            execUsat(state, instr);
            break;
        case InstrKind::I_LDREG:
            execLdReg(state, memory, instr);
            break;
        case InstrKind::I_LD:
            execLd(state, memory, instr);
            break;
        case InstrKind::I_ST:
            execSt(state, memory, instr);
            break;
        case InstrKind::I_LDP:
            execLdp(state, memory, instr);
            break;
        case InstrKind::I_SBIT:
            execSbit(state, instr);
            break;
        case InstrKind::I_J:
            execJ(state, instr);
            break;
        case InstrKind::I_SYSCALL:
            execSyscall(state, instr);
            break;
        default:
            break;
    }
}


}
