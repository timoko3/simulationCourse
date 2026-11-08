#include "dbtExecutor.hpp"
#include "cpu.hpp"
#include "memory/permissionFaultHandler.hpp"
#include "smcController.hpp"

namespace simulator{

void DbtExecutor::run(Cpu& cpu, Memory& memory, std::size_t threshold)
{
    PermissionFaultHandler faultHandler(memory);
    SmcController smcController(cpu, memory, this);
    ExitSlot* incomingExit = nullptr;
    std::shared_ptr<HostBasicBlock> incomingBlock;

    while (true) {
        try {
            const Addr pc = cpu.getState().pc;
            const auto curHostBasicBlock = translationCache_->lookup(pc);
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
                    faultHandler.execute([&] { cpu.interpretBlock(memory); });
                    continue;
                }

                const auto& guestBlock = cpu.getBasicBlock(memory, pc);
                memory.protectCode(pc, guestBlock.size() * sizeof(Word));
                std::shared_ptr<HostBasicBlock> translatedBlock = BinaryTranslator_.translate(pc, guestBlock);
                blockLinker_.registerBlock(translatedBlock);
                block = translationCache_->lookupUpdate(pc, [&translatedBlock](Addr) -> const std::shared_ptr<HostBasicBlock>& {
                    return translatedBlock;
                });
            }

            if (incomingExit) {
                blockLinker_.link(*incomingExit, *block);
            }
            incomingBlock.reset();

            incomingExit = faultHandler.execute([&] {
                return block->getEntryPoint()(&cpu.getState(), &memory, block->getDescriptor());
            });
            incomingBlock = blockLinker_.getOwner(incomingExit);
        }
        catch (const CodeWriteFault& fault) {
            incomingExit = nullptr;
            incomingBlock.reset();
            smcController.handleSMC(fault);
        }
    }
}

void DbtExecutor::invalidateCode()
{
    translationCache_ = std::make_unique<TranslationCache>(T_CACHE_SIZE);
    executionCounts_.clear();
}

};
