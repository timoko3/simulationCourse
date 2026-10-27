#pragma once

#include <memory>

#include "../backend.hpp"
#include "../instructionEmitters.hpp"

namespace llvm::orc {
class LLJIT;
}

namespace simulator {

namespace llvmEmitters {
struct Context;
}

class LlvmBackend : public TranslationBackend {
    using EmitContext = llvmEmitters::Context;

    std::shared_ptr<llvm::orc::LLJIT> jit_;
    EmitterTable<EmitContext> emitters_;


public:
    LlvmBackend();
    ~LlvmBackend() override;

    std::unique_ptr<HostBasicBlock> translate(Addr, const BasicBlock&) override;
};

}
