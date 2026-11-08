#include "blockLinker.hpp"
#include "hostBasicBlock.hpp"

#include <stdexcept>

namespace simulator {

BlockLinker::~BlockLinker()
{
    for (const auto& [exit, owner] : owners_) {
        if (auto block = owner.lock()) {
            block->linker_ = nullptr;
            exit->target = nullptr;
        }
    }
}

void BlockLinker::registerBlock(const std::shared_ptr<HostBasicBlock>& block)
{
    if (!block) {
        throw std::invalid_argument("Cannot register a null block");
    }
    if (block->linker_ == this) {
        return;
    }
    if (block->linker_) {
        throw std::logic_error("Block already belongs to another linker");
    }

    auto* descriptor = block->getDescriptor();
    incoming_.try_emplace(descriptor);

    try {
        for (auto& exit : descriptor->exits) {
            owners_.emplace(&exit, block);
        }
    }
    catch (...) {
        for (auto& exit : descriptor->exits) {
            owners_.erase(&exit);
        }
        incoming_.erase(descriptor);
        throw;
    }

    block->linker_ = this;
}

void BlockLinker::link(ExitSlot& exit, HostBasicBlock& target)
{
    if (owners_.find(&exit) == owners_.end() || target.linker_ != this) {
        throw std::logic_error("Both blocks must belong to this linker");
    }

    auto* descriptor = target.getDescriptor();
    if (exit.target == descriptor) {
        return;
    }

    incoming_.at(descriptor).insert(&exit);
    unlink(exit);
    exit.target = descriptor;
}

void BlockLinker::unlink(ExitSlot& exit) noexcept
{
    if (!exit.target) {
        return;
    }

    const auto incoming = incoming_.find(exit.target);
    if (incoming != incoming_.end()) {
        incoming->second.erase(&exit);
    }
    exit.target = nullptr;
}

void BlockLinker::unlinkBlock(HostBasicBlock& block) noexcept
{
    if (block.linker_ != this) {
        return;
    }

    auto* descriptor = block.getDescriptor();
    const auto incoming = incoming_.find(descriptor);
    if (incoming != incoming_.end()) {
        for (auto* exit : incoming->second) {
            exit->target = nullptr;
        }
        incoming_.erase(incoming);
    }

    for (auto& exit : descriptor->exits) {
        unlink(exit);
        owners_.erase(&exit);
    }

    block.linker_ = nullptr;
}

std::shared_ptr<HostBasicBlock> BlockLinker::getOwner(ExitSlot* exit) const
{
    if (!exit) {
        return nullptr;
    }

    const auto owner = owners_.find(exit);
    if (owner == owners_.end()) {
        throw std::logic_error("Exit slot does not belong to this linker");
    }

    return owner->second.lock();
}

}
