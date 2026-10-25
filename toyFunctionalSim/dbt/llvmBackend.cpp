#include "llvmBackend.hpp"

#include <stdexcept>
#include <utility>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>

namespace simulator{

LlvmBackend::LlvmBackend()
{
    if (llvm::InitializeNativeTarget() || llvm::InitializeNativeTargetAsmPrinter()) {
        throw std::runtime_error("Failed to initialize the native LLVM target");
    }

    auto jit = llvm::orc::LLJITBuilder().create();
    if (!jit) {
        throw std::runtime_error("Failed to create LLVM JIT: " +
                                 llvm::toString(jit.takeError()));
    }
    jit_ = std::move(*jit);
}

LlvmBackend::~LlvmBackend() = default;

std::unique_ptr<HostBasicBlock> 
LlvmBackend::translate(Addr, const BasicBlock&) {
    throw std::runtime_error("LLVM translation is not implemented yet");
}

}
