#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include "simplePipeline/SimplePipeline.hpp"
#include "isa/definitions.hpp"

namespace simulator{

using Register = uint32_t;
constexpr std::size_t kNumRegs = 32;

struct CpuState{
    Register gpr_regs[kNumRegs];
    Register pc;
    Memory* memory;
};

struct Memory{
    std::vector<uint8_t> data; 
};

class Cpu{
    CpuState state_;
    SimplePipeline simpPipeline_;
public:
    void runCpu();
    void loadInstr(const Word instr);
    void loadProgram(const std::vector<Word>& program);

    CpuState getState(){ return state_; }
};

}