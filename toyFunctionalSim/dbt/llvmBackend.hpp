#pragma once

#include <memory>

#include "backend.hpp"

namespace llvm::orc {
class LLJIT;
}

namespace simulator {

class LlvmBackend : public TranslationBackend {
    std::shared_ptr<llvm::orc::LLJIT> jit_;

public:
    LlvmBackend();
    ~LlvmBackend() override;

    std::unique_ptr<HostBasicBlock> translate(Addr, const BasicBlock&) override;
};

}
