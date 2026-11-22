#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include <llvm-c/Disassembler.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/TargetParser/Triple.h>

#include "isa/isa.hpp"

namespace simulator {

class LlvmJitDump {
    std::filesystem::path directory_;
    std::filesystem::path blockDirectory_;
    std::string symbolName_;
    std::string triple_;
    std::string branchTarget_;
    std::uint64_t translation_ = 0;
    std::uint64_t codeSize_ = 0;
    Addr guestPc_ = 0;
    std::unique_ptr<void, decltype(&LLVMDisasmDispose)> disassembler_{nullptr, &LLVMDisasmDispose};

public:
    LlvmJitDump(const std::filesystem::path& directory, const llvm::Triple& triple);

    LlvmJitDump(const LlvmJitDump&) = delete;
    LlvmJitDump& operator=(const LlvmJitDump&) = delete;
    LlvmJitDump(LlvmJitDump&&) = delete;
    LlvmJitDump& operator=(LlvmJitDump&&) = delete;

    void begin(Addr pc, llvm::StringRef symbolName);
    void saveGuest(const BasicBlock& block) const;
    void saveIr(const llvm::Module& module, llvm::StringRef stage) const;
    llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>> saveObject(std::unique_ptr<llvm::MemoryBuffer> object);
    void saveNative(const std::uint8_t* code);
};

}
