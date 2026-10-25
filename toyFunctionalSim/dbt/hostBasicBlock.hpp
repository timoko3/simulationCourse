#pragma once

#include <utility>
#include <vector>

#include "isa/definitions.hpp"

namespace simulator {

class HostBasicBlock {
private:
    Addr startPc_;
    std::vector<Byte> code_;

public:
    explicit HostBasicBlock(Addr startPc, std::vector<Byte> code = {})
        : startPc_(startPc), code_(std::move(code)) {}

    HostBasicBlock(const HostBasicBlock&) = delete;
    HostBasicBlock& operator=(const HostBasicBlock&) = delete;

    HostBasicBlock(HostBasicBlock&&) noexcept = default;
    HostBasicBlock& operator=(HostBasicBlock&&) noexcept = default;

    Addr getStartPc() const { return startPc_; }
    const std::vector<Byte>& getCode() const { return code_; }
};

}
