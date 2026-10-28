#pragma once

#include "emitterHelpers.hpp"
#include "runtimeEmitters.hpp"
#include "isa/isa.hpp"

namespace simulator::llvmEmitters {

template<class Operation>
void emitRegRegReg(Context& ctx, const Instruction& instruction) {
    auto* lhs = loadReg(ctx, instruction.src1);
    auto* rhs = loadReg(ctx, instruction.src2);

    storeReg(ctx, instruction.dst, Operation{}(ctx.builder, lhs, rhs));
}

template<class Operation>
void emitRegRegImm(Context& ctx, const Instruction& instruction) {
    auto* lhs = loadReg(ctx, instruction.src1);
    auto* rhs = immediate(ctx, instruction.src2);

    storeReg(ctx, instruction.dst, Operation{}(ctx.builder, lhs, rhs));
}

template<class Operation, Word Instruction::* ImmediateField = &Instruction::src1>
void emitRegImm(Context& ctx, const Instruction& instruction) {
    auto* value = immediate(ctx, instruction.*ImmediateField);
    
    storeReg(ctx, instruction.dst, Operation{}(ctx.builder, value));
}

template<class Operation>
void emitRegReg(Context& ctx, const Instruction& instruction) {
    auto* value = loadReg(ctx, instruction.src1);
    
    storeReg(ctx, instruction.dst, Operation{}(ctx.builder, value));
}

template<llvm::Value* (*Address)(Context&, const Instruction&)>
void emitLoad(Context& ctx, const Instruction& instruction) {
    auto* address = Address(ctx, instruction);
    auto* value = loadGuestWord(ctx, address);

    storeReg(ctx, instruction.dst, value);
}

template<llvm::Value* (*Address)(Context&, const Instruction&)>
void emitStore(Context& ctx, const Instruction& instruction) {
    auto* address = Address(ctx, instruction);
    auto* value = loadReg(ctx, instruction.src2);

    storeGuestWord(ctx, address, value);
}

template<llvm::Value* (*Address)(Context&, const Instruction&)>
void emitLoadPair(Context& ctx, const Instruction& instruction) {
    auto* address = Address(ctx, instruction);
    auto* nextAddress = ctx.builder.CreateAdd(address, ctx.builder.getInt32(sizeof(Word)));

    auto* first = loadGuestWord(ctx, address);
    auto* second = loadGuestWord(ctx, nextAddress);

    storeReg(ctx, instruction.dst, first);
    storeReg(ctx, instruction.dst2, second);
}

}
