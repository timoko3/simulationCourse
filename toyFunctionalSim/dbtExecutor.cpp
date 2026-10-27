#include "dbtExecutor.hpp"
#include "cpu.hpp"

namespace simulator{

void DbtExecutor::run(Cpu& cpu, Memory& memory, std::size_t threshold)
{
    while (true) {
        const Addr pc = cpu.getState().pc;
        const auto curHostBasicBlock = translationCache_.lookup(pc);
        if (curHostBasicBlock != std::nullopt) {
            const auto block = curHostBasicBlock->get();
            block->getEntryPoint()(&cpu.getState(), &memory);
        }
        else {
            auto& count = executionCounts_[pc];
            if (count < threshold) {
                ++count;
                cpu.interpretBlock(memory);
            }
            else{
                std::shared_ptr<HostBasicBlock> translatedBlock = BinaryTranslator_.translate(pc, cpu.getBasicBlock(memory, pc));
                const auto block = translationCache_.lookupUpdate(pc, [&translatedBlock](Addr) -> const std::shared_ptr<HostBasicBlock>& {
                                                                                                    return translatedBlock;
                                                                                                });
                block->getEntryPoint()(&cpu.getState(), &memory);
            }
        }
    }
}

};
