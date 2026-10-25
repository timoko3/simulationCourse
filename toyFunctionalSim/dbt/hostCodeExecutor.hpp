#pragma once

#include "hostBasicBlock.hpp"

namespace simulator{

class CpuState;
class Memory;

class HostCodeExecutor {
public:
    static void execute(const HostBasicBlock& block,
                        CpuState& state,
                        Memory& memory);
};

}