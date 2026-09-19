#include "cpu.hpp"

namespace simulator{

stepResult
Cpu::step(Memory& memory){
    Word rawInstr            = simpPipeline_.fetch_.instrFetch(memory, state_.pc);
    Instruction decodedInstr = simpPipeline_.decode_.instrDecode(rawInstr);
    simpPipeline_.exec_.execInstr(state_, memory, decodedInstr);

    return SR_NORMAL;
}

}
