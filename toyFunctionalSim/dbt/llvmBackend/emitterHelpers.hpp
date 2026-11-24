#pragma once

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include "emitContext.hpp"
#include "../blockDescriptor.hpp"
#include "isa/isa.hpp"
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

template<Word Instruction::* ImmediateField = &Instruction::src2>
llvm::Value* emitRegImmAddress(Context& ctx, const Instruction& instruction) {
    auto* base = loadReg(ctx, instruction.src1);
    auto* offset = immediate(ctx, instruction.*ImmediateField);

    return ctx.builder.CreateAdd(base, offset);
}

template<Word Instruction::* RegisterField = &Instruction::src2>
llvm::Value* emitRegRegAddress(Context& ctx, const Instruction& instruction) {
    auto* base = loadReg(ctx, instruction.src1);
    auto* offset = loadReg(ctx, instruction.*RegisterField);

    return ctx.builder.CreateAdd(base, offset);
}

inline void emitBlockExit(Context& ctx) {
    if (!ctx.exitIndex) {
        ctx.builder.CreateRet(llvm::ConstantPointerNull::get(ctx.builder.getPtrTy()));
        return;
    }

    auto* function = ctx.builder.GetInsertBlock()->getParent();
    auto* returnBlock = llvm::BasicBlock::Create(ctx.builder.getContext(), "return", function);
    auto* chainBlock = llvm::BasicBlock::Create(ctx.builder.getContext(), "chain", function);

    auto* pointerType = ctx.builder.getPtrTy();
    auto* exitSlotType = llvm::StructType::get(ctx.builder.getContext(), {pointerType}, false);
    auto* exitsType = llvm::ArrayType::get(exitSlotType, 2);
    auto* descriptorType = llvm::StructType::get(ctx.builder.getContext(), {pointerType, exitsType}, false);

    auto* exits = ctx.builder.CreateStructGEP(descriptorType, ctx.descriptor, 1);
    auto* slot = ctx.builder.CreateInBoundsGEP(
        exitsType, exits, {ctx.builder.getInt32(0), ctx.exitIndex});
    auto* targetAddress = ctx.builder.CreateStructGEP(exitSlotType, slot, 0);
    auto* target = ctx.builder.CreateLoad(ctx.builder.getPtrTy(), targetAddress, "linked_block");
    auto* linked = ctx.builder.CreateIsNotNull(target);

    ctx.builder.CreateCondBr(linked, chainBlock, returnBlock);

    ctx.builder.SetInsertPoint(returnBlock);
    ctx.builder.CreateRet(slot);

    ctx.builder.SetInsertPoint(chainBlock);
    auto* entryAddress = ctx.builder.CreateStructGEP(descriptorType, target, 0);
    auto* entry = ctx.builder.CreateLoad(ctx.builder.getPtrTy(), entryAddress, "linked_entry");
    auto* call = ctx.builder.CreateCall(
        function->getFunctionType(), entry, {ctx.state, ctx.memory, target});
    call->setTailCallKind(llvm::CallInst::TCK_MustTail);
    ctx.builder.CreateRet(call);
}

inline void requireImmediateBitIndex(llvm::Value* value) {
    const auto* index = llvm::dyn_cast<llvm::ConstantInt>(value);

    if (!index || index->getZExtValue() >= 32) {
        throw std::invalid_argument("Bit index or saturation width must be below 32");
    }
}

}
