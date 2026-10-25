#include "hostCodeExecutor.hpp"

namespace simulator {

void HostCodeExecutor::execute(const HostBasicBlock& block, CpuState& state, Memory& memory)
{
    block.getEntryPoint()(&state, &memory);
}

}
