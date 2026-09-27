#include "cpu.hpp"

namespace simulator{

void
Cpu::stepBlock(Memory& memory){
    BasicBlock curBasicBlock = iCache_.lookupUpdate(state_.pc,     
                                                    [this, &memory](Addr startPc) {
                                                        return createBasicBlock(memory, startPc);
                                                    });

    for(auto instr : curBasicBlock){
        simpPipeline_.exec_.execInstr(state_, memory, instr);
    }
}

BasicBlock Cpu::createBasicBlock(Memory& memory, Addr startPc) {
    BasicBlock block;
    Addr interBlockPc = startPc;

    Instruction decodedInstr = {};
    decodedInstr.instrKind = InstrKind::I_NOP;

    while( decodedInstr.instrKind != InstrKind::I_BEQ &&
           decodedInstr.instrKind != InstrKind::I_J   &&
           decodedInstr.instrKind != InstrKind::I_SYSCALL){
        Word rawInstr            = simpPipeline_.fetch_.instrFetch(memory, interBlockPc);
        decodedInstr = simpPipeline_.decode_.instrDecode(rawInstr);

        block.push_back(decodedInstr);

        interBlockPc += sizeof(Word);
    }

    return block;
}

}
