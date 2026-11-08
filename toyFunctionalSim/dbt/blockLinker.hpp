#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "blockDescriptor.hpp"

namespace simulator {

class HostBasicBlock;

class BlockLinker {
public:
    BlockLinker() = default;
    ~BlockLinker();

    BlockLinker(const BlockLinker&) = delete;
    BlockLinker& operator=(const BlockLinker&) = delete;
    BlockLinker(BlockLinker&&) = delete;
    BlockLinker& operator=(BlockLinker&&) = delete;

    void registerBlock(const std::shared_ptr<HostBasicBlock>& block);
    void link(ExitSlot& exit, HostBasicBlock& target);
    void unlink(ExitSlot& exit) noexcept;
    void unlinkBlock(HostBasicBlock& block) noexcept;

    std::shared_ptr<HostBasicBlock> getOwner(ExitSlot* exit) const;

private:
    std::unordered_map<BlockDescriptor*, std::unordered_set<ExitSlot*>> incoming_;
    std::unordered_map<ExitSlot*, std::weak_ptr<HostBasicBlock>> owners_;
};

}
