#pragma once

#include "hostBasicBlock.hpp"

namespace simulator{

struct CpuState;
struct Memory;

class HostCodeExecutor {
public:
    static void execute(const HostBasicBlock& block,
                        CpuState& state,
                        Memory& memory);
};

}
