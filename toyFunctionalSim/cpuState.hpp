#pragma once

#include <cstdint>

namespace simulator{

using Register = uint32_t;
constexpr std::size_t kNumRegs = 32;

struct CpuState{
    Register gpr_regs[kNumRegs];
    Register pc;
};

}