#pragma once

#include "stateEmitters.hpp"
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

}
