#pragma once

#include "memory.hpp"
#include "cpuState.hpp"
#include "isa/isa.hpp"

namespace simulator{

class ExecStage{

public:
    void execInstr(CpuState& state, Memory& memory, Instruction instr); 
};

}