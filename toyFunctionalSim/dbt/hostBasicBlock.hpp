#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

#include "isa/definitions.hpp"
#include "blockDescriptor.hpp"

namespace simulator {

class BlockLinker;

class HostBasicBlock {
public:
    using EntryPoint = BlockDescriptor::EntryPoint;

    HostBasicBlock(Addr startPc, EntryPoint entryPoint, std::shared_ptr<void> codeOwner)
        : startPc_(startPc), codeOwner_(std::move(codeOwner)), descriptor_(std::make_unique<BlockDescriptor>()) {
        if (!entryPoint) {
            throw std::invalid_argument("Host block entry point must not be null");
        }
        if (!codeOwner_) {
            throw std::invalid_argument("Host block code owner must not be null");
        }

        descriptor_->entry = entryPoint;
    }

    ~HostBasicBlock();

    HostBasicBlock(const HostBasicBlock&) = delete;
    HostBasicBlock& operator=(const HostBasicBlock&) = delete;

    HostBasicBlock(HostBasicBlock&&) = delete;
    HostBasicBlock& operator=(HostBasicBlock&&) = delete;

    Addr getStartPc() const { return startPc_; }
    EntryPoint getEntryPoint() const { return descriptor_->entry; }
    BlockDescriptor* getDescriptor() { return descriptor_.get(); }

private:
    friend class BlockLinker;

    Addr startPc_;
    std::shared_ptr<void> codeOwner_;
    std::unique_ptr<BlockDescriptor> descriptor_;
    BlockLinker* linker_ = nullptr;
};

}
