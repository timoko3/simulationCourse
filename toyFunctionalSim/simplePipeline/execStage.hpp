#pragma once

#include "memory.hpp"
#include "cpuState.hpp"
#include "isa/isa.hpp"

namespace simulator{

class ExecStage{

    inline void execAdd(CpuState& state, Instruction instr);

    inline void execLi(CpuState& state, Instruction instr);

    inline void execSyscall(CpuState& state, Instruction instr);
public:
    void execInstr(CpuState& state, Memory& memory, Instruction instr); 
};

}