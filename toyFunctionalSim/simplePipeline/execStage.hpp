#pragma once

#include "memory.hpp"
#include "cpuState.hpp"
#include "isa/isa.hpp"
#include "syscallHandlers.hpp"

namespace simulator{

class ExecStage{

    SyscallHandlers syscallHandlers_;

    void requireAligned(Addr addr);

public:
    void execInstr(CpuState& state, Memory& memory, const Instruction& instr);

    void execAdd(CpuState& state, const Instruction& instr);
    void execAddi(CpuState& state, const Instruction& instr);
    void execBeq(CpuState& state, const Instruction& instr);
    void execBext(CpuState& state, const Instruction& instr);
    void execLi(CpuState& state, const Instruction& instr);
    void execXor(CpuState& state, const Instruction& instr);
    void execCls(CpuState& state, const Instruction& instr);
    void execUsat(CpuState& state, const Instruction& instr);
    void execLdReg(CpuState& state, Memory& memory, const Instruction& instr);
    void execLd(CpuState& state, Memory& memory, const Instruction& instr);
    void execSt(CpuState& state, Memory& memory, const Instruction& instr);
    void execLdp(CpuState& state, Memory& memory, const Instruction& instr);
    void execSbit(CpuState& state, const Instruction& instr);

    void execJ(CpuState& state, const Instruction& instr);

    void execSyscall(CpuState& state, const Instruction& instr);
};

}
