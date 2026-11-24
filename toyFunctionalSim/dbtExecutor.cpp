#include "dbtExecutor.hpp"
#include "cpu.hpp"
#include "memory/permissionFaultHandler.hpp"
#include "smcController.hpp"

#include <functional>
#include <utility>

namespace simulator{

void DbtExecutor::run(Cpu& cpu, Memory& memory, std::size_t threshold)
{
    PermissionFaultHandler faultHandler(memory);
    SmcController smcController(cpu, memory, this);
    ExitSlot* incomingExit = nullptr;

    while (true) {
        try {
            const Addr pc = cpu.getState().pc;
            auto block = translationCache_->find(pc);

            if (!block) {
                auto& count = executionCounts_[pc];
                if (count < threshold) {
                    ++count;
                    incomingExit = nullptr;
                    faultHandler.execute([&] { cpu.interpretBlock(memory); });
                    continue;
                }

                auto translated = BinaryTranslator_.translate(pc, cpu.getBasicBlock(memory, pc));
                blockLinker_.registerBlock(*translated);
                linkIncomingExit(incomingExit, *translated);
                block = std::cref(translationCache_->implace(pc, std::move(translated)));
            }
            else {
                linkIncomingExit(incomingExit, *block->get());
            }

            incomingExit = faultHandler.execute([&] {
                return block->get()->getEntryPoint()(&cpu.getState(), &memory,
                                                    block->get()->getDescriptor());
            });
        }
        catch (const CodeWriteFault& fault) {
            incomingExit = nullptr;
            smcController.handleSMC(fault);
        }
    }
}

void DbtExecutor::invalidateCode()
{
    translationCache_ = std::make_unique<TranslationCache>(T_CACHE_SIZE);
    executionCounts_.clear();
}

void DbtExecutor::linkIncomingExit(ExitSlot*& incomingExit, HostBasicBlock& block)
{
    if (incomingExit) {
        blockLinker_.link(*incomingExit, block);
        incomingExit = nullptr;
    }
}

};
