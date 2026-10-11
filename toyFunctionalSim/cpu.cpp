#include "cpu.hpp"

namespace simulator{

void Cpu::stepInstruction(Memory& memory) {
    const Word raw = simpPipeline_.fetch_.instrFetch(memory, state_.pc);
    const Instruction instr = simpPipeline_.decode_.instrDecode(raw);

    simpPipeline_.exec_.execInstr(state_, memory, instr);
}

void
Cpu::stepBlock(Memory& memory){
    const BasicBlock& curBasicBlock = getBasicBlock(memory, state_.pc);

    for(const auto& instr : curBasicBlock){
        simpPipeline_.exec_.execInstr(state_, memory, instr);
    }
}

const BasicBlock&
Cpu::getBasicBlock(Memory& memory, Addr startPc) {
    // The cache copies the loaded block before this local storage expires.
    BasicBlock loadedBlock;
    return iCache_.lookupUpdate(startPc, 
                                [this, &memory, &loadedBlock](Addr pc) -> const BasicBlock& {
                                    loadedBlock = createBasicBlock(memory, pc);
                                    return loadedBlock;
                                });
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
