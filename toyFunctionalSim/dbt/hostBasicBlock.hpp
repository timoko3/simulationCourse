#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

#include "isa/definitions.hpp"

namespace simulator {

struct CpuState;
struct Memory;

class HostBasicBlock {
public:
    using EntryPoint = void (*)(CpuState*, Memory*);

private:
    Addr startPc_;
    EntryPoint entryPoint_;
    std::shared_ptr<void> codeOwner_;

public:
    HostBasicBlock(Addr startPc, EntryPoint entryPoint, std::shared_ptr<void> codeOwner)
        : startPc_(startPc), entryPoint_(entryPoint), codeOwner_(std::move(codeOwner)) {
        if (!entryPoint_) {
            throw std::invalid_argument("Host block entry point must not be null");
        }
        if (!codeOwner_) {
            throw std::invalid_argument("Host block code owner must not be null");
        }
    }

    HostBasicBlock(const HostBasicBlock&) = delete;
    HostBasicBlock& operator=(const HostBasicBlock&) = delete;

    HostBasicBlock(HostBasicBlock&&) noexcept = default;
    HostBasicBlock& operator=(HostBasicBlock&&) noexcept = default;

    Addr getStartPc() const { return startPc_; }
    EntryPoint getEntryPoint() const { return entryPoint_; }
};

}
