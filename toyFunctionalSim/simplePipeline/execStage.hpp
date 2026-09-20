#pragma once

#include "memory.hpp"
#include "cpuState.hpp"
#include "isa/isa.hpp"

namespace simulator{

class ExecStage{

    void requireAligned(Addr addr);

public:
    void execInstr(CpuState& state, Memory& memory, Instruction instr); 

    void execAdd(CpuState& state, Instruction instr);
    void execAddi(CpuState& state, Instruction instr);
    void execBeq(CpuState& state, Instruction instr);
    void execBext(CpuState& state, Instruction instr);
    void execLi(CpuState& state, Instruction instr);
    void execXor(CpuState& state, Instruction instr);
    void execCls(CpuState& state, Instruction instr);
    void execUsat(CpuState& state, Instruction instr);
    void execLdReg(CpuState& state, Memory& memory, Instruction instr);
    void execLd(CpuState& state, Memory& memory, Instruction instr);
    void execSt(CpuState& state, Memory& memory, Instruction instr);
    void execLdp(CpuState& state, Memory& memory, Instruction instr);
    void execSbit(CpuState& state, Instruction instr);

    void execJ(CpuState& state, Instruction instr);

    void execSyscall(CpuState& state, Instruction instr);
};

}
