#include "cpu.hpp"

#include "threadedExecutor.hpp"

namespace simulator {

void Cpu::run(Memory& memory)
{   
    ThreadedExecutor::run(*this, memory);
}

} // namespace simulator