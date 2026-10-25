#include "hostCodeExecutor.hpp"

#include <stdexcept>

namespace simulator {

void HostCodeExecutor::execute(const HostBasicBlock&, CpuState&, Memory&)
{
    throw std::runtime_error("Host code execution is not implemented yet");
}

}
