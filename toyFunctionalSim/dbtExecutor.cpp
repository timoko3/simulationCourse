#include "dbtExecutor.hpp"
#include "cpu.hpp"

namespace simulator{

void DbtExecutor::run(Cpu& cpu, Memory& memory, std::size_t threshold)
{
    ExitSlot* incomingExit = nullptr;
    std::shared_ptr<HostBasicBlock> incomingBlock;

    while (true) {
        const Addr pc = cpu.getState().pc;
        const auto curHostBasicBlock = translationCache_.lookup(pc);
        std::shared_ptr<HostBasicBlock> block;

        if (curHostBasicBlock != std::nullopt) {
            block = curHostBasicBlock->get();
        }
        else {
            auto& count = executionCounts_[pc];
            if (count < threshold) {
                ++count;
                incomingExit = nullptr;
                incomingBlock.reset();
                cpu.interpretBlock(memory);
                continue;
            }

            std::shared_ptr<HostBasicBlock> translatedBlock = BinaryTranslator_.translate(pc, cpu.getBasicBlock(memory, pc));
            blockLinker_.registerBlock(translatedBlock);
            block = translationCache_.lookupUpdate(pc, [&translatedBlock](Addr) -> const std::shared_ptr<HostBasicBlock>& {
                return translatedBlock;
            });
        }

        if (incomingExit) {
            blockLinker_.link(*incomingExit, *block);
        }
        incomingBlock.reset();

        incomingExit = block->getEntryPoint()(&cpu.getState(), &memory, block->getDescriptor());
        incomingBlock = blockLinker_.getOwner(incomingExit);
    }
}

};
