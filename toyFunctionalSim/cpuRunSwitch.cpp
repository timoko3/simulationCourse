#include "cpu.hpp"

namespace simulator {

void Cpu::run(Memory& memory)
{
    while (step(memory) == SR_NORMAL) {
    }
}

}