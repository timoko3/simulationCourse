#include "cpu.hpp"

namespace simulator {

void Cpu::run(Memory& memory)
{
    while (true) {
        stepBlock(memory);
    }
}

}