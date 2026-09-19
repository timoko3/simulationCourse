#pragma once

#include <cstdint>

#include "isa/definitions.hpp"

namespace simulator{

constexpr std::size_t kNumRegs = 32;

struct CpuState{
    Register gpr_regs[kNumRegs];
    Register pc;
};

}