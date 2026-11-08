#include "cpu.hpp"

namespace simulator{

void Cpu::stepInstruction(Memory& memory) {
    const Word raw = simpPipeline_.fetch_.instrFetch(memory, state_.pc);
    const Instruction instruction = simpPipeline_.decode_.instrDecode(raw);

    memory.protectCode(state_.pc, sizeof(Word));
    executeInstruction(memory, instruction);
}

void
Cpu::interpretBlock(Memory& memory){
    const BasicBlock& curBasicBlock = getBasicBlock(memory, state_.pc);

    memory.protectCode(state_.pc, curBasicBlock.size() * sizeof(Word));
    for(const auto& instr : curBasicBlock){
        executeInstruction(memory, instr);
    }
}

const BasicBlock&
Cpu::getBasicBlock(Memory& memory, Addr startPc) {
    // The cache copies the loaded block before this local storage expires.
    BasicBlock loadedBlock;
    return iCache_->lookupUpdate(startPc,
                                [this, &memory, &loadedBlock](Addr pc) -> const BasicBlock& {
                                    loadedBlock = createBasicBlock(memory, pc);
                                    return loadedBlock;
                                });
}

BasicBlock Cpu::createBasicBlock(Memory& memory, Addr startPc) {
    BasicBlock block;
    Addr interBlockPc = startPc;

    Instruction decodedInstr = {};

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

void Cpu::executeInstruction(Memory& memory, const Instruction& instruction)
{
    simpPipeline_.exec_.execInstr(state_, memory, instruction);
}

void Cpu::invalidateCode()
{
    iCache_ = std::make_unique<iCache>(I_CACHE_SIZE);
}

}
