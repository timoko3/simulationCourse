#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

#include "isa/definitions.hpp"
#include "blockDescriptor.hpp"

namespace simulator {

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

    HostBasicBlock(const HostBasicBlock&) = delete;
    HostBasicBlock& operator=(const HostBasicBlock&) = delete;

    HostBasicBlock(HostBasicBlock&&) noexcept = default;
    HostBasicBlock& operator=(HostBasicBlock&&) noexcept = default;

    Addr getStartPc() const { return startPc_; }
    EntryPoint getEntryPoint() const { return descriptor_->entry; }
    BlockDescriptor* getDescriptor() { return descriptor_.get(); }

private:
    Addr startPc_;
    std::shared_ptr<void> codeOwner_;
    std::unique_ptr<BlockDescriptor> descriptor_;
};

}
