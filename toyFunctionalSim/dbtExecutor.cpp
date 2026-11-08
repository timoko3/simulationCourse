#include "dbtExecutor.hpp"
#include "cpu.hpp"

#include <vector>

namespace simulator{

void DbtExecutor::run(Cpu& cpu, Memory& memory, std::size_t threshold)
{
    ExitSlot* incomingExit = nullptr;

    while (true) {
        const Addr pc = cpu.getState().pc;
        const auto curHostBasicBlock = translationCache_.lookup(pc);
        std::shared_ptr<HostBasicBlock> block;

        if (curHostBasicBlock != std::nullopt) {
            block = curHostBasicBlock->get();
            if (incomingExit) {
                incomingExit->target = block->getDescriptor();
            }
        }
        else {
            auto& count = executionCounts_[pc];
            if (count < threshold) {
                ++count;
                incomingExit = nullptr;
                cpu.interpretBlock(memory);
                continue;
            }

            std::shared_ptr<HostBasicBlock> translatedBlock = BinaryTranslator_.translate(pc, cpu.getBasicBlock(memory, pc));
            block = cacheBlock(pc, translatedBlock, incomingExit);
        }

        incomingExit = block->getEntryPoint()(&cpu.getState(), &memory, block->getDescriptor());
    }
}

std::shared_ptr<HostBasicBlock> 
DbtExecutor::cacheBlock(Addr pc, const std::shared_ptr<HostBasicBlock>& block, ExitSlot* incomingExit)
{
    std::vector<std::shared_ptr<HostBasicBlock>> previousBlocks;
    if (translationCache_.getResidentCount() == translationCache_.getSize()) {
        previousBlocks.reserve(translationCache_.getResidentCount());
        for (const auto& resident : translationCache_.getResidentPages()) {
            previousBlocks.push_back(resident.page);
        }
    }

    const auto cachedBlock = translationCache_.lookupUpdate(pc, [&block](Addr) -> const std::shared_ptr<HostBasicBlock>& {
        return block;
    });

    for (const auto& previous : previousBlocks) {
        const auto resident = translationCache_.lookup(previous->getStartPc());
        if (resident && resident->get() == previous) {
            continue;
        }

        for (const auto& current : translationCache_.getResidentPages()) {
            for (auto& exit : current.page->getDescriptor()->exits) {
                if (exit.target == previous->getDescriptor()) {
                    exit.target = nullptr;
                }
            }
        }
    }

    if (incomingExit) {
        incomingExit->target = cachedBlock->getDescriptor();
    }

    return cachedBlock;
}

};
