#pragma once

#include <llvm/IR/Module.h>

#include "emitContext.hpp"

namespace simulator::llvmEmitters {

inline constexpr char READ_WORD_SYMBOL[] = "sim_read_word";
inline constexpr char WRITE_WORD_SYMBOL[] = "sim_write_word";
inline constexpr char SYSCALL_SYMBOL[] = "sim_syscall";

inline llvm::Value* loadGuestWord(Context& ctx, llvm::Value* address) {
    auto* module = ctx.builder.GetInsertBlock()->getModule();
    auto* signature = llvm::FunctionType::get(
        ctx.builder.getInt32Ty(),
        {ctx.builder.getPtrTy(), ctx.builder.getInt32Ty()}, false);
    auto function = module->getOrInsertFunction(READ_WORD_SYMBOL, signature);

    return ctx.builder.CreateCall(function, {ctx.memory, address});
}

inline void storeGuestWord(Context& ctx, llvm::Value* address, llvm::Value* value) {
    auto* module = ctx.builder.GetInsertBlock()->getModule();
    auto* signature = llvm::FunctionType::get(
        ctx.builder.getVoidTy(),
        {ctx.builder.getPtrTy(), ctx.builder.getInt32Ty(), ctx.builder.getInt32Ty()}, false);
    auto function = module->getOrInsertFunction(WRITE_WORD_SYMBOL, signature);

    ctx.builder.CreateCall(function, {ctx.memory, address, value});
}

inline void emitGuestSyscall(Context& ctx, llvm::Value* code) {
    auto* module = ctx.builder.GetInsertBlock()->getModule();
    auto* signature = llvm::FunctionType::get(
        ctx.builder.getVoidTy(),
        {ctx.builder.getPtrTy(), ctx.builder.getInt32Ty()}, false);
    auto function = module->getOrInsertFunction(SYSCALL_SYMBOL, signature);

    ctx.builder.CreateCall(function, {ctx.state, code});
}

}
