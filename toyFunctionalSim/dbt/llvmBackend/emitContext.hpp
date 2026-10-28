#pragma once

#include <llvm/IR/IRBuilder.h>

#include "isa/definitions.hpp"

namespace simulator::llvmEmitters {

struct Context {
    llvm::IRBuilder<>& builder;
    llvm::Value* state;
    llvm::Value* memory;
    Addr guestPc;
    llvm::Value* nextPc = nullptr;
};

}
