#pragma once

#include <stdexcept>

#include "isa/definitions.hpp"

namespace simulator {

struct CpuState;
struct Memory;

class HostBasicBlock {
private:
    Addr startPc_;

public:
    HostBasicBlock(Addr startPc) : startPc_(startPc) {}

    HostBasicBlock(const HostBasicBlock&) = delete;
    HostBasicBlock& operator=(const HostBasicBlock&) = delete;

    Addr getStartPc() const { return startPc_; }

    void execute(CpuState& state, Memory& memory) const {}
};

}
