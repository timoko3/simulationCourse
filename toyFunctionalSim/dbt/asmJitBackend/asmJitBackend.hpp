#pragma once

#include <stdexcept>

#include "../backend.hpp"

namespace simulator {

class AsmJitBackend : public TranslationBackend {
public:
    std::unique_ptr<HostBasicBlock> translate(Addr, const BasicBlock&) override {
        throw std::runtime_error("AsmJit translation is not implemented");
    }
};

}
