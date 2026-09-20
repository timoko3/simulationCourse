#pragma once

#include "memory.hpp"
#include "cpuState.hpp"
#include "isa/isa.hpp"

namespace simulator{

class ExecStage{

    void requireAligned(Addr addr);

    inline void execAdd(CpuState& state, Instruction instr);
    inline void execAddi(CpuState& state, Instruction instr);
    inline void execBeq(CpuState& state, Instruction instr);
    inline void execBext(CpuState& state, Instruction instr);

    inline void execLi(CpuState& state, Instruction instr);

    inline void execXor(CpuState& state, Instruction instr);
    inline void execCls(CpuState& state, Instruction instr);
    inline void execUsat(CpuState& state, Instruction instr);
    inline void execLdReg(CpuState& state, Memory& memory, Instruction instr);
    inline void execLd(CpuState& state, Memory& memory, Instruction instr);
    inline void execSt(CpuState& state, Memory& memory, Instruction instr);
    inline void execLdp(CpuState& state, Memory& memory, Instruction instr);
    inline void execSbit(CpuState& state, Instruction instr);

    inline void execJ(CpuState& state, Instruction instr);

    inline void execSyscall(CpuState& state, Instruction instr);
public:
    void execInstr(CpuState& state, Memory& memory, Instruction instr); 
};

}
