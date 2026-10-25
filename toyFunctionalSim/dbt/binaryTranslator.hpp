#pragma once

#include <memory>
#include <stdexcept>

#include "hostBasicBlock.hpp"
#include "backend.hpp"
#include "llvmBackend.hpp"
#include "asmJitBackend.hpp"
#include "isa/isa.hpp"
#include "toyFunctionalSim/simulatorSettings.hpp"

namespace simulator {

class BinaryTranslator{
    std::unique_ptr<TranslationBackend> backend_;
public:
    explicit BinaryTranslator(DbtLib backendType) {
        switch (backendType) {
            case DbtLib::LLVM:
                backend_ = std::make_unique<LlvmBackend>();
                break;
            case DbtLib::ASM_JIT:
                backend_ = std::make_unique<AsmJitBackend>();
                break;
            default:
                throw std::invalid_argument("Unknown DBT backend");
            }
    }

    std::unique_ptr<HostBasicBlock> translate(Addr pc, const BasicBlock& block);
};

}
