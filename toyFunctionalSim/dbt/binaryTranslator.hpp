#pragma once

#include <memory>
#include <stdexcept>

#include "hostBasicBlock.hpp"
#include "backend.hpp"
#include "llvmBackend/llvmBackend.hpp"
#include "asmJitBackend/asmJitBackend.hpp"
#include "isa/isa.hpp"
#include "toyFunctionalSim/simulatorSettings.hpp"

namespace simulator {

class BinaryTranslator{
    std::unique_ptr<TranslationBackend> backend_;
    
public:
    explicit BinaryTranslator(DbtLib backendType) {
        switch (backendType) {
            case DbtLib::LLVM:
#if SIMCOURSE_HAS_LLVM
                backend_ = std::make_unique<LlvmBackend>();
                break;
#else
                throw std::runtime_error("LLVM backend is disabled in this build");
#endif
            case DbtLib::ASM_JIT:
                backend_ = std::make_unique<AsmJitBackend>();
                break;
            default:
                throw std::invalid_argument("Unknown DBT backend");
            }
    }

    std::unique_ptr<HostBasicBlock> translate(Addr pc, const BasicBlock& block) {
        return backend_->translate(pc, block);
    }
};

}
