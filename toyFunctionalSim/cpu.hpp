#pragma once

#include <cstdint>
#include <vector>

namespace simulator{

using Register = uint32_t;
constexpr std::size_t kNumRegs = 32;

struct CpuState{
    Register gpr_regs[kNumRegs];
    Register pc;
    Memory* memory;
};

struct Memory {
    std::vector<uint8_t> data; 
};

class Cpu {
    CpuState state_;
    
};

}