#pragma once

#include <cstddef>
#include <type_traits>

#include "emitContext.hpp"
#include "toyFunctionalSim/cpuState.hpp"

namespace simulator::llvmEmitters {

static_assert(std::is_standard_layout_v<CpuState>);

inline llvm::Value* regAddress(Context& ctx, Word reg) {
    const auto offset = offsetof(CpuState, gpr_regs) + reg * sizeof(Register);

    return ctx.builder.CreateConstGEP1_64(ctx.builder.getInt8Ty(), ctx.state, offset);
}

inline llvm::Value* loadReg(Context& ctx, Word reg) {
    return ctx.builder.CreateLoad(ctx.builder.getInt32Ty(), regAddress(ctx, reg));
}

inline void storeReg(Context& ctx, Word reg, llvm::Value* value) {
    ctx.builder.CreateStore(value, regAddress(ctx, reg));
}

inline llvm::Value* immediate(Context& ctx, Word value) {
    return ctx.builder.getInt32(value);
}

inline llvm::Value* pcAddress(Context& ctx) {
    return ctx.builder.CreateConstGEP1_64(ctx.builder.getInt8Ty(), ctx.state, offsetof(CpuState, pc));
}

inline llvm::Value* loadPc(Context& ctx) {
    return ctx.builder.CreateLoad(ctx.builder.getInt32Ty(), pcAddress(ctx), "pc");
}

inline void storePc(Context& ctx, llvm::Value* value) {
    ctx.builder.CreateStore(value, pcAddress(ctx));
}

}
