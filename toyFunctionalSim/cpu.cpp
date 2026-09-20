#include "cpu.hpp"

namespace simulator{

stepResult
Cpu::step(Memory& memory){
    Word rawInstr            = simpPipeline_.fetch_.instrFetch(memory, state_.pc);
    Instruction decodedInstr = iCache_.lookupUpdate(rawInstr,     
                                                    [this](const Word& word) {
                                                        return simpPipeline_.decode_.instrDecode(word);
                                                    });
    simpPipeline_.exec_.execInstr(state_, memory, decodedInstr);

    return SR_NORMAL;
}

}
