#pragma once

#include <memory>

#include <llvm/Passes/PassBuilder.h>

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

    llvm::FunctionAnalysisManager functionAnalyses_;
    llvm::ModuleAnalysisManager moduleAnalyses_;
    llvm::PassBuilder passBuilder_;
    llvm::FunctionPassManager passes_;


public:
    LlvmBackend();
    ~LlvmBackend() override;

    std::unique_ptr<HostBasicBlock> translate(Addr, const BasicBlock&) override;
};

}
